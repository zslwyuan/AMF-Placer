// See LICENSE for license details.
package sifive.fpgashells.shell.xilinx

import chisel3._
import chisel3.experimental.{attach, Analog, IO}
import freechips.rocketchip.config._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util.SyncResetSynchronizerShiftReg
import sifive.fpgashells.clocks._
import sifive.fpgashells.shell._
import sifive.fpgashells.ip.xilinx._
import sifive.blocks.devices.chiplink._
import sifive.fpgashells.devices.xilinx.xilinxvcu440mig._
import sifive.fpgashells.devices.xilinx.xdma._
import sifive.fpgashells.ip.xilinx.xxv_ethernet._

class SysClockVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: ClockInputDesignInput, val shellInput: ClockInputShellInput)
  extends LVDSClockInputXilinxPlacedOverlay(name, designInput, shellInput)
{
  val node = shell { ClockSourceNode(freqMHz = 250, jitterPS = 50)(ValName(name)) }

  shell { InModuleBody {
    shell.xdc.addPackagePin(io.p, "Y47")
    shell.xdc.addPackagePin(io.n, "Y48")
    shell.xdc.addIOStandard(io.p, "DIFF_SSTL12")
    shell.xdc.addIOStandard(io.n, "DIFF_SSTL12")
  } }
}
class SysClockVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: ClockInputShellInput)(implicit val valName: ValName)
  extends ClockInputShellPlacer[VCU440ShellBasicOverlays]
{
    def place(designInput: ClockInputDesignInput) = new SysClockVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}

class RefClockVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: ClockInputDesignInput, val shellInput: ClockInputShellInput)
  extends LVDSClockInputXilinxPlacedOverlay(name, designInput, shellInput) {
  val node = shell { ClockSourceNode(freqMHz = 125, jitterPS = 50)(ValName(name)) }

  shell { InModuleBody {
    shell.xdc.addPackagePin(io.p, "AK47")
    shell.xdc.addPackagePin(io.n, "AK48")
    shell.xdc.addIOStandard(io.p, "DIFF_SSTL12")
    shell.xdc.addIOStandard(io.n, "DIFF_SSTL12")
  } }
}
class RefClockVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: ClockInputShellInput)(implicit val valName: ValName)
  extends ClockInputShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: ClockInputDesignInput) = new RefClockVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}
//TODO：not match
class SDIOVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: SPIDesignInput, val shellInput: SPIShellInput)
  extends SDIOXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {//micro SD card
    val packagePinsWithPackageIOs = Seq(("BN43", IOPin(io.spi_clk)),
                                        ("BN46", IOPin(io.spi_cs)),//cmd
                                        ("BL40", IOPin(io.spi_dat(0))),
                                        ("BL39", IOPin(io.spi_dat(1))),
                                        ("BM41", IOPin(io.spi_dat(2))),
                                        ("BM42", IOPin(io.spi_dat(3))))

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS18")//VCC_J8
    } }
    packagePinsWithPackageIOs drop 1 foreach { case (pin, io) => {
      shell.xdc.addPullup(io)
      shell.xdc.addIOB(io)
    } }
  } }
}
class SDIOVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: SPIShellInput)(implicit val valName: ValName)
  extends SPIShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: SPIDesignInput) = new SDIOVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}

class UARTVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: UARTDesignInput, val shellInput: UARTShellInput)
  extends UARTXilinxPlacedOverlay(name, designInput, shellInput, true)
{
  shell { InModuleBody {
    val packagePinsWithPackageIOs = Seq(("BK20", IOPin(io.ctsn.get)),//RTS
                                        ("BL20", IOPin(io.rtsn.get)),//CTS
                                        ("BN22", IOPin(io.rxd)),//RXD
                                        ("BN21", IOPin(io.txd)))//TXD

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS18")
      shell.xdc.addIOB(io)
    } }
  } }
}
class UARTVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: UARTShellInput)(implicit val valName: ValName)
  extends UARTShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: UARTDesignInput) = new UARTVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}


object LEDVCU440PinConstraints {
  val pins = Seq("AW18", "AV18", "AU12", "AT12", "BF32", "AU15", "AT15", "AW13","AV13")
}
class LEDVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: LEDDesignInput, val shellInput: LEDShellInput)
  extends LEDXilinxPlacedOverlay(name, designInput, shellInput, packagePin = Some(LEDVCU440PinConstraints.pins(shellInput.number)), ioStandard = "LVCMOS12")
class LEDVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: LEDShellInput)(implicit val valName: ValName)
  extends LEDShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: LEDDesignInput) = new LEDVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}

object ButtonVCU440PinConstraints {
  val pins = Seq("AV24", "AU24", "AT22", "AT23", "BD23")
}
class ButtonVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: ButtonDesignInput, val shellInput: ButtonShellInput)
  extends ButtonXilinxPlacedOverlay(name, designInput, shellInput, packagePin = Some(ButtonVCU440PinConstraints.pins(shellInput.number)), ioStandard = "LVCMOS18")
class ButtonVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: ButtonShellInput)(implicit val valName: ValName)
  extends ButtonShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: ButtonDesignInput) = new ButtonVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}

object SwitchVCU440PinConstraints {
  val pins = Seq("AT28", "AV26", "J16", "D21")
}
class SwitchVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: SwitchDesignInput, val shellInput: SwitchShellInput)
  extends SwitchXilinxPlacedOverlay(name, designInput, shellInput, packagePin = Some(SwitchVCU440PinConstraints.pins(shellInput.number)), ioStandard = "LVCMOS12")
class SwitchVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: SwitchShellInput)(implicit val valName: ValName)
  extends SwitchShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: SwitchDesignInput) = new SwitchVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}

case object VCU440DDRSize extends Field[BigInt](0x40000000L * 4) // 4GB
class DDRVCU440PlacedOverlay(val shell: VCU440ShellBasicOverlays, name: String, val designInput: DDRDesignInput, val shellInput: DDRShellInput)
  extends DDRPlacedOverlay[XilinxVCU440MIGPads](name, designInput, shellInput)
{
  val size = p(VCU440DDRSize)

  val migParams = XilinxVCU440MIGParams(address = AddressSet.misaligned(di.baseAddress, size))
  val mig = LazyModule(new XilinxVCU440MIG(migParams))
  val ioNode = BundleBridgeSource(() => mig.module.io.cloneType)
  val topIONode = shell { ioNode.makeSink() }
  val ddrUI     = shell { ClockSourceNode(freqMHz = 200) }
  val areset    = shell { ClockSinkNode(Seq(ClockSinkParameters())) }
  areset := designInput.wrangler := ddrUI

  def overlayOutput = DDROverlayOutput(ddr = mig.node)
  def ioFactory = new XilinxVCU440MIGPads(size)

  InModuleBody { ioNode.bundle <> mig.module.io }

  shell { InModuleBody {
    require (shell.sys_clock.get.isDefined, "Use of DDRVCU440Overlay depends on SysClockVCU440Overlay")
    val (sys, _) = shell.sys_clock.get.get.overlayOutput.node.out(0)
    val (ui, _) = ddrUI.out(0)
    val (ar, _) = areset.in(0)
    val port = topIONode.bundle.port
    io <> port
    ui.clock := port.c0_ddr4_ui_clk
    ui.reset := /*!port.mmcm_locked ||*/ port.c0_ddr4_ui_clk_sync_rst
    port.c0_sys_clk_i := sys.clock.asUInt
    port.sys_rst := sys.reset // pllReset
    port.c0_ddr4_aresetn := !ar.reset

    val allddrpins = Seq("AC44","Y51","AB45","AA50","W45","Y45","Y50","W44","AC51","AB51","AC43","W43","AA44","AB41",
    "AC47","AA42","AC42","AB49","AB44","AB42","AC49","Y41","AA43","AA47","AB47","Y43","AC48","AC46","V42","U40","T42",
    "T43","T40","R40","R41","U42","R46","R48","V46","U44","R45","R47","U45","U46","T48","T50","W49","V49","T49","U49",
    "W50","U50","R52","T53","V54","V52","R51","T52","U52","V53","AD41","AG42","AH42","AE43","AE41","AG41","AH43","AE42",
    "AG47","AF45","AH46","AG45","AG46","AE45","AH47","AF47","AF50","AE47","AE50","AE48","AF48","AG50","AF49","AD50","AG52",
    "AE52","AE51","AF54","AD51","AG54","AG51","AE53","R43","T45","V48","T54","AF43","AD45","AD49","AF53","R42","T44",
    "W48","U54","AF42","AD44","AD48","AF52","V43","U47","W51","W53","AH44","AE46","AH49","AH51")

    (IOPin.of(io) zip allddrpins) foreach { case (io, pin) => shell.xdc.addPackagePin(io, pin) }
  } }

  shell.sdc.addGroup(pins = Seq(mig.island.module.blackbox.io.c0_ddr4_ui_clk))
}
class DDRVCU440ShellPlacer(shell: VCU440ShellBasicOverlays, val shellInput: DDRShellInput)(implicit val valName: ValName)
  extends DDRShellPlacer[VCU440ShellBasicOverlays] {
  def place(designInput: DDRDesignInput) = new DDRVCU440PlacedOverlay(shell, valName.name, designInput, shellInput)
}


abstract class VCU440ShellBasicOverlays()(implicit p: Parameters) extends UltraScaleShell{
  // PLL reset causes
  val pllReset = InModuleBody { Wire(Bool()) }

  val sys_clock = Overlay(ClockInputOverlayKey, new SysClockVCU440ShellPlacer(this, ClockInputShellInput()))
  val ref_clock = Overlay(ClockInputOverlayKey, new RefClockVCU440ShellPlacer(this, ClockInputShellInput()))
  val led       = Seq.tabulate(8)(i => Overlay(LEDOverlayKey, new LEDVCU440ShellPlacer(this, LEDShellInput(color = "red", number = i))(valName = ValName(s"led_$i"))))
  val switch    = Seq.tabulate(4)(i => Overlay(SwitchOverlayKey, new SwitchVCU440ShellPlacer(this, SwitchShellInput(number = i))(valName = ValName(s"switch_$i"))))
  val button    = Seq.tabulate(5)(i => Overlay(ButtonOverlayKey, new ButtonVCU440ShellPlacer(this, ButtonShellInput(number = i))(valName = ValName(s"button_$i"))))
  val ddr       = Overlay(DDROverlayKey, new DDRVCU440ShellPlacer(this, DDRShellInput()))
}

case object VCU440ShellPMOD extends Field[String]("JTAG")
case object VCU440ShellPMOD2 extends Field[String]("JTAG")

class WithVCU440ShellPMOD(device: String) extends Config((site, here, up) => {
  case VCU440ShellPMOD => device
})

// Change JTAG pinouts to VCU440 J53
// Due to the level shifter is from 1.2V to 3.3V, the frequency of JTAG should be slow down to 1Mhz
class WithVCU440ShellPMOD2(device: String) extends Config((site, here, up) => {
  case VCU440ShellPMOD2 => device
})

class WithVCU440ShellPMODJTAG extends WithVCU440ShellPMOD("JTAG")
class WithVCU440ShellPMODSDIO extends WithVCU440ShellPMOD("SDIO")

// Reassign JTAG pinouts location to PMOD J53
class WithVCU440ShellPMOD2JTAG extends WithVCU440ShellPMOD2("PMODJ53_JTAG")

class VCU440Shell()(implicit p: Parameters) extends VCU440ShellBasicOverlays
{
  val pmod_is_sdio  = p(VCU440ShellPMOD) == "SDIO"
  val pmod_j53_is_jtag = p(VCU440ShellPMOD2) == "PMODJ53_JTAG"
  val jtag_location = Some(if (pmod_is_sdio) (if (pmod_j53_is_jtag) "PMOD_J53" else "FMC_J2") else "PMOD_J52")

  // Order matters; ddr depends on sys_clock
  val uart      = Overlay(UARTOverlayKey, new UARTVCU440ShellPlacer(this, UARTShellInput()))
  val sdio      = if (pmod_is_sdio) Some(Overlay(SPIOverlayKey, new SDIOVCU440ShellPlacer(this, SPIShellInput()))) else None
  val topDesign = LazyModule(p(DesignKey)(designParameters))

  // Place the sys_clock at the Shell if the user didn't ask for it
  designParameters(ClockInputOverlayKey).foreach { unused =>
    val source = unused.place(ClockInputDesignInput()).overlayOutput.node
    val sink = ClockSinkNode(Seq(ClockSinkParameters()))
    sink := source
  }

  override lazy val module = new LazyRawModuleImp(this) {
    val reset = IO(Input(Bool()))
    xdc.addPackagePin(reset, "BC27")
    xdc.addIOStandard(reset, "LVCMOS18")

    val reset_ibuf = Module(new IBUF)
    reset_ibuf.io.I := reset

    val sysclk: Clock = sys_clock.get() match {
      case Some(x: SysClockVCU440PlacedOverlay) => x.clock
    }

    val powerOnReset: Bool = PowerOnResetFPGAOnly(sysclk)
    sdc.addAsyncPath(Seq(powerOnReset))

    // val ereset: Bool = chiplink.get() match {
    //   case Some(x: ChipLinkVCU440PlacedOverlay) => !x.ereset_n
    //   case _ => false.B
    // }

    pllReset := (reset_ibuf.io.O || powerOnReset)
  }
}
