set fo [open "${pahtPrefix}unpredictableMacros" "w"]
set ram32s [get_cells $allCells -filter {REF_NAME =~ "RAM32X1D"}]
foreach curCell $ram32s {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set ram64s [get_cells $allCells -filter {REF_NAME =~ "RAM64X1S"}]
foreach curCell $ram64s {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set RAM64Ms [get_cells $allCells -filter {REF_NAME =~ "RAM64M"}]
foreach curCell $RAM64Ms {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set RAM64X1Ds [get_cells $allCells -filter {REF_NAME =~ "RAM64X1D"}]
foreach curCell $RAM64X1Ds {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set RAM32Ms [get_cells $allCells -filter {REF_NAME =~ "RAM32M"}]
foreach curCell $RAM32Ms {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set RAM32X1Ss [get_cells $allCells -filter {REF_NAME =~ "RAM32X1S"}]
foreach curCell $RAM32X1Ss {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set RAM256X1Ds [get_cells $allCells -filter {REF_NAME =~ "RAM256X1D"}]
foreach curCell $RAM256X1Ds {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}


set AsyncRegs [get_cells $allCells -filter { ASYNC_REG == "TRUE" }  ]
foreach curCell $AsyncRegs {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts  $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}

set XPMs [get_cells $allCells -filter { XPM_CDC != "" } ]
foreach curCell $XPMs {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}


set specialCells [get_cells -hierarchical -filter { KEEP =~  "*yes*" } ]
foreach curCell $specialCells {
    set tmpLoc [get_property LOC $curCell]
    set tmpBEL [get_property BEL $curCell]
    puts $fo "name=> $curCell loc=>  $tmpLoc bel=>  $tmpBEL"
}


close $fo
