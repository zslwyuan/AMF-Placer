# AMF-Placer

## Introduction

AMF-Placer is an open-source analytical mixed-size FPGA placer supporting mixed-size placement on FPGA, with an interface to
Xilinx Vivado. To speed up the convergence and improve the quality of the placement, AMF-Placer is equipped with
a series of new techniques for wirelength optimization, cell spreading, packing, and legalization. Based on a set
of the latest large open-source benchmarks from various domains for Xilinx Ultrascale FPGAs, experimental results
indicate that AMF-Placer can improve HPWL by 20.4%-89.3% and reduce runtime by 8.0%-84.2%, compared to the
baseline. Furthermore, utilizing the parallelism of the proposed algorithms, with 8 threads, the placement procedure
can be accelerated by 2.41x on average. Detailed Docygen-based documentation can be accessed **[here](https://zslwyuan.github.io/AMF-Placer/)**.

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
3. parallelize the implementation of each stage of placement based on multi-threading
4. modularized function implementation for easier further development
5. flexible and extensible JSON-based placement configuration
6. support placement check-point importing/exporting
7. a set of pre-implementation benchmarks from latest practical FPGA applications

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
## Some Experimental Results

Apart from the fundamental mechanisms to support macro placement, some optional optimization techniques are evaluated:

1. SA-based initial placement
2. interconnection-density-aware pseudo net weight
3. utilization-guided search of spreading window
4. forgetting-rate-based cell spreading update
5. progressive macro legalization

Below are the comparison data which are normalized. We will keep improving our implementation.

|          | faceDetect |       |         |       |  halfsqueezenet  |       |         |       | optimsoc |       |         |       |   minimap_GENE   |       |         |       |
|:--------:|:----------:|:-----:|:-------:|:-----:|:----------------:|:-----:|:-------:|:-----:|:--------:|:-----:|:-------:|:-----:|:----------------:|:-----:|:-------:|:-----:|
|          |    HPWL    | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |   HPWL   | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |
| proposed |   446908   | 1.000 |   105   | 1.054 |      339764      | 1.000 |   129   | 1.000 |  1771538 | 1.000 |   466   | 1.000 |      1275346     | 1.000 |   558   | 1.000 |
|   cfg1   |   479221   | 1.072 |   109   | 1.088 |      360567      | 1.061 |   132   | 1.023 |  9034564 | 5.100 |   871   | 1.870 |      5132073     | 4.024 |   1265  | 2.266 |
|   cfg2   |   487060   | 1.090 |   124   | 1.238 |      431386      | 1.270 |   240   | 1.857 |  1825819 | 1.031 |   543   | 1.164 |      1307249     | 1.025 |   626   | 1.121 |
|   cfg3   |   465208   | 1.041 |   131   | 1.315 |      491385      | 1.446 |   517   | 4.007 |  1841786 | 1.040 |   577   | 1.239 |      1790566     | 1.404 |   1019  | 1.826 |
|   cfg4   |   450199   | 1.007 |   100   | 1.000 |      351649      | 1.035 |   131   | 1.015 |  2658863 | 1.501 |   525   | 1.126 |      1291565     | 1.013 |   561   | 1.005 |
|   cfg5   |   511514   | 1.145 |   134   | 1.344 |      443765      | 1.306 |   138   | 1.067 |  1880431 | 1.061 |   546   | 1.172 |      1430330     | 1.122 |   708   | 1.268 |
| baseline |   794201   | 1.777 |   234   | 2.338 |      1865746     | 5.491 |   329   | 2.549 |  2231978 | 1.260 |   559   | 1.199 |     11886747     | 9.320 |   3539  | 6.339 |


|          |  OpenPiton |       |         |       | digitRecognition |       |         |       |  MemN2N  |       |         |       | BLSTM_midDensity |       |         |       |
|:--------:|:----------:|:-----:|:-------:|:-----:|:----------------:|:-----:|:-------:|:-----:|:--------:|:-----:|:-------:|:-----:|:----------------:|:-----:|:-------:|:-----:|
|          |    HPWL    | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |   HPWL   | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |
| proposed |   1139189  | 1.000 |   283   | 1.035 |      726929      | 1.000 |   254   | 1.097 |  801403  | 1.000 |   318   | 1.130 |      484650      | 1.000 |   326   | 1.241 |
|   cfg1   |   6218009  | 5.458 |   350   | 1.280 |      768796      | 1.058 |   256   | 1.105 |  963416  | 1.202 |   363   | 1.287 |      492194      | 1.016 |   285   | 1.083 |
|   cfg2   |   1157479  | 1.016 |   296   | 1.079 |      773957      | 1.065 |   272   | 1.174 |  862212  | 1.076 |   343   | 1.216 |      511861      | 1.056 |   296   | 1.126 |
|   cfg3   |   1159003  | 1.017 |   326   | 1.190 |      730071      | 1.004 |   321   | 1.389 |  916176  | 1.143 |   411   | 1.460 |      496540      | 1.025 |   364   | 1.383 |
|   cfg4   |   1212149  | 1.064 |   274   | 1.000 |      728094      | 1.002 |   231   | 1.000 |  822742  | 1.027 |   282   | 1.000 |      653010      | 1.347 |   307   | 1.169 |
|   cfg5   |   1142927  | 1.003 |   301   | 1.098 |      997207      | 1.372 |   244   | 1.053 |  923670  | 1.153 |   353   | 1.252 |      722495      | 1.491 |   263   | 1.000 |
| baseline |   1432535  | 1.258 |   297   | 1.086 |      1047885     | 1.442 |   298   | 1.288 |  1610425 | 2.010 |   547   | 1.942 |      907383      | 1.872 |   325   | 1.237 |



Below is the comparison of AMF-Placer Placement (upper ones) and Vivado Placement (lower ones): yellow for CARRY macros, red for MUX macros, green for BRAM macros, purple for DSP macros, blue for LUTRAM macros. The view of device is rotated left by 90 degree.

<img src="https://zslwyuan.github.io/AMF-Placer/placement.png" alt="Placement" title="Placement" width="800" /> 

## Dependencies
1. opengl, freeglut, glew
2. eigen 3.3.8 (source code included)
3. MKL
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
