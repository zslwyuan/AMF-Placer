**Introduction**

To enable the performance optimization of application mapping on modern field-programmable gate arrays (FPGAs),
certain critical path portions of the designs might be prearranged into many multi-cell macros during synthesis.
These movable macros with constraints of shape and resources lead to challenging mixed-size placement for FPGA
designs which cannot be addressed by previous works of analytical placers. In this work, we propose AMF-Placer,
an open-source analytical mixed-size FPGA placer supporting mixed-size placement on FPGA, with an interface to
Xilinx Vivado. To speed up the convergence and improve the quality of the placement, AMF-Placer is equipped with
a series of new techniques for wirelength optimization, cell spreading, packing, and legalization. Based on a set
of the latest large open-source benchmarks from various domains for Xilinx Ultrascale FPGAs, experimental results
indicate that AMF-Placer can improve HPWL by 20.4%-89.3% and reduce runtime by 8.0%-84.2%, compared to the
baseline. Furthermore, utilizing the parallelism of the proposed algorithms, with 8 threads, the placement procedure
can be accelerated by 2.41x on average.

**License**

For non-commercial usage of this open-source project, users should comply the Apache License attached in the root directory.
For commercial usage of this open-source project, users must contact authors (Wei ZHANG, eeweiz@ust.hk; Tingyuan LIANG, tliang@connect.ust.hk) for authorization.

**Documentation Hierarchy**

* [Basic Project Introduction](@ref intro)
* [Get Started](@ref getStarted)
* [Publications](@ref publication)
* [Implementation Explanation](@ref explanation)

**Motivations**

1. Just reinvent the wheel for fun, try to build a complete flow and implement some state-of-art techniques in the latest paper.
2. Resolve some existing constraints in some previous works and consider more practical situations, like FPGA mixed-size placement with a series of optimization.
3. A beginner-friendly placement framework with clear hierarchy and detailed Doxygen-based documentation. We hope that it can lower the overhead for people who are also interested in this research area.
4. Currently, this framework is under development and it is still far from our goals and the practical demands, but we are happy to share our progress in this GitHub repository. If you have any questions/problems/suggestions, please contact feel free to contact us (tliang AT connect DOT ust DOT hk)


**Features**

1. supports placeemnt with a large number of mixed-size macros with shape constraints in practical FPGA applications.
2. a set of optional optimization techniques to improve mixed-size FPGA placement QoR
3. parallelize the implementation of each stage of placement based on multi-threading
4. modularized function implementation for easier further development
5. flexible and extensible JSON-based placement configuration
6. a set of pre-implementation benchmarks from latest practical FPGA applications

**Implementation Overview**

<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 

(last updated Aug 17, 2021)