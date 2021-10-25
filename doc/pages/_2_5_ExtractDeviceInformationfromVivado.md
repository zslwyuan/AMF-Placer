@page _2_5_ExtractDeviceInformationfromVivado Extract Device Information from Vivado
# Extract Device Information from Vivado

Users can go through the following steps to extract the information of specific device in Vivado. Please note that we have provided device information for VCU108 which can be found in "benchmarks/VCU108/device". Please note that for a device, you can go through this extraction flow just ONE TIME. You DON'T need to do this every time your run AMFPlacer.

* a. Please ensure that your Vivado has the license for the specific device, so you can open the Device window by clicking on the top bar "Window->Device"

* b. Open the Tcl script "benchmarks/vivadoScripts/extractDeviceInfo.tcl" in the project directory where you can find the below content at the beginning and modify them according to the comments suggesting.
```tcl
# replace this with the name for your device
set deviceName "VCU108" 
# replace this path with your one to specify where to store the files of extracted data
set targetFolderPath "/home/tingyuan/tmpFiles/" 
```
* c. Open your design of your target device or just create empty project of the target device. Open the Device window by clicking on the top bar "Window->Device" and execut the command below in the Tcl console of Vivado. It might take tens of minutes or 6 hours to finish the extraction due to the slow Tcl execution (get_site_pins is SOOOO slow...) related to strings and IOs in Vivado. (You can run the script at night and go to bed ealier..)
```tcl
# replace XXXXX to indicate where your AMFPlacer is located.
source XXXXX/AMF-Placer/benchmarks/vivadoScripts/extractDeviceInfo.tcl
```
* d. Finally, you should be able to find the extracted files in target folder path set by you. Below is an example showing the generated files for VCU108:

<center>
<img src="deviceFiles.png" align="center" alt="Device Files" title="Device Files" width="300"  /> 
</center>

Please note that since the exact location of the extracted site/BEL/pins are not provided, the Tcl script uses a Python script to map the sites/BEL/pin to specifc location based on their names and hierarchy. If users change their target devices from VCU108 or Xilinx Ultrascale Series products, they might need to change the Python script to adapt to some other FPGA architectures.
