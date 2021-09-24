# Get Started {#getStarted}

Here, we will go through some basic steps for users to build the placer and run a placement flow for a new design/device with a new placement configuration:
1. Build the Placer
2. Run An Example
3. Set the Placement Configuration in JSON file
4. Extract Design Information from Vivado
5. Extract Device Information from Vivado
6. (Optional) Load the Output Placement in Vivado
7. (Optional) Customize the Placement Flow

**1. Build the Placer**

We provide a script to build the placer and users can run the following command in the project root directory:

```
./build.sh
```

**2. Run An Example**

Below is a command that users can try to run in the "build" directory. It will run the placement flow according to a given JSON configuration, where design/device/paramters are specified.

```
./AMFPlacer ../benchmarks/testConfig/OpenPiton.json
```

**3. Set the Placement Configuration in JSON file**

By checking the content of the JSON file in the command argument above, you may notice the major settings of our placer. 
Generally, we need to let the placer know where are the data of the design and devices, whether the placer should dump some log text/archieve files for further checking or evaluation, and some parameters related to the algorithms in the placer.

Below, we explain some of the settings. If users target at Xilinx VCU108, users may only need to change the parameters related to the design benchmark, which we mark with \[DESIGN\].
```
{
    "vivado extracted design information file": ==> the location of the design netlist zip file [DESIGN]
    "vivado extracted device information file": ==> the location of the device zip file [DEVICE]
    "special pin offset info file": ==> the location of a information file indicating the offset of pins relative to the coordinate of the device site. (e.g. PCIE bank spans in a long range) [DEVICE]
    "cellType2fixedAmo file": ==> the location of a information file indicating the resource demand of each type of design standard cells [DEVICE]
    "cellType2sharedCellType file":  ==> the location of a information file indicating the resource demand of each type of design standard cells [DEVICE]
    "sharedCellType2BELtype file": "../benchmarks/VCU108/compatibleTable/sharedCellType2BELtype",
    "mergedSharedCellType2sharedCellType": "../benchmarks/VCU108/compatibleTable/mergedSharedCellType2sharedCellType",
    "unpredictable macro file": "../benchmarks/VCU108/design/OpenPiton/OpenPiton_unpredictableMacros",
    "fixed units file": "../benchmarks/VCU108/design/OpenPiton/OpenPiton_fixedUnits",
    "clock file": "../benchmarks/VCU108/design/OpenPiton/OpenPiton_clocks",
    "designCluster": "../benchmarks/VCU108/design/OpenPiton/OpenPiton_clusters.zip",
    "Dump Cluster file": "./dumpClusters",
    "Dump Cluster Simulated Annealig file": "./dumpSATrace",
    // "DumpCLBLegalization" : "./DumpCLBLegalization",
    // "SLICEL_LUT": "./SLICEL_LUT_density",
    // "DumpLUTFFCoordTrace": "./DumpLUTFFCoordTrace",
    // "DumpCARRYCoordTrace": "./DumpCARRYCoordTrace",
    // "Dump MacroDensity": "./macroDensity",
    // "Dump Cell Density": "./Density",
    // "DumpLUTCoordTrace": "./DumpLUTCoordTrace",
    // "DumpDSPCoordTrace": "./DumpDSPCoordTrace",
    // "DumpFFCoordTrace": "./DumpFFCoordTrace",
    // "DumpAllCoordTrace" : "./DumpAllCoordTrace",
    "GlobalPlacerPrintHPWL": "true",
    "DumpCLBPacking" : "./DumpCLBPacking",
    "DumpLUTFFPair": "./DumpLUTFFPair",
    "DumpClockUtilization": "true",
    // "DumpMacroLegalization" : "./DumpMacroLegalizationLog",
    // "MacroLegalizationVerbose" : "true",
    // "CLBLegalizationVerbose" : "true",
    "Simulated Annealing restartNum": "600",
    "Simulated Annealing IterNum": "30000000",
    // "RandomInitialPlacement" : "true",
    "DrawNetAfterEachIteration": "false",
    "PseudoNetWeight": "0.0025",
    "GlobalPlacementIteration": "30",
    "clockRegionXNum": "5",
    "clockRegionYNum": "8",
    "clockRegionDSPNum": "30",
    "clockRegionBRAMNum": "96",
    "jobs": "8",
    "y2xRatio": "0.4",
    "ClusterPlacerVerbose": "false",
    "GlobalPlacerVerbose": "false",
    // "SpreaderSimpleExpland": "true",
    // "pseudoNetWeightConsiderNetNum" : "false",
    // "disableSpreadingConvergeRatio" : "true",
    "drawClusters": "false",
    "MKL": "true",
    "dumpDirectory": "../../../Documents/placerDumpData/",
}
```