set fo [open "${pahtPrefix}allCellPinNet" "w"]
set allCells [xilinx::designutils::get_leaf_cells *]
foreach curCell $allCells {
    set curCelltype [get_property REF_NAME $curCell]
    puts $fo "curCell=> $curCell type=> $curCelltype"
    set cellPins [get_pins  -leaf -of_objects [get_cells $curCell]]
    foreach curPin $cellPins {
        set pinDir [get_property DIRECTION $curPin]
        set pinNameAtCell [get_property REF_PIN_NAME $curPin]
        set pinNet [get_nets -of [get_pins $curPin]]
        set tmp_net $pinNet
        set tmp_net_driver_pin [get_pins  -leaf -of_objects  $tmp_net -filter {DIRECTION == OUT}]
        puts $fo "   pin=> $curPin refpin=> $pinNameAtCell dir=> $pinDir net=> $pinNet drivepin=> $tmp_net_driver_pin"
    }
}
close $fo

set fo [open "/home/tingyuan/Dropbox/AMF-Placer/benchmarks/VCU108/design/faceDetect/faceDetect_clocks" "w"]
set clocks [get_nets -hierarchical -top_net_of_hierarchical_group -filter { TYPE == "GLOBAL_CLOCK" } ]
foreach curClock $clocks {
    set tmp_net_driver_pin [get_pins  -leaf -of_objects  $curClock -filter {DIRECTION == OUT}]
    puts $fo  "$tmp_net_driver_pin"
}
close $fo