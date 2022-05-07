/**
 * @file GlobalPlacer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of GlobalPlacer class and its internal modules and APIs which
 * organize/configure other modules to handle global placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _GLOBALPLACER
#define _GLOBALPLACER

#include "CLBLegalizer.h"
#include "ClusterPlacer.h"
#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "Eigen/SparseCore"
#include "GeneralSpreader.h"
#include "MacroLegalizer.h"
#include "PlacementInfo.h"
#include "PlacementTimingOptimizer.h"
#include "WirelengthOptimizer.h"
#include "dumpZip.h"
#include "osqp++/osqp++.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief GlobalPlacer accounts for the general iterations in global placement
 *
 * including wirelength optimization, cell spreading, legalization, area adjustion...
 *
 */
class GlobalPlacer
{
  public:
    /**
     * @brief Construct a new Global Placer object based on placement information
     *
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg the user-defined placement configuration
     * @param resetLegalizationInfo
     */
    GlobalPlacer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                 bool resetLegalizationInfo = true);
    ~GlobalPlacer()
    {
        if (clusterPlacer)
            delete clusterPlacer;
        if (WLOptimizer)
            delete WLOptimizer;
        if (generalSpreader)
            delete generalSpreader;
        if (BRAMDSPLegalizer)
            delete BRAMDSPLegalizer;
        if (CARRYMacroLegalizer)
            delete CARRYMacroLegalizer;
        if (mCLBLegalizer)
            delete mCLBLegalizer;
        if (lCLBLegalizer)
            delete lCLBLegalizer;
    }

    /**
     * @brief cluster elements and conduct initial placement at coarse-grained level
     *
     */
    void clusterPlacement();

    /**
     * @brief wirelength optimization + cell spreading + legalization + area adjustion
     *
     * @param iterNum the current placement iteration
     * @param continuePreviousIteration whether we should use the information from previous iterations
     * @param lowerBoundIterNum the iteration number for the lower-bound wirelength optimization
     * @param enableMacroPseudoNet2Site enable pseudo nets for macro legalization
     * @param stopStrictly stop strictly even the placement wirelength metrics are not converged.
     * @param spreadRegionBinNumLimit the maximum number of bins in a spreadRegion
     * @param timingOptimizer the pointer of the timing optimizer (if nullptr, timing optimization will be disabled)
     */
    void GlobalPlacement_CLBElements(int iterNum, bool continuePreviousIteration = false, int lowerBoundIterNum = 6,
                                     bool enableMacroPseudoNet2Site = false, bool stopStrictly = false,
                                     unsigned int spreadRegionBinNumLimit = 10000000,
                                     PlacementTimingOptimizer *timingOptimizer = nullptr);

    /**
     * @brief fix the locations of CLB and let macros move without low pseudo net at initial stages, e.g., just after
     * initial placement
     *
     * @param iterNum current iteration number of global placement
     * @param pseudoNetWeight given pseudo net weight for non-CLB elements
     */
    void GlobalPlacement_fixedCLB(int iterNum, float pseudoNetWeight);

    /**
     * @brief Set the initial pseudo net weight for the global placer
     *
     * please note that this pseudo net will be updated according to the placement progress
     *
     * @param weight
     */
    inline void setPseudoNetWeight(float weight)
    {
        oriPseudoNetWeight = weight;
    }

    /**
     * @brief Get the current pseudo net weight
     *
     * usually we can record the current pseudo net weight for later placement-related procedure
     *
     * @return float
     */
    inline float getPseudoNetWeight()
    {
        return oriPseudoNetWeight;
    }

    /**
     * @brief Get the number of macro pseudo net enhancements
     *
     * the macro legalization pseudo net will be enhanced as the number of previous enhancements is increased.
     *
     * @return int
     */
    inline int getMacroPseudoNetEnhanceCnt()
    {
        return WLOptimizer->getMacroPseudoNetEnhanceCnt();
    }

    /**
     * @brief Get the current macro legalization weight object
     *
     * @return float
     */
    inline float getMacroLegalizationWeight()
    {
        return WLOptimizer->getMacroLegalizationWeight();
    }

    /**
     * @brief Set the macro legalization parameters (including legalization counter and pseudo net weight)
     *
     * @param cnt the number of previous macro pseudo net enhancements
     * @param macroLegalizationWeight macro pseudo net weight
     */
    inline void setMacroLegalizationParameters(int cnt, float macroLegalizationWeight)
    {
        WLOptimizer->setMacroLegalizationParameters(cnt, macroLegalizationWeight);
    }

    /**
     * @brief fix the macro legalization result
     *
     */
    inline void setMacroLegalizationFixed()
    {
        macroLegalizationFixed = true;
    }

    /**
     * @brief Set the distance upperbound of neighbor LUT/FF-like elements for area adjustion
     *
     * @param _threshold the distance upperbound
     */
    inline void setNeighborDisplacementUpperbound(float _threshold)
    {
        neighborDisplacementUpperbound = _threshold;
    }

    /**
     * @brief cell spreading for all types of elements
     *
     * we make it public since sometime we allow external functions to call cell spreading for flexible improvement
     *
     * @param currentIteration the current global placement iteration in this round
     * @param spreadRegionSizeLimit the maximum number of bins in a spreadRegion
     * @param displacementLimit
     */
    void spreading(int currentIteration, int spreadRegionSizeLimit = 100000000, float displacementLimit = -10);

    inline WirelengthOptimizer *getWirelengthOptimizer()
    {
        assert(WLOptimizer);
        return WLOptimizer;
    }

    int timingDrivenDetailedPlacement_shortestPath_intermediate(PlacementTimingOptimizer *timingOptimizer);

  private:
    PlacementInfo *placementInfo;

    // settings
    std::map<std::string, std::string> &JSONCfg;
    bool verbose = false;
    bool dumpOptTrace = false;
    float y2xRatio = 1.0;

    /**
     * @brief set adaptive pseudo net weight according to net density
     *
     * we assign higher weight for the pseudo nets between anchors and elements which connects to more nets
     *
     */
    bool pseudoNetWeightConsiderNetNum = true;

    /**
     * @brief diable cell spreading forget ratio
     *
     * we use the forget ratio to control the sensitivity of cell spreading during the location updateing (newFinalLoc =
     * expectedLoc * forgetRatio + originalLoc * (1-forgetRatio)  )
     *
     */
    bool disableSpreadingForgetRatio = false;

    /**
     * @brief cluster placer is used for initial placement
     *
     */
    ClusterPlacer *clusterPlacer = nullptr;

    /**
     * @brief wirelength optimizer is based on numerial model and used to reduce the overall wirelength with various
     * terms, e.g., terms for wirelength, legalization and timing...
     *
     */
    WirelengthOptimizer *WLOptimizer = nullptr;

    /**
     * @brief a general spreading will spread cells to meet cell density requirements
     *
     */
    GeneralSpreader *generalSpreader = nullptr;

    /**
     * @brief legalize multi-site BRAM/DSP elements
     *
     */
    MacroLegalizer *BRAMDSPLegalizer = nullptr;

    /**
     * @brief legalize multi-site CARRY elements
     *
     */
    MacroLegalizer *CARRYMacroLegalizer = nullptr;

    /**
     * @brief legalize single-site SLICEM-CLB elements
     *
     */
    CLBLegalizer *mCLBLegalizer = nullptr;

    /**
     * @brief legalize single-site SLICEL-CLB elements
     *
     */
    CLBLegalizer *lCLBLegalizer = nullptr;

    /**
     * @brief update pseudo net weight according to placement progress
     *
     * @param pseudoNetWeight current pseudo net weight for final convergence
     * @param curIter current iteration in the placement procedure
     */
    void updatePseudoNetWeight(float &pseudoNetWeight, int curIter);

    /**
     * @brief legalize specific types of macro to the target regions
     *
     * @param curIteration
     * @param timingDriven disable the evaluation of HPWL change but use check displacement change
     */
    void macroLegalize(int curIteration, bool timingDriven = false,
                       PlacementTimingOptimizer *timingOptimizer = nullptr);

    void printPlacedUnits(std::ostream &os);
    void dumpLUTCoordinate();
    void dumpDSPCoordinate(bool enforced = false);
    void dumpBRAMCoordinate(bool enforced = false);
    void dumpFFCoordinate();
    void dumpLUTFFCoordinate(bool enforced = false);
    void dumpAllCellsCoordinate();
    void dumpCARRYCoordinate();
    void dumpCoord();
    inline float random_float(float min, float max)
    {
        return ((float)random() / RAND_MAX) * (max - min) + min;
    }

    /*
     * some counters indicating the numbers of the dumped log and naming them accordingly
     *
     */

    int LUTCoordinateDumpCnt = 0;
    int DSPCoordinateDumpCnt = 0;
    int BRAMCoordinateDumpCnt = 0;
    int FFCoordinateDumpCnt = 0;
    int LUTFFCoordinateDumpCnt = 0;
    int CARRYCoordinateDumpCnt = 0;
    int allCoordinateDumpCnt = 0;

    /**
     * @brief we use the forget ratio to control the sensitivity of cell spreading during the location updateing
     * (newFinalLoc = expectedLoc * forgetRatio + originalLoc * (1-forgetRatio)  )
     *
     */
    float spreadingForgetRatio = 0.2;

    /**
     * @brief the similarity between upperbound wirelength and lowerbound wirelength
     *
     */
    float progressRatio = 0.0;

    /**
     * @brief record the legalization displacement of DSP/BRAM
     *
     */
    float averageMacroLegalDisplacement = 100000;

    /**
     * @brief record the legalization displacement of CARRY
     *
     */
    float averageCarryLegalDisplacement = 100000;

    /**
     * @brief record the legalization displacement of SLICEM-Macro
     *
     */
    float averageMCLBLegalDisplacement = 100000;

    /**
     * @brief record the change history of HPWL
     *
     */
    std::deque<float> historyHPWLs;
    float minHPWL;

    /**
     * @brief upperbound HPWL is obtained just after cell spreading and legalization
     *
     */
    float upperBoundHPWL;

    /**
     * @brief lowerbound HPWL is obtained just after wirelength optimization
     *
     */
    float lowerBoundHPWL;

    /**
     * @brief when legalization is started, the HPWL might have a significant change due to the legalization
     * displacement/pseudo nets
     *
     */
    bool updateMinHPWLAfterLegalization = false;

    /**
     * @brief the small change of minHPWL might imply that the optimization converges.
     *
     */
    bool HPWLChangeLittle = false;

    /**
     * @brief macros are binded to sites and such binding will lead to pseudo nets between anchors and elements
     *
     */
    bool macrosBindedToSites = false;

    /**
     * @brief macros are located closely to their legalized locations
     *
     */
    bool macroCloseToSite = false;

    /**
     * @brief lock the macro binding and bypass serveral legalizaiton iterations for wirelength stable convergence
     *
     */
    bool macroLocked = false;

    /**
     * @brief fix the macros to the exact legal location
     *
     */
    bool macroLegalizationFixed = false;

    /**
     * @brief directly legalize macros without the phase of rough legalization (each standard cell in a macro will be
     * legalized separately)
     *
     */
    bool directMacroLegalize = false;

    bool dumpClockUtilization = false;

    bool DSPCritical = false;

    int macroLockedIterCnt = 0;
    std::deque<float> historyAverageDisplacement;

    /**
     * @brief record the pseudo net weight at the end of placement functions for later iterations. This information can
     * be recorded in PlacementInfo for checkpoint dumping
     *
     */
    float oriPseudoNetWeight = -1.0;

    /**
     * @brief the pseudo net weight in the iterations of placement functions
     *
     */
    float pseudoNetWeight = 1.0;

    /**
     * @brief the distance threshold for the neighbor detection during area adjustion
     *
     */
    float neighborDisplacementUpperbound = -1.0;

    bool hasUserDefinedClusterInfo = false;
    bool enableClockRegionAware = false;
    bool timingOptEnabled = false;
    bool printHPWL = false;
};

#endif