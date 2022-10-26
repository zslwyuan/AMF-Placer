# Introduction {#_1_intro}

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

<center>
<img src="overview.png" alt="Implementation Overview" title="Implementation Overview" width="800" /> 
</center>

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
  
AMF-Placer 2.0 relies on the following dependencies and we sincerely appreciate the efforts of the authors of the related open-source project.
Each of them has corresponding licensing policy and please confirm that you are following the rules mentioned in the licenses. 
In the source code of AMF-Placer 2.0, we only include the open-source projects under the Apache License 2.0 or MIT License. 
The other dependencies will be downloaded when you are building the project and interact with the main body of AMF-Placer 2.0 via libraries and APIs.

1. eigen 3.3.8 (MPL2 license, source code will be downlaoded into the building directory during building if you have not installed it)
2. PaToH (academic use only, library will be downlaoded into the building directory)
3. Asmjit (Apache License, source code included in src/lib/3rdParty/asmjit for fast placement rendering)
4. Blend2d (Apache License, source code included in src/lib/3rdParty/blend2d for fast placement rendering)
5. Qt5 (for GUI, you can install it on Ubuntu by: sudo apt-get install qt5-default )
6. KDTree (MIT License, source code included in src/lib/3rdParty/KDTree)
7. Maximal Cardinality Matching (MIT License, source code included in src/lib/3rdParty/MaximalCardinalityMatching))
8. Min Cost Flow (MIT License, source code included in src/lib/3rdParty/minCostFlow



**Todo List**

1. clock tree synthesis
2. ckock-related optimization
