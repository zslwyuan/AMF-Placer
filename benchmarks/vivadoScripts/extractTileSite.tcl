
set fo [open "/home/tingyuan/Downloads/VCU108DeviceSite" "w"]

set allBELs [get_bels]
#set allSites [get_sites]

foreach curBEL $allBELs {
    set curSite [get_sites -of_objects $curBEL]
    set clockRegion [get_property CLOCK_REGION [get_sites $curSite]]
    set curTile [get_tiles -of_objects [get_sites $curSite]]
    set tileType [get_property TYPE $curTile]
    set siteType [get_property SITE_TYPE $curSite]
    puts $fo  "bel=> $curBEL site=> $curSite tile=> $curTile clockRegion=> $clockRegion sitetype=> $siteType tiletype=> $tileType"
}
close $fo

# extract PCIE

set fo [open "/home/tingyuan/Downloads/PCIEPin2Sw0" "w"]
set curPCIESite [get_sites PCIE_3_1_X0Y0]
set PCIEPins [get_site_pins -of_objects [get_sites $curPCIESite]]

foreach curPin $PCIEPins {
    set swTile [get_tiles -of_objects [get_nodes -of_objects [get_site_pins $curPin]] -filter {TYPE =~ "INT_INTERFACE_PCIE*"}]
    puts $fo  "pin=> $curPin swtile=> $swTile"
}
close $fo