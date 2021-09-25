set fo [open "${pahtPrefix}fixedUnits" "w"]

set fixedUnits [get_cells $allCells -filter {IS_LOC_FIXED == true}]

foreach curCell $fixedUnits {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

close $fo
