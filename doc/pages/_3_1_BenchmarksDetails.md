@page _3_1_BenchmarksDetails Benchmarks
# Benchmarks

As mentioned in the introduction, we collect the latest practical open-source benchmarks from various domains.

|  Benchmarks   | Rosetta FaceDetection | SpooNN | OptimSoC | MiniMap2 | OpenPiton | MemN2N | BLSTM  | Rosetta DigitRecog |
| :-----------: | :-------------------: | :----: | :------: | :------: | :-------: | :----: | :----: | :----------------: |
|     #LUT      |         68945         | 63095  |  186183  |  407586  |  180388   | 184997 | 118967 |       151636       |
|      #FF      |         56987         | 70987  |  248983  |  252624  |  111966   | 84694  | 54690  |       105580       |
|    #CARRY     |         4978          |  2091  |   1715   |  19826   |   1712    | 11528  |  2762  |        1970        |
|     #Mux      |         2177          |  217   |  27037   |   180    |   13696   |  4466  | 36210  |        4662        |
|    #LUTRAM    |          255          |  251   |   901    |   251    |    752    |  3500  |  1147  |        251         |
|     #DSP      |          101          |  165   |    51    |   528    |    58     |  312   |  258   |         1          |
|     #BRAM     |          141          |  208   |   218    |   283    |    147    |  148   |  812   |        379         |
|     #Cell     |        134450         | 137937 |  468150  |  681889  |  309145   | 289721 | 215101 |       265775       |
|    #Macro     |         3582          |  1135  |  21882   |   8746   |   8278    |  5775  | 14651  |        3061        |
| #siteForMacro |         55666         | 23079  |  89004   |  191263  |   48066   | 118960 | 171822 |       55754        |
|  MacroRatio   |         40\%          |  16\%  |   19\%   |   28\%   |   15\%    |  41\%  |  80\%  |        21\%        |

* Rosetta FaceDetection/DigitRecog: Y. Zhou, U. Gupta, S. Dai, R. Zhao, N. Srivastava, H. Jin, J. Featherston, Y.-H. Lai, G. Liu, G. A. Velasquez et al., “Rosetta: A realistic high-level synthesis benchmark suite for software programmable fpgas,” in Proceedings of the 2018 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays, 2018, pp. 269–278.
* SpooNN: K. Kara, “Spoonn: Fpga-based neural network inference library,” 2018.
* OptimSoC: S. Wallentowitz, P. Wagner, M. Tempelmeier, T. Wild, and A. Herkersdorf, “Open tiled manycore system-on-chip,” arXiv preprint arXiv:1304.5081, 2013.
* MiniMap2: L. Guo, J. Lau, Z. Ruan, P. Wei, and J. Cong, “Hardware acceleration of long read pairwise overlapping in genome sequencing: A race between fpga and gpu,” in 2019 IEEE 27th Annual International Symposium on Field-Programmable Custom Computing Machines (FCCM). IEEE, 2019, pp. 127–135.
* OpenPiton: J. Balkind, M. McKeown, Y. Fu, T. Nguyen, Y. Zhou, A. Lavrov, M. Shahrad, A. Fuchs, S. Payne, X. Liang et al., “Openpiton: An open source manycore research framework,” ACM SIGPLAN Notices, vol. 51, no. 4, pp. 217–232, 2016.
* MemN2N: S. Sukhbaatar, A. Szlam, J. Weston, and R. Fergus, “End-to-end memory networks,” arXiv preprint arXiv:1503.08895, 2015.
* BLSTM: V. Rybalkin, N. Wehn, M. R. Yousefi, and D. Stricker, “Hardware architecture of bidirectional long short-term memory neural network for optical character recognition,” in Design, Automation & Test in Europe Conference & Exhibition (DATE), 2017. IEEE, 2017, pp. 1390–1395.
