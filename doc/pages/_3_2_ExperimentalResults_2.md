@page _3_2_ExperimentalResults_2 Experimental Results of AMF-Placer 2.0
# Experimental Results of AMF-Placer 2.0

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