# Introduction {#_1_intro}

**Motivations**

1. Just reinvent the wheel for fun, try to build a complete flow and implement some state-of-art techniques in the latest paper.
2. Resolve some existing constraints in some previous works and consider more practical situations, like FPGA mixed-size placement with a series of optimization.
3. A beginner-friendly placement framework with clear hierarchy and detailed Doxygen-based documentation. We hope that it can lower the overhead for people who are also interested in this research area.
4. Currently, this framework is under development and it is still far from our goals and the practical demands, but we are happy to share our progress in this GitHub repository. If you have any questions/problems/suggestions, please contact feel free to contact us (tliang AT connect DOT ust DOT hk)

/home/tingyuan/Dropbox/AMF-Placer
**Features**

1. supports placeemnt with a large number of mixed-size macros with shape constraints in practical FPGA applications.
2. a set of optional optimization techniques to improve mixed-size FPGA placement QoR
3. parallelize the implementation of each stage of placement based on multi-threading
4. modularized function implementation for easier further development
5. flexible and extensible JSON-based placement configuration
6. a set of pre-implementation benchmarks from latest practical FPGA applications

**Implementation Overview**

<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 

**Project Hiearchy**

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

**Dependencies**
1. opengl, freeglut, glew
2. eigen 3.3.8 (source code included)
3. MKL
4. PaToH (library included)
5. osqp (source code included)


**Todo List**
1. stable clock legalization
2. basic static timing analysis (doing)
3. timing term in analytical model (doing)
4. timing-driven detailed placement

