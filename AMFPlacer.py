import sys
import multiprocessing as mp
import logging
import os
import argparse
# for consistency between python2 and python3
root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if root_dir not in sys.path:
    sys.path.append(root_dir)
from amfplacer.utils.utils_cpp import parseJSONFile, setOMPThread, PaintDataBase, print_info, print_status, plotPlacement
from amfplacer.HiFPlacer.designInfo.designInfo_cpp import DesignInfo
from amfplacer.HiFPlacer.deviceInfo.deviceInfo_cpp import DeviceInfo
from amfplacer.HiFPlacer.placement.placementInfo.placementInfo_cpp import PlacementInfo
from amfplacer.HiFPlacer.placement.packing.packing_cpp import InitialPacker, ParallelCLBPacker, IncrementalBELPacker
from amfplacer.HiFPlacer.placement.placementTiming.placementTiming_cpp import PlacementTimingOptimizer
from amfplacer.HiFPlacer.placement.globalPlacement.globalPlacement_cpp import GlobalPlacer, WirelengthOptimizer
#from amfplacer.utils.utils_cpp import setOMPThread

logger = logging.getLogger(__name__)
                                    
class AMFPlacer:
    PlacementInfo = None
    '''
    @brief information related to the device (BELs, Sites, Tiles, Clock Regions)
    '''
    deviceInfo = None

    ''' 
    @brief information related to the design (cells, pins and nets)
    '''
    designInfo = None

    '''
     * @brief inforamtion related to placement (locations, interconnections, status, constraints, legalization)
     *
     '''
    placementInfo = None

    '''
     * @brief initially packing for macro extraction based on pre-defined rules
     *
     '''
    initialPacker = None

    '''
     * @brief incremental pairing of some FFs and LUTs into small macros
     *
     '''
    incrementalBELPacker = None

    '''
     * @brief global placer acconting for initial placement, quadratic placement, cell spreading and macro legalization.
     *
     '''
    globalPlacer = None

    '''
     * @brief final packing of instances into CLB sites
     *
     '''
    parallelCLBPacker = None

    '''
     * @brief the user-defined settings of placement
     *
     '''
    JSON = None
    def __init__(self, args):
        JSONFileName = args.config
        self.JSON = parseJSONFile(JSONFileName)
        assert(self.JSON.get("vivado extracted device information file", None) != None)
        assert(self.JSON.get("special pin offset info file", None) != None)
        assert(self.JSON.get("vivado extracted design information file", None) != None)
        assert(self.JSON.get("cellType2fixedAmo file", None) != None)
        assert(self.JSON.get("cellType2sharedCellType file", None) != None)
        assert(self.JSON.get("sharedCellType2BELtype file", None) != None)
        assert(self.JSON.get("GlobalPlacementIteration", None) != None)
        assert(self.JSON.get("dumpDirectory", None) != None)


        if not os.path.exists(self.JSON["dumpDirectory"]):
            try:
                os.makedirs(self.JSON["dumpDirectory"])
            except Exception as e:
                logging.error(e)

        #setOMPThread(self.JSON["jobs"])
        setOMPThread(1)

        self.deviceInfo = DeviceInfo(self.JSON, "VCU108")
        self.deviceInfo.printStat(False)

        self.designInfo = DesignInfo(self.JSON, self.deviceInfo)
        #self.designInfo.fromStringToCellType()

        self.paintData = PaintDataBase()
        
    def plot(self):
        plotPlacement(self.paintData)
        print_info("create plot thread successfully.")

    def place(self):
        # initialize placement information, including how to map cells to BELs
        self.placementInfo = PlacementInfo(self.designInfo, self.deviceInfo, self.JSON)
        self.placementInfo.setPaintDataBase(self.paintData)
        # we have to pack cells in design info into placement units in placement info with packer
        self.initialPacker = InitialPacker(self.designInfo, self.deviceInfo, self.placementInfo, self.JSON)
        self.initialPacker.pack()
        self.placementInfo.resetLUTFFDeterminedOccupation()

        self.placementInfo.printStat(False)
        self.placementInfo.createGridBins(5.0, 5.0) 
        self.placementInfo.verifyDeviceForDesign()

        self.placementInfo.buildSimpleTimingGraph()

        self.timingOptimizer = PlacementTimingOptimizer(self.placementInfo, self.JSON)
        longPathThr = self.placementInfo.getLongPathThresholdLevel()
        mediumPathThr = self.placementInfo.getMediumPathThresholdLevel()

        # go through several glable placement iterations to get initial placement
        self.globalPlacer = GlobalPlacer(self.placementInfo, self.JSON, True)

        # enable the timing optimization, start initial placement and global placement.
        self.globalPlacer.clusterPlacement()
        self.timingOptimizer.clusterLongPathInOneClockRegion(longPathThr, 0.5)
        self.globalPlacer.GlobalPlacement_fixedCLB(1, 0.0002)

        self.placementInfo.setDSPInnerDelay()
        self.globalPlacer.GlobalPlacement_CLBElements(int(int(self.JSON["GlobalPlacementIteration"]) / 3), False, 5, True, True, \
                                                  200, self.timingOptimizer)

        ## -----------------
        self.timingOptimizer.clusterLongPathInOneClockRegion(longPathThr, 0.5)

        self.globalPlacer.setPseudoNetWeight(self.globalPlacer.getPseudoNetWeight() * 0.85)
        self.globalPlacer.setMacroLegalizationParameters(int(self.globalPlacer.getMacroPseudoNetEnhanceCnt() * 0.8),
                                                     self.globalPlacer.getMacroLegalizationWeight() * 0.8)
        self.placementInfo.createGridBins(2.5, 2.5)
        self.placementInfo.adjustLUTFFUtilization(-10, True)
        # self.globalPlacer.spreading(-1)
        self.globalPlacer.GlobalPlacement_CLBElements(int(int(self.JSON["GlobalPlacementIteration"]) * 2 / 9), True, 5, True,
                                                  True, 200, self.timingOptimizer)
        self.placementInfo.clearPU2ClockRegionCenters()
        print_info("Current Total HPWL = " + str(self.placementInfo.updateB2BAndGetTotalHPWL()))

        # pack simple LUT-FF pairs and go through several global placement iterations
        self.incrementalBELPacker = IncrementalBELPacker(self.designInfo, self.deviceInfo, self.placementInfo, self.JSON)
        self.incrementalBELPacker.LUTFFPairing(4.0)
        self.incrementalBELPacker.FFPairing(4.0)
        self.placementInfo.printStat(False)
        print_info("Current Total HPWL = " + str(self.placementInfo.updateB2BAndGetTotalHPWL()))

        self.timingOptimizer.clusterLongPathInOneClockRegion(longPathThr, 0.5)

        self.globalPlacer.setPseudoNetWeight(self.globalPlacer.getPseudoNetWeight() * 0.85)
        self.globalPlacer.setMacroLegalizationParameters(int(self.globalPlacer.getMacroPseudoNetEnhanceCnt() * 0.8),
                                                     self.globalPlacer.getMacroLegalizationWeight() * 0.8)
        self.globalPlacer.setNeighborDisplacementUpperbound(3.0)

        self.globalPlacer.GlobalPlacement_CLBElements(int(int(self.JSON["GlobalPlacementIteration"]) * 2 / 9), True, 5, True,
                                                  True, 25, self.timingOptimizer)
        # self.placementInfo.clearPU2ClockRegionCenters

        self.globalPlacer.setPseudoNetWeight(self.globalPlacer.getPseudoNetWeight() * 0.9)
        self.globalPlacer.setMacroLegalizationParameters(int(self.globalPlacer.getMacroPseudoNetEnhanceCnt() * 0.9),
                                                     self.globalPlacer.getMacroLegalizationWeight() * 0.9)
        self.placementInfo.createGridBins(2, 2)
        self.placementInfo.adjustLUTFFUtilization(-10, True)
        # self.placementInfo.getself.designInfo().resetNetEnhanceRatio()
        # self.timingOptimizer.enhanceNetWeight_LevelBased(mediumPathThr)
        self.globalPlacer.setNeighborDisplacementUpperbound(2.0)

        # self.timingOptimizer.moveDriverIntoBetterClockRegion(longPathThr, 0.75)
        self.globalPlacer.GlobalPlacement_CLBElements(int(int(self.JSON["GlobalPlacementIteration"]) * 2 / 9), True, 5, True,
                                                  True, 25, self.timingOptimizer)
        self.JSON["SpreaderSimpleExpland"] = "True"
        # self.placementInfo.clearPU2ClockRegionCenters
        self.globalPlacer.GlobalPlacement_CLBElements(int(int(self.JSON["GlobalPlacementIteration"]) / 2), True, 5, True, False,
                                                  25, self.timingOptimizer)

        # # currently, some fixed/packed flag cannot be stored in the check-point (TODO)
        # clearSomeAttributesCannotRecord()

        # # test the check-point mechanism
        # self.placementInfo.dumpPlacementUnitInformation(self.JSON["dumpDirectory"] + "/PUInfoBeforeFinalPacking")
        # self.placementInfo.loadPlacementUnitInformation(self.JSON["dumpDirectory"] + "/PUInfoBeforeFinalPacking.gz")
        # print_info("Current Total HPWL = " + str(self.placementInfo.updateB2BAndGetTotalHPWL()))

        self.timingOptimizer.conductStaticTimingAnalysis(False)
        # finally pack the elements into sites on the FPGA device

        self.parallelCLBPacker = ParallelCLBPacker(self.designInfo, self.deviceInfo, self.placementInfo, self.JSON, 3, 10, 0.25, 0.5, 6, 10, 0.02, "first",
                                  self.timingOptimizer, self.globalPlacer.getWirelengthOptimizer())
        #self.parallelCLBPacker = ParallelCLBPacker(self.designInfo, self.deviceInfo, self.placementInfo, self.JSON, 3, 10, 0.25, 0.5, 6, 10, 0.02, "first",
        #                          self.timingOptimizer, self.globalPlacer)
        self.parallelCLBPacker.packCLBs(30, True, False)
        self.parallelCLBPacker.setPULocationToPackedSite()
        self.timingOptimizer.conductStaticTimingAnalysis(False)
        self.placementInfo.checkClockUtilization(True)
        print_info("Current Total HPWL = " + str(self.placementInfo.updateB2BAndGetTotalHPWL()))
        self.placementInfo.resetLUTFFDeterminedOccupation()
        self.parallelCLBPacker.updatePackedMacro(True, True)
        self.placementInfo.dumpOverflowClockUtilization()
        self.placementInfo.adjustLUTFFUtilization(1, True)
        self.placementInfo.dumpCongestion(self.JSON["dumpDirectory"] + "/congestionInfo")

        #if (parallelCLBPacker)
        #    delete parallelCLBPacker

        # currently, some fixed/packed flag cannot be stored in the check-point (TODO)
        self.placementInfo.clearSomeAttributesCannotRecord()
        self.placementInfo.dumpPlacementUnitInformation(self.JSON["dumpDirectory"] + "/PUInfoFinal")
        self.placementInfo.checkClockUtilization(True)

        print_status("Placement Done")
        print_info("Current Total HPWL = " + str(self.placementInfo.updateB2BAndGetTotalHPWL())) 
    
    def run(self):
        place = mp.Process(target=self.place)
        if self.JSON.get("guiEnable", False):
            plot = mp.Process(target=self.plot)
            plot.start()
            place.start()
            plot.join()
            place.join()
        else:
            place.start()
            place.join()





if "__main__" == __name__:

    try:
        parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                         description='EAMF-Placer: A FPGA Placement Framework.')
        parser.add_argument('--config', default="./benchmarks/testConfig/OpenPition.json", type=str, required=True, help='path the parameter json file')
        parser.add_argument('--guiEnable', default=False, type=bool, help='plot placement layout in real time')
        args = parser.parse_args()
        placer = AMFPlacer(args)
        if args.guiEnable:
            placer.JSON["guiEnable"] = "true"
        placer.run()

    except Exception as e: 
        print(e)
    
        
