set targetFolderPath "/home/tingyuan/tmpFiles/" 

set fo [open "${targetFolderPath}/NetDelay" "w"]
set allCells [xilinx::designutils::get_leaf_cells *]
set logicCells [get_cells $allCells -filter {LOC =~ "SLICE*" && (REF_NAME =~ "LUT*" || REF_NAME =~ "F*" || REF_NAME =~ "MUX*" || REF_NAME =~ "CAR*" )}]
set cnt 0
foreach curCell $logicCells {
    set locDriven [get_property LOC [get_cells $curCell]]
    set cellPins [get_pins  -leaf -of_objects [get_cells $curCell]  -filter {DIRECTION == IN}]
    foreach pinDriven $cellPins {
        incr cnt
        if {$cnt == 100 } {
            set cnt 0
            set pinNet [get_nets -of [get_pins $pinDriven]]
            set tmp_net $pinNet
            set driverPin [get_pins  -leaf -of_objects  $tmp_net -filter {DIRECTION == OUT}]
            set driverCell [get_cells -of_objects [get_pins $driverPin]  -filter {LOC =~ "SLICE*" && (REF_NAME =~ "LUT*" || REF_NAME =~ "F*" || REF_NAME =~ "MUX*" || REF_NAME =~ "CAR*" )}]
            set driverNum [llength $driverCell]
            if ($driverNum>0) {
                set locDriver [get_property LOC [get_cells $driverCell]]
                set delay [get_property FAST_MAX [lindex [get_net_delays -of_objects [get_nets  $tmp_net] -to [get_pins $pinDriven] ] 0 ]]
                puts $fo "pinDriven=> $pinDriven locDriven=> $locDriven"
                puts $fo "driverPin=> $driverPin locDriver=> $locDriver"
                puts $fo "delay=> $delay"
            }
        }
    }
}
close $fo