# AMF-Placer

![DocumentationFlow](https://github.com/zslwyuan/AMF-Placer/actions/workflows/main.yml/badge.svg) ![CMakeFlow](https://github.com/zslwyuan/AMF-Placer/actions/workflows/cmake.yml/badge.svg)   <img src="https://raw.githubusercontent.com/zslwyuan/AMF-Placer/cloc_code/cloc_code.svg" alt="LoCCode" title="LoCCode" height="18" /> 
  <img src="https://raw.githubusercontent.com/zslwyuan/AMF-Placer/cloc_code/cloc_comment.svg" alt="LoCComment" title="LoCComment" height="18" /> 



## Introduction

AMF-Placer is an open-source analytical mixed-size FPGA placer supporting mixed-size placement on FPGA, with an interface to
Xilinx Vivado. To speed up the convergence and improve the quality of the placement, AMF-Placer is equipped with
a series of new techniques for wirelength optimization, cell spreading, packing, and legalization. Based on a set
of the latest large open-source benchmarks from various domains for Xilinx Ultrascale FPGAs, experimental results
indicate that AMF-Placer can improve HPWL by 20.4%-89.3% and reduce runtime by 8.0%-84.2%, compared to the
baseline. Furthermore, utilizing the parallelism of the proposed algorithms, with 8 threads, the placement procedure
can be accelerated by 2.41x on average.  **Detailed Docygen-based documentation (e.g, introduction, usage, implementation and experimental results) can be accessed [here](https://zslwyuan.github.io/AMF-Placer/)**.

<p align="center">
<img src="https://zslwyuan.github.io/AMF-Placer/OpenPiton_converge.gif" alt="Convergence (OpenPiton)" title="Convergence (OpenPiton)" width="100" />  <img src="https://zslwyuan.github.io/AMF-Placer/MiniMap2_converge.gif" alt="Convergence (MiniMap2)" title="Convergence (MiniMap2)" width="100" /> <img src="https://zslwyuan.github.io/AMF-Placer/optimsoc_converge.gif" alt="Convergence (OptimSoC)" title="Convergence (OptimSoC)" width="100" /> 
</p>

## License

This project is developed by [Reconfiguration Computing Systems Lab](https://eeweiz.home.ece.ust.hk/), Hong Kong University of Science and Technology (HKUST).

For non-commercial usage of this open-source project, users should comply the Apache License attached in the root directory.
For commercial usage of this open-source project, users must contact authors (Wei ZHANG, eeweiz@ust.hk; Tingyuan LIANG, tliang@connect.ust.hk) for authorization.

## Publications

\[1\] AMF-Placer: High-Performance Analytical Mixed-size Placer for FPGA[(pre-print)](https://github.com/zslwyuan/AMF-Placer/blob/main/doc/paper/iccad2021.pdf)
```
@INPROCEEDINGS{AMFPlacer,  author={Liang, Tingyuan and Chen, Gengjie and Zhao, Jieru and Feng, Liang and Sinha, Sharad and Zhang, Wei},  booktitle={2021 IEEE/ACM International Conference on Computer-Aided Design (ICCAD)},   title={AMF-Placer: High-Performance Analytical Mixed-size Placer for FPGA},   year={2021},  volume={},  number={},  pages={1-6},}
```

## Motivations

1. Just reinvent the wheel for fun, try to build a complete flow and implement some state-of-art techniques in the latest paper.
2. Resolve some existing constraints in some previous works and consider more practical situations, like FPGA mixed-size placement with a series of optimization.
3. A beginner-friendly placement framework with clear hierarchy and detailed Doxygen-based documentation. We hope that it can lower the overhead for people who are also interested in this research area.
4. Currently, this framework is under development and it is still far from our goals and the practical demands, but we are happy to share our progress in this GitHub repository. If you have any questions/problems/suggestions, please contact feel free to contact us (tliang AT connect DOT ust DOT hk)

## Features

1. supports placeemnt with a large number of mixed-size macros with shape constraints in practical FPGA applications.
2. a set of optional optimization techniques to improve mixed-size FPGA placement QoR
3. parallelizes the implementation of each stage of placement based on multi-threading
4. modularized function implementation for easier further development
5. flexible and extensible JSON-based placement configuration
6. supports placement check-point importing/exporting
7. a set of pre-implementation benchmarks from latest practical FPGA applications
8. provides a set of Tcl scripts which extracts design netlist from Vivado and exports post-placement information to Vivado

## Implementation Overview

<p align="center">
<img src="https://zslwyuan.github.io/AMF-Placer/overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 
</p>

## Project Hiearchy

Below is a hiearchy tree of this project. As for the details, please refer to the class information and collaboration diagram in the Doxygen documentation and trace the implementation, e.g., AMFPlacer, GlobalPlacer, and PlacementInfo.
```
├── benchmarks  // benchmark information
│   ├── analysisScripts  // experimental result analysis Python scripts
│   ├── testConfig       // some test settings of placer
│   ├── VCU108           // information of design and device for VCU108
│   │   ├── compatibleTable    // mapping information between design and device
│   │   ├── design             // design information
│   │   └── device             // device information
│   └── vivadoScripts    // some Vivado Tcl scripts to extract design/device information
├── build                // potential target directory for built output
├── doc                  // documentation-related materials
└── src                  // source code of the project
    ├── app              // application (e.g., AMFPlacer)
    │   └── AMFPlacer    // A High-Performance Analytical Mixed-size Placer for FPGA
    └── lib              // libraries for application implementation
        ├── 3rdParty     // third-party libraries
        ├── HiFPlacer    // our placement function modules
        │   ├── designInfo
        │   ├── deviceInfo
        │   ├── placement
        │   │   ├── globalPlacement
        │   │   ├── legalization
        │   │   ├── packing
        │   │   ├── placementInfo
        │   │   └── placementTiming
        │   └── problemSolvers
        └── utils        // some minor utility functions
```

## Dependencies
1. opengl, freeglut, glew
2. eigen 3.3.8 (source code included)
3. MKL (optional)
4. PaToH (library included)
5. osqp (source code included)


## Todo List
1. stable clock legalization
2. basic static timing analysis (doing)
3. timing term in analytical model (doing)
4. timing-driven detailed placement


## Issue Report

This project is under active development and far from perfect. We do want to make the placer useful for people in the community. Therefore,
* If you have any question/problem, please feel free to create an issue in the [GitHub Issue](https://github.com/zslwyuan/AMF-Placer/issues) or email us (Tingyuan LIANG, tliang AT connect DOT ust DOT hk)
* We sincerely welcome code contribution to this project or suggestion in any approach!



(last updated Aug 17, 2021)
