/**
 * @file MacroLegalizer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of MacroLegalizer class and its internal modules and APIs which
 * map DSP/BRAM/CARRY macros to legal location
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _MACROLEGALIZER
#define _MACROLEGALIZER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "MinCostBipartiteMatcher.h"
#include "PlacementInfo.h"
#include "dumpZip.h"
#include "sysInfo.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <omp.h>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief MacroLegalizer maps DSP/BRAM/CARRY macros to legal location.
 *
 * (RAMB36E2 will be treated as 1+1 RAMB18E2, one of which is a virtual cell)
 *
 * The legalization procedure will only conduct rough legalization in the early iterations of global placement, and it
 * will conduct exact legalization following rough legalization when the macros are close enough to their potential
 * legal positions.
 */
class MacroLegalizer
{
  public:
    /**
     * @brief Construct a new MacroLegalizer object
     *
     * @param legalizerName the name string of legalizer for log dumping
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param deviceInfo device information
     * @param macroTypesToLegalize a vector of Cell Type string indicating the target types handled by this
     * MacroLegalizer
     * @param JSONCfg  the user-defined placement configuration
     */
    MacroLegalizer(std::string legalizerName, PlacementInfo *placementInfo, DeviceInfo *deviceInfo,
                   std::vector<DesignInfo::DesignCellType> &macroTypesToLegalize,
                   std::map<std::string, std::string> &JSONCfg);
    ~MacroLegalizer()
    {
        if (minCostBipartiteMatcher)
            delete minCostBipartiteMatcher;
    }

    /**
     * @brief conduct legalization and map the PlacementUnit of one of the given types to sites
     *
     * @param exactLegalization true to ensure elements in a macro are consecutive
     * @param directLegalization direct legalize the macros without rough legalization phase
     */
    void legalize(bool exactLegalization = false, bool directLegalization = false);

    /**
     * @brief Get the average displacement of exact legalization for the involved PlacementUnit
     *
     * Exact legalization ensures elements in a macro are placed consecutively.
     *
     * @return float
     */
    inline float getAverageDisplacementOfExactLegalization()
    {
        if (finalAverageDisplacement > 1000)
            return finalAverageDisplacement;
        float tmpAverageDisplacement = 0.0;
        for (auto PUSitePair : PULevelMatching)
        {
            tmpAverageDisplacement += std::fabs(PUSitePair.first->X() - PUSitePair.second->X()) +
                                      std::fabs(PUSitePair.first->Y() - PUSitePair.second->Y());
        }
        tmpAverageDisplacement /= PULevelMatching.size();

        return tmpAverageDisplacement;
    }

    /**
     * @brief Get the average displacement of rough legalization for the involved PlacementUnit
     *
     * Rough legalization does not guarantee that elements in a macro are placed consecutively.
     *
     * @return float
     */
    inline float getAverageDisplacementOfRoughLegalization()
    {
        return roughAverageDisplacement;
    }

    void dumpMatching(bool fixedColumn = false, bool enforce = false);

    /**
     * @brief Set the intitial parameters of the legalizer
     *
     * @param displacementThr displacement threshold to detect potential legal sites
     * @param candidateNum the maximum number of final candidate sites
     * @param _candidateFactor we are allowed to detect a excessive number (>candidateNum) of initial candidates
     */
    void setIntitialParameters(float displacementThr, int candidateNum, int _candidateFactor = -1)
    {
        initialDisplacementThreshold = displacementThr;
        initialMaxNumCandidate = candidateNum;
        if (_candidateFactor > 1)
        {
            candidateFactor = _candidateFactor;
        }
    }

    /**
     * @brief reset the mapped flag of the involved sites.
     *
     * A mapped site will not be binded to another PlacementUnit.
     *
     */
    void resetSitesMapped();

    void setClockRegionAware(bool _clockRegionAware)
    {
        clockRegionAware = _clockRegionAware;
    }

    void setClockRegionCasLegalization(bool _clockRegionCasLegalization)
    {
        clockRegionCasLegalization = _clockRegionCasLegalization;
    }

  private:
    std::string legalizerName;
    PlacementInfo *placementInfo;
    DeviceInfo *deviceInfo;

    /**
     * @brief compatiblePlacementTable describes the type mapping from design to device, where a cell can be placed
     * (which BEL in which site)
     *
     */
    PlacementInfo::CompatiblePlacementTable *compatiblePlacementTable;

    /**
     * @brief a vector of Cell Type string indicating the target types handled by this MacroLegalizer
     *
     */
    std::vector<DesignInfo::DesignCellType> macroTypesToLegalize;

    /**
     * @brief a reference of the locations of cells (in cellId order)
     *
     */
    std::vector<PlacementInfo::Location> &cellLoc;

    std::map<std::string, std::string> &JSONCfg;

    /**
     * @brief min-cost bipartite matching solver for the legalization
     *
     */
    MinCostBipartiteMatcher *minCostBipartiteMatcher = nullptr;

    /**
     * @brief a vector storing the Design cells which have NOT been legalized
     *
     */
    std::vector<DesignInfo::DesignCell *> macroCellsToLegalize;

    /**
     * @brief a vector storing the cells in macros which SHOULD be legalized
     *
     */
    std::vector<DesignInfo::DesignCell *> initialMacrosToLegalize;

    /**
     * @brief a set storing the macros which have NOT been legalized
     *
     */
    std::set<PlacementInfo::PlacementUnit *> macroUnitsToLegalizeSet;

    /**
     * @brief a map record the potential sites of different site types
     *
     */
    std::map<DesignInfo::DesignCellType, std::vector<DeviceInfo::DeviceSite *>> macroType2Sites;

    /**
     * @brief record the mapping from cells to the candidate sites which are NOT binded to other cells
     *
     * Please be aware that a cell might be binded of multiple sites.
     *
     */
    std::map<DesignInfo::DesignCell *, std::vector<DeviceInfo::DeviceSite *>> macro2Sites;

    /**
     * @brief a cache record the candidate sites within a given displacement threshold for each cell in the macros
     *
     */
    std::map<DesignInfo::DesignCell *, std::vector<DeviceInfo::DeviceSite *> *> macro2SitesInDisplacementThreshold;

    /**
     * @brief map sites to temperary indexes for bipartite matching
     *
     */
    std::map<DeviceInfo::DeviceSite *, int> rightSiteIds;

    /**
     * @brief a vector for the candidate sites for bipartite matching
     *
     */
    std::vector<DeviceInfo::DeviceSite *> siteList;

    /**
     * @brief the adjacent list of the bipartite graph
     *
     */
    std::vector<std::vector<std::pair<int, float>>> adjList;

    /**
     * @brief a set of cells in macros binded to corresponding DeviceSites
     *
     */
    std::set<DesignInfo::DesignCell *> matchedMacroCells;

    /**
     * @brief a set of DeviceSites binded to corresponding PlacementUnits
     *
     */
    std::set<DeviceInfo::DeviceSite *> matchedSites;

    /**
     * @brief record the binding between design standard cells and DeviceSites as a vector of pairs
     *
     */
    std::vector<std::pair<DesignInfo::DesignCell *, DeviceInfo::DeviceSite *>> cellLevelMatching;

    /**
     * @brief record the binding between PlacementUnits and DeviceSites as a vector of pairs
     *
     */
    std::vector<std::pair<PlacementInfo::PlacementUnit *, DeviceInfo::DeviceSite *>> PULevelMatching;

    int DumpMacroLegalizationCnt = 0;

    /**
     * @brief displacement threshold to detect potential legal sites
     *
     */
    float displacementThreshold = 30;

    /**
     * @brief the maximum number of final candidate sites
     *
     */
    int maxNumCandidate = 30;

    /**
     * @brief the number of BRAM columns on the target device
     *
     */
    int BRAMColumnNum = -1;

    /**
     * @brief the number of DSP columns on the target device
     *
     */
    int DSPColumnNum = -1;

    /**
     * @brief the number of CARRY columns on the target device
     *
     */
    int CARRYColumnNum = -1;

    /**
     * @brief the number of BRAM rows on the target device
     *
     */
    int BRAMRowNum = -1;

    /**
     * @brief the number of DSP rows on the target device
     *
     */
    int DSPRowNum = -1;

    /**
     * @brief the number of CARRY rows on the target device
     *
     */
    int CARRYRowNum = -1;

    /**
     * @brief the floating-point X location of the BRAM columns on the device
     *
     */
    std::vector<float> BRAMColumnXs;

    /**
     * @brief the floating-point X location of the DSP columns on the device
     *
     */
    std::vector<float> DSPColumnXs;

    /**
     * @brief the floating-point X location of the CARRY columns on the device
     *
     */
    std::vector<float> CARRYColumnXs;

    /**
     * @brief record the sites in each column of BRAM
     *
     */
    std::vector<std::vector<DeviceInfo::DeviceSite *>> BRAMColumn2Sites;

    /**
     * @brief record the sites in each column of DSP
     *
     */
    std::vector<std::vector<DeviceInfo::DeviceSite *>> DSPColumn2Sites;

    /**
     * @brief record the sites in each column of CARRY
     *
     */
    std::vector<std::vector<DeviceInfo::DeviceSite *>> CARRYColumn2Sites;

    /**
     * @brief record the PlacementUnits in each column of BRAM Sites
     *
     */
    std::vector<std::deque<PlacementInfo::PlacementUnit *>> BRAMColumn2PUs;

    /**
     * @brief record the PlacementUnits in each column of DSP Sites
     *
     */
    std::vector<std::deque<PlacementInfo::PlacementUnit *>> DSPColumn2PUs;

    /**
     * @brief record the PlacementUnits in each column of CARRY
     *
     */
    std::vector<std::deque<PlacementInfo::PlacementUnit *>> CARRYColumn2PUs;

    /**
     * @brief record the number of cells (Macro contains multiple cells) in each column for BRAM
     *
     */
    std::vector<int> BRAMColumnUntilization;

    /**
     * @brief record the number of cells (Macro contains multiple cells) in each column for DSP
     *
     */
    std::vector<int> DSPColumnUntilization;

    /**
     * @brief record the number of cells (Macro contains multiple cells) in each column for CARRY
     *
     */
    std::vector<int> CARRYColumnUntilization;

    /**
     * @brief record the PlacementUnits in each column of BRAM site
     *
     */
    std::map<DesignInfo::DesignCell *, int> BRAMCell2Column;

    /**
     * @brief record the PlacementUnits in each column of DSP site
     *
     */
    std::map<DesignInfo::DesignCell *, int> DSPCell2Column;

    /**
     * @brief record the PlacementUnits in each column of CARRY site
     *
     */
    std::map<DesignInfo::DesignCell *, int> CARRYCell2Column;

    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> PU2LegalSites;

    /**
     * @brief record the mapping from PlacementUnits to exact DeviceSite location X
     *
     */
    std::map<PlacementInfo::PlacementUnit *, float> PU2X;

    /**
     * @brief record the mapping from PlacementUnits to exact DeviceSite location Y
     *
     */
    std::map<PlacementInfo::PlacementUnit *, float> PU2Y;

    /**
     * @brief record the exact site X (column id) of involved PlacementUnits
     *
     * unused currently and just for debugging
     *
     */
    std::map<PlacementInfo::PlacementUnit *, int> PU2SiteX;

    /**
     * @brief record the column id for the binded cells in involved PlacementUnits
     *
     * i.e., if a PlacementUnit is PlacementMacro, the cells in it might be allowed to bind to different columns during
     * rough legalization.
     *
     */
    std::map<PlacementInfo::PlacementUnit *, std::vector<int>> PU2Columns;

    /**
     * @brief the PlacementUnits which shoudl be mapped to BRAM site
     *
     */
    std::set<PlacementInfo::PlacementUnit *> BRAMPUs;

    /**
     * @brief the PlacementUnits which shoudl be mapped to DSP site
     *
     */
    std::set<PlacementInfo::PlacementUnit *> DSPPUs;

    /**
     * @brief the PlacementUnits which shoudl be mapped to CARRY BEL
     *
     */
    std::set<PlacementInfo::PlacementUnit *> CARRYPUs;

    bool enableBRAMLegalization = false;
    bool enableDSPLegalization = false;
    bool enableCARRYLegalization = false;
    bool verbose = false;
    float y2xRatio = 1.0;
    bool clockRegionAware = false;
    bool clockRegionCasLegalization = false;
    int clockRegionHeightOfDSE_BRAM = 24;

    /**
     * @brief the average displacement of exact legalization for the involved PlacementUnit
     *
     */
    float finalAverageDisplacement = 10000.0;

    /**
     * @brief the average displacement of fixed column (but not exactly consective) legalization for the involved
     * PlacementUnit
     *
     * During the fixed column legalization, cells in a macro will be constrainted on one column.
     *
     */
    float fixedColumnAverageDisplacement = 10000.0;

    /**
     * @brief the average displacement of rough legalization for the involved PlacementUnit
     *
     */
    float roughAverageDisplacement = 10000.0;

    /**
     * @brief displacement threshold to detect potential legal sites
     *
     */
    float initialDisplacementThreshold = 30;

    /**
     * @brief the maximum number of final candidate sites
     *
     */
    int initialMaxNumCandidate = 30;

    /**
     * @brief the number of the parallel multi-threading workers to handle the legalization problems
     *
     */
    int nJobs = 1;

    /**
     * @brief we are allowed to detect a excessive number (>candidateNum) of initial candidates. candidateFactor is to
     * control the excessive ratio.
     *
     */
    int candidateFactor = 5;

    /**
     * @brief get the PlacementMacro(s) which SHOULD be legalized
     *
     */
    void getMacrosToLegalize();

    /**
     * @brief find available sites for each specific macro type required by the constructor
     *
     */
    void findMacroType2AvailableSites();

    /**
     * @brief resolve the overflow columns during fixed column legalization by spreading "outliers" to neighbor columns
     *
     */
    void resolveOverflowColumns();

    /**
     * @brief find potential sites for each PlacementUnit
     *
     * @param fixedColumn true if we want to find potential sites for PlacementUnit in a given column
     */
    void findPossibleLegalLocation(bool fixedColumn = false);

    /**
     * @brief map the macros to the columns according to the locations of the cells in it
     *
     * @param directLegalization direct legalize the macros without rough legalization phase
     */
    void mapMacrosToColumns(bool directLegalization);

    /**
     * @brief find the closest column for a given location X
     *
     * @param curX given location X
     * @param Xs the location X for the resource columns
     * @return int
     */
    int findCorrespondingColumn(float curX, std::vector<float> &Xs);

    /**
     * @brief Create a bipartite graph between PlacementUnit and potential DeviceSites
     *
     */
    void createBipartiteGraph();

    /**
     * @brief conduct rough legalization.
     *
     * Rough legalization does not guarantee that elements in a macro are placed consecutively. During rough
     * legalization, each cell in a macro will be "legalized" individually as a general standard cell withouth the shape
     * constraints.
     *
     */
    void roughlyLegalize();

    /**
     * @brief conduct fixed-column legalization as a step in exact legalization. During fixed-column legalization, cells
     * in PlacementUnit (macro) can be only mapped to the same column.
     *
     * @param directLegalization direct legalize the macros without rough legalization phase
     */
    void fixedColumnLegalize(bool directLegalization);

    /**
     * @brief update the locations of the legalization anchors for the PlacementUnits.
     *
     * This function might be called more than one time during implementation so we have to specify the type of
     * legalization and whether we want to update the displacement value for the control of some optimizations.
     *
     * @param isRoughLegalization specify the type of legalization
     * @param updateDisplacement whether we want to update the displacement value for the control of some optimizations
     */
    void updatePUMatchingLocation(bool isRoughLegalization = true, bool updateDisplacement = true);

    /**
     * @brief finally dynamic programming to legalize the macros which have been mapped to the columns.
     *
     * This function will call DP function for each specific type of macros
     *
     */
    void finalLegalizeBasedOnDP();

    /**
     * @brief DP function for the legalization of a specific type of macros in the same column
     *
     * @param colNum total number of the column of the target type of PlacementUnit
     * @param Column2Sites a vector record the sites in the columns
     * @param Column2PUs  a vector record the macros in the columns
     * @return float
     */
    float DPForMinHPWL(int colNum, std::vector<std::vector<DeviceInfo::DeviceSite *>> &Column2Sites,
                       std::vector<std::deque<PlacementInfo::PlacementUnit *>> &Column2PUs);

    /**
     * @brief record the matching in private list and update the list of cells which are not matched by the
     * bi-partite matching
     *
     */
    void updateMatchingAndUnmatchedMacroCells();

    /**
     * @brief spread PlacementUnits accross columns to resolve resource overflow
     *
     * @param columnNum the number of columns
     * @param columnUntilization a vector reording the utilization usage of each column
     * @param column2Sites a vector reording device sites in each column
     * @param column2PUs a vector reording PlacementUnits in each column
     * @param cell2Column a map recording the column id for each PlacementUnit
     */
    void spreadMacros(int columnNum, std::vector<int> &columnUntilization,
                      std::vector<std::vector<DeviceInfo::DeviceSite *>> &column2Sites,
                      std::vector<std::deque<PlacementInfo::PlacementUnit *>> &column2PUs,
                      std::map<DesignInfo::DesignCell *, int> &cell2Column, float budgeRatio = 1);

    /**
     * @brief find the column which contains the most of cells in a macro in a specific range of columns
     *
     * @param minId the begin column
     * @param maxId the end column
     * @param ids the column ids of the cells in the macro
     * @return int
     */
    int findIdMaxWithRecurence(int minId, int maxId, std::vector<int> &ids);

    /**
     * @brief Set the sites which are binded as mapped so they will not be mapped to other elements in the netlist
     *
     */
    void setSitesMapped();

    /**
     * @brief find candidate sites for the cells left to be matched
     *
     */
    void findMacroCell2SitesInDistance(bool checkClockRegion)
    {
        macro2SitesInDisplacementThreshold.clear();

        int macrosNum = macroCellsToLegalize.size();

        for (int i = 0; i < macrosNum; i++)
        {
            macro2SitesInDisplacementThreshold[macroCellsToLegalize[i]] = nullptr;
        }

#pragma omp parallel for
        for (int i = 0; i < macrosNum; i++)
        {
            DesignInfo::DesignCell *curCell = macroCellsToLegalize[i];
            macro2SitesInDisplacementThreshold[curCell] = placementInfo->findNeiborSiteFromBinGrid(
                curCell, cellLoc[curCell->getCellId()].X, cellLoc[curCell->getCellId()].Y, displacementThreshold,
                candidateFactor * maxNumCandidate, checkClockRegion);
        }
    }

    /**
     * @brief clear the information of candidate sites for the cells left to be matched
     *
     */
    void resetMacroCell2SitesInDistance()
    {
        int macrosNum = macroCellsToLegalize.size();
        for (int i = 0; i < macrosNum; i++)
        {
            DesignInfo::DesignCell *curCell = macroCellsToLegalize[i];
            delete macro2SitesInDisplacementThreshold[curCell];
        }
    }

    /**
     * @brief clear the mapping information and reset the mapping parameters
     *
     */
    inline void resetSettings()
    {
        displacementThreshold = initialDisplacementThreshold;
        maxNumCandidate = initialMaxNumCandidate;
        matchedMacroCells.clear();
        matchedSites.clear();
        cellLevelMatching.clear();
        PULevelMatching.clear();
    }

    /**
     * @brief check how many sites are required by the given PlacementUnit
     *
     * @param tmpPUUnit  the given PlacementUnit
     * @return int
     */
    int getMarcroCellNum(PlacementInfo::PlacementUnit *tmpMacroUnit);

    inline void swapPU(PlacementInfo::PlacementUnit **A, PlacementInfo::PlacementUnit **B)
    {
        PlacementInfo::PlacementUnit *C = *A;
        *A = *B;
        *B = C;
    }

    void sortPUsByPU2Y(std::deque<PlacementInfo::PlacementUnit *> &PUs);

    void sortSitesBySiteY(std::vector<DeviceInfo::DeviceSite *> &sites);

    inline float getDisplacement(PlacementInfo::Location &macroLoc, DeviceInfo::DeviceSite *curSite)
    {
        return std::fabs(macroLoc.X - curSite->X()) + y2xRatio * std::fabs(macroLoc.Y - curSite->Y());
    }

    inline float getDisplacement(PlacementInfo::PlacementUnit *curPU, DeviceInfo::DeviceSite *curSite)
    {
        return std::fabs(curPU->X() - curSite->X()) + y2xRatio * std::fabs(curPU->Y() - curSite->Y());
    }

    /**
     * @brief get the HPWL change when the given DesignCell moves to the given DeviceSite
     *
     * @param curCell the given DesignCell
     * @param curSite  the given DeviceSite
     * @return float
     */
    inline float getHPWLChange(DesignInfo::DesignCell *curCell, DeviceInfo::DeviceSite *curSite)
    {
        float oriHPWL = 0.0;
        float newHPWL = 0.0;
        auto tmpPU = placementInfo->getPlacementUnitByCell(curCell);
        float PUX = 0.0, PUY = 0.0;
        auto nets = placementInfo->getPlacementUnitId2Nets()[tmpPU->getId()];
        float numCellsInMacro = 1.0;
        if (dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
        {
            PUX = curSite->X();
            PUY = curSite->Y();
        }
        else if (PlacementInfo::PlacementMacro *tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
        {

            PUX = curSite->X() - tmpMacro->getCellOffsetXInMacro(curCell);
            PUY = curSite->Y() - tmpMacro->getCellOffsetYInMacro(curCell);
            numCellsInMacro = tmpMacro->getCells().size();
        }

        for (auto curNet : nets)
        {
            if (curNet->getDesignNet()->getPins().size() > 1000) // it could be clock
                continue;
            oriHPWL += curNet->getHPWL(y2xRatio);
            newHPWL += curNet->getNewHPWLByTrying(tmpPU, PUX, PUY, y2xRatio);
        }
        return (newHPWL - oriHPWL) / numCellsInMacro;
        // return std::fabs(macroLoc.X - curSite->X()) + std::fabs(macroLoc.Y - curSite->Y());
        // placementInfo->getPlacementUnitByCell(curCell);
    }

    /**
     * @brief get the HPWL change when the given PlacementUnit moves to the given DeviceSite
     *
     * @param tmpPU the given PlacementUnit
     * @param curSite  the given DeviceSite
     * @return float
     */
    inline float getHPWLChange(PlacementInfo::PlacementUnit *tmpPU, DeviceInfo::DeviceSite *curSite)
    {
        float oriHPWL = 0.0;
        float newHPWL = 0.0;
        float PUX = 0.0, PUY = 0.0;
        auto nets = placementInfo->getPlacementUnitId2Nets()[tmpPU->getId()];

        PUX = curSite->X();
        PUY = curSite->Y();

        for (auto curNet : nets)
        {
            if (curNet->getDesignNet()->getPins().size() > 1000) // it could be clock
                continue;
            oriHPWL += curNet->getHPWL(y2xRatio);
            newHPWL += curNet->getNewHPWLByTrying(tmpPU, PUX, PUY, y2xRatio);
        }

        return (newHPWL - oriHPWL);
    }

    /**
     * @brief get the HPWL change when the given PlacementUnit moves to the given location
     *
     * @param tmpPU  the given PlacementUnit
     * @param PUX given location X
     * @param PUY given location Y
     * @return float
     */
    inline float getHPWLChange(PlacementInfo::PlacementUnit *tmpPU, float PUX, float PUY)
    {

        float oriHPWL = 0.0;
        float newHPWL = 0.0;
        auto nets = placementInfo->getPlacementUnitId2Nets()[tmpPU->getId()];

        for (auto curNet : nets)
        {
            if (curNet->getDesignNet()->getPins().size() > 1000) // it could be clock
                continue;
            oriHPWL += curNet->getHPWL(y2xRatio);
            newHPWL += curNet->getNewHPWLByTrying(tmpPU, PUX, PUY, y2xRatio);
        }

        return (newHPWL - oriHPWL);
    }

    inline void swapSitePtr(DeviceInfo::DeviceSite **siteA, DeviceInfo::DeviceSite **siteB)
    {
        DeviceInfo::DeviceSite *tmp = *siteA;
        *siteA = *siteB;
        *siteB = tmp;
    }

    inline int sortPartition(DesignInfo::DesignCell *curCell, std::vector<DeviceInfo::DeviceSite *> &sites, int low,
                             int high, PlacementInfo::Location &macroLoc)
    {
        int pivot, index, i;
        index = low;
        pivot = high;
        for (i = low; i < high; i++)
        {
            // finding index of pivot.
            // if (a[i] < a[pivot])
            // if (getDisplacement(macroLoc, sites[i]) < getDisplacement(macroLoc, sites[pivot]))
            if (getHPWLChange(curCell, sites[i]) < getHPWLChange(curCell, sites[pivot]))
            {
                swapSitePtr(&sites[i], &sites[index]);
                index++;
            }
        }
        swapSitePtr(&sites[pivot], &sites[index]);
        return index;
    }
    inline int RandomPivotPartition(DesignInfo::DesignCell *curCell, std::vector<DeviceInfo::DeviceSite *> &sites,
                                    int low, int high, PlacementInfo::Location &macroLoc)
    {
        // Random selection of pivot.
        int pvt;
        pvt = (high + low) / 2;
        // pvt = low + n % (high - low + 1); // Randomizing the pivot value from sub-array.
        swapSitePtr(&sites[high], &sites[pvt]);
        return sortPartition(curCell, sites, low, high, macroLoc);
    }
    void quick_sort_WLChange(DesignInfo::DesignCell *curCell, std::vector<DeviceInfo::DeviceSite *> &sites, int p,
                             int q, PlacementInfo::Location &macroLoc)
    {
        // recursively sort the list
        int pindex;
        if (p < q)
        {
            pindex = RandomPivotPartition(curCell, sites, p, q, macroLoc); // randomly choose pivot
            // Recursively implementing QuickSort.
            quick_sort_WLChange(curCell, sites, p, pindex - 1, macroLoc);
            quick_sort_WLChange(curCell, sites, pindex + 1, q, macroLoc);
        }
    }

    inline void swapPUs(PlacementInfo::PlacementUnit **PUA, PlacementInfo::PlacementUnit **PUB)
    {
        PlacementInfo::PlacementUnit *tmp = *PUA;
        *PUA = *PUB;
        *PUB = tmp;
    }

    inline int sortPartition(std::vector<PlacementInfo::PlacementUnit *> &PUs, int low, int high)
    {
        int pivot, index, i;
        index = low;
        pivot = high;
        for (i = low; i < high; i++)
        {
            // finding index of pivot.
            // if (a[i] < a[pivot])
            // if (getDisplacement(macroLoc, PUs[i]) < getDisplacement(macroLoc, PUs[pivot]))
            if (PUs[i]->X() < PUs[pivot]->X())
            {
                swapPUs(&PUs[i], &PUs[index]);
                index++;
            }
        }
        swapPUs(&PUs[pivot], &PUs[index]);
        return index;
    }
    inline int RandomPivotPartition(std::vector<PlacementInfo::PlacementUnit *> &PUs, int low, int high)
    {
        // Random selection of pivot.
        int pvt, n;
        n = random();
        pvt = low + n % (high - low + 1); // Randomizing the pivot value from sub-array.
        swapPUs(&PUs[high], &PUs[pvt]);
        return sortPartition(PUs, low, high);
    }
    void quick_sort_locX(std::vector<PlacementInfo::PlacementUnit *> &PUs, int p, int q)
    {
        // recursively sort the list
        int pindex;
        if (p < q)
        {
            pindex = RandomPivotPartition(PUs, p, q); // randomly choose pivot
            // Recursively implementing QuickSort.
            quick_sort_locX(PUs, p, pindex - 1);
            quick_sort_locX(PUs, pindex + 1, q);
        }
    }
};

#endif