@page _5_3_macroLegalization Parallel Progressive Macro Legalization
# Parallel Progressive Macro Legalization

1-to-1 legalization cannot handle macro legalization. To solve this challenge of mixed-size FPGA placement, we propose progressive macro legalization for the 1-to-many legalization of a large number of macros in AMF-Placer. AMF-Placer will only conduct rough legalization in the early iterations of global placement, and it will conduct exact legalization following rough legalization when the macros are close enough to their potential legal positions.

For source code details, please check the classes: MacroLegalizer or  CLBLegalizer. MacroLegalizer will deal with DSP/BRAM/CARRY macros which will span across multiple device sites, while CLBLegalizer will deal with LUTRAM/MUX macros which can be packed in one device site.

**Rough Legalization Phase**

Cells in a macro will be legalized individually with min-cost bipartite matching algorithm, with the objective of wirelength minimization. Pseudo nets will link the macro to the cells’ potential legalization locations during the phase of wirelength optimization, as the figure shown below:

<center>
<img src="roughLegalization.png" alt="Rough Legalization Phase" title="Rough Legalization Phase" width="600" /> 
</center>

In the early stage of placement, the density of instances is high and their locations are highly unstable. Meanwhile, macros consisting of DSPs, LUTRAMs, or BRAMs are required to be mapped to the sparse legal regions and some of them might span tens of sites. In this situation, direct legalization or cell spreading for macros might lead to serious displacement and trap the placement in bad local optima. AMF-Placer's solution gradually strengthens the weights of the legalization pseudo nets, smoothly legalizes the large macros, and preserves space for wirelength optimization.

In this phase, each cells in the macro will find a set of candidate sites within a given displacement threshold and a bipartite graph between cells and device sites will be created. As the macros spread over the device, the bipartite graph might consist of multiple independent connected subgraphs. Accordingly, the matching procedures for these subgraphs can be fully parallelized.

**Exact Legalization Phase**

Cells in the macros keep "voting" during rough legalization. After a few iterations of wirelength optimization, cell spreading and rough legalization, as pseudo weights iteratively increase, macros get closer to the legal locations. If the average displacement from cells to sites in rough legalization is lower than a threshold, exact (strict) legalization will follow the rough legalization to ensure that the standard cells in a macro must be placed in adjacent sites in the same column. 

The macros will be mapped to FPGA columns with greedy algorithm according to their cells's mapping to the columns.  For macros in the column, they will be sorted according to their y-coordinates. Then, for each column, the legalization problem can be transformed into knapsack problem, optimal solution of which can be found by dynamic programming. An concrete example of this flow is shown in the figure below.

Since the macros are close to their potential legal positions and the placement tends to be stable at this stage, we assume that the locations of the other instances will remain unchanged during the intra-column legalization of macros. Therefore, the intra-column legalization procedures for different columns can be parallelized.

<center>
<img src="exactLegalization.png" alt="Exact Legalization Phase" title="Exact Legalization Phase" width="600" /> 
</center>
