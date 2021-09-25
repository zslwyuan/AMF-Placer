# replace this with the name for your device
set deviceName "VCU108" 
# replace this path with your one to specify where to store the files of extracted data
set targetFolderPath "/home/tingyuan/tmpFiles/" 

exec mkdir "${targetFolderPath}/${deviceName}"
set pahtPrefix "${targetFolderPath}/${deviceName}/${deviceName}_"
set script_path [ file dirname [ file normalize [ info script ] ] ]
source "${script_path}/extractTileSite.tcl"

exec zip -j "${pahtPrefix}DeviceSite.zip" "${pahtPrefix}DeviceSite"
exec rm "${pahtPrefix}DeviceSite"

unset env(PYTHONPATH)
unset env(PYTHONHOME)

exec python3 "${script_path}/../VCU108/preprocessPython/exportDeviceLocation.py" "${targetFolderPath}" "${deviceName}"
exec zip -j "${pahtPrefix}exportSiteLocation.zip" "${pahtPrefix}exportSiteLocation"
exec rm "${pahtPrefix}exportSiteLocation"

exec python3 "${script_path}/../VCU108/preprocessPython/getPinOffset.py" "${targetFolderPath}" "${deviceName}"
