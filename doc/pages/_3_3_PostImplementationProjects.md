@page _3_3_DownloadPostImplementationVivadoProjects Download Post-Implementation Vivado Projects
# Download Post-Implementation Vivado Projects

For users' testing and exploration, we provide the Vivado projects of the benchmarks with post-implementation designs (size of each is 100-1000MB) on: 

|   Online Storage    |                                                     Link                                                     |                                             Link                                             |                                              Link                                              |                                              Link                                              |                                              Link                                               |                                             Link                                             |                                            Link                                             |                                                   Link                                                   |
| :-----------------: | :----------------------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------------------: |
|    Google Drive     | [Rosetta FaceDetection ](https://drive.google.com/file/d/10ZmeYW4b2oSkpu4rnDMG29kqwPJ8FKa0/view?usp=sharing) | [SpooNN](https://drive.google.com/file/d/1LRg-HHw9Zir_V572_zzimhxik4FPOTWI/view?usp=sharing) | [OptimSoC](https://drive.google.com/file/d/1Sx-ng7H-prkP6KbSuIT_DM_Hn0qa5fQv/view?usp=sharing) | [MiniMap2](https://drive.google.com/file/d/1Dp1nL9KYuBgBjU2-1eL3IzYpl4OFD7As/view?usp=sharing) | [OpenPiton](https://drive.google.com/file/d/1b0sWwoWq6XyiqWWxUxLlI9rAszVmR5WI/view?usp=sharing) | [MemN2N](https://drive.google.com/file/d/1hGsxzdfVD9OaRRtxnqqOXju8A8X4AKOv/view?usp=sharing) | [BLSTM](https://drive.google.com/file/d/1XpWyHGnZIo71DkctqxEEht1clh5go6SE/view?usp=sharing) | [Rosetta DigitRecog](https://drive.google.com/file/d/13wEQTSIW8CsKQeb23WbsntRGp2CF2voG/view?usp=sharing) |
| Tencent Weiyun Disk |                         [Rosetta FaceDetection ](https://share.weiyun.com/1le7iJjW)                          |                         [SpooNN](https://share.weiyun.com/mOqrz0JT)                          |                         [OptimSoC](https://share.weiyun.com/nIFDLOX0)                          |                         [MiniMap2](https://share.weiyun.com/9K4Pmtmv)                          |                         [OpenPiton](https://share.weiyun.com/RVxP3RX8)                          |                         [MemN2N](https://share.weiyun.com/MDolGB8H)                          |                         [BLSTM](https://share.weiyun.com/HzfUK7do)                          |                         [Rosetta DigitRecog](https://share.weiyun.com/rbJhbFvn)                          |



Users can directly open the .xpr file with Vivado. There might be some warnings because your Vivado version is not matched with the IP cores in the design or some IP core instances cannot be found in your system. Please ignore them and do not update the ip core or re-synthesis because these operations will change the netlist. Besides, if users use the benchmarks in their works, please cite the papers of the related designs and comply with their open-source licence conditions according to (@subpage _3_1_BenchmarksDetails).


<center>
<img src="errors.png" align="center" alt="openImpled errors" title="openImpled errors" width="300" /> 
</center>


For the latest results of our timing-driven placement, we have prepared Vivado placement checkpoint and our post-AMF-placement checkpoint with various configuration and Vivado versions in [Google Drive](https://drive.google.com/drive/folders/1hEo9_n9WjYeoUC_lRI71KXp6jStc4iRn?usp=sharing) and [Tencent Weiyun Disk](https://share.weiyun.com/w50lBmqZ)

1. Cfg0: all the optimization techniques are enabled as the configuration.
2. Cfg1: disable path-length-aware clustering before partitioning
3. Cfg2: disable blockage-aware spreading and anchor insertion
4. Cfg3: disable WNS-aware timing criticality pseudo net weight
5. Cfg4: disable path-length-aware parallel packing
6. Cfg5: disable sector-guided site candidate selection and just use the original small square window
7. Cfg6: disable sector-guided site candidate selection and simply enlarge the square window