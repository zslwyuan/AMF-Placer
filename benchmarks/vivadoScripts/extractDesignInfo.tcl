# open_checkpoint "/Softwares/chipyard/fpga/generated-src/chipyard.fpga.vcu108.VCU108FPGATestHarness.RocketVCU108Config/obj/post_route.dcp"
# replace this with the name for your benchmark
set benchmarkName "Gemmini" 
# replace this path with your one to specify where to store the files of extracted data
set targetFolderPath "/home/tingyuan/tmpFiles/" 

exec mkdir "${targetFolderPath}/${benchmarkName}"
set pahtPrefix "${targetFolderPath}/${benchmarkName}/${benchmarkName}_"
set script_path [ file dirname [ file normalize [ info script ] ] ]
source "${script_path}/extractNetlist.tcl"
source "${script_path}/extractLUTRAMs.tcl"
source "${script_path}/extractFixedUnits.tcl"
exec zip -j "${pahtPrefix}allCellPinNet.zip" "${pahtPrefix}allCellPinNet"
exec rm "${pahtPrefix}allCellPinNet"