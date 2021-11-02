@page _5_4_packing Mixed-size Packing
# Mixed-size Packing

For the final packing, a parallelized packing algorithm with high quality has been proposed by UTPlacerF. It allows the FPGA sites to search their corresponding candidate packing solutions concurrently and then negotiate together during synchronization. It handles exception instances that cannot be packed during the parallel method with the sequential conventional ripping-up algorithm. 

We adapt this high-performance algorithm to our mixed-size placement scenarios with several major modifications:  

1. Some instances might be pre-packed into CLB sites by the macro legalization, but these pre-packed CLBs can be filled with other instances as long as it is legal. It should be noticed that a cell (e.g., FF, CARRY, MUX) might lead to the route-thru cost of FF/LUT BEL slots. This is considered by AMF-Placer.
2. AMF-Placer encodes each candidate packing cluster for a specific site with hash function to avoid many redundant/duplicated packing attempts to improve performance and quality of packing since we found that there are some candidates are exactly the same although they are generated via different packing orders.
3. AMF-Placer takes clock legalization into consideration during packing and conduct clock half-column-level parallel packing to ensure the packing in one half-column will not lead to clocking violation. (e.g., VCU108 requires that the cells in one clock half-column are driven by at most 12 clocks.)
4. By iteratively increasing the ripping-up window size and processing independent exception instances concurrently, we also parallelize the exception handling algorithm and achieve further acceleration;


For source code details, please check the classes: ParallelCLBPacker . 
