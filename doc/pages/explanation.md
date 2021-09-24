# Implementation Explanation {#explanation}

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

<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 
