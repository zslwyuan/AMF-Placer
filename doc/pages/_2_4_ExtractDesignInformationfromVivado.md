@page _2_4_ExtractDesignInformationfromVivado Extract Design Information from Vivado
# Extract Design Information from Vivado

Users can go through the following steps to extract the information of specific designs. Please note that we have provide a set of benchmarks for VCU108 which can be found in "benchmarks/VCU108/design". Please note that for a design, you can go through this extraction flow just ONE TIME. You DON'T need to do this every time your run AMFPlacer.

* a. Please ensure that your design has sucessfully gone through the general Vivado flow to "Generate Bistream". In this way, Vivado will check your design to avoid some bugs in  frond-end design stages, and meanwhile we can extrat information of your design, even part of which are some of them are blackbox IPs in Vivado, from the "Implemented Design" which has gone through the placement and routing of Vivado.
* b. Open the Tcl script "benchmarks/vivadoScripts/extractDesignInfo.tcl" in the project directory where you can find the below content at the beginning and modify them according to the comments suggesting.
```tcl
# replace this with the name for your benchmark
set benchmarkName "OpenPiton" 
# replace this path with your one to specify where to store the files of extracted data
set targetFolderPath "/home/tingyuan/tmpFiles/" 
```
* c. Open your design in Vivado and "Open Implemented Design" and execut the command below in the Tcl console of Vivado. It might take tens of minutes or ~1hour to finish the extraction due to the slow Tcl execution related to strings and IOs in Vivado.
```tcl
# replace XXXXX to indicate where your AMFPlacer is located.
source XXXXX/AMF-Placer/benchmarks/vivadoScripts/extractDesignInfo.tcl
```
* d. Finally, you should be able to find the extracted files in target folder path set by you. Below is an example showing the generated files for OpenPiton:

<center>
<img src="designFiles.png" align="center"  alt="Design Files" title="Design Files" width="300" /> 
</center>