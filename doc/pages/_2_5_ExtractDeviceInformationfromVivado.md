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


Below is an example showing what text file is in the extracted archive file, e.g. VCU108_DeviceSite.zip in the screenshot. 
We organize the content in the file mainly for user readibility and easier debugging/checking in Vivado. The information of each BEL will cost a single line. 

Each line in the file is related to a BEL on the device and a BEL is the smallest unit of resource on FPGA device. In the line, the name, site, tile, clock region and the type of site/tile of the BEL will be provided.

\verbatim
```perl
bel=> SLICE_X115Y0/F6LUT site=> SLICE_X115Y0 tile=> CLE_M_X69Y0 clockRegion=> X3Y0 sitetype=> SLICEM tiletype=> CLE_M
bel=> DSP48E2_X3Y0/DSP_A_B_DATA site=> DSP48E2_X3Y0 tile=> DSP_X69Y0 clockRegion=> X3Y0 sitetype=> DSP48E2 tiletype=> DSP
bel=> SLICE_X121Y0/HFF2 site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> RAMB18_X15Y1/RAMB18E2_U site=> RAMB18_X15Y1 tile=> BRAM_X73Y0 clockRegion=> X4Y0 sitetype=> RAMB181 tiletype=> BRAM
bel=> RAMB18_X15Y0/RAMBFIFO18 site=> RAMB18_X15Y0 tile=> BRAM_X73Y0 clockRegion=> X4Y0 sitetype=> RAMBFIFO18 tiletype=> BRAM
bel=> RAMB36_X15Y0/RAMBFIFO36E2 site=> RAMB36_X15Y0 tile=> BRAM_X73Y0 clockRegion=> X4Y0 sitetype=> RAMBFIFO36 tiletype=> BRAM
bel=> SLICE_X122Y0/A5LUT site=> SLICE_X122Y0 tile=> CLEL_R_X73Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/CARRY8 site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/CFF site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/CFF2 site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/D5LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/D6LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/DFF site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/DFF2 site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/E5LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/E6LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/EFF site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/EFF2 site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F5LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F6LUT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F7MUX_AB site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F7MUX_CD site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F7MUX_EF site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F7MUX_GH site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F8MUX_BOT site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> SLICE_X121Y0/F8MUX_TOP site=> SLICE_X121Y0 tile=> CLEL_R_X72Y0 clockRegion=> X4Y0 sitetype=> SLICEL tiletype=> CLEL_R
bel=> RAMB18_X15Y1/RAMB18E2_U site=> RAMB18_X15Y1 tile=> BRAM_X73Y0 clockRegion=> X4Y0 sitetype=> RAMB181 tiletype=> BRAM
```
\endverbatim

As it can be noticed, the extracted device information does not contain exact location information but only the site/tile names. To ge the exact coordinate of the BELs/sites, the Tcl script will call a Python script to analyze the extracted information and map the BELs/Sites to coordinates. We conduct such mapping based on some previous ISPD benchmark dataset. This procedure will generate a new archive where the text file contains information of the sites on the FPGA device.

In the generated file, each line corresponds to a FPGA site's name, tile, clock region, site type, tile type, location coordinate X/Y and the BELs insides the site. An example is shown below.

\verbatim
```perl
site=> RAMB36_X10Y71 tile=> BRAM_X50Y355 clockRegionName=> X2Y5 sitetype=> RAMBFIFO36 tiletype=> BRAM centerx=> 50.75 centery=> 357.35 BELs=> [RAMB36_X10Y71/RAMBFIFO36E2]
site=> SLICE_X16Y157 tile=> CLEL_L_X10Y157 clockRegionName=> X0Y2 sitetype=> SLICEL tiletype=> CLEL_L centerx=> 9.75 centery=> 157.0 BELs=> [SLICE_X16Y157/A5LUT,SLICE_X16Y157/A6LUT,SLICE_X16Y157/AFF,SLICE_X16Y157/AFF2,SLICE_X16Y157/B5LUT,SLICE_X16Y157/B6LUT,SLICE_X16Y157/BFF,SLICE_X16Y157/BFF2,SLICE_X16Y157/C5LUT,SLICE_X16Y157/C6LUT,SLICE_X16Y157/CARRY8,SLICE_X16Y157/CFF,SLICE_X16Y157/CFF2,SLICE_X16Y157/D5LUT,SLICE_X16Y157/D6LUT,SLICE_X16Y157/DFF,SLICE_X16Y157/DFF2,SLICE_X16Y157/E5LUT,SLICE_X16Y157/E6LUT,SLICE_X16Y157/EFF,SLICE_X16Y157/EFF2,SLICE_X16Y157/F5LUT,SLICE_X16Y157/F6LUT,SLICE_X16Y157/F7MUX_AB,SLICE_X16Y157/F7MUX_CD,SLICE_X16Y157/F7MUX_EF,SLICE_X16Y157/F7MUX_GH,SLICE_X16Y157/F8MUX_BOT,SLICE_X16Y157/F8MUX_TOP,SLICE_X16Y157/F9MUX,SLICE_X16Y157/FFF,SLICE_X16Y157/FFF2,SLICE_X16Y157/G5LUT,SLICE_X16Y157/G6LUT,SLICE_X16Y157/GFF,SLICE_X16Y157/GFF2,SLICE_X16Y157/H5LUT,SLICE_X16Y157/H6LUT,SLICE_X16Y157/HFF,SLICE_X16Y157/HFF2]
site=> DSP48E2_X2Y61 tile=> DSP_X51Y150 clockRegionName=> X2Y2 sitetype=> DSP48E2 tiletype=> DSP centerx=> 52.25 centery=> 153.42499999999998 BELs=> [DSP48E2_X2Y61/DSP_ALU,DSP48E2_X2Y61/DSP_A_B_DATA,DSP48E2_X2Y61/DSP_C_DATA,DSP48E2_X2Y61/DSP_MULTIPLIER,DSP48E2_X2Y61/DSP_M_DATA,DSP48E2_X2Y61/DSP_OUTPUT,DSP48E2_X2Y61/DSP_PREADD,DSP48E2_X2Y61/DSP_PREADD_DATA]
site=> HPIO_VREF_SITE_X1Y5 tile=> HPIO_L_X51Y150 clockRegionName=> X3Y2 sitetype=> HPIO_VREF_SITE tiletype=> HPIO_L centerx=> 52.75 centery=> 150.0 BELs=> [HPIO_VREF_SITE_X1Y5/HPIO_VREF1,HPIO_VREF_SITE_X1Y5/HPIO_VREF2]
site=> HPIOBDIFFINBUF_X1Y60 tile=> HPIO_L_X51Y150 clockRegionName=> X3Y2 sitetype=> HPIOBDIFFINBUF tiletype=> HPIO_L centerx=> 52.75 centery=> 150.0 BELs=> [HPIOBDIFFINBUF_X1Y60/DIFFINBUF]
site=> IOB_X1Y149 tile=> HPIO_L_X51Y150 clockRegionName=> X3Y2 sitetype=> HPIOB tiletype=> HPIO_L centerx=> 52.75 centery=> 171.42857142857144 BELs=> [IOB_X1Y149/IBUFCTRL,IOB_X1Y149/INBUF,IOB_X1Y149/OUTBUF,IOB_X1Y149/OUTINV,IOB_X1Y149/PAD,IOB_X1Y149/PULL]
```
\endverbatim

Meanwhile, the detailed information of the PCIE pins are also extracted because most of pins of BELs have tiny location offsets relative to the center of site (e.g., CLB) and we ignore them while the pin offsets of the PCIE slot are significant and cannot be ignored.

This is a temporary work-around solution since currently we don't have any detailed information of low-level physical architecture of UltraScale/+ FPGAs. We will try to conduct experiments on open-source FPGA archicture in the future.