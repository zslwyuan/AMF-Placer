@page _3_2_ExperimentalResults_1 Experimental Results of AMF-Placer 1.0
# Experimental Results of AMF-Placer 1.0

Apart from the fundamental mechanisms to support macro placement, some optional optimization techniques are evaluated:

1. SA-based initial placement
2. interconnection-density-aware pseudo net weight
3. utilization-guided search of spreading window
4. forgetting-rate-based cell spreading update
5. progressive macro legalization

Since existing open-source analytical FPGA placers do not support mixed-size FPGA placement of aforementioned macros on Ultrascale devices, for comprehensive comparison, according to some state-of-the-art solutions, we implement baseline placement solution with the following features:
 
1.  quadratic placement, cell spreading and clock region planning algorithms from RippleFPGA\[1\] and clock-aware initial clustering from \[2\].
2.  resource demand adjustment and packing algorithms from extended UTPlaceF\[3\]
3.  SA initial placement
4.  necessary modifications to support macro placement, e.g., macro legalization/packing, but without Tech2-5 shown above.
5.  parallelized


Below are the comparison data which are normalized. We will keep improving our implementation.


|           | faceDetect |       |         |       | halfsqueezenet |       |         |       | optimsoc |       |         |       | minimap_GENE |       |         |       |
| :-------: | :--------: | :---: | :-----: | :---: | :------------: | :---: | :-----: | :---: | :------: | :---: | :-----: | :---: | :----------: | :---: | :-----: | :---: |
|           |    HPWL    | Rhpwl | time(s) | Rtime |      HPWL      | Rhpwl | time(s) | Rtime |   HPWL   | Rhpwl | time(s) | Rtime |     HPWL     | Rhpwl | time(s) | Rtime |
| proposed  |   446908   | 1.000 |   105   | 1.054 |     339764     | 1.000 |   129   | 1.000 | 1771538  | 1.000 |   466   | 1.000 |   1275346    | 1.000 |   558   | 1.000 |
| w/o tech1 |   479221   | 1.072 |   109   | 1.088 |     360567     | 1.061 |   132   | 1.023 | 9034564  | 5.100 |   871   | 1.870 |   5132073    | 4.024 |  1265   | 2.266 |
| w/o tech2 |   487060   | 1.090 |   124   | 1.238 |     431386     | 1.270 |   240   | 1.857 | 1825819  | 1.031 |   543   | 1.164 |   1307249    | 1.025 |   626   | 1.121 |
| w/o tech3 |   465208   | 1.041 |   131   | 1.315 |     491385     | 1.446 |   517   | 4.007 | 1841786  | 1.040 |   577   | 1.239 |   1790566    | 1.404 |  1019   | 1.826 |
| w/o tech4 |   450199   | 1.007 |   100   | 1.000 |     351649     | 1.035 |   131   | 1.015 | 2658863  | 1.501 |   525   | 1.126 |   1291565    | 1.013 |   561   | 1.005 |
| w/o tech5 |   511514   | 1.145 |   134   | 1.344 |     443765     | 1.306 |   138   | 1.067 | 1880431  | 1.061 |   546   | 1.172 |   1430330    | 1.122 |   708   | 1.268 |
| baseline  |   794201   | 1.777 |   234   | 2.338 |    1865746     | 5.491 |   329   | 2.549 | 2231978  | 1.260 |   559   | 1.199 |   11886747   | 9.320 |  3539   | 6.339 |


|           | OpenPiton |       |         |       | digitRecognition |       |         |       | MemN2N  |       |         |       | BLSTM_midDensity |       |         |       |
| :-------: | :-------: | :---: | :-----: | :---: | :--------------: | :---: | :-----: | :---: | :-----: | :---: | :-----: | :---: | :--------------: | :---: | :-----: | :---: |
|           |   HPWL    | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |  HPWL   | Rhpwl | time(s) | Rtime |       HPWL       | Rhpwl | time(s) | Rtime |
| proposed  |  1139189  | 1.000 |   283   | 1.035 |      726929      | 1.000 |   254   | 1.097 | 801403  | 1.000 |   318   | 1.130 |      484650      | 1.000 |   326   | 1.241 |
| w/o tech1 |  6218009  | 5.458 |   350   | 1.280 |      768796      | 1.058 |   256   | 1.105 | 963416  | 1.202 |   363   | 1.287 |      492194      | 1.016 |   285   | 1.083 |
| w/o tech2 |  1157479  | 1.016 |   296   | 1.079 |      773957      | 1.065 |   272   | 1.174 | 862212  | 1.076 |   343   | 1.216 |      511861      | 1.056 |   296   | 1.126 |
| w/o tech3 |  1159003  | 1.017 |   326   | 1.190 |      730071      | 1.004 |   321   | 1.389 | 916176  | 1.143 |   411   | 1.460 |      496540      | 1.025 |   364   | 1.383 |
| w/o tech4 |  1212149  | 1.064 |   274   | 1.000 |      728094      | 1.002 |   231   | 1.000 | 822742  | 1.027 |   282   | 1.000 |      653010      | 1.347 |   307   | 1.169 |
| w/o tech5 |  1142927  | 1.003 |   301   | 1.098 |      997207      | 1.372 |   244   | 1.053 | 923670  | 1.153 |   353   | 1.252 |      722495      | 1.491 |   263   | 1.000 |
| baseline  |  1432535  | 1.258 |   297   | 1.086 |     1047885      | 1.442 |   298   | 1.288 | 1610425 | 2.010 |   547   | 1.942 |      907383      | 1.872 |   325   | 1.237 |

The dominant algorithm for each stage in the proposed placement flow can be parallelized and in the table below, acceleration ratios are demonstrated by changing the number of threads and evaluating placement runtime.


| \#threads | Rosetta FaceDetect | SpooNN | OptimSoC | MiniMap2 | OpenPiton | Rosetta DigitRecog | MemN2N | BLSTM |
| :-------: | :----------------: | :----: | :------: | :------: | :-------: | :----------------: | :----: | :---: |
| 8 threads |       2.17x        | 2.07x  |  2.50x   |  2.86x   |   2.63x   |       2.22x        | 2.61x  | 2.23x |
| 4 threads |       2.01x        | 1.96x  |  2.29x   |  2.57x   |   2.37x   |       2.04x        | 2.35x  | 1.97x |
| 2 threads |       1.56x        | 1.52x  |  1.64x   |  1.81x   |   1.65x   |       1.54x        | 1.68x  | 1.77x |
| 1 threads |       1.00x        | 1.00x  |  1.00x   |  1.00x   |   1.00x   |       1.00x        | 1.00x  | 1.00x |

Below is the comparison of AMF-Placer Placement (upper ones) and Vivado Placement (lower ones): yellow for CARRY macros, red for MUX macros, green for BRAM macros, purple for DSP macros, blue for LUTRAM macros. The view of device is rotated left by 90 degree.

<center>
<img src="placement.png" alt="Placement" title="Placement" width="800" /> 
</center>

The placement congestion level of AMF-Placer is similar to Vivado and below is a figure for the most congested benchmarks (MiniMap2 with PCIE banks and OpenPiton with DDR Interface):

<center>
<img src="congestionExample.png" alt="Congestion Example" title="Congestion Example" width="800" /> 
</center>



**The Runtime Log of ICCAD-2021 Benchmarks**

Since we keep developing AMF-Placer to meet more expectation from reviewers and users (especially for timing optimization, multi-SLR/die optimization and more applications), we provide [the runtime log files](https://github.com/zslwyuan/AMF-Placer/tree/main/benchmarks/testConfig/testConfigSets/outputs) of the old version of placer which generated the result in the ICCAD-2021 paper. We suggest open the log files with VSCode since it seems that VSCode can highlight different parts of the log files for review convenience.

The paper only considered the improvement of wirelength, while now AMF-Placer considers the timing and the clock region impact during global placement and the resultant placements are more practical (e.g., the delays of critical paths are reduced by 20~40%). We are happy to pave the way for people who targets at the practical improvements to gain better results so please feel free to let us know your expectation or demands in the GitHub Issue.

**References in this page:**

\[1\] C.-W. Pui, G. Chen, W.-K. Chow, K.-C. Lam, J. Kuang, P. Tu, H. Zhang, E. F. Young, and B. Yu, “Ripplefpga: A routability-driven placement for large-scale heterogeneous fpgas,” in 2016 IEEE/ACM International Conference on Computer-Aided Design (ICCAD). IEEE, 2016, pp. 1–8.

\[2\] J. Chen, Z. Lin, Y.-C. Kuo, C.-C. Huang, Y.-W. Chang, S.-C. Chen, C.-H. Chiang, and S.-Y. Kuo, “Clock-aware placement for large-scale heterogeneous fpgas,” IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 39, no. 12, pp. 5042–5055, 2020.

\[3\] W. Li, S. Dhar, and D. Z. Pan, “Utplacef: A routability-driven fpga placer with physical and congestion aware packing,” IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 37, no. 4, pp. 869–882, 2017.