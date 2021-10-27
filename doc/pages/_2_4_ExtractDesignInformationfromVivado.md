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


Below is an example showing what text file is in the extracted archive file, e.g. OpenPiton_allCellPinNet.zip in the screenshot. 
We organize the content in the file mainly for user readibility and easier debugging/checking in Vivado. The information of each cell will cost multiple lines. 

The first line for a cell in the design netlist will be the one begin with "curCell=> " and provide the name and type of the cell.

Then the following lines after this cell declaration are the lines for the pins on this cell. Each of these lines begins with "pin=>". 
The pin information will keep loading until another line beginning with "curCell=> " is found.
For each pin, the name, reference name on cell, IO direction relative to the cell, the name of connected net and the name of the driver pin of the connected net will be provided in one line.

\verbatim
```perl
curCell=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9 type=> LUT6
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/O refpin=> O dir=> OUT net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9_n_0 drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I0 refpin=> I0 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data[223] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data_reg[223]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I1 refpin=> I1 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data[95] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data_reg[95]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I2 refpin=> I2 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[3] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[3]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I3 refpin=> I3 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data[159] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data_reg[159]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I4 refpin=> I4 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[4] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[4]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_9/I5 refpin=> I5 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data[31] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_data_reg[31]/Q
curCell=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0] type=> FDRE
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/Q refpin=> Q dir=> OUT net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/Q
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/C refpin=> C dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/aclk drivepin=> design_1_i/xilinx_dma_pcie_ep_0/inst/xdma_0_i/inst/pcie3_ip_i/inst/xdma_0_pcie3_ip_gt_top_i/phy_clk_i/bufg_gt_userclk/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/CE refpin=> CE dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_0 drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[7]_i_1/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/D refpin=> D dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/p_0_in[0] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0]_i_1/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]/R refpin=> R dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/<const0> drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/GND/G
curCell=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2 type=> MUXF7
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2/O refpin=> O dir=> OUT net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2_n_0 drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2/I0 refpin=> I0 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0]_i_6_n_0 drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0]_i_6/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2/I1 refpin=> I1 dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0]_i_7_n_0 drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data[0]_i_7/O
   pin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r1_data_reg[0]_i_2/S refpin=> S dir=> IN net=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[2] drivepin=> design_1_i/axis_dwidth_converter_0/inst/gen_downsizer_conversion.axisc_downsizer_0/r0_out_sel_next_r_reg[2]/Q
```
\endverbatim

Besides, we also extract the clock information/fixed elements/vendor primitive macros of the design from Vivado. 

In the clock information file, e.g., OpenPiton_clocks, each line will be the name of clock driver pin. This information will be used for clock legalization.

In the fixed element file, e.g., OpenPiton_fixedUnits, each line will describe a fixed element with its name, location site and location BEL in the site.

In the unpredictable macro file, e.g., OpenPiton_unpredictableMacros, each line will describe a macro that AMF-Placer cannot precisely/properly extract currently (usually they are the clock-domain-crossing FFs or small low-bitwidth LUTRAMs), with their name, their location site in the post-implementation design and the location BEL. Usually, the site of these element can be changed but their locations relative to those elements in the same site/neighbor site cannot be changed. Mose of the macros we can infer their build rules but for some other macros we cannot infer. Therefore, for these "unpredictable" macros, we need to extract the information from Vivado instead of building them via InitialPacker.