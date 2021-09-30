# Benchmarks and Experimental Results {#_3_benchmarkInfo}

**Benchmarks**

As mentioned in the introduction, we collect the latest practical open-source benchmarks from various domains.

| Benchmarks    | Rosetta FaceDetection  | SpooNN | OptimSoC | MiniMap2 | OpenPiton | MemN2N | BLSTM  |  Rosetta DigitRecog |
|:-------------:|:----------------------:|:------:|:--------:|:--------:|:---------:|:------:|:------:|:-------------------:|
| #LUT          | 68945                  | 63095  | 186183   | 407586   | 180388    | 184997 | 118967 | 151636              |
| #FF           | 56987                  | 70987  | 248983   | 252624   | 111966    | 84694  | 54690  | 105580              |
| #CARRY        | 4978                   | 2091   | 1715     | 19826    | 1712      | 11528  | 2762   | 1970                |
| #Mux          | 2177                   | 217    | 27037    | 180      | 13696     | 4466   | 36210  | 4662                |
| #LUTRAM       | 255                    | 251    | 901      | 251      | 752       | 3500   | 1147   | 251                 |
| #DSP          | 101                    | 165    | 51       | 528      | 58        | 312    | 258    | 1                   |
| #BRAM         | 141                    | 208    | 218      | 283      | 147       | 148    | 812    | 379                 |
| #Cell         | 134450                 | 137937 | 468150   | 681889   | 309145    | 289721 | 215101 | 265775              |
| #Macro        | 3582                   | 1135   | 21882    | 8746     | 8278      | 5775   | 14651  | 3061                |
| #siteForMacro | 55666                  | 23079  | 89004    | 191263   | 48066     | 118960 | 171822 | 55754               |
| MacroRatio    | 40\%                   | 16\%   | 19\%     | 28\%     | 15\%      | 41\%   | 80\%   | 21\%               |

* Rosetta FaceDetection/DigitRecog: Y. Zhou, U. Gupta, S. Dai, R. Zhao, N. Srivastava, H. Jin, J. Featherston, Y.-H. Lai, G. Liu, G. A. Velasquez et al., “Rosetta: A realistic high-level synthesis benchmark suite for software programmable fpgas,” in Proceedings of the 2018 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays, 2018, pp. 269–278.
* SpooNN: K. Kara, “Spoonn: Fpga-based neural network inference library,” 2018.
* OptimSoC: S. Wallentowitz, P. Wagner, M. Tempelmeier, T. Wild, and A. Herkersdorf, “Open tiled manycore system-on-chip,” arXiv preprint arXiv:1304.5081, 2013.
* MiniMap2: L. Guo, J. Lau, Z. Ruan, P. Wei, and J. Cong, “Hardware acceleration of long read pairwise overlapping in genome sequencing: A race between fpga and gpu,” in 2019 IEEE 27th Annual International Symposium on Field-Programmable Custom Computing Machines (FCCM). IEEE, 2019, pp. 127–135.
* OpenPiton: J. Balkind, M. McKeown, Y. Fu, T. Nguyen, Y. Zhou, A. Lavrov, M. Shahrad, A. Fuchs, S. Payne, X. Liang et al., “Openpiton: An open source manycore research framework,” ACM SIGPLAN Notices, vol. 51, no. 4, pp. 217–232, 2016.
* MemN2N: S. Sukhbaatar, A. Szlam, J. Weston, and R. Fergus, “End-to-end memory networks,” arXiv preprint arXiv:1503.08895, 2015.
* BLSTM: V. Rybalkin, N. Wehn, M. R. Yousefi, and D. Stricker, “Hardware architecture of bidirectional long short-term memory neural network for optical character recognition,” in Design, Automation & Test in Europe Conference & Exhibition (DATE), 2017. IEEE, 2017, pp. 1390–1395.


**Experimental Results**

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

<img src="placement.png" alt="Placement" title="Placement" width="800" /> 