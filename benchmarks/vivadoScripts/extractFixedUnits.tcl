set fo [open "/home/tingyuan/Downloads/OpenPiton_fixedUnits" "w"]
set allCells [xilinx::designutils::get_leaf_cells *]

set fixedUnits [get_cells $allCells -filter {IS_LOC_FIXED == true}]

foreach curCell $fixedUnits {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

close $fo
