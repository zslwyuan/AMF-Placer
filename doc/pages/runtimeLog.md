# Runtime Log Explanation {#runtimeLog}

In this section, we will explain the information that the placer prints in the terminal, to help users to understand what is going on with AMFPlacer. Please note that AMFPlacer can also dump more information to files as long as users set configurations.

```bash
./AMFPlacer ../benchmarks/testConfig/OpenPiton.json
```

Below is an complete output in actual order shown in the terminal when users run the command above.

**1. Parse the placement configuration:** We implemented a simple JSON parser to parse the input placement configuration and here we let users to check whether the configuration is exactly the one they expect.
<hr>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Placer configuration is loaded and the information is shown below, please check: <br/>
&emsp;   "ClusterPlacerVerbose"&emsp;   ====&emsp;   "false" <br/>
&emsp;   "DrawNetAfterEachIteration"&emsp;   ====&emsp;   "false" <br/>
&emsp;   "Dump Cluster Simulated Annealig file"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//./dumpSATrace" <br/>
&emsp;   "Dump Cluster file"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//./dumpClusters" <br/>
&emsp;   "DumpAllCoordTrace"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//./DumpAllCoordTrace" <br/>
&emsp;   "DumpCLBPacking"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//./DumpCLBPacking" <br/>
&emsp;   "DumpClockUtilization"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//true" <br/>
&emsp;   "DumpLUTFFPair"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//./DumpLUTFFPair" <br/>
&emsp;   "GlobalPlacementIteration"&emsp;   ====&emsp;   "30" <br/>
&emsp;   "GlobalPlacerPrintHPWL"&emsp;   ====&emsp;   "true" <br/>
&emsp;   "GlobalPlacerVerbose"&emsp;   ====&emsp;   "false" <br/>
&emsp;   "MKL"&emsp;   ====&emsp;   "true" <br/>
&emsp;   "PseudoNetWeight"&emsp;   ====&emsp;   "0.0025" <br/>
&emsp;   "Simulated Annealing IterNum"&emsp;   ====&emsp;   "30000000" <br/>
&emsp;   "Simulated Annealing restartNum"&emsp;   ====&emsp;   "600" <br/>
&emsp;   "cellType2fixedAmo file"&emsp;   ====&emsp;   "../benchmarks/VCU108/compatibleTable/cellType2fixedAmo" <br/>
&emsp;   "cellType2sharedCellType file"&emsp;   ====&emsp;   "../benchmarks/VCU108/compatibleTable/cellType2sharedCellType" <br/>
&emsp;   "clock file"&emsp;   ====&emsp;   "../benchmarks/VCU108/design/OpenPiton/OpenPiton_clocks" <br/>
&emsp;   "clockRegionBRAMNum"&emsp;   ====&emsp;   "96" <br/>
&emsp;   "clockRegionDSPNum"&emsp;   ====&emsp;   "30" <br/>
&emsp;   "clockRegionXNum"&emsp;   ====&emsp;   "5" <br/>
&emsp;   "clockRegionYNum"&emsp;   ====&emsp;   "8" <br/>
&emsp;   "designCluster"&emsp;   ====&emsp;   "../benchmarks/VCU108/design/OpenPiton/OpenPiton_clusters.zip" <br/>
&emsp;   "drawClusters"&emsp;   ====&emsp;   "false" <br/>
&emsp;   "dumpDirectory"&emsp;   ====&emsp;   "../../../Documents/placerDumpData//../../../Documents/placerDumpData/" <br/>
&emsp;   "fixed units file"&emsp;   ====&emsp;   "../benchmarks/VCU108/design/OpenPiton/OpenPiton_fixedUnits" <br/>
&emsp;   "jobs"&emsp;   ====&emsp;   "8" <br/>
&emsp;   "mergedSharedCellType2sharedCellType"&emsp;   ====&emsp;   "../benchmarks/VCU108/compatibleTable/mergedSharedCellType2sharedCellType" <br/>
&emsp;   "sharedCellType2BELtype file"&emsp;   ====&emsp;   "../benchmarks/VCU108/compatibleTable/sharedCellType2BELtype" <br/>
&emsp;   "special pin offset info file"&emsp;   ====&emsp;   "../benchmarks/VCU108/device/PCIEPin2SwXY" <br/>
&emsp;   "unpredictable macro file"&emsp;   ====&emsp;   "../benchmarks/VCU108/design/OpenPiton/OpenPiton_unpredictableMacros" <br/>
&emsp;   "vivado extracted design information file"&emsp;   ====&emsp;   "../benchmarks/VCU108/design/OpenPiton/OpenPiton_allCellPinNet.zip" <br/>
&emsp;   "vivado extracted device information file"&emsp;   ====&emsp;   "../benchmarks/VCU108/device/exportSiteLocation.zip" <br/>
&emsp;   "y2xRatio"&emsp;   ====&emsp;   "0.4" <br/>
<hr>

**2. Load the device information:** AMFPlacer will load the device information from the files specified in the JSON files and print out some information for checking. The information will include: (1) whether some resource elements will be mapped in some other types during placement since they might share resources on the device; (2) how many clock regions are on the device and how they are organized; (3) the numbers/types of tiles/sites/BELs; (4) whether users have provided complete information of the device (some resources, which are not used by the design, can has no specification in the device files.)
<hr>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   mapping BELType [SLICEM_CARRY8] to [SLICEL_CARRY8] when creating PlacementBins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   mapping BELType [SLICEM_LUT] to [SLICEL_LUT] when creating PlacementBins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   mapping BELType [SLICEM_FF] to [SLICEL_FF] when creating PlacementBins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   mapping BELType [SLICEM_MUXF7] to [SLICEL_MUXF7] when creating PlacementBins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   mapping BELType [SLICEM_MUXF8] to [SLICEL_MUXF8] when creating PlacementBins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 8x5(YxX) clock regions on the device <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   New Device Info Created. (elapsed time: 3.503 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #ExtractedTile= 71312 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #ExtractedSite= 81110 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #ExtractedBEL= 2734042 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Tile(28 types): AMS BRAM CFG_CFG CLEL_L CLEL_R CLE_M CLE_M_R CMAC_CMAC_FT DSP GTH_R GTY_QUAD_LEFT_FT HPIO_L HRIO_L ILMAC_ILMAC_FT PCIE RCLK_BRAM_L RCLK_BRAM_R RCLK_CLEL_L RCLK_CLEL_R RCLK_CLEL_R_L RCLK_CLEL_R_R RCLK_CLE_M_L RCLK_CLE_M_R RCLK_INT_L RCLK_INT_R RCLK_RCLK_BRAM_L_AUXCLMP_FT RCLK_RCLK_BRAM_L_BRAMCLMP_FT XIPHY_L  <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Site(38 types): BITSLICE_CONTROL BITSLICE_RX_TX BITSLICE_TX BUFCE_LEAF_X16 BUFCE_ROW BUFGCE BUFGCE_DIV BUFGCTRL BUFG_GT BUFG_GT_SYNC CMAC_SITE CONFIG_SITE DSP48E2 GTHE3_CHANNEL GTHE3_COMMON GTYE3_CHANNEL GTYE3_COMMON HARD_SYNC HPIOB HPIOBDIFFINBUF HPIOBDIFFOUTBUF HPIO_VREF_SITE HRIO HRIODIFFINBUF HRIODIFFOUTBUF ILKN_SITE MMCME3_ADV PCIE_3_1 PLLE3_ADV PLL_SELECT_SITE RAMB181 RAMBFIFO18 RAMBFIFO36 RIU_OR SLICEL SLICEM SYSMONE1 XIPHY_FEEDTHROUGH  <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   BEL(131 types): A5LUT A6LUT AFF AFF2 B5LUT B6LUT BFF BFF2 BSCAN1 BSCAN2 BSCAN3 BSCAN4 BUFCE BUFCE0 BUFCE1 BUFCE10 BUFCE11 BUFCE12 BUFCE13 BUFCE14 BUFCE15 BUFCE2 BUFCE3 BUFCE4 BUFCE5 BUFCE6 BUFCE7 BUFCE8 BUFCE9 BUFGCE_DIV BUFGCTRL BUFG_GT BUFG_GT_SYNC C5LUT C6LUT CARRY8 CFF CFF2 CMAC CONTROL D5LUT D6LUT DCIRESET DFF DFF2 DIFFINBUF DIFFOUTBUF DNA_PORT DSP_ALU DSP_A_B_DATA DSP_C_DATA DSP_MULTIPLIER DSP_M_DATA DSP_OUTPUT DSP_PREADD DSP_PREADD_DATA E5LUT E6LUT EFF EFF2 EFUSE_USR F5LUT F6LUT F7MUX_AB F7MUX_CD F7MUX_EF F7MUX_GH F8MUX_BOT F8MUX_TOP F9MUX FFF FFF2 FRAME_ECC G5LUT G6LUT GCLK_TEST_DELAY GFF GFF2 GTHE3_CHANNEL GTHE3_COMMON GTYE3_CHANNEL GTYE3_COMMON H5LUT H6LUT HFF HFF2 HPIO_VREF1 HPIO_VREF2 IBUFCTRL IBUFDS0_GTE3 IBUFDS0_GTYE3 IBUFDS1_GTE3 IBUFDS1_GTYE3 ICAP_BOT ICAP_TOP ILKN INBUF IPAD1 IPAD2 MASTER_JTAG MMCME3_ADV OBUFDS0_GTE3 OBUFDS0_GTYE3 OBUFDS1_GTE3 OBUFDS1_GTYE3 OPAD1 OPAD2 OUTBUF OUTINV PAD PCIE_3_1 PLLE3_ADV PLL_SELECT PULL RAMB18E2_L RAMB18E2_U RAMB36E2 RAMBFIFO18 RAMBFIFO36E2 REFCLK0N REFCLK0P REFCLK1N REFCLK1P RIU_OR RXTX_BITSLICE STARTUP SYN_UNIT SYSMONE1 TRISTATE_TX_BITSLICE USR_ACCESS XIPHY_FEEDTHROUGH  <br/>
<hr>

**3. Load the design information:** AMFPlacer will load the design information from the files specified in the JSON files and print out some information for checking. The information will include: (1) whether there is any problem in the design file (e.g., duplicated elements dumped by Vivado); (2)  the numbers/types of elements in the design netlist; 
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Design Information Loading. (elapsed time: 3.503 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc2_chip_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc2_iob_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc2_uart_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_ariane_bootrom_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_ariane_clint_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_ariane_debug_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_ariane_plic_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_mem_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_sd_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/chipset_impl/noc3_uart_to_xbar/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/offchip_processor_noc2_v2c/ <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs. <br/>
duplicated cell: DesignCell: (LUT5) chipset/offchip_processor_noc3_v2c/ <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Connected Cell Pairs in Small Nets = 4069970 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #global clock=17 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Design User-Defined Cluster Information Loading. (elapsed time: 23.124 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Nets Enhanced for userDefinedClusters = 12051 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Cells in userDefinedClusters = 174364 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   New Design Info Created. (elapsed time: 23.758 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Cell= 309133 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Net= 350820 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   CellTypes(44 types): LUT1 LUT2 LUT3 LUT4 LUT5 LUT6 LUT6_2 FDCE FDPE FDRE FDSE LDCE AND2B1L CARRY8 DSP48E2 MUXF7 MUXF8 SRL16E SRLC32E RAM32M16 RAM32M RAM32X1D RAM32X1S RAMB18E2 RAMB36E2 BUFGCE BUFGCTRL IOBUF IBUF IBUFDS IOBUFDS IOBUFE3 MMCME3_ADV OBUF PCIE_3_1 BSCANE2 RXTX_BITSLICE BITSLICE_CONTROL TX_BITSLICE_TRI OSERDESE3 RIU_OR PLLE3_ADV HPIO_VREF OBUFDS_DUAL_BUF  <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #LUTCnt: 180376 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #FFCnt: 111966 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #CarryCnt: 1712 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #MuxCnt: 13696 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #LUTRAMCnt: 752 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #DSPCnt: 58 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #BRAMCnt: 147 <br/>
<hr>


**4. Load the legalization information and initialize PlacementInfo:** AMFPlacer will load the legalization information from the files specified in the JSON files and print out some information for checking. The information will mainly include the capability of different sites on device and which BELs can be mapped to which types of sites. After loading the legalization information, AMFPlacer will initialize a container of all the placement information.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Loading compatiblePlacementTable (elapsed time: 23.758 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (SLICEL_CARRY8). They are :CARRY8 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (SLICEM_CARRY8). They are :CARRY8 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 16 slot(s) in a site for cell type : (SLICEM_LUT). They are :A5LUT,A6LUT,B5LUT,B6LUT,C5LUT,C6LUT,D5LUT,D6LUT,E5LUT,E6LUT,F5LUT,F6LUT,G5LUT,G6LUT,H5LUT,H6LUT <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 16 slot(s) in a site for cell type : (SLICEL_LUT). They are :A5LUT,A6LUT,B5LUT,B6LUT,C5LUT,C6LUT,D5LUT,D6LUT,E5LUT,E6LUT,F5LUT,F6LUT,G5LUT,G6LUT,H5LUT,H6LUT <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 16 slot(s) in a site for cell type : (SLICEL_FF). They are :AFF,AFF2,BFF,BFF2,CFF,CFF2,DFF,DFF2,EFF,EFF2,FFF,FFF2,GFF,GFF2,HFF,HFF2 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 16 slot(s) in a site for cell type : (SLICEM_FF). They are :AFF,AFF2,BFF,BFF2,CFF,CFF2,DFF,DFF2,EFF,EFF2,FFF,FFF2,GFF,GFF2,HFF,HFF2 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 4 slot(s) in a site for cell type : (SLICEL_MUXF7). They are :F7MUX_AB,F7MUX_CD,F7MUX_EF,F7MUX_GH <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 4 slot(s) in a site for cell type : (SLICEM_MUXF7). They are :F7MUX_AB,F7MUX_CD,F7MUX_EF,F7MUX_GH <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 2 slot(s) in a site for cell type : (SLICEL_MUXF8). They are :F8MUX_TOP,F8MUX_BOT <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 2 slot(s) in a site for cell type : (SLICEM_MUXF8). They are :F8MUX_TOP,F8MUX_BOT <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (DSP_ALU). They are :DSP_ALU <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (RAMB18E2_L). They are :RAMB18E2_L <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (RAMB18E2_U). They are :RAMB18E2_U <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BUFG_GT_SYNC). They are :BUFG_GT_SYNC <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (HPIOB_INPUT). They are :INBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (HRIO_INPUT). They are :INBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (PCIE_3_1). They are :PCIE_3_1 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (HPIOB_OUTPUT). They are :OUTBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (IOBUF). They are :INBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (HRIO_OUTPUT). They are :OUTBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (GTHE3_CHANNEL). They are :OUTBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (MMCME3_ADV). They are :OUTBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BUFG_GT). They are :BUFG_GT <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (GTHE3_COMMON). They are :GTHE3_COMMON <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 2 slot(s) in a site for cell type : (IBUFDS_GTH). They are :IBUFDS0_GTE3,IBUFDS1_GTE3 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 2 slot(s) in a site for cell type : (IBUFDS_GTY). They are :IBUFDS0_GTYE3,IBUFDS1_GTYE3 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (HPIOBDIFFINBUF_DIFFINBUF). They are :DIFFINBUF <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BUFCE). They are :BUFCE <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (CONFIG_SITE_BSCAN1). They are :BSCAN1 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (SYSMONE1). They are :SYSMONE1 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BUFGCE_DIV). They are :BUFGCE_DIV <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (RXTX_BITSLICE). They are :RXTX_BITSLICE <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BITSLICE_RX_TX_OSERDESE3). They are :OSERDESE3 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BUFGCTRL). They are :BUFGCTRL <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (BITSLICE_CONTROL). They are :CONTROL <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (TX_BITSLICE_TRI). They are :TRISTATE_TX_BITSLICE <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (RIU_OR). They are :RIU_OR <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (PLLE3_ADV). They are :PLLE3_ADV <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 2 slot(s) in a site for cell type : (HPIO_VREF). They are :HPIO_VREF1,HPIO_VREF2 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   There are 1 slot(s) in a site for cell type : (OBUFDS_DUAL_BUF). They are :OUTINV <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   New Compatible Placement Table Loaded. (elapsed time: 23.759 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   New Placement Info Created. (elapsed time: 23.761 s) <br/>
<hr>


**5. Identify macros from the netlist:** AMFPlacer will identify the macros of various types from the netlist based on pre-defined patterns. A netlist with PlacementInfo::PlacementNet will be created for the placement. In this netlist, the macros will be instantiated as PlacementInfo::PlacementMacro while the other elements will be instantiated as PlacementInfo::PlacementUnpackedCell. 
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   InitialPacker Finding Macros (elapsed time: 23.761 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #CARRY Macro: 523 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #CARRY Macro Cells: 32565 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   CARRY Macro Extracted. (elapsed time: 23.848 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Mux Macro: 7496 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Mux Macro Cells: 49088 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Mux Macro Extracted. (elapsed time: 23.965 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #BRAM Macro: 120 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #BRAM Macro Cells: 288 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   BRAM Macro Extracted. (elapsed time: 23.972 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #DSP Macro: 17 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #DSP Macro Cells: 57 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   DSP Macro Extracted. (elapsed time: 23.979 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #CLB Macro: 517 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #CLB Macro Cells: 1527 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   CLB Macro Extracted. (elapsed time: 23.987 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   InitialPacker Pairing LUTs and FFs. (elapsed time: 23.988 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   InitialPacker: LUTTO1FFPackedCnt=47495 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   InitialPacker Paired LUTs and FFs (#Pairs = 47495) (elapsed time: 24.267 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   InitialPacker Finding unpacked units (elapsed time: 24.267 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #Cells In Macro = 178515 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   InitialPacker Loading Nets (elapsed time: 24.458 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=110625 (elapsed time: 25.308 s) <br/>
<hr>


**6. Initialize cell bin grid:** AMFPlacer will divide the device into a mesh grid of bins, and each bin (i.e., PlacementInfo::PlacementBinInfo) will record the elements locating in it. Based on these information, we can evaluate the cell density and find the neighbor elements during later cell spreading/area adjustion. Meanwhile, during building the grid, AMFPlacer will verify whether each types of elements (BEL) can be mapped to the bin grid.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   set BEL type for each cell (elapsed time: 25.494 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinX: -0.400000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinY: 0.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxX: 86.400002 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxY: 479.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Placement Unit(s): 218754 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Unpacked Placement Unit(s): 162586 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Macro Placement Unit(s): 56168 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (ILKN_SITE) is not mapped to bin grid. e.g. [ILKN_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HPIOBDIFFOUTBUF) is not mapped to bin grid. e.g. [HPIOBDIFFOUTBUF_X0Y11]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (PLL_SELECT_SITE) is not mapped to bin grid. e.g. [PLL_SELECT_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (XIPHY_FEEDTHROUGH) is not mapped to bin grid. e.g. [XIPHY_FEEDTHROUGH_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HRIODIFFOUTBUF) is not mapped to bin grid. e.g. [HRIODIFFOUTBUF_X0Y11]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HRIODIFFINBUF) is not mapped to bin grid. e.g. [HRIODIFFINBUF_X0Y3]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (BUFCE_ROW) is not mapped to bin grid. e.g. [BUFCE_ROW_X69Y3]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (RAMBFIFO36) is not mapped to bin grid. e.g. [RAMB36_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (GTYE3_CHANNEL) is not mapped to bin grid. e.g. [GTYE3_CHANNEL_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HARD_SYNC) is not mapped to bin grid. e.g. [HARD_SYNC_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (BUFCE_LEAF_X16) is not mapped to bin grid. e.g. [BUFCE_LEAF_X16_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (CMAC_SITE) is not mapped to bin grid. e.g. [CMAC_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Bin Grid Size: Y: 96 X:18 (elapsed time: 25.754 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Bin Grid for Density Control Created (elapsed time: 25.754 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Device Information Verified for Design Information (elapsed time: 25.765 s) <br/>
<hr>


**7. Initialize timing information:** AMFPlacer will initialize the PlacementTimingInfo which is currently used to simply improve the timing of the placement. Here, we will create the DAG (timing graph) between combinational logic elements and timing end-points (e.g., registers, BRAMs, DSPs, IOs and clocks) and conduct levelization to identify long paths in the timing graph. Based on these information, we will enhance some nets in the original netlist. The more comprehensive STA engine will be merged in the following months.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: building simple timing graph (TimingNode is DesignCell) (elapsed time: 25.765 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: Timing graph starts forward levalization (elapsed time: 26.249 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   PlacementTimingInfo: total level = 53 details: 0(141886), 1(50533), 2(26765), 3(21937), 4(8323), 5(7429), 6(7552), 7(5026), 8(5243), 9(3139), 10(4829), 11(2788), 12(2128), 13(9517), 14(1800), 15(5131), 16(6255), 17(5234), 18(4603), 19(2982), 20(1889), 21(3360), 22(670), 23(491), 24(230), 25(493), 26(220), 27(454), 28(491), 29(379), 30(471), 31(374), 32(230), 33(247), 34(222), 35(214), 36(202), 37(557), 38(1288), 39(1126), 40(712), 41(78), 42(22), 43(34), 44(18), 45(18), 46(18), 47(18), 48(18), 49(18), 50(18), 51(18), 52(4),  <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: Timing graph finished forward levalization (elapsed time: 26.414 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: Timing graph starts backward levalization (elapsed time: 26.414 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: Timing graph finished backward levalization (elapsed time: 26.505 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingInfo: built simple timing graph (elapsed time: 26.551 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   pseudoNetWeightConsiderNetNum option is turn on: 0.004123 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingOptimizer: enhanceNetWeight_LevelBased starts. (elapsed time: 26.551 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=5.763714) (elapsed time: 26.660 s) <br/>
<hr>


**8. Simulated annealing-based initial cluster-level placement:** Based on the connectivity and clock domains the design will be partitioned into several clusters and place on the device based on SA algorithm to get a better start point for later global placement iterations. We implement the SA algorithm in a multi-threading approach, where we start the independent SA procedures in different threads to cover more situations. Therefore, AMFPlacer will print out the resultant HPWL for each SA procedure.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Cluster Placement Start. (elapsed time: 26.660 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Clustering Start. (elapsed time: 26.660 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=383) for clock [dbg_hub/inst/BSCANID.u_xsdbm_id/SWITCH_N_EXT_BSCAN.u_bufg_icon_tck/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1047) for clock [chipset/chipset_impl/piton_sd_top/sd_init/FSM_sequential_cb_state[1]_i_3_bufg_place/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1408) for clock [chipset/chipset_impl/mc_top/i_ddr4_0/inst/u_ddr4_infrastructure/u_bufg_riuClk/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=67) for clock [DOUTA_reg[65]_i_2__0/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1194) for clock [chipset/chipset_impl/piton_sd_top/sdc_controller/clock_divider0/sd_clk_bufgmux/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=457) for clock [chipset/chipset_impl/mc_top/i_ddr4_0/inst/u_ddr4_infrastructure/u_bufg_dbg_clk/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=2870) for clock [chipset/chipset_rst_n_ff_reg_bufg_place/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=67) for clock [DOUTA_reg[65]_i_2/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are 17 global clocks and totally 8 clock clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are totally 0 long-path clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are totally 211261 Single-PU clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: #clusterNet=257529 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   partitioned iter#11 #node: 29839 #net: 53816 max_cut: 1792 max_imbal: 0.400000 get too large cut: 1040 failed to find cut meeting requirements of min-cut or size balance. cluster[0].size=9503 cluster[1].size=20336 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Recursive partitioning generates 13 clusters and the numbers of the placement units for them are: 10625-24802(DSP:11,BRAM:3), 11545-26712(DSP:0,BRAM:0), 11806-27225(DSP:0,BRAM:0), 11869-28520(DSP:0,BRAM:0), 12623-29700(DSP:11,BRAM:0), 13678-27387(DSP:16,BRAM:56), 14171-26337(DSP:0,BRAM:16), 15512-30459(DSP:0,BRAM:64), 21729-41256(DSP:1,BRAM:10), 18004-32111(DSP:0,BRAM:40), 19879-42289(DSP:16,BRAM:8), 25229-47482(DSP:0,BRAM:43), 32084-69294(DSP:3,BRAM:51),  <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Average cluster size is too large = 34890 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=383) for clock [dbg_hub/inst/BSCANID.u_xsdbm_id/SWITCH_N_EXT_BSCAN.u_bufg_icon_tck/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1047) for clock [chipset/chipset_impl/piton_sd_top/sd_init/FSM_sequential_cb_state[1]_i_3_bufg_place/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1408) for clock [chipset/chipset_impl/mc_top/i_ddr4_0/inst/u_ddr4_infrastructure/u_bufg_riuClk/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=67) for clock [DOUTA_reg[65]_i_2__0/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=1194) for clock [chipset/chipset_impl/piton_sd_top/sdc_controller/clock_divider0/sd_clk_bufgmux/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=457) for clock [chipset/chipset_impl/mc_top/i_ddr4_0/inst/u_ddr4_infrastructure/u_bufg_dbg_clk/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=2870) for clock [chipset/chipset_rst_n_ff_reg_bufg_place/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   build a cluset (size=67) for clock [DOUTA_reg[65]_i_2/O]. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are 17 global clocks and totally 8 clock clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are totally 0 long-path clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: There are totally 211261 Single-PU clusters are identified. <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: #clusterNet=257529 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   partitioned iter#23 #node: 29839 #net: 53816 max_cut: 5381 max_imbal: 0.400000 get too large cut: 1040 failed to find cut meeting requirements of min-cut or size balance. cluster[0].size=9503 cluster[1].size=20336 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Recursive partitioning generates 13 clusters and the numbers of the placement units for them are: 10625-24802(DSP:11,BRAM:3), 11545-26712(DSP:0,BRAM:0), 11806-27225(DSP:0,BRAM:0), 11869-28520(DSP:0,BRAM:0), 12623-29700(DSP:11,BRAM:0), 13678-27387(DSP:16,BRAM:56), 14171-26337(DSP:0,BRAM:16), 15512-30459(DSP:0,BRAM:64), 21729-41256(DSP:1,BRAM:10), 18004-32111(DSP:0,BRAM:40), 19879-42289(DSP:16,BRAM:8), 25229-47482(DSP:0,BRAM:43), 32084-69294(DSP:3,BRAM:51),  <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Average cluster size is too large = 34890 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Placement Unit Clustering Done. (elapsed time: 43.758 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   SA-based Cluster Placement Start. (elapsed time: 43.758 s) <br/>
 98% [||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ] <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   SA optimization ratio (finalE/initE) = 0.995833 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   SA optimization initE = 4060813.872098 <br/>
 1(4.06081e+06) 2(4.06081e+06) 3(4.06081e+06) 4(4.06081e+06) 5(4.06081e+06) 6(4.06002e+06) 7(4.06081e+06) 8(4.06081e+06) 9(4.05909e+06) 10(4.06575e+06) <br/>
 11(4.06093e+06) 12(4.05985e+06) 13(4.06105e+06) 14(4.06448e+06) 15(4.06575e+06) 16(4.06144e+06) 17(4.07281e+06) 18(4.06436e+06) 19(4.06927e+06) 20(4.06958e+06) <br/>
 21(4.07197e+06) 22(4.07111e+06) 23(4.0684e+06) 24(4.06853e+06) 25(4.0674e+06) 26(4.064e+06) 27(4.06937e+06) 28(4.06705e+06) 29(4.06937e+06) 30(4.06644e+06) <br/>
 31(4.06937e+06) 32(4.06937e+06) 33(4.07683e+06) 34(4.07414e+06) 35(4.06906e+06) 36(4.06887e+06) 37(4.05465e+06) 38(4.05633e+06) 39(4.0634e+06) 40(4.05795e+06) <br/>
 41(4.07151e+06) 42(4.05675e+06) 43(4.06116e+06) 44(4.06855e+06) 45(4.07389e+06) 46(4.06218e+06) 47(4.07301e+06) 48(4.07389e+06) 49(4.06549e+06) 50(4.0634e+06) <br/>
 51(4.06419e+06) 52(4.0726e+06) 53(4.06219e+06) 54(4.06429e+06) 55(4.08356e+06) 56(4.07225e+06) 57(4.06519e+06) 58(4.06202e+06) 59(4.05992e+06) 60(4.0503e+06) <br/>
 61(4.06434e+06) 62(4.06709e+06) 63(4.06191e+06) 64(4.06709e+06) 65(4.07533e+06) 66(4.07533e+06) 67(4.07051e+06) 68(4.06227e+06) 69(4.05725e+06) 70(4.05946e+06) <br/>
 71(4.06921e+06) 72(4.07533e+06) 73(4.06376e+06) 74(4.06973e+06) 75(4.06814e+06) 76(4.06973e+06) 77(4.0601e+06) 78(4.06973e+06) 79(4.06973e+06) 80(4.06759e+06) <br/>
 81(4.05839e+06) 82(4.05839e+06) 83(4.05839e+06) 84(4.05839e+06) 85(4.05839e+06) 86(4.05839e+06) 87(4.05839e+06) 88(4.05839e+06) 89(4.04655e+06) 90(4.04655e+06) <br/>
 91(4.04655e+06) 92(4.04655e+06) 93(4.04655e+06) 94(4.04655e+06) 95(4.04655e+06) 96(4.04655e+06) 97(4.06507e+06) 98(4.05965e+06) 99(4.06017e+06) 100(4.06623e+06) <br/>
 101(4.07725e+06) 102(4.08447e+06) 103(4.08587e+06) 104(4.07951e+06) 105(4.06081e+06) 106(4.06081e+06) 107(4.06081e+06) 108(4.06081e+06) 109(4.06081e+06) 110(4.06081e+06) <br/>
 111(4.05389e+06) 112(4.06081e+06) 113(4.06575e+06) 114(4.06575e+06) 115(4.06575e+06) 116(4.06575e+06) 117(4.06575e+06) 118(4.06575e+06) 119(4.06575e+06) 120(4.05804e+06) <br/>
 121(4.07381e+06) 122(4.05628e+06) 123(4.07334e+06) 124(4.06773e+06) 125(4.06311e+06) 126(4.07475e+06) 127(4.07475e+06) 128(4.07431e+06) 129(4.06415e+06) 130(4.06937e+06) <br/>
 131(4.06673e+06) 132(4.06869e+06) 133(4.06526e+06) 134(4.05716e+06) 135(4.05726e+06) 136(4.05911e+06) 137(4.06542e+06) 138(4.06765e+06) 139(4.0638e+06) 140(4.06152e+06) <br/>
 141(4.05574e+06) 142(4.07381e+06) 143(4.06117e+06) 144(4.06721e+06) 145(4.07389e+06) 146(4.06437e+06) 147(4.07389e+06) 148(4.07389e+06) 149(4.07389e+06) 150(4.04389e+06) <br/>
 151(4.05009e+06) 152(4.06181e+06) 153(4.06577e+06) 154(4.08227e+06) 155(4.06165e+06) 156(4.08356e+06) 157(4.06521e+06) 158(4.06893e+06) 159(4.07147e+06) 160(4.06178e+06) <br/>
 161(4.06115e+06) 162(4.06709e+06) 163(4.06709e+06) 164(4.06152e+06) 165(4.06709e+06) 166(4.06709e+06) 167(4.05009e+06) 168(4.06709e+06) 169(4.07147e+06) 170(4.06147e+06) <br/>
 171(4.06815e+06) 172(4.05752e+06) 173(4.06916e+06) 174(4.07533e+06) 175(4.07214e+06) 176(4.07004e+06) 177(4.05797e+06) 178(4.06823e+06) 179(4.05985e+06) 180(4.06973e+06) <br/>
 181(4.06015e+06) 182(4.06578e+06) 183(4.06208e+06) 184(4.05461e+06) 185(4.05691e+06) 186(4.04491e+06) 187(4.05839e+06) 188(4.05839e+06) 189(4.05839e+06) 190(4.05839e+06) <br/>
 191(4.04958e+06) 192(4.05839e+06) 193(4.04655e+06) 194(4.04655e+06) 195(4.04655e+06) 196(4.04655e+06) 197(4.04655e+06) 198(4.04655e+06) 199(4.04655e+06) 200(4.04655e+06) <br/>
 201(4.07051e+06) 202(4.0657e+06) 203(4.06811e+06) 204(4.07623e+06) 205(4.06996e+06) 206(4.08638e+06) 207(4.0662e+06) 208(4.05544e+06) 209(4.06081e+06) 210(4.06081e+06) <br/>
 211(4.06081e+06) 212(4.06068e+06) 213(4.06081e+06) 214(4.06081e+06) 215(4.06081e+06) 216(4.06081e+06) 217(4.05805e+06) 218(4.06575e+06) 219(4.06575e+06) 220(4.05109e+06) <br/>
 221(4.06575e+06) 222(4.05467e+06) 223(4.06575e+06) 224(4.06575e+06) 225(4.06103e+06) 226(4.0647e+06) 227(4.06196e+06) 228(4.06381e+06) 229(4.06525e+06) 230(4.07475e+06) <br/>
 231(4.07475e+06) 232(4.07468e+06) 233(4.06937e+06) 234(4.05188e+06) 235(4.06293e+06) 236(4.06937e+06) 237(4.06937e+06) 238(4.06161e+06) 239(4.05937e+06) 240(4.06076e+06) <br/>
 241(4.07683e+06) 242(4.0629e+06) 243(4.06835e+06) 244(4.0668e+06) 245(4.06917e+06) 246(4.06397e+06) 247(4.07097e+06) 248(4.05745e+06) 249(4.05862e+06) 250(4.06615e+06) <br/>
 251(4.06895e+06) 252(4.07389e+06) 253(4.07389e+06) 254(4.07389e+06) 255(4.05292e+06) 256(4.06496e+06) 257(4.06339e+06) 258(4.05466e+06) 259(4.07212e+06) 260(4.06833e+06) <br/>
 261(4.06267e+06) 262(4.06605e+06) 263(4.06669e+06) 264(4.05475e+06) 265(4.06709e+06) 266(4.06065e+06) 267(4.06709e+06) 268(4.05588e+06) 269(4.06709e+06) 270(4.06709e+06) <br/>
 271(4.06709e+06) 272(4.06709e+06) 273(4.06277e+06) 274(4.05319e+06) 275(4.07533e+06) 276(4.07069e+06) 277(4.07533e+06) 278(4.07533e+06) 279(4.073e+06) 280(4.07533e+06) <br/>
 281(4.05339e+06) 282(4.06973e+06) 283(4.06973e+06) 284(4.06549e+06) 285(4.0505e+06) 286(4.06973e+06) 287(4.0472e+06) 288(4.06973e+06) 289(4.05677e+06) 290(4.05839e+06) <br/>
 291(4.05839e+06) 292(4.05337e+06) 293(4.05839e+06) 294(4.05839e+06) 295(4.05839e+06) 296(4.05839e+06) 297(4.04655e+06) 298(4.04655e+06) 299(4.04655e+06) 300(4.04655e+06) <br/>
 301(4.04655e+06) 302(4.04655e+06) 303(4.04655e+06) 304(4.04655e+06) 305(4.06655e+06) 306(4.07356e+06) 307(4.07978e+06) 308(4.0584e+06) 309(4.0827e+06) 310(4.07291e+06) <br/>
 311(4.0512e+06) 312(4.08386e+06) 313(4.06065e+06) 314(4.04856e+06) 315(4.06081e+06) 316(4.06081e+06) 317(4.06081e+06) 318(4.06081e+06) 319(4.06081e+06) 320(4.06081e+06) <br/>
 321(4.06575e+06) 322(4.06575e+06) 323(4.06575e+06) 324(4.06575e+06) 325(4.06575e+06) 326(4.06575e+06) 327(4.06575e+06) 328(4.06575e+06) 329(4.06603e+06) 330(4.06995e+06) <br/>
 331(4.06225e+06) 332(4.0733e+06) 333(4.07475e+06) 334(4.06545e+06) 335(4.06399e+06) 336(4.06298e+06) 337(4.06937e+06) 338(4.06861e+06) 339(4.06937e+06) 340(4.06868e+06) <br/>
 341(4.06521e+06) 342(4.06199e+06) 343(4.06238e+06) 344(4.06296e+06) 345(4.05848e+06) 346(4.07067e+06) 347(4.06264e+06) 348(4.06501e+06) 349(4.07683e+06) 350(4.05755e+06) <br/>
 351(4.06617e+06) 352(4.07057e+06) 353(4.07389e+06) 354(4.07235e+06) 355(4.07389e+06) 356(4.07389e+06) 357(4.06792e+06) 358(4.06256e+06) 359(4.06269e+06) 360(4.07389e+06) <br/>
 361(4.07077e+06) 362(4.07496e+06) 363(4.06822e+06) 364(4.073e+06) 365(4.05542e+06) 366(4.07425e+06) 367(4.07434e+06) 368(4.05918e+06) 369(4.06709e+06) 370(4.05945e+06) <br/>
 371(4.06709e+06) 372(4.06709e+06) 373(4.06709e+06) 374(4.06318e+06) 375(4.06709e+06) 376(4.05856e+06) 377(4.07017e+06) 378(4.06695e+06) 379(4.06158e+06) 380(4.07099e+06) <br/>
 381(4.07533e+06) 382(4.0738e+06) 383(4.06247e+06) 384(4.06889e+06) 385(4.06165e+06) 386(4.06839e+06) 387(4.05882e+06) 388(4.06968e+06) 389(4.06881e+06) 390(4.06845e+06) <br/>
 391(4.06973e+06) 392(4.06266e+06) 393(4.05839e+06) 394(4.05839e+06) 395(4.05839e+06) 396(4.05839e+06) 397(4.05839e+06) 398(4.05839e+06) 399(4.05839e+06) 400(4.05839e+06) <br/>
 401(4.04655e+06) 402(4.04655e+06) 403(4.04655e+06) 404(4.04655e+06) 405(4.04655e+06) 406(4.04655e+06) 407(4.04655e+06) 408(4.04655e+06) 409(4.06416e+06) 410(4.07988e+06) <br/>
 411(4.07318e+06) 412(4.07382e+06) 413(4.06312e+06) 414(4.09249e+06) 415(4.06832e+06) 416(4.05584e+06) 417(4.06081e+06) 418(4.06081e+06) 419(4.05413e+06) 420(4.06081e+06) <br/>
 421(4.06081e+06) 422(4.06081e+06) 423(4.06081e+06) 424(4.06081e+06) 425(4.06575e+06) 426(4.06575e+06) 427(4.06575e+06) 428(4.06575e+06) 429(4.06575e+06) 430(4.06575e+06) <br/>
 431(4.06124e+06) 432(4.06553e+06) 433(4.0732e+06) 434(4.07475e+06) 435(4.06307e+06) 436(4.07191e+06) 437(4.07475e+06) 438(4.06665e+06) 439(4.07475e+06) 440(4.07475e+06) <br/>
 441(4.06937e+06) 442(4.06937e+06) 443(4.06937e+06) 444(4.06937e+06) 445(4.06877e+06) 446(4.06937e+06) 447(4.06827e+06) 448(4.06379e+06) 449(4.05401e+06) 450(4.07473e+06) <br/>
 451(4.06759e+06) 452(4.07244e+06) 453(4.07236e+06) 454(4.05067e+06) 455(4.07126e+06) 456(4.06722e+06) 457(4.06345e+06) 458(4.06754e+06) 459(4.07389e+06) 460(4.05298e+06) <br/>
 461(4.07e+06) 462(4.07389e+06) 463(4.07152e+06) 464(4.07389e+06) 465(4.07349e+06) 466(4.06772e+06) 467(4.08208e+06) 468(4.07388e+06) 469(4.08356e+06) 470(4.06647e+06) <br/>
 471(4.06839e+06) 472(4.04809e+06) 473(4.06709e+06) 474(4.06673e+06) 475(4.06709e+06) 476(4.06709e+06) 477(4.06709e+06) 478(4.06709e+06) 479(4.05708e+06) 480(4.06709e+06) <br/>
 481(4.07439e+06) 482(4.0649e+06) 483(4.06148e+06) 484(4.05745e+06) 485(4.06184e+06) 486(4.05592e+06) 487(4.0743e+06) 488(4.07533e+06) 489(4.069e+06) 490(4.06973e+06) <br/>
 491(4.06973e+06) 492(4.06723e+06) 493(4.06973e+06) 494(4.05633e+06) 495(4.06973e+06) 496(4.05746e+06) 497(4.05839e+06) 498(4.05839e+06) 499(4.05839e+06) 500(4.05839e+06) <br/>
 501(4.05236e+06) 502(4.05839e+06) 503(4.05839e+06) 504(4.05745e+06) 505(4.04655e+06) 506(4.04655e+06) 507(4.04655e+06) 508(4.04655e+06) 509(4.04655e+06) 510(4.04655e+06) <br/>
 511(4.04655e+06) 512(4.04655e+06) 513(4.07253e+06) 514(4.07643e+06) 515(4.07612e+06) 516(4.0703e+06) 517(4.07352e+06) 518(4.07101e+06) 519(4.08018e+06) 520(4.06325e+06) <br/>
 521(4.05902e+06) 522(4.05859e+06) 523(4.06081e+06) 524(4.06081e+06) 525(4.06081e+06) 526(4.06081e+06) 527(4.06081e+06) 528(4.05833e+06) 529(4.06575e+06) 530(4.05311e+06) <br/>
 531(4.05949e+06) 532(4.05973e+06) 533(4.06418e+06) 534(4.06575e+06) 535(4.05682e+06) 536(4.06575e+06) 537(4.07005e+06) 538(4.05968e+06) 539(4.07475e+06) 540(4.06998e+06) <br/>
 541(4.06946e+06) 542(4.04694e+06) 543(4.07475e+06) 544(4.06372e+06) 545(4.06937e+06) 546(4.06937e+06) 547(4.06305e+06) 548(4.06774e+06) 549(4.06419e+06) 550(4.06937e+06) <br/>
 551(4.06937e+06) 552(4.05893e+06) 553(4.05903e+06) 554(4.062e+06) 555(4.0658e+06) 556(4.07683e+06) 557(4.06091e+06) 558(4.07683e+06) 559(4.07127e+06) 560(4.0702e+06) <br/>
 561(4.07389e+06) 562(4.07389e+06) 563(4.06842e+06) 564(4.06881e+06) 565(4.07389e+06) 566(4.06089e+06) 567(4.07389e+06) 568(4.07048e+06) 569(4.08356e+06) 570(4.06154e+06) <br/>
 571(4.07715e+06) 572(4.07666e+06) 573(4.05888e+06) 574(4.07022e+06) 575(4.06954e+06) 576(4.07244e+06) 577(4.06164e+06) 578(4.05195e+06) 579(4.06709e+06) 580(4.06709e+06) <br/>
 581(4.06192e+06) 582(4.06709e+06) 583(4.05283e+06) 584(4.05225e+06) 585(4.07533e+06) 586(4.06012e+06) 587(4.05582e+06) 588(4.069e+06) 589(4.05811e+06) 590(4.07533e+06) <br/>
 591(4.07133e+06) 592(4.05598e+06) 593(4.06816e+06) 594(4.06434e+06) 595(4.06099e+06) 596(4.06532e+06) 597(4.06973e+06) 598(4.06835e+06) 599(4.06301e+06) 600(4.06316e+06) <br/>
 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   SA final occupied region num = 13 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   SAPlace handle 13 cluster(s) and the final placement cost is 4043890.524981 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   SA-based Cluster Placement Done. (elapsed time: 91.498 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   dumping cluster information to ../../../Documents/placerDumpData//./dumpClusters (elapsed time: 91.498 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   1&emsp;   0&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   7&emsp;   1&emsp;    <br/>
0&emsp;   2&emsp;   3&emsp;   12&emsp; 2&emsp;    <br/>
0&emsp;   2&emsp;   4&emsp;   5&emsp;   2&emsp;    <br/>
0&emsp;   1&emsp;   2&emsp;   4&emsp;   2&emsp;    <br/>
0&emsp;   1&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   1&emsp;   0&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   5&emsp;   0&emsp;    <br/>
0&emsp;   2&emsp;   3&emsp;   11&emsp;  0&emsp;    <br/>
0&emsp;   0&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   3&emsp;   3&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Cluster Placement Done. (elapsed time: 92.028 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacement Total HPWL = 610080.855020 <br/>
<hr>

**9. Global placement iterations:** Now AMFPlacer will start the global placement iterations (wirelength optimization, cell spreading, legalization, area adjustion...). For each iteration, the placer will print information indicating the convergence progress, e.g., upper-bound HPWL, lower-bound HPWL, legalization displacement, overall pseudo net weights, and clock utilization. Meanwhile, for this placement, we let AMFPlacer to dump the locations of the elements to files for each iteration. This can be disabled in configuration file. The information can be visualized with Python script in "./benchmarks/analysisScripts". We strongly suggest users disable this behavior since disk IOs (element locations and compression) will significantly slow down the placement procedure.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_fixedCLB started (elapsed time: 92.046 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-0.gz (elapsed time: 92.046 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-0.gz (elapsed time: 92.799 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   addPseudoNetForMacros is disabled according to placer configuration. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-1.gz (elapsed time: 93.337 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-1.gz (elapsed time: 94.097 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#0 Done (elapsed time: 94.099 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-2.gz (elapsed time: 94.278 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-2.gz (elapsed time: 95.049 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_CLBElements started (elapsed time: 95.051 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.000125 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.000250 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=257292.656250 pseudoNetWeight=0.002500 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  0 Done HPWL=257292.656250 (elapsed time: 95.551 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-3.gz (elapsed time: 95.551 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-3.gz (elapsed time: 96.632 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 96.799 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 96.855 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 96.950 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 100.294 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 101.419 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  0 Done HPWL=2608110.250000 (elapsed time: 101.438 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-4.gz (elapsed time: 101.438 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-4.gz (elapsed time: 102.457 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=2608110.250000 pseudoNetWeight=0.002500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   10  11  6&emsp;    <br/>
4&emsp;   4&emsp;   7&emsp;   5&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   3&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=2608110.250000 progressRatio=0.249150 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=10.136745 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.249150 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=2608110.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=2608110.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.000517 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.000689 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=249127.468750 pseudoNetWeight=0.003445 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  1 Done HPWL=249127.468750 (elapsed time: 103.332 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-5.gz (elapsed time: 103.332 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-5.gz (elapsed time: 104.406 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 104.568 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 104.577 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 104.590 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 105.352 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 105.524 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  1 Done HPWL=2360929.000000 (elapsed time: 105.543 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-6.gz (elapsed time: 105.543 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-6.gz (elapsed time: 106.700 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=2360929.000000 pseudoNetWeight=0.003445 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   8&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   12  7&emsp;    <br/>
4&emsp;   4&emsp;   5&emsp;   5&emsp;   4&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   5&emsp;   5&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=2360929.000000 progressRatio=0.259420 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=9.476791 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.259420 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=2360929.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=2360929.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.001182 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 61147 PU(s) and 95461 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.001419 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 88262 PU(s) and 164046 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=272468.062500 pseudoNetWeight=0.004729 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  2 Done HPWL=272468.062500 (elapsed time: 107.705 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-7.gz (elapsed time: 107.705 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-7.gz (elapsed time: 108.748 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 108.923 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 108.936 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 108.949 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 109.559 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 109.671 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  2 Done HPWL=1890665.250000 (elapsed time: 109.691 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-8.gz (elapsed time: 109.691 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-8.gz (elapsed time: 110.726 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1890665.250000 pseudoNetWeight=0.004729 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   8&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   9&emsp;   12  7&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   4&emsp;    <br/>
3&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
4&emsp;   4&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1890665.250000 progressRatio=0.312767 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=6.939034 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.312767 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1890665.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1890665.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.002229 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 62223 PU(s) and 97664 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.002548 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 84637 PU(s) and 157401 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=261564.593750 pseudoNetWeight=0.006369 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  3 Done HPWL=261564.593750 (elapsed time: 111.652 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-9.gz (elapsed time: 111.652 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-9.gz (elapsed time: 112.708 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 112.892 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 112.901 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 112.912 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 113.460 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 113.590 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  3 Done HPWL=1636363.875000 (elapsed time: 113.607 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-10.gz (elapsed time: 113.607 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-10.gz (elapsed time: 114.642 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1636363.875000 pseudoNetWeight=0.006369 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   12  5&emsp;    <br/>
3&emsp;   5&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   2&emsp;   3&emsp;   2&emsp;    <br/>
1&emsp;   3&emsp;   2&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1636363.875000 progressRatio=0.332828 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=6.256060 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.332828 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1636363.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1636363.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.003832 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 62510 PU(s) and 100632 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.004258 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 82638 PU(s) and 153652 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=261481.296875 pseudoNetWeight=0.008515 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  4 Done HPWL=261481.296875 (elapsed time: 115.551 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-11.gz (elapsed time: 115.551 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-11.gz (elapsed time: 116.642 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 116.830 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 116.841 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 116.856 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 311 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 44 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 117.293 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 117.387 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  4 Done HPWL=1784235.250000 (elapsed time: 117.407 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-12.gz (elapsed time: 117.407 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-12.gz (elapsed time: 118.425 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1784235.250000 pseudoNetWeight=0.008515 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  5&emsp;    <br/>
4&emsp;   5&emsp;   6&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   4&emsp;    <br/>
3&emsp;   4&emsp;   4&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1636363.875000 progressRatio=0.315932 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=6.823567 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.315932 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1784235.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1636363.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.006300 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 63357 PU(s) and 103364 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.006873 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 83486 PU(s) and 153490 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=276537.531250 pseudoNetWeight=0.011454 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  5 Done HPWL=276537.531250 (elapsed time: 119.383 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-13.gz (elapsed time: 119.383 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-13.gz (elapsed time: 120.386 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 120.571 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 120.580 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 120.592 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 339 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 47 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 121.139 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 121.258 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  5 Done HPWL=1709143.125000 (elapsed time: 121.276 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-14.gz (elapsed time: 121.276 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-14.gz (elapsed time: 122.276 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1709143.125000 pseudoNetWeight=0.011454 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
4&emsp;   5&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
3&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
3&emsp;   2&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
3&emsp;   2&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=351234.000000 minHPWL=1636363.875000 err/minHPWL=0.214643 progressRatio=0.335263 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=6.180511 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.335263 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1709143.125000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1636363.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.009945 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 62302 PU(s) and 100119 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.010710 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 79337 PU(s) and 145723 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=318871.062500 pseudoNetWeight=0.015300 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  6 Done HPWL=318871.062500 (elapsed time: 123.129 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-15.gz (elapsed time: 123.129 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-15.gz (elapsed time: 124.141 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 124.333 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 124.345 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 124.356 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 124.821 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 124.888 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  6 Done HPWL=1273336.875000 (elapsed time: 124.908 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-16.gz (elapsed time: 124.908 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-16.gz (elapsed time: 125.911 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1273336.875000 pseudoNetWeight=0.015300 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   12  4&emsp;    <br/>
4&emsp;   5&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   6&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
3&emsp;   2&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=439050.593750 minHPWL=1273336.875000 err/minHPWL=0.344803 progressRatio=0.435716 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=3.993266 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.435716 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1273336.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1273336.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.014762 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 65555 PU(s) and 107306 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.015747 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 83084 PU(s) and 149086 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=312214.093750 pseudoNetWeight=0.019683 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  7 Done HPWL=312214.093750 (elapsed time: 126.866 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-17.gz (elapsed time: 126.866 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-17.gz (elapsed time: 127.900 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 128.087 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 128.096 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 128.109 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 128.526 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 128.612 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 128.613 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 128.706 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  7 Done HPWL=1246360.375000 (elapsed time: 128.728 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-18.gz (elapsed time: 128.728 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-18.gz (elapsed time: 129.783 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1246360.375000 pseudoNetWeight=0.019683 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   12  4&emsp;    <br/>
3&emsp;   5&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   5&emsp;   1&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=362300.343750 minHPWL=1246360.375000 err/minHPWL=0.290687 progressRatio=0.435798 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=3.992006 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=100000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.435798 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1246360.375000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1246360.375000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.021523 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 68127 PU(s) and 116800 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.022789 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 79663 PU(s) and 143901 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=316512.218750 pseudoNetWeight=0.025322 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  8 Done HPWL=316512.218750 (elapsed time: 130.673 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-19.gz (elapsed time: 130.673 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-19.gz (elapsed time: 131.683 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 131.868 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 131.875 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 131.887 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 132.522 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 132.664 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 132.665 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 132.771 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  8 Done HPWL=1487031.000000 (elapsed time: 132.789 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-20.gz (elapsed time: 132.789 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-20.gz (elapsed time: 133.816 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =10000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =10000.000000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  8 Done (elapsed time: 134.450 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1487031.000000 pseudoNetWeight=0.025322 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   11  7&emsp;    <br/>
3&emsp;   5&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=335299.093750 minHPWL=1246360.375000 err/minHPWL=0.269023 progressRatio=0.395223 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=4.698179 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=11.451761 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.395223 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1487031.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1246360.375000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.031425 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 61753 PU(s) and 103184 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.033079 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 76039 PU(s) and 134784 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=395727.093750 pseudoNetWeight=0.033079 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  9 Done HPWL=395727.093750 (elapsed time: 135.331 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-21.gz (elapsed time: 135.331 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-21.gz (elapsed time: 136.431 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 136.642 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 136.647 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 136.653 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 158 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 38 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 137.079 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 137.144 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  9 Done HPWL=1021982.625000 (elapsed time: 137.163 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-22.gz (elapsed time: 137.163 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-22.gz (elapsed time: 138.186 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =11.451761 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =4.416123 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  9 Done (elapsed time: 138.783 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1021982.625000 pseudoNetWeight=0.033079 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   9&emsp;   6&emsp;    <br/>
3&emsp;   5&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   6&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=400495.656250 minHPWL=1021982.625000 err/minHPWL=0.391881 progressRatio=0.565941 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.582544 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=9.013987 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.565941 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1021982.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-23.gz (elapsed time: 138.845 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-23.gz (elapsed time: 139.877 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: FinalLUTFF-0.gz (elapsed time: 139.878 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: FinalLUTFF-0.gz (elapsed time: 140.886 s) <br/>
<hr>

**10. Re-initialize the bin grid:** During the global placement, AMFPlacer will re-initialize the bin grid and make the bins smaller so the bin grid will be more fine-grained to conduct detailed cell spreading to resolve resource overflow in local regions.
<hr>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (ILKN_SITE) is not mapped to bin grid. e.g. [ILKN_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HPIOBDIFFOUTBUF) is not mapped to bin grid. e.g. [HPIOBDIFFOUTBUF_X0Y11]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (PLL_SELECT_SITE) is not mapped to bin grid. e.g. [PLL_SELECT_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (XIPHY_FEEDTHROUGH) is not mapped to bin grid. e.g. [XIPHY_FEEDTHROUGH_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HRIODIFFOUTBUF) is not mapped to bin grid. e.g. [HRIODIFFOUTBUF_X0Y11]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HRIODIFFINBUF) is not mapped to bin grid. e.g. [HRIODIFFINBUF_X0Y3]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (BUFCE_ROW) is not mapped to bin grid. e.g. [BUFCE_ROW_X69Y3]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (RAMBFIFO36) is not mapped to bin grid. e.g. [RAMB36_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (GTYE3_CHANNEL) is not mapped to bin grid. e.g. [GTYE3_CHANNEL_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (HARD_SYNC) is not mapped to bin grid. e.g. [HARD_SYNC_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (BUFCE_LEAF_X16) is not mapped to bin grid. e.g. [BUFCE_LEAF_X16_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Site Type (CMAC_SITE) is not mapped to bin grid. e.g. [CMAC_SITE_X0Y0]. It might be not critical if the design will not utilize this kind of sites. Please check the compatible table you defined. <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Bin Grid Size: Y: 240 X:44 (elapsed time: 141.157 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Bin Grid for Density Control Created (elapsed time: 141.157 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 141.160 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 141.277 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 25 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 141.405 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 141.418 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 141.424 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 152 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 2748 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 154 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 147.790 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 29 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 834 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 66 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 148.254 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 148.254 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 148.380 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_CLBElements started (elapsed time: 148.380 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.042467 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 148.742 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 35080 PU(s) and 50227 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.044489 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 149.293 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 61515 PU(s) and 102131 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=538756.375000 pseudoNetWeight=0.040445 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  0 Done HPWL=538756.375000 (elapsed time: 149.628 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-24.gz (elapsed time: 149.628 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-24.gz (elapsed time: 150.695 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 10 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 20 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 42 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 150.856 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 150.894 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 13 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 43 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 150.952 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 43 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1065 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 126 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 154.231 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 9 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 238 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 65 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 154.895 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 154.896 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 155.024 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  0 Done HPWL=1116374.500000 (elapsed time: 155.041 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-25.gz (elapsed time: 155.041 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-25.gz (elapsed time: 156.087 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =9.013987 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.490334 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  0 Done (elapsed time: 156.572 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1116374.500000 pseudoNetWeight=0.040445 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   9&emsp;   10  5&emsp;    <br/>
4&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
4&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
4&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   5&emsp;   5&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=260270.687500 minHPWL=1021982.625000 err/minHPWL=0.254672 progressRatio=0.645877 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.072132 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=8.520712 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.645877 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1116374.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.055048 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 156.912 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.057441 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 157.437 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=455682.250000 pseudoNetWeight=0.047867 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  1 Done HPWL=455682.250000 (elapsed time: 157.731 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-26.gz (elapsed time: 157.731 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-26.gz (elapsed time: 158.797 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 9 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 36 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 46 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 158.992 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 159.001 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 159.010 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 23 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 650 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 135 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 159.750 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 32 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 29 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 159.859 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 159.860 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 159.994 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  1 Done HPWL=1121828.500000 (elapsed time: 160.011 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-27.gz (elapsed time: 160.011 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-27.gz (elapsed time: 161.084 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =8.520712 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.179929 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  1 Done (elapsed time: 161.560 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1121828.500000 pseudoNetWeight=0.047867 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   9&emsp;   10  5&emsp;    <br/>
4&emsp;   4&emsp;   5&emsp;   7&emsp;   4&emsp;    <br/>
4&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
4&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   4&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=238953.718750 minHPWL=1021982.625000 err/minHPWL=0.233814 progressRatio=0.582427 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.461866 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.469617 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.582427 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1121828.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.054507 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 161.895 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 57867 PU(s) and 88553 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.056687 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 162.466 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 73183 PU(s) and 125096 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=433150.843750 pseudoNetWeight=0.043605 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  2 Done HPWL=433150.843750 (elapsed time: 162.794 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-28.gz (elapsed time: 162.794 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-28.gz (elapsed time: 163.821 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 16 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 56 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 33 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 164.033 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 164.042 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 164.055 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 26 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 990 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 59 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 164.826 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 165.210 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 165.211 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 165.359 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  2 Done HPWL=1060927.875000 (elapsed time: 165.376 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-29.gz (elapsed time: 165.376 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-29.gz (elapsed time: 166.446 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.469617 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.114875 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  2 Done (elapsed time: 166.963 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1060927.875000 pseudoNetWeight=0.043605 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   10  5&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   4&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   4&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
1&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 progressRatio=0.584214 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.449327 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=8.047653 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.584214 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1060927.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.071449 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 167.336 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 60549 PU(s) and 93810 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.074095 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 167.928 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 75909 PU(s) and 133845 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=436603.062500 pseudoNetWeight=0.052925 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  3 Done HPWL=436603.062500 (elapsed time: 168.253 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-30.gz (elapsed time: 168.253 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-30.gz (elapsed time: 169.288 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 10 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 36 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 50 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 169.493 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 169.503 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 169.515 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 22 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 551 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 68 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 170.254 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 16 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 31 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 170.371 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 170.371 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 170.514 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  3 Done HPWL=1081516.875000 (elapsed time: 170.533 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-31.gz (elapsed time: 170.533 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-31.gz (elapsed time: 171.593 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =8.047653 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.079425 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  3 Done (elapsed time: 172.092 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1081516.875000 pseudoNetWeight=0.052925 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   8&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 progressRatio=0.580273 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.477117 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.713297 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.580273 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1081516.875000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=1021982.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.093292 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 172.429 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 60035 PU(s) and 93280 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.096509 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 172.985 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 75451 PU(s) and 130120 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=445875.062500 pseudoNetWeight=0.064339 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  4 Done HPWL=445875.062500 (elapsed time: 173.313 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-32.gz (elapsed time: 173.313 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-32.gz (elapsed time: 174.361 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 11 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 36 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 35 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 174.560 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 174.568 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 174.582 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 175.956 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 176.097 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 176.098 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 176.290 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  4 Done HPWL=976644.187500 (elapsed time: 176.321 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-33.gz (elapsed time: 176.321 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-33.gz (elapsed time: 177.605 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.713297 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.231104 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  4 Done (elapsed time: 178.173 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=976644.187500 pseudoNetWeight=0.064339 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   12  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   5&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.624721 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.190399 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.289489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.624721 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.119062 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 178.601 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.122902 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 179.160 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=451468.781250 pseudoNetWeight=0.076814 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  5 Done HPWL=451468.781250 (elapsed time: 179.460 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-34.gz (elapsed time: 179.460 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-34.gz (elapsed time: 180.514 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 37 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 180.717 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 180.731 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 180.743 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 181.746 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 181.843 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 181.844 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 181.991 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  5 Done HPWL=986875.000000 (elapsed time: 182.009 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-35.gz (elapsed time: 182.009 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-35.gz (elapsed time: 183.041 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.289489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.077697 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  5 Done (elapsed time: 183.548 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=986875.000000 pseudoNetWeight=0.076814 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.625489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.185921 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.013724 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.625489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=986875.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-36.gz (elapsed time: 183.610 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-36.gz (elapsed time: 184.654 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: FinalLUTFF-1.gz (elapsed time: 184.655 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: FinalLUTFF-1.gz (elapsed time: 185.662 s) <br/>
<hr>

**11. Map elements in long timing paths into clock regions:** Since cross-clock-region routing will lead to high delay, we set extra pseudonets to force the elements in the long paths placed in clock regions.
<hr>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   PlacementTimingOptimizer: clustering long path in one clock region <br/>
maxClockRegionWeight: 1408 totalClockRegionWeight:2261 #extractedCellIds=2261 #extractedPUs=665 pathLength=54 <br/>
maxClockRegionWeight: 1370 totalClockRegionWeight:2254 #extractedCellIds=4515 #extractedPUs=1324 pathLength=54 <br/>
maxClockRegionWeight: 4777 totalClockRegionWeight:15521 #extractedCellIds=20036 #extractedPUs=9632 pathLength=44 <br/>
maxClockRegionWeight: 5066 totalClockRegionWeight:15520 #extractedCellIds=35556 #extractedPUs=17939 pathLength=44 <br/>
maxClockRegionWeight: 1213 totalClockRegionWeight:1689 #extractedCellIds=37245 #extractedPUs=19176 pathLength=40 <br/>
maxClockRegionWeight: 1716 totalClockRegionWeight:2114 #extractedCellIds=39359 #extractedPUs=20566 pathLength=40 <br/>
maxClockRegionWeight: 1389 totalClockRegionWeight:1692 #extractedCellIds=41051 #extractedPUs=21806 pathLength=40 <br/>
maxClockRegionWeight: 2071 totalClockRegionWeight:2114 #extractedCellIds=43165 #extractedPUs=23196 pathLength=40 <br/>
maxClockRegionWeight: 815 totalClockRegionWeight:815 #extractedCellIds=43980 #extractedPUs=23491 pathLength=39 <br/>
maxClockRegionWeight: 815 totalClockRegionWeight:815 #extractedCellIds=44795 #extractedPUs=23786 pathLength=38 <br/>
maxClockRegionWeight: 3532 totalClockRegionWeight:7152 #extractedCellIds=51947 #extractedPUs=29178 pathLength=36 <br/>
maxClockRegionWeight: 2526 totalClockRegionWeight:5232 #extractedCellIds=57179 #extractedPUs=32825 pathLength=35 <br/>
maxClockRegionWeight: 335 totalClockRegionWeight:623 #extractedCellIds=57802 #extractedPUs=33114 pathLength=28 <br/>
maxClockRegionWeight: 607 totalClockRegionWeight:623 #extractedCellIds=58425 #extractedPUs=33403 pathLength=28 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   ClusterPlacer: largest long-path cluster size=0 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingOptimizer: enhanceNetWeight_LevelBased starts. (elapsed time: 185.764 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementTimingOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=5.763714) (elapsed time: 185.902 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Current Total HPWL = 1582260.575034 <br/>
<hr>



**12. Incremental LUT/FF packing:** AMFPlacer will pack some LUTs/FFs which are close to each other in the middle of the placement procedure, which can reduce the problem size and facilitate convergence and final packing.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Pairing LUTs and FFs. (elapsed time: 185.919 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   IncrementalBELPacker: LUTTO1FFPackedCnt=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   IncrementalBELPacker: LUTTOMultiFFPackedCnt=5306 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Updating Cell-PlacementUnit Mapping (elapsed time: 185.984 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Loading Nets (elapsed time: 186.202 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=110625 (elapsed time: 187.174 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Paired LUTs and FFs (#Pairs = 5306) (elapsed time: 187.404 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker: dumping LUTFFPair archieve to: ../../../Documents/placerDumpData//./DumpLUTFFPair-0.gz (elapsed time: 187.404 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker: dumped LUTFFPair archieve to: ../../../Documents/placerDumpData//./DumpLUTFFPair-0.gz (elapsed time: 187.417 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Pairing FFs. (elapsed time: 187.472 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   IncrementalBELPacker: 2FFPairedCnt=26111 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   IncrementalBELPacker: FFCouldBePackedCnt=55411 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Updating Cell-PlacementUnit Mapping (elapsed time: 187.717 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Loading Nets (elapsed time: 187.940 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=84514 (elapsed time: 188.886 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker Paired LUTs and FFs (#Pairs = 5306) (elapsed time: 189.101 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker: dumping LUTFFPair archieve to: ../../../Documents/placerDumpData//./DumpLUTFFPair-1.gz (elapsed time: 189.101 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   IncrementalBELPacker: dumped LUTFFPair archieve to: ../../../Documents/placerDumpData//./DumpLUTFFPair-1.gz (elapsed time: 189.112 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinX: -0.400000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinY: 0.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxX: 86.400002 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxY: 479.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Placement Unit(s): 187337 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Unpacked Placement Unit(s): 99752 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Macro Placement Unit(s): 87585 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Current Total HPWL = 1567816.286928 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_CLBElements started (elapsed time: 189.188 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.128579 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 189.560 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.132475 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 190.041 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=536745.437500 pseudoNetWeight=0.077926 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  0 Done HPWL=536745.437500 (elapsed time: 190.302 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-37.gz (elapsed time: 190.302 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-37.gz (elapsed time: 191.328 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 14 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 28 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 45 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 191.537 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 191.578 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 191.626 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 32 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1303 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 119 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 196.489 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 57 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 70 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 197.547 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 197.547 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 197.547 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 200.091 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 200.092 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 200.092 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 200.256 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  0 Done HPWL=1112794.625000 (elapsed time: 200.274 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-38.gz (elapsed time: 200.274 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-38.gz (elapsed time: 201.293 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.013724 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.967719 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  0 Done (elapsed time: 201.839 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1112794.625000 pseudoNetWeight=0.077926 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   5&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   3&emsp;   2&emsp;   2&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=85724.710938 minHPWL=976644.187500 err/minHPWL=0.087775 progressRatio=0.645672 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.073226 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.580754 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.645672 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1112794.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.121059 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 202.188 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.124518 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 202.684 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=438415.375000 pseudoNetWeight=0.069177 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  1 Done HPWL=438415.375000 (elapsed time: 202.939 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-39.gz (elapsed time: 202.939 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-39.gz (elapsed time: 203.971 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 45 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 204.182 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 204.192 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 204.205 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 63 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1613 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 89 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 206.412 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 19 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 405 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 46 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 206.666 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 206.666 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 206.666 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 206.666 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 206.821 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  1 Done HPWL=1720225.000000 (elapsed time: 206.840 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-40.gz (elapsed time: 206.840 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-40.gz (elapsed time: 207.956 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.580754 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.771661 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  1 Done (elapsed time: 208.545 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1720225.000000 pseudoNetWeight=0.069177 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   12  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
3&emsp;   5&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
3&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
3&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.440332 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=3.923733 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=13.152198 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.440332 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1720225.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.164352 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 35588 PU(s) and 71780 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.168794 <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 51931 PU(s) and 104640 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=564341.187500 pseudoNetWeight=0.088839 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  2 Done HPWL=564341.187500 (elapsed time: 209.404 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-41.gz (elapsed time: 209.404 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-41.gz (elapsed time: 210.412 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 24 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 30 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 210.616 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 210.623 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 210.633 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 31 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 911 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 72 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 211.632 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 231 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 40 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 211.845 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 211.845 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 211.845 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 214.180 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 214.180 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 214.180 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 214.344 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  2 Done HPWL=1200417.000000 (elapsed time: 214.363 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-42.gz (elapsed time: 214.363 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-42.gz (elapsed time: 215.408 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =13.152198 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.580328 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  2 Done (elapsed time: 215.986 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1200417.000000 pseudoNetWeight=0.088839 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   7&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   12  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.635808 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.127112 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=6.804361 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.635808 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1200417.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.205883 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 216.343 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.211162 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 216.828 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=541295.250000 pseudoNetWeight=0.105581 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  3 Done HPWL=541295.250000 (elapsed time: 217.151 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-43.gz (elapsed time: 217.151 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-43.gz (elapsed time: 218.218 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 18 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 51 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 218.426 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 218.434 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 218.447 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 38 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1021 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 82 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 219.523 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 149 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 55 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 219.744 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 219.744 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 219.744 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 222.103 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 222.103 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 222.103 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 222.262 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  3 Done HPWL=1279700.000000 (elapsed time: 222.281 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-44.gz (elapsed time: 222.281 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-44.gz (elapsed time: 223.279 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =6.804361 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.081686 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  3 Done (elapsed time: 223.835 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1279700.000000 pseudoNetWeight=0.105581 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   5&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   10  5&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   4&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.596754 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=2.364144 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=7.129639 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.596754 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1279700.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.274441 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 224.191 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 39950 PU(s) and 79071 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.295192 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 224.765 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   set user-defined cluster pseudo net for 54954 PU(s) and 113455 cell(s). <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=567342.562500 pseudoNetWeight=0.127499 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  4 Done HPWL=567342.562500 (elapsed time: 225.094 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-45.gz (elapsed time: 225.094 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-45.gz (elapsed time: 226.069 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 46 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 226.283 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 226.290 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 226.299 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 31 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 942 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 58 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 227.172 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 42 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 38 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 227.370 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 227.370 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 227.370 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 227.370 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 227.534 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  4 Done HPWL=1093326.125000 (elapsed time: 227.555 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-46.gz (elapsed time: 227.555 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-46.gz (elapsed time: 228.598 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =7.129639 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =2.036791 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  4 Done (elapsed time: 229.169 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=1093326.125000 pseudoNetWeight=0.127499 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   4&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 progressRatio=0.674617 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.927101 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=5.794473 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.674617 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=1093326.125000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=976644.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.371099 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 229.521 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.398715 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 230.050 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=565260.250000 pseudoNetWeight=0.149102 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  5 Done HPWL=565260.250000 (elapsed time: 230.292 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-47.gz (elapsed time: 230.292 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-47.gz (elapsed time: 231.284 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 7 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 14 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 29 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 231.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 231.492 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 231.508 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 51 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1688 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 76 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 232.773 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 206 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 58 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 233.072 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 233.072 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 233.073 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 233.073 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 233.231 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  5 Done HPWL=967227.062500 (elapsed time: 233.252 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-48.gz (elapsed time: 233.252 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-48.gz (elapsed time: 234.244 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =5.794473 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.621706 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  5 Done (elapsed time: 234.810 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=967227.062500 pseudoNetWeight=0.149102 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=383385.906250 minHPWL=967227.062500 err/minHPWL=0.396376 progressRatio=0.724489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.711118 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=5.478625 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.724489 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=967227.062500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=967227.062500 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-49.gz (elapsed time: 234.889 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-49.gz (elapsed time: 235.864 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: FinalLUTFF-2.gz (elapsed time: 235.866 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: FinalLUTFF-2.gz (elapsed time: 236.814 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_CLBElements started (elapsed time: 236.816 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.490250 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 237.235 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.526202 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 237.753 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=562744.500000 pseudoNetWeight=0.170722 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  0 Done HPWL=562744.500000 (elapsed time: 238.011 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-50.gz (elapsed time: 238.011 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-50.gz (elapsed time: 239.032 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 7 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 15 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 27 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 239.233 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 239.256 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 239.291 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 50 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1812 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 112 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 242.264 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 16 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 349 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 86 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 242.937 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 242.937 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 242.937 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 244.438 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 244.438 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 244.438 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 244.599 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  0 Done HPWL=879305.625000 (elapsed time: 244.619 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-51.gz (elapsed time: 244.619 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-51.gz (elapsed time: 245.628 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =5.478625 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.492386 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  0 Done (elapsed time: 246.255 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=879305.625000 pseudoNetWeight=0.170722 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=251778.921875 minHPWL=879305.625000 err/minHPWL=0.286338 progressRatio=0.765073 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.562531 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=4.925534 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.765073 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=879305.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=879305.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.635153 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 246.605 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.681100 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 247.110 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=562356.187500 pseudoNetWeight=0.192081 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  1 Done HPWL=562356.187500 (elapsed time: 247.378 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-52.gz (elapsed time: 247.378 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-52.gz (elapsed time: 248.437 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 5 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 11 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 52 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 248.642 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 248.650 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 248.664 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 250.390 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 137 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 61 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 250.832 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 250.832 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 250.833 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 250.833 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 250.991 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  1 Done HPWL=911492.000000 (elapsed time: 251.010 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-53.gz (elapsed time: 251.010 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-53.gz (elapsed time: 252.025 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =4.925534 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.358904 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  1 Done (elapsed time: 252.613 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=911492.000000 pseudoNetWeight=0.192081 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=207309.703125 minHPWL=879305.625000 err/minHPWL=0.235765 progressRatio=0.748437 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.620845 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=4.582246 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.748437 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=911492.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=879305.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.827346 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 252.963 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=0.886442 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 253.425 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=577926.750000 pseudoNetWeight=0.217679 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  2 Done HPWL=577926.750000 (elapsed time: 253.647 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-54.gz (elapsed time: 253.647 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-54.gz (elapsed time: 254.632 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 31 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 254.834 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 254.842 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 254.859 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 29 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 971 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 67 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 256.003 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 9 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 217 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 54 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 256.366 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 256.366 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 256.366 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 256.366 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 256.520 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  2 Done HPWL=880300.812500 (elapsed time: 256.540 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-55.gz (elapsed time: 256.540 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-55.gz (elapsed time: 257.607 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =4.582246 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.454935 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  2 Done (elapsed time: 258.159 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=880300.812500 pseudoNetWeight=0.217679 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
3&emsp;   3&emsp;   7&emsp;   10  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=104471.921875 minHPWL=879305.625000 err/minHPWL=0.118812 progressRatio=0.776864 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.523205 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=4.225908 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.776864 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=880300.812500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=879305.625000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.062675 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 258.514 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.137687 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 258.991 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=584544.750000 pseudoNetWeight=0.243656 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  3 Done HPWL=584544.750000 (elapsed time: 259.237 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-56.gz (elapsed time: 259.237 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-56.gz (elapsed time: 260.225 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 12 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 28 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 260.426 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 260.437 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 260.450 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 42 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1256 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 79 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 261.737 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 367 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 63 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 262.113 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 262.113 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 262.113 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 263.660 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 263.660 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 263.660 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 263.821 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  3 Done HPWL=868701.437500 (elapsed time: 263.841 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-57.gz (elapsed time: 263.841 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-57.gz (elapsed time: 264.823 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =4.225908 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.314408 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  3 Done (elapsed time: 265.361 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=868701.437500 pseudoNetWeight=0.243656 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   10  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=48549.613281 minHPWL=868701.437500 err/minHPWL=0.055888 progressRatio=0.788439 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.486116 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=3.628968 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.788439 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=868701.437500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=868701.437500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.355936 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 265.711 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.450595 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 266.156 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=598841.937500 pseudoNetWeight=0.271352 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  4 Done HPWL=598841.937500 (elapsed time: 266.387 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-58.gz (elapsed time: 266.387 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-58.gz (elapsed time: 267.458 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 35 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 267.692 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 267.700 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 267.715 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 30 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1132 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 202 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 269.082 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 80 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 64 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 269.446 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 269.446 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 269.446 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 269.446 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 269.603 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  4 Done HPWL=837375.187500 (elapsed time: 269.624 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-59.gz (elapsed time: 269.624 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-59.gz (elapsed time: 270.651 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =3.628968 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.321952 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  4 Done (elapsed time: 271.197 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=837375.187500 pseudoNetWeight=0.271352 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   10  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=44889.656250 minHPWL=837375.187500 err/minHPWL=0.053608 progressRatio=0.817778 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.398324 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=3.543340 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.817778 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=837375.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=837375.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.747995 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 271.547 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=1.915485 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 272.016 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=602421.562500 pseudoNetWeight=0.298294 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  5 Done HPWL=602421.562500 (elapsed time: 272.243 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-60.gz (elapsed time: 272.243 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-60.gz (elapsed time: 273.264 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 42 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 273.467 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 273.474 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 273.489 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 35 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1124 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 76 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 274.736 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 9 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 225 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 59 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 275.102 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 275.102 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 275.102 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 276.633 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 276.633 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 276.633 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 276.791 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  5 Done HPWL=834386.125000 (elapsed time: 276.810 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-61.gz (elapsed time: 276.810 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-61.gz (elapsed time: 277.921 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =3.543340 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.329736 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  5 Done (elapsed time: 278.467 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=834386.125000 pseudoNetWeight=0.298294 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   7&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=42988.253906 minHPWL=834386.125000 err/minHPWL=0.051521 progressRatio=0.822470 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.385054 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=3.253202 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.822470 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=834386.125000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=834386.125000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-62.gz (elapsed time: 278.549 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-62.gz (elapsed time: 279.565 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: FinalLUTFF-3.gz (elapsed time: 279.566 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: FinalLUTFF-3.gz (elapsed time: 280.513 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer GlobalPlacement_CLBElements started (elapsed time: 280.514 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=2.301873 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 280.864 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=2.520854 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 281.353 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=611683.750000 pseudoNetWeight=0.327225 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  0 Done HPWL=611683.750000 (elapsed time: 281.586 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-63.gz (elapsed time: 281.586 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-63.gz (elapsed time: 282.602 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 49 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 282.801 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 282.823 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 282.846 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 40 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1271 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 99 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 285.006 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 7 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 250 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 66 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 285.662 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 285.662 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 285.662 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 285.662 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 285.820 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  0 Done HPWL=797315.500000 (elapsed time: 285.840 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-64.gz (elapsed time: 285.840 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-64.gz (elapsed time: 286.883 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =3.253202 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.182897 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  0 Done (elapsed time: 287.554 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=797315.500000 pseudoNetWeight=0.327225 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   4&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   10  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=54701.964844 minHPWL=797315.500000 err/minHPWL=0.068608 progressRatio=0.852978 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.303477 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.808723 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.852978 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=2.986265 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 287.918 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=3.268442 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 288.373 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=613217.312500 pseudoNetWeight=0.354071 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  1 Done HPWL=613217.312500 (elapsed time: 288.584 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-65.gz (elapsed time: 288.584 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-65.gz (elapsed time: 289.574 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 5 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 10 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 36 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 289.774 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 289.791 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 289.807 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 43 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1274 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 66 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 291.065 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 210 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 61 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 291.447 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 291.447 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 291.447 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 292.962 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 292.963 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 292.963 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 293.126 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  1 Done HPWL=818378.375000 (elapsed time: 293.145 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-66.gz (elapsed time: 293.145 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-66.gz (elapsed time: 294.127 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =2.808723 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.062488 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  1 Done (elapsed time: 294.696 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=818378.375000 pseudoNetWeight=0.354071 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=41276.160156 minHPWL=797315.500000 err/minHPWL=0.051769 progressRatio=0.841000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.334565 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.757881 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.841000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=818378.375000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=3.890680 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 295.047 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=4.255989 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 295.509 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=623687.437500 pseudoNetWeight=0.385197 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  2 Done HPWL=623687.437500 (elapsed time: 295.732 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-67.gz (elapsed time: 295.732 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-67.gz (elapsed time: 296.746 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 296.954 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 296.968 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 296.981 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 48 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1649 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 54 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 298.219 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 142 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 63 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 298.540 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 298.541 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 298.541 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 298.541 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 298.695 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  2 Done HPWL=807092.562500 (elapsed time: 298.715 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-68.gz (elapsed time: 298.715 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-68.gz (elapsed time: 299.750 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =2.757881 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.114936 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  2 Done (elapsed time: 300.315 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=807092.562500 pseudoNetWeight=0.385197 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=26526.386719 minHPWL=797315.500000 err/minHPWL=0.033270 progressRatio=0.856695 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.294066 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.951777 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.856695 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=807092.562500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=5.027760 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 300.683 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=5.497018 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 301.184 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=629154.062500 pseudoNetWeight=0.416098 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  3 Done HPWL=629154.062500 (elapsed time: 301.449 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-69.gz (elapsed time: 301.449 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-69.gz (elapsed time: 302.510 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 7 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 14 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 34 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 302.726 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 302.737 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 302.760 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 37 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1360 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 74 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 303.990 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 93 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 59 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 304.301 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 304.301 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 304.301 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 304.301 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 304.465 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  3 Done HPWL=801965.812500 (elapsed time: 304.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-70.gz (elapsed time: 304.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-70.gz (elapsed time: 305.462 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =2.951777 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.014515 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  3 Done (elapsed time: 306.002 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=801965.812500 pseudoNetWeight=0.416098 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=19672.783203 minHPWL=797315.500000 err/minHPWL=0.024674 progressRatio=0.864491 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.274673 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.887678 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.864491 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=801965.812500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=6.467658 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 306.352 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=7.067906 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 306.811 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=635998.500000 pseudoNetWeight=0.447887 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  4 Done HPWL=635998.500000 (elapsed time: 307.041 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-71.gz (elapsed time: 307.041 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-71.gz (elapsed time: 308.166 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 12 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 24 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 41 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 308.370 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 308.383 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 308.395 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 42 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1429 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 69 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 309.628 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 2 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 10 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 69 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 309.932 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 309.932 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 309.932 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 309.932 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 310.085 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  4 Done HPWL=802943.312500 (elapsed time: 310.104 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-72.gz (elapsed time: 310.104 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-72.gz (elapsed time: 311.128 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =2.887678 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =0.983475 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  4 Done (elapsed time: 311.714 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=802943.312500 pseudoNetWeight=0.447887 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=10886.076172 minHPWL=797315.500000 err/minHPWL=0.013653 progressRatio=0.869486 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.262492 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=3.164771 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.869486 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=802943.312500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=797315.500000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=8.293151 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 312.066 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=9.058721 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 312.523 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=642555.312500 pseudoNetWeight=0.481009 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  5 Done HPWL=642555.312500 (elapsed time: 312.732 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-73.gz (elapsed time: 312.732 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-73.gz (elapsed time: 313.767 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 9 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 18 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 32 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 313.971 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 313.977 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 313.984 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 27 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 887 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 106 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 315.154 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 12 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 701 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 48 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 315.463 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 315.463 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 315.463 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 315.463 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 315.618 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  5 Done HPWL=794175.187500 (elapsed time: 315.637 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-74.gz (elapsed time: 315.637 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-74.gz (elapsed time: 316.657 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =3.164771 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =0.874584 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  5 Done (elapsed time: 317.318 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=794175.187500 pseudoNetWeight=0.481009 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   7&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=13343.363281 minHPWL=794175.187500 err/minHPWL=0.016802 progressRatio=0.880636 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.235964 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=3.504189 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.880636 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=794175.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=794175.187500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=10.570364 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 317.705 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=11.541230 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 318.211 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=647182.062500 pseudoNetWeight=0.513953 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  6 Done HPWL=647182.062500 (elapsed time: 318.436 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-75.gz (elapsed time: 318.436 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-75.gz (elapsed time: 319.476 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 319.673 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 319.683 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 319.693 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 39 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1213 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 65 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 320.799 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 5 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 70 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 60 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 321.071 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 321.071 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 321.071 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 321.071 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 321.227 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  6 Done HPWL=788116.250000 (elapsed time: 321.246 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-76.gz (elapsed time: 321.246 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-76.gz (elapsed time: 322.294 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =3.504189 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =1.018320 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  6 Done (elapsed time: 322.871 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=788116.250000 pseudoNetWeight=0.513953 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=12715.843750 minHPWL=788116.250000 err/minHPWL=0.016134 progressRatio=0.888509 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.217766 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.724427 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.888509 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=788116.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=788116.250000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=13.412956 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 323.224 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=14.639013 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 323.688 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=653313.437500 pseudoNetWeight=0.547170 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  7 Done HPWL=653313.437500 (elapsed time: 323.888 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-77.gz (elapsed time: 323.888 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-77.gz (elapsed time: 324.993 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 8 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 32 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 325.260 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 325.266 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 325.276 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 33 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1084 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 81 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 326.438 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 4 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 115 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 81 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 326.682 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 326.682 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 326.682 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 328.611 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 328.611 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 328.611 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 328.806 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  7 Done HPWL=774800.687500 (elapsed time: 328.825 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-78.gz (elapsed time: 328.825 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-78.gz (elapsed time: 329.925 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Rough Legalization =2.724427 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Rough Legalization =0.937294 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  7 Done (elapsed time: 330.750 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=774800.687500 pseudoNetWeight=0.547170 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   6&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=20408.953125 minHPWL=774800.687500 err/minHPWL=0.026341 progressRatio=0.902732 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.185956 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.977435 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.902732 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=774800.687500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=774800.687500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=17.739901 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 331.255 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=20.321819 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 331.775 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=652413.000000 pseudoNetWeight=0.578720 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  8 Done HPWL=652413.000000 (elapsed time: 331.993 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-79.gz (elapsed time: 331.993 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-79.gz (elapsed time: 333.016 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 5 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 10 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 29 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 333.218 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 333.225 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 333.234 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 24 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 778 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 67 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 334.208 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 5 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 209 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 51 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 334.474 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 334.474 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 334.474 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 334.474 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 334.633 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  8 Done HPWL=768034.562500 (elapsed time: 334.652 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-80.gz (elapsed time: 334.652 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-80.gz (elapsed time: 335.676 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Exact Legalization =10000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Exact Legalization =10000.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   CARRY Average Displacement Of Exact Legalization =10000.000000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   mCLBLegalizer: Launch. (elapsed time: 335.678 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   BRAMDSPLegalizer: Launch. (elapsed time: 336.070 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   CARRYMacroLegalizer: Launch. (elapsed time: 337.662 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  8 Done (elapsed time: 338.632 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=768034.562500 pseudoNetWeight=0.578720 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   4&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=21684.130859 minHPWL=768034.562500 err/minHPWL=0.028233 progressRatio=0.906745 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.177221 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=2.458156 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=0 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.906745 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=768034.562500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=768034.562500 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=24.571577 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 338.971 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   WLOptimizer macroLegalizePseudoNet=28.137650 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=0.000000) (elapsed time: 339.423 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   update pseudo net gor clockt egion <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after QP=670918.062500 pseudoNetWeight=0.610952 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   WLOptimizer Iteration#  9 Done HPWL=670918.062500 (elapsed time: 339.605 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-81.gz (elapsed time: 339.605 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-81.gz (elapsed time: 340.636 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 3 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 6 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 29 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_CARRY8] (elapsed time: 340.832 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF8] (elapsed time: 340.840 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_MUXF7] (elapsed time: 340.846 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 31 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 1099 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 87 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_LUT] (elapsed time: 341.816 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 2 overflowed bins <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   found 7 cells in them <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   spread for 74 iterations <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GeneralSpreader: accomplished spreadPlacementUnits for type: [SLICEL_FF] (elapsed time: 342.074 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 342.074 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 342.074 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 342.074 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 342.230 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Spreader Iteration#  9 Done HPWL=763244.937500 (elapsed time: 342.250 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-82.gz (elapsed time: 342.250 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-82.gz (elapsed time: 343.264 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   DSPBRAM Average Displacement Of Exact Legalization =0.071629 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   MCLB Average Displacement Of Exact Legalization =0.442550 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   CARRY Average Displacement Of Exact Legalization =1.127539 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Legalization Iteration#  9 Done (elapsed time: 343.266 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   HPWL after LG=763244.937500 pseudoNetWeight=0.610952 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  4&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   1&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   4&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   3&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   macroCloseToSite=1 and minHPWL is updated to be: 757820.375000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: Routability-oriented area adjustion is reset. (elapsed time: 343.339 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   err=22944.164062 minHPWL=757820.375000 err/minHPWL=0.030277 progressRatio=0.929527 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL / lowerBoundHPWL=1.129527 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   averageMacroLegalDisplacementL=0.071629 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroCloseToSite=1 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   macroLegalizationFixed=1 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   progressRatio=0.929527 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   upperBoundHPWL=757820.375000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   minHPWL=757820.375000 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Global Placer: B2B converge (elapsed time: 343.339 s) <br/>
<hr>

**13. Final packing:** AMFPlacer will pack elements into corresponding sites on the FPGA device with an improved parallelized algorithm.
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   MacroLegalizer: dumping MacroLegalization archieve to: BRAMDSPLegalizerDumpMacroLegalization-0.gz (elapsed time: 343.339 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   MacroLegalizer: dumped MacroLegalization archieve to: BRAMDSPLegalizerDumpMacroLegalization-0.gz (elapsed time: 343.342 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   MacroLegalizer: dumping MacroLegalization archieve to: CARRYMacroLegalizerDumpMacroLegalization-0.gz (elapsed time: 343.342 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   MacroLegalizer: dumped MacroLegalization archieve to: CARRYMacroLegalizerDumpMacroLegalization-0.gz (elapsed time: 343.354 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   CLBLegalizer: dumping CLBLegalization archieve to: DumpCLBLegalization-0.gz (elapsed time: 343.354 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   CLBLegalizer: dumped CLBLegalization archieve to: DumpCLBLegalization-0.gz (elapsed time: 343.359 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-83.gz (elapsed time: 343.359 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: ../../../Documents/placerDumpData//./DumpAllCoordTrace-83.gz (elapsed time: 344.372 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumping coordinate archieve to: FinalLUTFF-4.gz (elapsed time: 344.374 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   GlobalPlacer: dumped coordinate archieve to: FinalLUTFF-4.gz (elapsed time: 345.354 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumping PU information archieve to: PUInfoBeforeFinalPacking.gz (elapsed time: 345.357 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumped PU information archieve to: PUInfoBeforeFinalPacking.gz (elapsed time: 347.017 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   loading PU coordinate archieve from: PUInfoBeforeFinalPacking.gz (elapsed time: 347.020 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Please note that the loaded PU location information should be compatible with the otherinformation in the placer! Otherwise, there could be potential errors <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   E.g., the initial packing which creates virtual cells should be done. The bin grid for all cell types should be created. <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=84328 (elapsed time: 351.122 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Current Total HPWL = 757820.292165 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: CARRY macros are mapped to sites. (elapsed time: 351.398 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: initialized. (elapsed time: 351.399 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: initial packCLBsIteration done. (elapsed time: 352.010 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#0 #Mapped PU=20557 (elapsed time: 352.011 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=757820.292165 (elapsed time: 352.238 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#0 #determined Slice=0 (elapsed time: 352.238 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#0 Distribution:  (elapsed time: 352.238 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#1 #Mapped PU=43892 (elapsed time: 355.686 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=757820.292165 (elapsed time: 355.916 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#1 #determined Slice=0 (elapsed time: 355.916 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#1 Distribution:  (elapsed time: 355.916 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#2 #Mapped PU=69439 (elapsed time: 361.441 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=757820.292165 (elapsed time: 361.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#2 #determined Slice=0 (elapsed time: 361.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#2 Distribution:  (elapsed time: 361.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#3 #Mapped PU=94476 (elapsed time: 366.780 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=762427.054584 (elapsed time: 367.064 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#3 #determined Slice=15123 (elapsed time: 367.064 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#3 Distribution: (1,4246), (2,4655), (3,2481), (4,6), (5,1), (6,1661), (7,192), (8,1), (14,1879), (16,1),  (elapsed time: 367.064 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#4 #Mapped PU=118338 (elapsed time: 372.587 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=766871.750586 (elapsed time: 372.852 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#4 #determined Slice=22503 (elapsed time: 372.852 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#4 Distribution: (1,2504), (2,7055), (3,3353), (4,3259), (5,525), (6,1814), (7,512), (8,249), (9,393), (10,33), (12,563), (13,1), (14,410), (15,244), (16,616), (17,49), (20,154), (21,12), (28,757),  (elapsed time: 372.852 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#5 #Mapped PU=138941 (elapsed time: 377.577 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=772063.077795 (elapsed time: 377.863 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#5 #determined Slice=26011 (elapsed time: 377.863 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#5 Distribution: (1,1744), (2,4414), (3,5136), (4,3672), (5,2145), (6,2785), (7,866), (8,817), (9,834), (10,207), (11,68), (12,407), (13,216), (14,248), (15,135), (16,453), (17,87), (18,727), (19,20), (20,66), (21,131), (22,20), (23,6), (24,1), (26,19), (28,340), (29,15), (30,432),  (elapsed time: 377.863 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#6 #Mapped PU=155198 (elapsed time: 381.571 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=776692.381872 (elapsed time: 381.844 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#6 #determined Slice=27858 (elapsed time: 381.844 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#6 Distribution: (1,1705), (2,3077), (3,2729), (4,4605), (5,2766), (6,3095), (7,1539), (8,1909), (9,1102), (10,893), (11,341), (12,594), (13,226), (14,301), (15,306), (16,266), (17,220), (18,443), (19,170), (20,426), (21,116), (22,68), (23,59), (24,63), (25,4), (26,38), (27,1), (28,304), (29,15), (30,266), (31,19), (32,192),  (elapsed time: 381.844 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#7 #Mapped PU=166330 (elapsed time: 384.532 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=782197.710411 (elapsed time: 384.809 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#7 #determined Slice=29165 (elapsed time: 384.809 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#7 Distribution: (1,1722), (2,2813), (3,1976), (4,3021), (5,2897), (6,3414), (7,2133), (8,1950), (9,1578), (10,1470), (11,799), (12,941), (13,455), (14,475), (15,364), (16,265), (17,195), (18,408), (19,213), (20,377), (21,287), (22,280), (23,51), (24,78), (25,100), (26,69), (27,6), (28,311), (29,15), (30,143), (31,35), (32,132), (33,58), (34,134),  (elapsed time: 384.809 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#8 #Mapped PU=173623 (elapsed time: 387.068 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=788629.130395 (elapsed time: 387.420 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#8 #determined Slice=30137 (elapsed time: 387.420 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#8 Distribution: (1,1728), (2,2598), (3,1841), (4,2554), (5,1991), (6,3039), (7,2304), (8,2492), (9,1748), (10,1604), (11,1212), (12,1257), (13,687), (14,793), (15,542), (16,442), (17,217), (18,382), (19,218), (20,385), (21,369), (22,314), (23,123), (24,211), (25,127), (26,68), (27,16), (28,270), (29,21), (30,176), (31,21), (32,114), (33,37), (34,171), (35,8), (36,57),  (elapsed time: 387.420 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#9 #Mapped PU=178784 (elapsed time: 389.287 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=796419.905599 (elapsed time: 389.686 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#9 #determined Slice=30907 (elapsed time: 389.686 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#9 Distribution: (1,1831), (2,2421), (3,1653), (4,2507), (5,1797), (6,2348), (7,2069), (8,2593), (9,1985), (10,1758), (11,1294), (12,1378), (13,965), (14,1017), (15,713), (16,677), (17,405), (18,440), (19,285), (20,409), (21,392), (22,324), (23,160), (24,254), (25,168), (26,123), (27,35), (28,233), (29,29), (30,160), (31,27), (32,145), (33,40), (34,187), (35,14), (36,68), (37,3),  (elapsed time: 389.686 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#10 #Mapped PU=181843 (elapsed time: 391.380 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=805903.154680 (elapsed time: 391.687 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#10 #determined Slice=31427 (elapsed time: 391.687 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#10 Distribution: (1,1864), (2,2358), (3,1528), (4,2240), (5,1841), (6,2219), (7,1726), (8,2467), (9,1988), (10,1931), (11,1404), (12,1441), (13,1071), (14,1100), (15,881), (16,844), (17,555), (18,550), (19,344), (20,464), (21,420), (22,390), (23,184), (24,276), (25,197), (26,140), (27,46), (28,250), (29,31), (30,129), (31,30), (32,155), (33,45), (34,209), (35,21), (36,83), (37,4), (38,1),  (elapsed time: 391.687 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#11 #Mapped PU=183494 (elapsed time: 393.274 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=815428.044923 (elapsed time: 393.607 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#11 #determined Slice=31735 (elapsed time: 393.607 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#11 Distribution: (1,1850), (2,2307), (3,1453), (4,2080), (5,1679), (6,2202), (7,1663), (8,2355), (9,1992), (10,2058), (11,1389), (12,1450), (13,1121), (14,1197), (15,939), (16,951), (17,655), (18,656), (19,428), (20,503), (21,474), (22,388), (23,188), (24,335), (25,220), (26,154), (27,59), (28,240), (29,43), (30,137), (31,28), (32,150), (33,48), (34,210), (35,22), (36,104), (37,6), (38,1),  (elapsed time: 393.607 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#12 #Mapped PU=184340 (elapsed time: 394.748 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=824356.336673 (elapsed time: 395.052 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#12 #determined Slice=31886 (elapsed time: 395.052 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#12 Distribution: (1,1862), (2,2227), (3,1390), (4,1990), (5,1612), (6,2014), (7,1691), (8,2325), (9,1966), (10,2090), (11,1437), (12,1517), (13,1136), (14,1223), (15,991), (16,993), (17,750), (18,712), (19,474), (20,538), (21,493), (22,403), (23,211), (24,351), (25,232), (26,187), (27,67), (28,228), (29,53), (30,136), (31,35), (32,149), (33,50), (34,210), (35,25), (36,110), (37,7), (38,1),  (elapsed time: 395.052 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#13 #Mapped PU=184776 (elapsed time: 396.129 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=831205.758871 (elapsed time: 396.429 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#13 #determined Slice=31970 (elapsed time: 396.429 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#13 Distribution: (1,1868), (2,2185), (3,1361), (4,1954), (5,1574), (6,1950), (7,1644), (8,2274), (9,1959), (10,2098), (11,1435), (12,1535), (13,1178), (14,1272), (15,1012), (16,1009), (17,777), (18,759), (19,526), (20,573), (21,505), (22,414), (23,216), (24,356), (25,246), (26,188), (27,81), (28,231), (29,54), (30,141), (31,35), (32,150), (33,52), (34,208), (35,27), (36,115), (37,6), (38,2),  (elapsed time: 396.429 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#14 #Mapped PU=185033 (elapsed time: 397.415 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=835436.013327 (elapsed time: 397.712 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#14 #determined Slice=31999 (elapsed time: 397.712 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#14 Distribution: (1,1862), (2,2170), (3,1353), (4,1938), (5,1549), (6,1905), (7,1632), (8,2279), (9,1929), (10,2060), (11,1431), (12,1526), (13,1189), (14,1301), (15,1022), (16,1059), (17,788), (18,778), (19,546), (20,601), (21,517), (22,420), (23,230), (24,356), (25,243), (26,191), (27,95), (28,234), (29,55), (30,139), (31,35), (32,152), (33,54), (34,209), (35,27), (36,116), (37,6), (38,2),  (elapsed time: 397.712 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#15 #Mapped PU=185166 (elapsed time: 398.844 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=837992.009479 (elapsed time: 399.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#15 #determined Slice=32001 (elapsed time: 399.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#15 Distribution: (1,1857), (2,2152), (3,1358), (4,1927), (5,1539), (6,1904), (7,1608), (8,2262), (9,1918), (10,2061), (11,1419), (12,1513), (13,1187), (14,1304), (15,1028), (16,1074), (17,789), (18,804), (19,562), (20,613), (21,534), (22,432), (23,225), (24,360), (25,246), (26,190), (27,98), (28,233), (29,58), (30,141), (31,34), (32,157), (33,54), (34,209), (35,27), (36,116), (37,6), (38,2),  (elapsed time: 399.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#16 #Mapped PU=185253 (elapsed time: 400.162 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=839646.632994 (elapsed time: 400.517 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#16 #determined Slice=32003 (elapsed time: 400.517 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#16 Distribution: (1,1854), (2,2143), (3,1354), (4,1925), (5,1538), (6,1904), (7,1606), (8,2247), (9,1911), (10,2043), (11,1419), (12,1508), (13,1185), (14,1296), (15,1030), (16,1093), (17,806), (18,815), (19,566), (20,613), (21,540), (22,432), (23,230), (24,367), (25,244), (26,197), (27,99), (28,234), (29,58), (30,141), (31,34), (32,156), (33,54), (34,210), (35,27), (36,116), (37,6), (38,2),  (elapsed time: 400.517 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#17 #Mapped PU=185308 (elapsed time: 401.201 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=840922.442490 (elapsed time: 401.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#17 #determined Slice=32005 (elapsed time: 401.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#17 Distribution: (1,1855), (2,2139), (3,1350), (4,1916), (5,1533), (6,1906), (7,1608), (8,2246), (9,1906), (10,2039), (11,1417), (12,1499), (13,1179), (14,1296), (15,1038), (16,1090), (17,802), (18,835), (19,571), (20,627), (21,542), (22,433), (23,231), (24,368), (25,244), (26,197), (27,100), (28,233), (29,58), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 401.484 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#18 #Mapped PU=185346 (elapsed time: 402.080 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=841647.494141 (elapsed time: 402.369 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#18 #determined Slice=32005 (elapsed time: 402.369 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#18 Distribution: (1,1855), (2,2138), (3,1350), (4,1913), (5,1531), (6,1904), (7,1610), (8,2245), (9,1901), (10,2034), (11,1412), (12,1492), (13,1177), (14,1300), (15,1045), (16,1094), (17,801), (18,836), (19,577), (20,633), (21,544), (22,434), (23,232), (24,367), (25,244), (26,198), (27,100), (28,232), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 402.369 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#19 #Mapped PU=185369 (elapsed time: 402.784 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=842280.787219 (elapsed time: 403.099 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#19 #determined Slice=32008 (elapsed time: 403.099 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#19 Distribution: (1,1857), (2,2139), (3,1349), (4,1910), (5,1530), (6,1900), (7,1610), (8,2246), (9,1899), (10,2031), (11,1413), (12,1488), (13,1175), (14,1296), (15,1043), (16,1105), (17,806), (18,837), (19,576), (20,637), (21,547), (22,434), (23,232), (24,367), (25,244), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 403.099 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#20 #Mapped PU=185374 (elapsed time: 403.404 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=842610.908828 (elapsed time: 403.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#20 #determined Slice=32008 (elapsed time: 403.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#20 Distribution: (1,1857), (2,2137), (3,1348), (4,1913), (5,1530), (6,1899), (7,1609), (8,2247), (9,1900), (10,2029), (11,1408), (12,1481), (13,1179), (14,1298), (15,1041), (16,1105), (17,804), (18,843), (19,579), (20,637), (21,549), (22,435), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 403.685 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#21 #Mapped PU=185379 (elapsed time: 403.946 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=842906.972587 (elapsed time: 404.268 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#21 #determined Slice=32008 (elapsed time: 404.268 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#21 Distribution: (1,1857), (2,2137), (3,1348), (4,1911), (5,1530), (6,1901), (7,1609), (8,2243), (9,1900), (10,2030), (11,1405), (12,1480), (13,1178), (14,1295), (15,1045), (16,1106), (17,802), (18,848), (19,578), (20,638), (21,552), (22,435), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 404.268 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#22 #Mapped PU=185381 (elapsed time: 404.489 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843092.412866 (elapsed time: 404.790 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#22 #determined Slice=32008 (elapsed time: 404.790 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#22 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1531), (6,1902), (7,1608), (8,2243), (9,1900), (10,2029), (11,1403), (12,1478), (13,1176), (14,1297), (15,1047), (16,1104), (17,804), (18,851), (19,578), (20,639), (21,552), (22,435), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 404.790 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#23 #Mapped PU=185381 (elapsed time: 405.008 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843135.664134 (elapsed time: 405.331 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#23 #determined Slice=32008 (elapsed time: 405.331 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#23 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1531), (6,1901), (7,1608), (8,2244), (9,1900), (10,2029), (11,1403), (12,1478), (13,1175), (14,1297), (15,1048), (16,1104), (17,803), (18,848), (19,579), (20,642), (21,552), (22,435), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 405.331 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#24 #Mapped PU=185381 (elapsed time: 405.543 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 405.920 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#24 #determined Slice=32008 (elapsed time: 405.920 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#24 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1531), (6,1901), (7,1608), (8,2244), (9,1900), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 405.920 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#25 #Mapped PU=185381 (elapsed time: 406.178 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 406.471 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#25 #determined Slice=32008 (elapsed time: 406.471 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#25 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 406.471 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#26 #Mapped PU=185381 (elapsed time: 406.694 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 407.029 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#26 #determined Slice=32008 (elapsed time: 407.029 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#26 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 407.029 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#27 #Mapped PU=185381 (elapsed time: 407.263 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 407.605 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#27 #determined Slice=32008 (elapsed time: 407.605 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#27 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 407.605 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#28 #Mapped PU=185381 (elapsed time: 407.815 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 408.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#28 #determined Slice=32008 (elapsed time: 408.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#28 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 408.153 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#29 #Mapped PU=185381 (elapsed time: 408.420 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 408.766 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#29 #determined Slice=32008 (elapsed time: 408.766 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#29 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 408.766 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#30 #Mapped PU=185381 (elapsed time: 409.016 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 409.329 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#30 #determined Slice=32008 (elapsed time: 409.329 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#30 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 409.329 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#31 #Mapped PU=185381 (elapsed time: 409.534 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: current HPWL=843165.984565 (elapsed time: 409.831 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#31 #determined Slice=32008 (elapsed time: 409.831 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: iter#31 Distribution: (1,1857), (2,2137), (3,1348), (4,1909), (5,1530), (6,1902), (7,1608), (8,2243), (9,1901), (10,2029), (11,1403), (12,1477), (13,1175), (14,1298), (15,1046), (16,1104), (17,805), (18,848), (19,579), (20,641), (21,552), (22,436), (23,231), (24,367), (25,245), (26,197), (27,100), (28,234), (29,59), (30,141), (31,35), (32,155), (33,54), (34,211), (35,27), (36,115), (37,7), (38,2),  (elapsed time: 409.831 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: finish iterative packing (elapsed time: 409.831 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: start exceptionHandling. (elapsed time: 410.014 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: starting parallel ripping up for 431 PUs and current displacement threshold for ripping up is 3.000000 (elapsed time: 410.014 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: there are 0 PUs left to be legalized and current displacement threshold for ripping up is 4.800000 (elapsed time: 412.303 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   there are 0 FFs(avgW=-nan) and 0 LUTs(avgW=-nan)  <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker::exceptionHandling done! (elapsed time: 412.303 s) <br/>
<hr>

**16. Fianlly export the placement result as Checkpoint/Tcl script:**
<hr>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumping CLBPacking archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-0.gz (elapsed time: 412.528 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumped CLBPacking archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-0.gz (elapsed time: 416.070 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #mappedPUCnt = 185805 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumping unpackable PUs archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-unpackablePUs-0.gz (elapsed time: 416.139 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumped unpackable PUs archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-unpackablePUs-0.gz (elapsed time: 416.238 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   #PUNeedMappingCnt = 185805 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumping placementTcl archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-0.tcl (elapsed time: 416.238 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker: dumped placementTcl archieve to: ../../../Documents/placerDumpData//./DumpCLBPacking-first-0.tcl (elapsed time: 416.579 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   5&emsp;    <br/>
2&emsp;   2&emsp;   6&emsp;   11  5&emsp;    <br/>
3&emsp;   4&emsp;   5&emsp;   6&emsp;   4&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   3&emsp;   3&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   4&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   2&emsp;   2&emsp;   1&emsp;   1&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   5&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Current Total HPWL = 844643.030011 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker Updating Cell-PlacementUnit Mapping (elapsed time: 417.769 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   ParallelCLBPacker Loading Nets (elapsed time: 417.835 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=24351 (elapsed time: 419.259 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF Utilization (elapsed time: 419.630 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Packablity (elapsed time: 419.630 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Packablity (elapsed time: 420.539 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF Utilization (elapsed time: 420.539 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusting LUT/FF utilization based on Routability (elapsed time: 420.539 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: adjusted LUT/FF utilization based on Routability (elapsed time: 420.699 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumping congestion rate to: congestionInfo (elapsed time: 420.699 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinX: -0.400000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinY: 0.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxX: 86.400002 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxY: 479.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Placement Unit(s): 33544 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Unpacked Placement Unit(s): 651 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Macro Placement Unit(s): 32893 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumping PU information archieve to: PUInfoBeforeFinal.gz (elapsed time: 421.249 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumped PU information archieve to: PUInfoBeforeFinal.gz (elapsed time: 422.521 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   loading PU coordinate archieve from: PUInfoBeforeFinal.gz (elapsed time: 422.538 s) <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   Please note that the loaded PU location information should be compatible with the otherinformation in the placer! Otherwise, there could be potential errors <br/>
<span style="background:#ffac33;color:#ff0d0d ">HiFPlacer WARNING: </span>&emsp;   E.g., the initial packing which creates virtual cells should be done. The bin grid for all cell types should be created. <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   reload placementNets and #register-related PU=24351 (elapsed time: 426.523 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinX: -0.400000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMinY: 0.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxX: 86.400002 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   globalMaxY: 479.000000 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Placement Unit(s): 33544 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Unpacked Placement Unit(s): 651 <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Total Macro Placement Unit(s): 32893 <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumping PU information archieve to: PUInfoFinal.gz (elapsed time: 426.614 s) <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   PlacementInfo: dumped PU information archieve to: PUInfoFinal.gz (elapsed time: 427.917 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock region untilization: <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   1&emsp;    <br/>
1&emsp;   1&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   0&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   3&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   2&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
0&emsp;   2&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
7&emsp;   0&emsp;   0&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Clock column untilization: <br/>
0&emsp;   0&emsp;   2&emsp;   2&emsp;   1&emsp;    <br/>
2&emsp;   2&emsp;   3&emsp;   3&emsp;   4&emsp;    <br/>
2&emsp;   2&emsp;   4&emsp;   5&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   5&emsp;   4&emsp;   3&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   2&emsp;   2&emsp;   2&emsp;    <br/>
2&emsp;   3&emsp;   3&emsp;   2&emsp;   2&emsp;    <br/>
0&emsp;   2&emsp;   2&emsp;   0&emsp;   0&emsp;    <br/>
<span style="background:#0a6800;color:#18ff00">HiFPlacer STATUS:  </span>&emsp;   Placement Done (elapsed time: 427.974 s) <br/>
<span style="background:#00726c;color:#00fff2">HiFPlacer INFO:  </span>&emsp;   Current Total HPWL = 844643.030011 <br/>