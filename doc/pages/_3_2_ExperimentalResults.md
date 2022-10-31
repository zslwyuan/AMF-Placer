@page _3_2_ExperimentalResults Experimental Results
# Experimental Results

**The Experimental Result of AMF-Placer 1.0**
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

**The Experimental Result of AMF-Placer 2.0 (Compared with Vivado)**

AMF-Placer 2.0 has supported timing-driven placement. Since existing open-source analytical FPGA placers do not support mixed-size FPGA placement of aforementioned macros on Ultrascale devices, for comprehensive comparison, we take the widely-used commercial tool Xilinx Vivado 2020.2 and 2021.2 as our baselines. Specially, Xilinx Vivado 2021.2 is driven by machine learning.

The experimental results are shown in the table below, where WNS represents the worst negative slack, CPD represents critical path delay, and RT represents the runtime of the entire placement flow. To evaluate the final timing quality, the placement results of AMF-Placer 2.0 will be loaded by Vivado router to implement actual routing, and different combinations of placer and router may lead to different results, which is also unveiled in Table below. Here, AMF represents AMF-Placer 2.0, V2020 represents Vivado 2020.2 and V2021 represents Vivado 2021.2. For example, AMF-V2020 represents the combination of AMF-Placer 2.0 and the router of Vivado 2020.1. The WNS metrics indicate how strict the designer-defined timing constraints are, with the consideration of clock skew on the device, and the CPD metrics mainly indicate the delay of the critical path. 


|         | Place-Route | BLSTM  | DigitRecog | FaceDetect | SpooNN | MemN2N | MiniMap2 | OpenPiton | Average |
| :-----: | :---------: | :----: | :--------: | :--------: | :----: | :----: | :------: | :-------: | :-----: |
| WNS(ns) |  AMF-V2020  | -0.389 |   -3.595   |   -0.384   | -0.491 | -1.601 |  0.049   |  -2.310   |    -    |
| WNS(ns) |  AMF-V2021  | -0.508 |   -4.373   |   -0.445   | -0.537 | -1.665 |  0.001   |  -2.556   |    -    |
| WNS(ns) |    V2020    | -0.562 |   -2.564   |   -0.243   | -0.779 | -0.669 |  0.037   |  -2.159   |    -    |
| WNS(ns) |    V2021    | -0.668 |   -3.249   |   -0.264   | -0.836 | -0.732 |  0.070   |  -2.436   |    -    |
| CPD(ns) |  AMF-V2020  |  8.40  |   11.64    |   15.39    |  8.50  | 11.60  |   7.96   |   12.32   |    -    |
| CPD(ns) |    Rnorm    | 0.981  |   1.097    |   1.009    | 0.967  | 1.087  |  0.999   |   1.012   |  1.022  |
| CPD(ns) |  AMF-V2021  |  8.51  |   12.41    |   15.45    |  8.55  | 11.66  |   8.01   |   12.56   |    -    |
| CPD(ns) |    Rnorm    | 0.994  |   1.169    |   1.013    | 0.972  | 1.093  |  1.006   |   1.032   |  1.040  |
| CPD(ns) |    V2020    |  8.56  |   10.61    |   15.24    |  8.79  | 10.67  |   7.96   |   12.17   |    -    |
| CPD(ns) |    Rnorm    |   1    |     1      |     1      |   1    |   1    |    1     |     1     |    1    |
| CPD(ns) |    V2021    |  8.67  |   11.29    |   15.27    |  8.85  | 10.73  |   7.94   |   12.44   |    -    |
| CPD(ns) |    Rnorm    | 1.012  |   1.065    |   1.002    | 1.006  | 1.006  |  0.996   |   1.022   |  1.016  |
|  RT(s)  |     AMF     |  413   |    648     |    292     |  261   |  898   |   1166   |    679    |    -    |
|  RT(s)  |    Rnorm    |  1.04  |    1.24    |    1.02    |  0.98  |  1.38  |   1.07   |   1.22    |  1.14   |
|  RT(s)  |    V2020    |  398   |    522     |    285     |  265   |  650   |   1094   |    555    |    -    |
|  RT(s)  |    Rnorm    |  1.00  |    1.00    |    1.00    |  1.00  |  1.00  |   1.00   |   1.00    |  1.00   |
|  RT(s)  |    V2021    |  396   |    529     |    300     |  271   |  759   |   1031   |    630    |    -    |
|  RT(s)  |    Rnorm    |  0.99  |    1.01    |    1.05    |  1.02  |  1.17  |   0.94   |   1.14    |  1.05   |

We also evaluate the individual impact of the major proposed optimizations for different placement phases in AMF-Placer 2.0. Here we show their impact by setting the placer configurations as follows, to  disable different specified optimization technique:

1. Cfg0: all the optimization techniques are enabled as the configuration 
2. Cfg1: disable path-length-aware clustering before partitioning 
3. Cfg2: disable blockage-aware spreading and anchor insertion
4. Cfg3: disable WNS-aware timing criticality pseudo net weight 
5. Cfg4: disable path-length-aware parallel packing 
6. Cfg5: disable sector-guided site candidate selection and just use the original small square window
7. Cfg6: disable sector-guided site candidate selection and simply enlarge the square window


|         | Configuration | BLSTM | DigitRecog | FaceDetect | SpooNN | MemN2N | MiniMap2 | OpenPiton | Average |
| :-----: | :-----------: | :---: | :--------: | :--------: | :----: | :----: | :------: | :-------: | :-----: |
| CPD(ns) |     Cfg0      | 8.40  |   11.64    |   15.39    |  8.50  | 11.60  |   7.96   |   12.32   |    -    |
| CPD(ns) |     Rnorm     |   1   |     1      |     1      |   1    |   1    |    1     |     1     |    1    |
| CPD(ns) |     Cfg1      | 9.31  |   13.28    |   16.25    | 10.18  | 13.30  |   8.31   |   12.72   |    -    |
| CPD(ns) |     Rnorm     | 1.108 |    1.14    |   1.056    | 1.197  | 1.146  |  1.044   |   1.033   |  1.103  |
| CPD(ns) |     Cfg2      | 8.63  |   13.09    |   17.73    | 10.85  | 12.64  |   8.44   |   13.64   |    -    |
| CPD(ns) |     Rnorm     | 1.027 |   1.125    |   1.152    | 1.276  |  1.09  |   1.06   |   1.107   |  1.120  |
| CPD(ns) |     Cfg3      | 8.38  |   12.46    |   17.22    | 11.02  | 12.36  |   8.63   |   12.93   |    -    |
| CPD(ns) |     Rnorm     | 0.997 |    1.07    |   1.119    | 1.297  | 1.066  |  1.084   |   1.05    |  1.098  |
| CPD(ns) |     Cfg4      | 8.40  |   12.21    |   15.42    |  8.65  | 11.74  |   7.96   |   12.67   |    -    |
| CPD(ns) |     Rnorm     | 1.000 |   1.049    |   1.002    | 1.018  | 1.012  |  1.001   |   1.029   |  1.016  |
| CPD(ns) |     Cfg5      | 8.38  |   12.99    |   15.39    |  8.76  | 11.70  |   7.96   |   12.73   |    -    |
| CPD(ns) |     Rnorm     | 0.998 |   1.116    |     1      |  1.03  | 1.008  |    1     |   1.033   |  1.026  |
| CPD(ns) |     Cfg6      | 8.39  |   11.79    |   15.35    |  8.68  | 11.93  |   7.94   |   12.90   |    -    |
| CPD(ns) |     Rnorm     | 0.999 |   1.013    |   0.997    | 1.021  | 1.028  |  0.997   |   1.047   |  1.015  |
|  RT(s)  |     Cfg0      |  413  |    648     |    292     |  261   |  898   |   1166   |    679    |    -    |
|  RT(s)  |     Rnorm     |   1   |     1      |     1      |   1    |   1    |    1     |     1     |  1.00   |
|  RT(s)  |     Cfg1      |  393  |    620     |    360     |  229   |  913   |   1318   |    730    |    -    |
|  RT(s)  |     Rnorm     | 0.95  |    0.96    |    1.23    |  0.88  |  1.02  |   1.13   |   1.08    |  1.04   |
|  RT(s)  |     Cfg2      |  431  |    616     |    230     |  227   |  629   |   1223   |    604    |    -    |
|  RT(s)  |     Rnorm     | 1.04  |    0.95    |    0.79    |  0.87  |  0.7   |   1.05   |   0.89    |  0.90   |
|  RT(s)  |     Cfg3      |  419  |    568     |    316     |  232   |  846   |   1257   |    805    |    -    |
|  RT(s)  |     Rnorm     | 1.01  |    0.88    |    1.08    |  0.89  |  0.94  |   1.08   |   1.19    |  1.01   |
|  RT(s)  |     Cfg4      |  432  |    668     |    312     |  279   |  930   |   1304   |    727    |    -    |
|  RT(s)  |     Rnorm     | 1.05  |    1.03    |    1.07    |  1.07  |  1.04  |   1.12   |   1.07    |  1.06   |
|  RT(s)  |     Cfg5      |  466  |    675     |    313     |  274   |  779   |   1265   |    677    |         |
|  RT(s)  |     Rnorm     | 1.13  |    1.04    |    1.07    |  1.05  |  0.87  |   1.08   |     1     |  1.03   |
|  RT(s)  |     Cfg6      |  456  |    661     |    314     |  271   |  896   |   1288   |    756    |    -    |
|  RT(s)  |     Rnorm     | 1.10  |    1.02    |    1.08    |  1.04  |  1.00  |   1.10   |   1.11    |  1.06   |

**Evaluation of Accuracy of AMF-Placer Integrated Static Timing Analysis**

 As shown in the below table, where "CPD" represents critical path delay, compared to the Vivado post-route exact critical path delay, the average relative error of our predicted CPD is 8.59% while the average relative error of Vivado pre-route CPD estimation is 7.29%. Our proposed net delay model is relatively optimistic since most of the samples in the dataset are not in congested regions while critical paths usually route through congested regions.

| Benchmark Name                       | BLSTM | Rosetta  DigitRecog | Rosetta FaceDetect | SpooNN | MemN2N | Minimap2 | OpenPiton | Average |
| ------------------------------------ | ----- | ------------------- | ------------------ | ------ | ------ | -------- | --------- | ------- |
| AMF CPD Prediction (ns)              | 7.23  | 9.62                | 18.37              | 9.34   | 9.62   | 7.61     | 11.23     | -       |
| \|Relative Error\| (\%)              | 15.56 | 8.94                | 7.35               | 6.39   | 9.83   | 4.43     | 7.64      | 8.59    |
| Vivado Pre-Route Prediction CPD (ns) | 9.17  | 12.25               | 18.59              | 8.54   | 12.02  | 8.19     | 12.59     | -       |
| \|Relative Error\| (\%)              | 7.13  | 16.00               | 6.25               | 2.69   | 12.61  | 2.93     | 3.52      | 7.29    |
| Vivado Post-Route CPD (ns)           | 8.56  | 10.56               | 19.83              | 8.78   | 10.67  | 7.96     | 12.16     | -       |



**References in this page:**

\[1\] C.-W. Pui, G. Chen, W.-K. Chow, K.-C. Lam, J. Kuang, P. Tu, H. Zhang, E. F. Young, and B. Yu, “Ripplefpga: A routability-driven placement for large-scale heterogeneous fpgas,” in 2016 IEEE/ACM International Conference on Computer-Aided Design (ICCAD). IEEE, 2016, pp. 1–8.

\[2\] J. Chen, Z. Lin, Y.-C. Kuo, C.-C. Huang, Y.-W. Chang, S.-C. Chen, C.-H. Chiang, and S.-Y. Kuo, “Clock-aware placement for large-scale heterogeneous fpgas,” IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 39, no. 12, pp. 5042–5055, 2020.

\[3\] W. Li, S. Dhar, and D. Z. Pan, “Utplacef: A routability-driven fpga placer with physical and congestion aware packing,” IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 37, no. 4, pp. 869–882, 2017.