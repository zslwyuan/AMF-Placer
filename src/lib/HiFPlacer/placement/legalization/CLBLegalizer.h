/**
 * @file CLBLegalizer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of CLBLegalizer class and its internal modules and APIs which
 * map CLBs (each of which consists of one site) to legal location. e.g. LUTRAM, except those CLBs in CARRY8_Chain.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _CLBLEGALIZER
#define _CLBLEGALIZER

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
 * @brief   CLBLegalizer maps CLBs (each of which consists of one site) to legal location.
 * e.g. LUTRAM, except those CLBs in CARRY8_Chain.
 *
 * It should be aware that on FPGA, there are 2 types of CLBs, SLICEM and SLICEL. SLICEM CLBs can be instantiated as
 * LUTRAM and Shifters while SLICEL can be only instantiated as general logic and carry chains. CLBLegalizer is used to
 * handle 1-to-1 PlacementUnit-DeviceSite legalization.
 *
 * The legalization procedure will only conduct rough legalization in the early iterations of global placement, and it
 * will conduct exact legalization following rough legalization when the macros are close enough to their potential
 * legal positions.
 *
 */
class CLBLegalizer
{
  public:
    /**
     * @brief Construct a new CLBLegalizer object
     *
     * @param legalizerName the name string of legalizer for log dumping
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param deviceInfo device information
     * @param siteTypesToLegalize a vector of Cell Type string indicating the target types handled by this CLBLegalizer
     * @param JSONCfg  the user-defined placement configuration
     */
    CLBLegalizer(std::string legalizerName, PlacementInfo *placementInfo, DeviceInfo *deviceInfo,
                 std::vector<std::string> &siteTypesToLegalize, std::map<std::string, std::string> &JSONCfg);
    ~CLBLegalizer()
    {
        if (minCostBipartiteMatcher)
            delete minCostBipartiteMatcher;
    }

    /**
     * @brief conduct legalization and map the PlacementUnit of one of the given types to sites
     *
     * @param exactLegalization true to ensure elements in a macro are placed consecutively.
     */
    void legalize(bool exactLegalization = false);

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
    void dumpMatching(bool fixedColumn = false, bool enforce = false);

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
     * @brief a vector of Cell Type string indicating the target types handled by this CLBLegalizer
     *
     */
    std::vector<std::string> siteTypesToLegalize;

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
     * @brief a vector storing the PlacementUnits which have NOT been legalized
     *
     */
    std::vector<PlacementInfo::PlacementUnit *> PUsToLegalize;

    /**
     * @brief a vector storing the PlacementUnits which SHOULD be legalized
     *
     */
    std::vector<PlacementInfo::PlacementUnit *> initialPUsToLegalize;

    /**
     * @brief a set storing the PlacementUnits which have NOT been legalized
     *
     */
    std::set<PlacementInfo::PlacementUnit *> PUsToLegalizeSet;

    /**
     * @brief a map record the potential sites of different site types
     *
     */
    std::map<std::string, std::vector<DeviceInfo::DeviceSite *>> siteType2Sites;

    /**
     * @brief record the mapping from PlacementUnits to the candidate sites which are NOT binded to PUs
     *
     * Please be aware that a PlacementUnit (i.e., PlacementMacro) might be binded of multiple sites.
     *
     */
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> PU2Sites;

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
     * @brief a map of map recording the HPWL overhead when binding a PlacementUnit to a specific site
     *
     */
    std::map<PlacementInfo::PlacementUnit *, std::map<DeviceInfo::DeviceSite *, float>> PU2Site2HPWLIncrease;

    /**
     * @brief a set of PlacementUnits binded to corresponding DeviceSites
     *
     */
    std::set<PlacementInfo::PlacementUnit *> matchedPUs;

    /**
     * @brief a set of DeviceSites binded to corresponding PlacementUnits
     *
     */
    std::set<DeviceInfo::DeviceSite *> matchedSites;

    /**
     * @brief record the binding between PlacementUnits and DeviceSites as a vector of pairs
     *
     */
    std::vector<std::pair<PlacementInfo::PlacementUnit *, DeviceInfo::DeviceSite *>> PULevelMatching;

    int DumpCLBLegalizationCnt = 0;

    /**
     * @brief displacement threshold to detect potential legal sites
     *
     */
    float displacementThreshold = 20;

    /**
     * @brief the maximum number of final candidate sites
     *
     */
    int maxNumCandidate = 20;

    /**
     * @brief we are allowed to detect a excessive number (>candidateNum) of initial candidates. candidateFactor is to
     * control the excessive ratio.
     *
     */
    int candidateFactor = 5;

    /**
     * @brief the number of the parallel multi-threading workers to handle the legalization problems
     *
     */
    int nJobs = 1;

    /**
     * @brief the number of SLICEM columns on the target device
     *
     */
    int MCLBColumnNum = -1;

    /**
     * @brief the number of SLICEL columns on the target device
     *
     */
    int LCLBColumnNum = -1;

    /**
     * @brief the number of SLICEM rows ocolumnsn the target device
     *
     */
    int MCLBRowNum = -1;

    /**
     * @brief the number of SLICEL rows on the target device
     *
     */
    int LCLBRowNum = -1;

    /**
     * @brief the floating-point X location of the SLICEM columns on the device
     *
     */
    std::vector<float> MCLBColumnXs;

    /**
     * @brief the floating-point X location of the SLICEL columns on the device
     *
     */
    std::vector<float> LCLBColumnXs;

    /**
     * @brief record the sites in each column of SLICEM
     *
     */
    std::vector<std::vector<DeviceInfo::DeviceSite *>> MCLBColumn2Sites;

    /**
     * @brief record the sites in each column of SLICEL
     *
     */
    std::vector<std::vector<DeviceInfo::DeviceSite *>> LCLBColumn2Sites;

    /**
     * @brief record the PlacementUnits in each column of SLICEM
     *
     */
    std::vector<std::deque<PlacementInfo::PlacementUnit *>> MCLBColumn2PUs;

    /**
     * @brief record the PlacementUnits in each column of SLICEL
     *
     */
    std::vector<std::deque<PlacementInfo::PlacementUnit *>> LCLBColumn2PUs;

    /**
     * @brief record the number of cells (Macro contains multiple cells) in each column of SLICEM
     *
     */
    std::vector<int> MCLBColumnUntilization;

    /**
     * @brief record the number of cells (Macro contains multiple cells) in each column of SLICEL
     *
     */
    std::vector<int> LCLBColumnUntilization;

    /**
     * @brief record the mapping from SLICEM CLB PlacementUnits to corresponding columns
     *
     */
    std::map<PlacementInfo::PlacementUnit *, int> MCLB2Column;

    /**
     * @brief record the mapping from SLICEL CLB PlacementUnits to corresponding columns
     *
     */
    std::map<PlacementInfo::PlacementUnit *, int> LCLB2Column;

    /**
     * @brief record the mapping from PlacementUnits to exact DeviceSites
     *
     */
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
     * @brief the PlacementUnits which shoudl be mapped to SLICEM
     *
     */
    std::set<PlacementInfo::PlacementUnit *> MCLBPUs;

    /**
     * @brief the PlacementUnits which shoudl be mapped to SLICEL
     *
     */
    std::set<PlacementInfo::PlacementUnit *> LCLBPUs;

    /**
     * @brief a cache record the candidate sites within a given displacement threshold  for each PlacementUnit
     *
     */
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *> *> PU2SitesInDisplacementThreshold;

    bool enableMCLBLegalization = false;
    bool enableLCLBLegalization = false;
    bool verbose = false;
    float y2xRatio = 1.0;

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
    float initialDisplacementThreshold = 20;

    /**
     * @brief the maximum number of final candidate sites
     *
     */
    int initialMaxNumCandidate = 20;

    /**
     * @brief get the PlacementUnits which SHOULD be legalized
     *
     */
    void getPUsToLegalize();

    /**
     * @brief find available sites for each specific type required by the constructor
     *
     */
    void findSiteType2AvailableSites();

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
     * @brief map PlacementUnit to the columns according to the locations of the cells in it
     *
     */
    void mapPUsToColumns();

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
     */
    void fixedColumnLegalize();

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
     * @brief finally dynamic programming to legalize the PlacementUnits which have been mapped to the columns.
     *
     * This function will call DP function for each specific type of PlacementUnits
     *
     */
    void finalLegalizeBasedOnDP();

    /**
     * @brief DP function for the legalization of a specific type of PlacementUnits in the same column
     *
     * @param colNum total number of the column of the target type of PlacementUnit
     * @param Column2Sites a vector record the sites in the columns
     * @param Column2PUs  a vector record the PlacementUnits in the columns
     * @return float
     */
    float DPForMinHPWL(int colNum, std::vector<std::vector<DeviceInfo::DeviceSite *>> &Column2Sites,
                       std::vector<std::deque<PlacementInfo::PlacementUnit *>> &Column2PUs);

    /**
     * @brief record the matching in private list and update the list of PlacementUnits which are not matched by the
     * bi-partite matching
     *
     */
    void updateMatchingAndUnmatchedPUs();

    /**
     * @brief spread PlacementUnits accross columns to resolve resource overflow
     *
     * @param columnNum the number of columns
     * @param columnUntilization a vector reording the utilization usage of each column
     * @param column2Sites a vector reording device sites in each column
     * @param column2PUs a vector reording PlacementUnits in each column
     * @param cell2Column a map recording the column id for each PlacementUnit
     */
    void spreadPUs(int columnNum, std::vector<int> &columnUntilization,
                   std::vector<std::vector<DeviceInfo::DeviceSite *>> &column2Sites,
                   std::vector<std::deque<PlacementInfo::PlacementUnit *>> &column2PUs,
                   std::map<PlacementInfo::PlacementUnit *, int> &cell2Column);

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
     * @brief clear the mapping information and reset the mapping parameters
     *
     */
    inline void resetSettings()
    {
        displacementThreshold = initialDisplacementThreshold;
        maxNumCandidate = initialMaxNumCandidate;
        matchedPUs.clear();
        matchedSites.clear();
        PULevelMatching.clear();
    }

    /**
     * @brief find candidate sites for the PlacementUnits left to be matched
     *
     */
    void findPU2SitesInDistance()
    {
        PU2SitesInDisplacementThreshold.clear();

        int PUsNum = PUsToLegalize.size();

        for (int i = 0; i < PUsNum; i++)
        {
            PU2SitesInDisplacementThreshold[PUsToLegalize[i]] = nullptr;
        }

#pragma omp parallel for
        for (int i = 0; i < PUsNum; i++)
        {
            PlacementInfo::PlacementUnit *curPU = PUsToLegalize[i];
            DesignInfo::DesignCell *curCell = nullptr;
            if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
            {
                curCell = unpackedCell->getCell();
            }
            else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
            {
                assert(curMacro->getCells().size() > 0);
                curCell = curMacro->getCells()[0];
            }
            assert(curCell);
            PU2SitesInDisplacementThreshold[curPU] = placementInfo->findNeiborSiteFromBinGrid(
                curCell, cellLoc[curCell->getCellId()].X, cellLoc[curCell->getCellId()].Y, displacementThreshold,
                candidateFactor * maxNumCandidate);
        }
    }

    /**
     * @brief clear the information of candidate sites for the PlacementUnits left to be matched
     *
     */
    void resetPU2SitesInDistance()
    {
        int PUsNum = PUsToLegalize.size();
        for (int i = 0; i < PUsNum; i++)
        {
            PlacementInfo::PlacementUnit *curPU = PUsToLegalize[i];
            delete PU2SitesInDisplacementThreshold[curPU];
        }
    }

    /**
     * @brief check how many sites are required by the given PlacementUnit
     *
     * @param tmpPUUnit  the given PlacementUnit
     * @return int
     */
    int getPUSiteNum(PlacementInfo::PlacementUnit *tmpPUUnit);

    inline void swapPU(PlacementInfo::PlacementUnit **A, PlacementInfo::PlacementUnit **B)
    {
        PlacementInfo::PlacementUnit *C = *A;
        *A = *B;
        *B = C;
    }

    void sortPUsByPU2Y(std::deque<PlacementInfo::PlacementUnit *> &PUs);

    void sortSitesBySiteY(std::vector<DeviceInfo::DeviceSite *> &sites);

    inline float getDisplacement(PlacementInfo::Location &PULoc, DeviceInfo::DeviceSite *curSite)
    {
        return std::fabs(PULoc.X - curSite->X()) + y2xRatio * std::fabs(PULoc.Y - curSite->Y());
    }

    inline float getDisplacement(PlacementInfo::PlacementUnit *curPU, DeviceInfo::DeviceSite *curSite)
    {
        return std::fabs(curPU->X() - curSite->X()) + y2xRatio * std::fabs(curPU->Y() - curSite->Y());
    }

    /**
     * @brief get the HPWL change when the given PlacementUnit moves to the given DeviceSite
     *
     * @param curPU the given PlacementUnit
     * @param curSite  the given DeviceSite
     * @return float
     */
    inline float getHPWLChange(PlacementInfo::PlacementUnit *curPU, DeviceInfo::DeviceSite *curSite)
    {
        assert(PU2Site2HPWLIncrease.find(curPU) != PU2Site2HPWLIncrease.end());
        if (PU2Site2HPWLIncrease[curPU].find(curSite) != PU2Site2HPWLIncrease[curPU].end())
        {
            return PU2Site2HPWLIncrease[curPU][curSite];
        }
        float oriHPWL = 0.0;
        float newHPWL = 0.0;
        float PUX = curSite->X();
        float PUY = curSite->Y();
        auto nets = placementInfo->getPlacementUnitId2Nets()[curPU->getId()];

        for (auto curNet : nets)
        {
            if (curNet->getDesignNet()->getPins().size() > 1000) // it could be clock
                continue;
            oriHPWL += curNet->getHPWL(y2xRatio);
            newHPWL += curNet->getNewHPWLByTrying(curPU, PUX, PUY, y2xRatio);
        }

        PU2Site2HPWLIncrease[curPU][curSite] = (newHPWL - oriHPWL);
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

    inline int sortPartition(PlacementInfo::PlacementUnit *curPU, std::vector<DeviceInfo::DeviceSite *> &sites, int low,
                             int high)
    {
        int pivot, index, i;
        index = low;
        pivot = high;
        for (i = low; i < high; i++)
        {
            // finding index of pivot.
            // if (a[i] < a[pivot])
            // if (getDisplacement(PULoc, sites[i]) < getDisplacement(PULoc, sites[pivot]))
            if (getHPWLChange(curPU, sites[i]) < getHPWLChange(curPU, sites[pivot]))
            {
                swapSitePtr(&sites[i], &sites[index]);
                index++;
            }
        }
        swapSitePtr(&sites[pivot], &sites[index]);
        return index;
    }
    inline int RandomPivotPartition(PlacementInfo::PlacementUnit *curPU, std::vector<DeviceInfo::DeviceSite *> &sites,
                                    int low, int high)
    {
        // Random selection of pivot.
        int pvt;
        pvt = (high + low) / 2; // Randomizing the pivot value from sub-array.
        swapSitePtr(&sites[high], &sites[pvt]);
        return sortPartition(curPU, sites, low, high);
    }
    void quick_sort_WLChange(PlacementInfo::PlacementUnit *curPU, std::vector<DeviceInfo::DeviceSite *> &sites, int p,
                             int q)
    {
        // recursively sort the list
        int pindex;
        if (p < q)
        {
            pindex = RandomPivotPartition(curPU, sites, p, q); // randomly choose pivot
            // Recursively implementing QuickSort.
            quick_sort_WLChange(curPU, sites, p, pindex - 1);
            quick_sort_WLChange(curPU, sites, pindex + 1, q);
        }
    }

    inline void swapPUs(PlacementInfo::PlacementUnit **siteA, PlacementInfo::PlacementUnit **siteB)
    {
        PlacementInfo::PlacementUnit *tmp = *siteA;
        *siteA = *siteB;
        *siteB = tmp;
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
            // if (getDisplacement(PULoc, PUs[i]) < getDisplacement(PULoc, PUs[pivot]))
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