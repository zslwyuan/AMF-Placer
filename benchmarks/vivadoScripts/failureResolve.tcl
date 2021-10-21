close $fo
set a [open "${script_path}/initialPlacementError"]
set lines [split [read $a] "\n"]
close $a;
exec rm "${script_path}/initialPlacementError"

set b [open "${script_path}/placementError" "w"]

set lineCnt 0
set placeBatch {}

foreach line $lines {
    incr lineCnt
    set placeBatch [concat $placeBatch $line]
    if {$lineCnt == 5 } {
        set result [catch {place_cell  $placeBatch }]
        if {$result} {
            puts $b $placeBatch
        }
        set lineCnt 0
        set placeBatch ""
    }
}
close $b


set a [open "./placementError"]
set lines [split [read $a] "\n"]
close $a;

set b [open "./finalPlacementError" "w"]

set lineCnt 0
set placeBatch {}

foreach line $lines {
    set result [catch {place_cell  $line }]
    if {$result} {
        puts $b $line
    }
}
