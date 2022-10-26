![DocumentationFlow](https://github.com/zslwyuan/AMF-Placer/actions/workflows/main.yml/badge.svg) ![CMakeFlow](https://github.com/zslwyuan/AMF-Placer/actions/workflows/cmake.yml/badge.svg)  <img src="https://raw.githubusercontent.com/zslwyuan/AMF-Placer/cloc_code/cloc_code.svg" alt="LoCCode" title="LoCCode" height="18" /> 
  <img src="https://raw.githubusercontent.com/zslwyuan/AMF-Placer/cloc_code/cloc_comment.svg" alt="LoCComment" title="LoCComment" height="18" /> 


<a class="el" href="https://github.com/zslwyuan/AMF-Placer">Access The GitHub Project</a>


**Introduction**

AMF-Placer 2.0 is an open-source comprehensive timing-driven Analytical Mixed-size FPGA placer. It supports mixed-size placement of heterogeneous resources (e.g., LUT/FF/LUTRAM/MUX/CARRY/DSP/BRAM) on FPGA. To speed up the convergence and improve the timing quality of the placement,  standing upon the shoulders of AMF-Placer 1.0, AMF-Placer 2.0 is equipped with a series of new techniques for timing optimization, including an effective regression-based timing model, placement-blockage-aware anchor insertion, WNS-aware timing-driven quadratic placement, and sector-guided detailed placement. Based on a set of the latest large open-source benchmarks from various domains for Xilinx Ultrascale FPGAs, experimental results indicate that critical path delays realized by AMF-Placer 2.0 are averagely 2.2% and 0.59% higher than those achieved by commercial tool Xilinx Vivavo 2020.2 and 2021.2  respectively. Meanwhile, the average runtime of placement procedure of AMF-Placer 2.0 is 14% and 8.5% higher than Xilinx Vivavo 2020.2 and 2021.2 respectively. Although limited by the absence of the exact timing model of the device, the information of design hierarchy and accurate routing feedback, AMF-Placer 2.0 is the first open-source FPGA placer which can handle the timing-driven mixed-size placement of practical complex designs with various FPGA resources and achieves the comparable quality to the latest commercial tools. **Detailed Doxygen-based documentation (e.g, introduction, usage, implementation and experimental results) can be accessed [here](https://zslwyuan.github.io/AMF-Placer/)**.


AMF-Placer 2.0 is the extension of AMF-Placer 1.0. AMF-Placer 1.0 is equipped with a series of new techniques for wirelength optimization, cell spreading, packing, and legalization. Based on a set of the latest large open-source benchmarks from various domains for Xilinx Ultrascale FPGAs, experimental results indicate that AMF-Placer 1.0 can improve HPWL by 20.4%-89.3% and reduce runtime by 8.0%-84.2%, compared to the baseline. Furthermore, utilizing the parallelism of the proposed algorithms, with 8 threads, the placement procedure can be accelerated by 2.41x on average. 

<center>
<img src="OpenPiton_converge.gif" alt="Convergence (OpenPiton)" title="Convergence (OpenPiton)" width="100" />    <img src="MiniMap2_converge.gif" alt="Convergence (MiniMap2)" title="Convergence (MiniMap2)" width="100" />   <img src="optimsoc_converge.gif" alt="Convergence (OptimSoC)" title="Convergence (OptimSoC)" width="100" />    <img src="MemN2N_converge.gif" alt="Convergence (MemN2N)" title="Convergence (MemN2N)" width="100" />  <img src="GUI.gif" alt="GUI" title="GUI" height="250" />   <img src="GUIdetailed.gif" alt="GUIdetailed" title="GUIdetailed" height="250" />  
</center>

**License**

This project is developed by [Reconfiguration Computing Systems Lab](https://eeweiz.home.ece.ust.hk/), Hong Kong University of Science and Technology (HKUST). Tingyuan Liang (tliang@connect.ust.hk), Gengjie Chen (chen_gengjie@hotmail.com), Jieru Zhao (zhao-jieru@sjtu.edu.cn), Sharad Sinha (sharad@iitgoa.ac.in) and Wei Zhang (eeweiz@ust.hk) are the major contributors of this project.

In this repo, we provide the **basic implementation of AMF-Placer 2.0, under the Apache License 2.0**, supporting comprehensive timing-driven placement with critical path delay and runtime which are downgraded slightly by ~5% on average. If you want to obtain **the advanced version of AMF-Placer 2.0** to reproduce the experimental results in the paper of [AMF-Placer 2.0](https://arxiv.org/abs/2210.08682) for academic evaluation or commercial usage, you are required to contact the authors Tingyuan Liang (tliang@connect.ust.hk) and Wei ZHANG (eeweiz@ust.hk) with your offcial instituation email and we will response in 72 hours. If you are commercial entities, you can also contact ttsamuel@ust.hk for licensing opportunities of the advanced version.

**Documentation Hierarchy**

* [Basic Project Introduction](@ref _1_intro): motivation, features and some experimental results.
* [Get Started](@ref _2_getStarted): guideline to use the placer for your work.
* [Benchmarks and Experimental Results](@ref _3_benchmarkInfo): practical benchmarks for evaluation and some experimental results at current stages.
* [Publications](@ref _4_publication): some papers which are the fundamental parts of this project.
* [Implementation Explanation](@ref _5_explanation): the concrete explaination of the major novel contributions, the function modules and the placement procedure.
* [Existing Problems When Exporting To Vivado](@ref _7_portabilityToVivadoProblem): the concrete explaination of some of our known problems when interacting with Vivado.
* [Some Failure Lessons](@ref _8_someFailureLesson): Some of our previous failure lessons might be helpful for people who also want to develop their own physical synthesis flow.


**Motivations**

1. Just reinvent the wheel for fun, try to build a complete flow and reproduce/improve some state-of-art techniques in the latest papers.
2. Resolve some existing constraints in some previous works and consider more practical situations, like FPGA mixed-size placement with a series of optimization from the perspectives of timing, clocking, routability-aware and parallelism.
3. A beginner-friendly placement framework with clear hierarchy and detailed Doxygen-based documentation. We hope that it can lower the overhead for people who are also interested in this research area.
4. Currently, this framework is under development and it is still far from our goals and the practical demands, but we are happy to share our progress in this GitHub repository. If you have any questions/problems/suggestions, please contact feel free to contact us (Tingyuan LIANG, tliang@connect.ust.hk)


**Features**

1. supports placeemnt with a large number of mixed-size macros with shape constraints in practical FPGA applications.
2. wirelength-driven, routability-aware, packing-aware, clock-aware, region-aware. 
3. initially timing-driven with basic embedded static timing analysis, WNS-aware global placement, efficient detailed placement
4. a set of optional optimization techniques to improve mixed-size FPGA placement QoR
5. parallelizes the implementation of each stage of placement based on multi-threading
6. modularized function implementation for easier further development
7. flexible and extensible JSON-based placement configuration
8. supports placement check-point importing/exporting
9. a set of pre-implementation benchmarks from latest practical FPGA applications
10. provides a set of Tcl scripts which extracts design netlist from Vivado and exports post-placement information to Vivado
11. A basic GUI for user to analyze the placement procedure to optimize the implementation


**Todo List**

1. clock tree synthesis
2. ckock-related optimization

**Implementation Overview**

<center>
<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 
</center>

**Acknowledgement**

We sincerely appreciate the kindly suggestions from reviewers, detailed explanations of UTPlaceF from [Dr. Wuxi Li](http://wuxili.net/#about), useful advice on Vivado metric usages from Dr. Stephen Yang, fruitful discussion on some previous works with [Ms. Yun Zhou](https://github.com/YunxZhou) and practical suggestions of the convenient usages of AMF-Placer from [Mr. Jing Mai](https://github.com/magic3007).


**Issue Report**

This project is under active development and far from perfect. We do want to make the placer useful for people in the community. Therefore,
* If you have any question/problem, please feel free to create an issue in the [GitHub Issue](https://github.com/zslwyuan/AMF-Placer/issues) or email us (Tingyuan LIANG, tliang@connect.ust.hk)
* We sincerely welcome code contribution to this project or suggestion in any approach!



(last updated Oct 26, 2022)
