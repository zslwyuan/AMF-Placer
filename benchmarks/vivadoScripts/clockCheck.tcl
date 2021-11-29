
set targetCol "SLICE_X57"
set fo [open "/home/tingyuan/Documents/AMF-Placer/benchmarks/vivadoScripts/clockInfoCheck" "w"]
for {set i 0} {$i < 300} {incr i} {
    set findClocks [get_nets  -top_net_of_hierarchical_group -of_objects [get_cells -of_objects  [get_sites "SLICE_X57Y${i}"]] -filter { TYPE == "GLOBAL_CLOCK" }]
    set clockNum [llength $findClocks]
    puts $fo "=================================${targetCol} Y ${i} with ${clockNum} clocks"
    foreach curClock $findClocks {
        set tmp_net_driver_pin [get_pins  -leaf -of_objects  $curClock -filter {DIRECTION == OUT}]
        puts $fo "clock: ${curClock} driverPin: ${tmp_net_driver_pin}"
    }
}
close $fo