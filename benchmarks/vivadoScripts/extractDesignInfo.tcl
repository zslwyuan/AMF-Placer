# replace this with the name for your benchmark
set benchmarkName "OpenPiton" 
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