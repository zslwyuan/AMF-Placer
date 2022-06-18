// See LICENSE for license details.
package sifive.fpgashells.devices.xilinx.xilinxvcu440mig

import Chisel._
import freechips.rocketchip.config._
import freechips.rocketchip.subsystem.BaseSubsystem
import freechips.rocketchip.diplomacy.{LazyModule, LazyModuleImp, AddressRange}

case object MemoryXilinxDDRKey extends Field[XilinxVCU440MIGParams]

trait HasMemoryXilinxVCU440MIG { this: BaseSubsystem =>
  val module: HasMemoryXilinxVCU440MIGModuleImp

  val xilinxvcu440mig = LazyModule(new XilinxVCU440MIG(p(MemoryXilinxDDRKey)))

  xilinxvcu440mig.node := mbus.toDRAMController(Some("xilinxvcu440mig"))()
}

trait HasMemoryXilinxVCU440MIGBundle {
  val xilinxvcu440mig: XilinxVCU440MIGIO
  def connectXilinxVCU440MIGToPads(pads: XilinxVCU440MIGPads) {
    pads <> xilinxvcu440mig
  }
}

trait HasMemoryXilinxVCU440MIGModuleImp extends LazyModuleImp
    with HasMemoryXilinxVCU440MIGBundle {
  val outer: HasMemoryXilinxVCU440MIG
  val ranges = AddressRange.fromSets(p(MemoryXilinxDDRKey).address)
  require (ranges.size == 1, "DDR range must be contiguous")
  val depth = ranges.head.size
  val xilinxvcu440mig = IO(new XilinxVCU440MIGIO(depth))

  xilinxvcu440mig <> outer.xilinxvcu440mig.module.io.port
}
