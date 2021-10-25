# Implementation Explanation {#_5_explanation}

Here, we will explain the placement flow by going through the functionality of the major modules in the placer. Of course, you can also make full use of the Doxygen diagram to trace the call graphs to understand their relationship. We try to avoid multi-level class during implementation so it is easier for beginners to trace the call graphs.

1. DesignInfo: Information related to FPGA designs, including design cells and their interconnections. 
2. DeviceInfo: Information class related to FPGA device, including the details of BEL/Site/Tile/ClockRegion. 
3. PlacementInfo: Information related to FPGA placement (wirelength optimization, cell spreading, legalization, packing) 
4. PlacementTimingInfo: The container which record the timing information related to placement. 
5. GlobalPlacer: accounts for the general iterations in global placement including wirelength optimization, cell spreading, legalization, area adjustion... 
    * a. ClusterPlacer: cluster nodes in the given netlist and place the clusters on the device based on simulated-annealing as initial placement. 
    * b. WirelengthOptimizer: builds numerical models based on the element locations and calls solvers to find an optimal solution of the placement. 
    * c. GeneralSpreader: accounts for the cell spreading, which controls the cell density of specific resource type, under the device constraints for specific regions. 
    * d. MacroLegalizer: maps DSP/BRAM/CARRY macros to legal location
    * e. CLBLegalizer: maps CLBs (each of which consists of one site) to legal location. e.g. LUTRAM, except those CLBs in CARRY8_Chain.
6. InitialPacker: identifies macros from the design netlist based on pattern matching
7. IncrementalBELPacker: incrementally packs some LUTs/FFs during global placement based on their distance, interconnection density and compatibility
8. PlacementTimingOptimizer: based on the timing information and the placement information, adjusts some terms in the quadratic models which can improve the timing.
9. ParallelCLBPacker: finally packs LUT/FF/MUX/CARRY elements into legal CLB sites in a parallel approach.

Below is the overall flow of AMFPlacer and a visualized procedure is shown in the figure below. If you are interested in the details of AMFPlacer runtime behaviors, you can find the concrete explanation here: (@subpage _6_runtimeLog).

1. AMFPlacer will be constructed based on the DesignInfo and DeviceInfo. 
2. At the beginning, AMFPlacer will construct the PlacementInfo based on the DesignInfo and DeviceInfn. 
3. The element attributes, locations and densities will be initialized in the PlacementInfo. 
4. InitialPacker will be called by AMFPlacer to identify PlacementMacro and PlacementUnpackedCell from the design netlist.
5. The PlacementTimingInfo inside PlacementInfo will be initialized by constructing a TimingGraph and checking the level of DesignCell in their corresponding timing paths.
6. GlobalPlacer will cluster the netlist based on clock information and the connectivity and initially place the cluster on the device based on SAPlacer.
7. GlobalPlacer will go through some global placement iteration (wirelength optimization, cell spreading, legalization)
8. PlacementInfo will set a more fine-grained cell density grid.
9. (Again) GlobalPlacer will go through some global placement iteration (wirelength optimization, cell spreading, legalization).
10. IncrementalBELPacker will pair some LUTs and FFs based on their locations as an intermediate process for later packing.
11. PlacementTimingOptimizer will enhence some net weights for timing optimization.
12. GlobalPlacer will enable PlacementInfo (i.e., set neighbor displacement upperbound to a positive value) to conduct packing-aware (Improved UTPlacerF solution, considering cell resource, timing and net density. Enable earlier cell packing determination based on history analysis. Enable parallel exception handling.) Please note that congestion-aware cell area adjustion will be automatically enabled during global placement according to the placement convergence progress.
13. Finally, when global placement converges, ParallelCLBPacker will pack the elements into the corresponding DeviceSite / DeviceBEL.

<center>
<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 
</center>