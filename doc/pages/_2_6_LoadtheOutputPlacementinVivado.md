@page _2_6_LoadtheOutputPlacementinVivado Load the Output Placement in Vivado
# Load the Output Placement in Vivado

After the placemnt completes, a Tcl file will be generated in the directory specified by "dumpDirectory" in the JSON configuration file. Currently, the mentioned Tcl file is named "DumpCLBPacking-first-0.tcl" since it is dumped by the ParallelCLBPacker.

You can go through the following steps to load the AMFPlacer result into Vivado Implementation.

* a. Please ensure that your Vivado has the license for the specific device, so you can open the Device window by clicking on the top bar "Window->Device"
* b. Open your design in Vivado and click on "Open Implemented Design" in the flow navigator and execut the command below in the Tcl console of Vivado.

```tcl
# replace XXXXX with the path specified by the "dumpDirectory" you specified in the configuration files.
source XXXXX/DumpCLBPacking-first-0.tcl
```
* c. Then you can wait until the placement and routing finish. The Tcl script will clear the Vivado placement result and packing information, which might take ~10 minutes for medium-size designs (50% resource). Then it will place the cells according to the AMFPlacer result, which might take ~20 minutes for medium-size designs (50% resource). Finally, it will call Vivado Placer to verify AMFPlacer's placement, handle a small number (tens) of Xilinx primitive cells (e.g., the clock buffer placement) and do routing. 

Below is a screenshot when the AMFPlacer's placement is loading on Vivado. The orange blocks means the cells are placed are set fixed on the device.

<center>
<img src="loadPlacement.png" align="center" alt="AMFPlacer's placement is loading on Vivado" title="AMFPlacer's placement is loading on Vivado" width="400" /> 
</center>

For users' testing and exploration, we provide the Vivado projects of the benchmarks with post-implementation designs (size of each is 100-1000MB) on: 

|   Online Storage    |                                                     Link                                                     |                                             Link                                             |                                              Link                                              |                                              Link                                              |                                              Link                                               |                                             Link                                             |                                            Link                                             |                                                   Link                                                   |
| :-----------------: | :----------------------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------------------: |
|    Google Drive     | [Rosetta FaceDetection ](https://drive.google.com/file/d/10ZmeYW4b2oSkpu4rnDMG29kqwPJ8FKa0/view?usp=sharing) | [SpooNN](https://drive.google.com/file/d/1LRg-HHw9Zir_V572_zzimhxik4FPOTWI/view?usp=sharing) | [OptimSoC](https://drive.google.com/file/d/1Sx-ng7H-prkP6KbSuIT_DM_Hn0qa5fQv/view?usp=sharing) | [MiniMap2](https://drive.google.com/file/d/1Dp1nL9KYuBgBjU2-1eL3IzYpl4OFD7As/view?usp=sharing) | [OpenPiton](https://drive.google.com/file/d/1b0sWwoWq6XyiqWWxUxLlI9rAszVmR5WI/view?usp=sharing) | [MemN2N](https://drive.google.com/file/d/1hGsxzdfVD9OaRRtxnqqOXju8A8X4AKOv/view?usp=sharing) | [BLSTM](https://drive.google.com/file/d/1XpWyHGnZIo71DkctqxEEht1clh5go6SE/view?usp=sharing) | [Rosetta DigitRecog](https://drive.google.com/file/d/13wEQTSIW8CsKQeb23WbsntRGp2CF2voG/view?usp=sharing) |
| Tencent Weiyun Disk |                         [Rosetta FaceDetection ](https://share.weiyun.com/1le7iJjW)                          |                         [SpooNN](https://share.weiyun.com/mOqrz0JT)                          |                         [OptimSoC](https://share.weiyun.com/nIFDLOX0)                          |                         [MiniMap2](https://share.weiyun.com/9K4Pmtmv)                          |                         [OpenPiton](https://share.weiyun.com/RVxP3RX8)                          |                         [MemN2N](https://share.weiyun.com/MDolGB8H)                          |                         [BLSTM](https://share.weiyun.com/HzfUK7do)                          |                         [Rosetta DigitRecog](https://share.weiyun.com/rbJhbFvn)                          |


Users can directly open the .xpr file with Vivado. There might be some warnings because your Vivado version is not matched with the IP cores in the design or some IP core instances cannot be found in your system. Please ignore them and do not update the ip core or re-synthesis because these operations will change the netlist. Besides, if users use the benchmarks in their works, please cite [the papers of the related designs and comply with their open-source licence conditions](@ref _3_benchmarkInfo).

<center>
<img src="errors.png" align="center" alt="openImpled errors" title="openImpled errors" width="300" /> 
</center>


Users can directly "Open Implemented Design" and source the tcl file generated by AMF-Placer.


<center>
<img src="openImpl.png" align="center" alt="openImpled Design" title="openImpled Design" width="200" /> 
</center>

This placement loading flow might be a bit tricky since we are trying to be compatible with Vivado. Theoratically, since we utilize "catch" in the Tcl script to handle the errors, the flow should not be stopped by those minor exceptions and we have tested this flow with Vivado 2019-2021. If you encounter unexpected problems, please feel free to let us know in [GitHub Issue](https://github.com/zslwyuan/AMF-Placer/issues).

There might be some known problems when you are trying to load the generated placement into Vivado. Please check [the existing problems when exporting to Vivado](@ref _7_portabilityToVivadoProblem) for reasons and solutions.