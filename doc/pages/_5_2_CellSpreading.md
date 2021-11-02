@page _5_2_CellSpreading Parallel Cell Spreading
# Parallel Cell Spreading

For a specific region on FPGA, the available resources of various types are limited. Therefore, the instances should be spread from the regions where demand for resources is outrunning supply, to other regions, as rough legalization. 

The fundamental cell spreading algorithm of AMF-Placer is based on widely adopted bi-partitioning rough legalization. The placer will find an overflowed bin, expand it into a corresponding larger window containing it, recursively partition the window, and spread the instances into bins in the window. 

The important difference between AMF-Placer's cell spreading algorithms and the other ones in previous works are that: (1) AMF-Placer conducts efficient utilization-guided search of spreading windows; (2) AMF-Placer realizes the parallel partitioning-based cell spreading; (3) AMF-Placer needs to handle the spreading of macros; (4) AMF-Placer's forgetting-rate cell spreading depress the aggressive cell spreading when encountering the uneven cell distribution of general real-world applications.

For source code details, please check the class: GeneralSpreader .

**Utilization-Guided Search of Spreading Windows**

To find a minimal rectangular window for spreading from an overflowed bin, some previous works conducted an enumeration search, which could be time-consuming, while some others expanded the window from the overflowed bin with a pre-determined expanding pattern, which might lead to unnecessary window expanding and undermine wirelength. In contrast, AMF-Placer will iteratively expand the window. For each expanding iteration, it will first check the resource utilization of the neighbor bins in four directions. Then, to expand the window, it will select the direction, where the neighbor bins have the lowest utilization compared to the other directions. This iterative procedure for an overflowed bin will continue until the window covers enough resource. 

<center>
<img src="smartWindow.png" alt="Utilization-Guided Search of Spreading Windows" title="Utilization-Guided Search of Spreading Windows" width="600" /> 
</center>

**Parallelized Cell Spreading of Mixed-size Elements**

To enable parallelized cell spreading, AMF-Placer will sort the overflowed bins according to their resource utilization. Those bins with higher utilization will have higher priority to find their spreading regions. During the speading window expanding for an overflowed bin, the bins in the window will be colored. The later window expanding for the other overflowed bins cannot cover the colored bins. The obtained windows will not overlap with each others and can conduct standard-cell-level spreading concurrently, as an example shown below.

<center>
<img src="parallelSpreading.png" alt="Parallel Spreading" title="Parallel Spreading" width="400" /> 
</center>

For a rectangular cell spreading window for an overflowed bin with sufficient resources, previous solutions required it to completely cover the macros inside it, which will lead to over-spreading and high HPWL when there are many large macros close to each other. For an extreame example, in order to resolve an overflowed bin, all the instances need to be spread over the entire device. 
To solve this problem, as inspired by ASIC mixed-size placers, AMF-Placer conducts two-phase cell spreading:

1. In the first phase, standard cells in macros will be released from the shape constraints and spread with the other common standard cells to resolve the resource overflow.
2. In the second phase, a macro's location will be updated to the average location of the standard cells in it.

By iteratively involving the two phases, resource overflows will be gradually resolved. 

**Forgetting-rate Based Cell Spreading and Constraints to Solve the Sensitivity**

The partitioning-based cell spreading algorithm is sensitive to the overflow. When a placement is compact, a single overflowed bin could cause cell spreading in a large region (or even the whole device) and seriously increase wirelength. Therefore, instead of directly updating instance location, we keep the location of an instance in last upper-bound placement as *L*. During cell spreading, when the instance is expected to spread to the location  *L'* according to cell spreading result, the actual updated location will be set to: (f*L'*+(1-f)\**L*), where f is forgetting rate and set according to the convergence stage. It can gradually depress the sensitivity of cell spreading as the placement tends to converge. An example is shown below in the figure.

Furthermore, to avoid significant disturbance to the placement when it is close to final convergence, AMF-Placer will limit the size of the cell spreading windows in the late placement procedure to constrain the movement of elements in one cell spreading iteration. 


<center>
<img src="forgetRate.png" alt="Forgetting-rate Based Cell Spreading" title="Forgetting-rate Based Cell Spreading" width="600" /> 
</center>

**Macro Spreading Deadlock Solution**

As shown in the figure below, macros might cause deadlocks in the common partitioning-based cell spreading algorithm because moving a macro spanning multiple bins from an overflowed region might cause new overflows in other regions. AMF-Placer's solution is to inject resource fluctuation to those bins trapped in deadlocks, i.e., periodically reduce and recover the resource supply of those bins, and squeeze out some instances from them. The extent of supply fluctuation for a bin is proportional to the number of times the bin gets overflowed. The fluctuation will stop immediately when there is no overflow.

<center>
<img src="Deadlock.png" alt="Macro Spreading Deadlock Solution" title="Macro Spreading Deadlock Solution" width="400" /> 
</center>
