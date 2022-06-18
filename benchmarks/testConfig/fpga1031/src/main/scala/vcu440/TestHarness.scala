package chipyard.fpga.vcu440
import chipyard.harness.ApplyHarnessBinders
import chipyard.iobinders.HasIOBinders
import chipyard._
import chisel3._
import freechips.rocketchip.config._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.tilelink._
import sifive.blocks.devices.spi._
import sifive.blocks.devices.uart._
import sifive.fpgashells.clocks._
import sifive.fpgashells.ip.xilinx._
import sifive.fpgashells.shell._
import sifive.fpgashells.shell.xilinx._

class VCU440FPGATestHarness(override implicit val p: Parameters) extends VCU440ShellBasicOverlays {

  def dp = designParameters

  val pmod_is_sdio  = p(VCU440ShellPMOD) == "SDIO"
  val jtag_location = Some(if (pmod_is_sdio) "FMC_J2" else "PMOD_J52")

  // Order matters; ddr depends on sys_clock
  val uart      = Overlay(UARTOverlayKey, new UARTVCU440ShellPlacer(this, UARTShellInput()))
  val sdio      = if (pmod_is_sdio) Some(Overlay(SPIOverlayKey, new SDIOVCU440ShellPlacer(this, SPIShellInput()))) else None
  //TODO:sys_clock 有没有用到
  //val jtag      = Overlay(JTAGDebugOverlayKey, new JTAGDebugVCU440ShellPlacer(this, JTAGDebugShellInput(location = jtag_location)))
  //val cjtag     = Overlay(cJTAGDebugOverlayKey, new cJTAGDebugVCU440ShellPlacer(this, cJTAGDebugShellInput()))
  //val jtagBScan = Overlay(JTAGDebugBScanOverlayKey, new JTAGDebugBScanVCU440ShellPlacer(this, JTAGDebugBScanShellInput()))
  //val fmc       = Overlay(PCIeOverlayKey, new PCIeVCU440FMCShellPlacer(this, PCIeShellInput()))
  //val edge      = Overlay(PCIeOverlayKey, new PCIeVCU440EdgeShellPlacer(this, PCIeShellInput()))
  //val sys_clock2 = Overlay(ClockInputOverlayKey, new SysClock2VCU440ShellPlacer(this, ClockInputShellInput()))
  //val ddr2       = Overlay(DDROverlayKey, new DDR2VCU440ShellPlacer(this, DDRShellInput()))

  val topDesign = LazyModule(p(BuildTop)(dp)).suggestName("chiptop")

// DOC include start: ClockOverlay
  // place all clocks in the shell
  require(dp(ClockInputOverlayKey).size >= 1)
  val sysClkNode = dp(ClockInputOverlayKey)(0).place(ClockInputDesignInput()).overlayOutput.node

  /*** Connect/Generate clocks ***/

  // connect to the PLL that will generate multiple clocks
  val harnessSysPLL = dp(PLLFactoryKey)()
  harnessSysPLL := sysClkNode

  // create and connect to the dutClock
  println(s"VCU440 FPGA Base Clock Freq: ${dp(DefaultClockFrequencyKey)} MHz")
  val dutClock = ClockSinkNode(freqMHz = dp(DefaultClockFrequencyKey))
  val dutWrangler = LazyModule(new ResetWrangler)
  val dutGroup = ClockGroup()
  dutClock := dutWrangler.node := dutGroup := harnessSysPLL
// DOC include end: ClockOverlay

  /*** UART ***/

// DOC include start: UartOverlay
  // 1st UART goes to the VCU440 dedicated UART

  val io_uart_bb = BundleBridgeSource(() => (new UARTPortIO(dp(PeripheryUARTKey).head)))
  dp(UARTOverlayKey).head.place(UARTDesignInput(io_uart_bb))
// DOC include end: UartOverlay

  /*** SPI ***/

  // 1st SPI goes to the VCU440 SDIO port

  val io_spi_bb = BundleBridgeSource(() => (new SPIPortIO(dp(PeripherySPIKey).head)))
  dp(SPIOverlayKey).head.place(SPIDesignInput(dp(PeripherySPIKey).head, io_spi_bb))

  /*** DDR ***/

  val ddrNode = dp(DDROverlayKey).head.place(DDRDesignInput(dp(ExtTLMem).get.master.base, dutWrangler.node, harnessSysPLL)).overlayOutput.ddr

  // connect 1 mem. channel to the FPGA DDR
  val inParams = topDesign match { case td: ChipTop =>
    td.lazySystem match { case lsys: CanHaveMasterTLMemPort =>
      lsys.memTLNode.edges.in(0)
    }
  }
  val ddrClient = TLClientNode(Seq(inParams.master))
  ddrNode := ddrClient

  // module implementation
  override lazy val module = new VCU440FPGATestHarnessImp(this)
}

class VCU440FPGATestHarnessImp(_outer: VCU440FPGATestHarness) extends LazyRawModuleImp(_outer) with HasHarnessSignalReferences {

  val vcu440Outer = _outer

  val reset = IO(Input(Bool()))
  _outer.xdc.addPackagePin(reset, "BC27")
  _outer.xdc.addIOStandard(reset, "LVCMOS18")

  val resetIBUF = Module(new IBUF)
  resetIBUF.io.I := ~reset

  val sysclk: Clock = _outer.sysClkNode.out.head._1.clock

  val powerOnReset: Bool = PowerOnResetFPGAOnly(sysclk)
  _outer.sdc.addAsyncPath(Seq(powerOnReset))

  // val ereset: Bool = _outer.chiplink.get() match {
  //   case Some(x: ChipLinkVCU440PlacedOverlay) => !x.ereset_n
  //   case _ => false.B
  // }

  _outer.pllReset := (resetIBUF.io.O || powerOnReset)

  // reset setup
  val hReset = Wire(Reset())
  hReset := _outer.dutClock.in.head._1.reset

  val buildtopClock = _outer.dutClock.in.head._1.clock
  val buildtopReset = WireInit(hReset)
  val dutReset = hReset.asAsyncReset
  val success = false.B

  childClock := buildtopClock
  childReset := buildtopReset

  // harness binders are non-lazy
  _outer.topDesign match { case d: HasTestHarnessFunctions =>
    d.harnessFunctions.foreach(_(this))
  }
  _outer.topDesign match { case d: HasIOBinders =>
    ApplyHarnessBinders(this, d.lazySystem, d.portMap)
  }

  // check the top-level reference clock is equal to the default
  // non-exhaustive since you need all ChipTop clocks to equal the default
  _outer.topDesign match {
    case d: HasReferenceClockFreq => require(d.refClockFreqMHz == p(DefaultClockFrequencyKey))
    case _ =>
  }
}
