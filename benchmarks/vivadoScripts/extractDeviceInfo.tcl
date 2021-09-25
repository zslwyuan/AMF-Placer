# replace this with the name for your device
set deviceName "VCU108" 
# replace this path with your one to specify where to store the files of extracted data
set targetFolderPath "/home/tingyuan/tmpFiles/" 

exec mkdir "${targetFolderPath}/${deviceName}"
set pahtPrefix "${targetFolderPath}/${deviceName}/${deviceName}_"
set script_path [ file dirname [ file normalize [ info script ] ] ]
source "${script_path}/extractTileSite.tcl"