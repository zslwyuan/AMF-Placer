#include "CLBLegalizer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

CLBLegalizer::CLBLegalizer(std::string legalizerName, PlacementInfo *placementInfo, DeviceInfo *deviceInfo,
                           std::vector<std::string> &siteTypesToLegalize, std::map<std::string, std::string> &JSONCfg)
    : legalizerName(legalizerName), placementInfo(placementInfo), deviceInfo(deviceInfo),
      compatiblePlacementTable(placementInfo->getCompatiblePlacementTable()), siteTypesToLegalize(siteTypesToLegalize),
      cellLoc(placementInfo->getCellId2location()), JSONCfg(JSONCfg)
{
    PUsToLegalize.clear();
    PU2Site2HPWLIncrease.clear();
    PU2X.clear();
    PU2Y.clear();
    PU2LegalSites.clear();

    for (auto curSiteType : siteTypesToLegalize)
    {
        if (curSiteType == "SLICEM")
        {
            enableMCLBLegalization = true;
        }
        if (curSiteType == "SLICEL")
        {
            enableLCLBLegalization = true;
        }
    }

    // since  LCLB might be allowed to mapped to MCLB but MCLB cannot be mapped to LCLB, we cannot handle them at the
    // same time
    assert(!(enableLCLBLegalization && enableMCLBLegalization));
    assert(siteTypesToLegalize.size() == 1);

    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
    {
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);
    }

    if (JSONCfg.find("CLBLegalizationVerbose") != JSONCfg.end())
    {
        verbose = JSONCfg["CLBLegalizationVerbose"] == "true";
    }

    if (JSONCfg.find("jobs") != JSONCfg.end())
    {
        nJobs = std::stoi(JSONCfg["jobs"]);
    }
}

void CLBLegalizer::legalize(bool exactLegalization)
{
    if (verbose)
        print_status("CLBLegalizer Started Legalization.");
    PU2Site2HPWLIncrease.clear();
    resetSettings();
    findSiteType2AvailableSites();
    getPUsToLegalize();
    roughlyLegalize();

    updatePUMatchingLocation(true, !exactLegalization);

    dumpMatching();

    if (exactLegalization)
    {
        if (verbose)
            print_status("CLBLegalizer Started Fixed-Column Legalization.");
        resetSettings();
        fixedColumnLegalize();
        updatePUMatchingLocation(false, true);
        finalLegalizeBasedOnDP();
        dumpMatching(exactLegalization);
    }

    setSitesMapped();
    if (verbose)
        print_status("CLBLegalizer Finished Legalization.");
}

void CLBLegalizer::roughlyLegalize()
{
    while (PUsToLegalize.size())
    {
        findPU2SitesInDistance();
        findPossibleLegalLocation(false);
        resetPU2SitesInDistance();

        createBipartiteGraph();
        minCostBipartiteMatcher =
            new MinCostBipartiteMatcher(PU2Sites.size(), rightSiteIds.size(), PU2Sites.size(), adjList, nJobs, verbose);

        minCostBipartiteMatcher->solve();
        updateMatchingAndUnmatchedPUs();

        displacementThreshold *= 2;
        if ((int)(maxNumCandidate * 2) > maxNumCandidate + 1)
            maxNumCandidate *= 2;
        else
            maxNumCandidate++;
        delete minCostBipartiteMatcher;
        minCostBipartiteMatcher = nullptr;
    }
}

void CLBLegalizer::fixedColumnLegalize()
{
    mapPUsToColumns();
    resolveOverflowColumns();

    PUsToLegalize = initialPUsToLegalize;

    while (PUsToLegalize.size())
    {
        findPU2SitesInDistance();
        findPossibleLegalLocation(true);
        resetPU2SitesInDistance();
        createBipartiteGraph();
        minCostBipartiteMatcher =
            new MinCostBipartiteMatcher(PU2Sites.size(), rightSiteIds.size(), PU2Sites.size(), adjList, nJobs, verbose);

        minCostBipartiteMatcher->solve();
        updateMatchingAndUnmatchedPUs();

        displacementThreshold *= 2;
        if ((int)(maxNumCandidate * 2) > maxNumCandidate + 1)
            maxNumCandidate *= 2;
        else
            maxNumCandidate++;
        delete minCostBipartiteMatcher;
        minCostBipartiteMatcher = nullptr;
    }
}

void CLBLegalizer::finalLegalizeBasedOnDP()
{
    PU2X.clear();
    PU2Y.clear();
    resetSettings();
    PU2LegalSites.clear();

    float tmpAverageDisplacement = 0.0;
    tmpAverageDisplacement += DPForMinHPWL(MCLBColumnNum, MCLBColumn2Sites, MCLBColumn2PUs);
    tmpAverageDisplacement += DPForMinHPWL(LCLBColumnNum, LCLBColumn2Sites, LCLBColumn2PUs);
    finalAverageDisplacement = tmpAverageDisplacement / PU2X.size();

    if (verbose)
        print_info("CLBLegalizer Macro Cell Average Final Legalization Displacement = " +
                   std::to_string(finalAverageDisplacement));
    placementInfo->setPULegalSite(PU2LegalSites);
    placementInfo->setPULegalXY(PU2X, PU2Y);
}

float CLBLegalizer::DPForMinHPWL(int colNum, std::vector<std::vector<DeviceInfo::DeviceSite *>> &Column2Sites,
                                 std::vector<std::deque<PlacementInfo::PlacementUnit *>> &Column2PUs)
{
    // final Legalization DP
    // i th macro (start from 0), j th row (start from 0)
    // f[i][j] = min(f[i-1][j-row[i]]+HPWLChange[i][j-row[i]+1],f[i][j-1])

    // std::string dumpFile =
    //     JSONCfg["DumpCLBLegalization"] + "-Exact-" + std::to_string(DumpCLBLegalizationCnt) + ".gz";

    // print_status("CLBLegalizer: dumping CLBLegalization archieve to: " + dumpFile);
    // DumpCLBLegalizationCnt++;
    // std::stringstream outfile0;

    float tmpTotalDisplacement = 0.0;
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> PU2Sites;
    PU2Sites.clear();
    for (auto &PUDeque : Column2PUs)
    {
        for (auto tmpPU : PUDeque)
        {
            assert(PU2Sites.find(tmpPU) == PU2Sites.end());
            PU2Sites[tmpPU] = std::vector<DeviceInfo::DeviceSite *>();
        }
    }

#pragma omp parallel for
    for (int c = 0; c < colNum; c++)
    {
        int numPUs = Column2PUs[c].size();
        if (!numPUs)
            continue;

        auto &curColSites = Column2Sites[c];
        auto &curColPU = Column2PUs[c];

        sortSitesBySiteY(curColSites);

        // initialize

        std::vector<std::vector<float>> f(numPUs, std::vector<float>(curColSites.size(), 1000000000.0));
        std::vector<std::vector<bool>> fChoice(
            numPUs, std::vector<bool>(curColSites.size(), 0)); // used for tracing back macro-site mapping

        int minLastPUTop = -1;

        int totalMacroCellNum = 0;
        int heightPURow = getPUSiteNum(curColPU[0]);
        totalMacroCellNum += heightPURow;
        float minHPWLChange = 1100000000.0;
        for (unsigned int j = heightPURow - 1; j < curColSites.size(); j++)
        {
            float curHPWLChange = getHPWLChange(curColPU[0], curColSites[j - heightPURow + 1]);
            if (curColSites[j]->getSiteY() - curColSites[j - heightPURow + 1]->getSiteY() != heightPURow - 1)
            {
                // we need to ensure that there is no occpupied sites in this range
                curHPWLChange = 1100000000.0;
            }
            if (curHPWLChange < minHPWLChange)
            {
                minHPWLChange = curHPWLChange;
                fChoice[0][j] = 1;
                if (numPUs == 1)
                {
                    minLastPUTop = j;
                }
            }
            f[0][j] = minHPWLChange;
        }

        // DP to minimize HPWL
        for (int i = 1; i < numPUs; i++)
        {
            float minVal = 100000000.0;
            int heightPURow = getPUSiteNum(curColPU[i]);
            totalMacroCellNum += heightPURow;
            for (unsigned int j = totalMacroCellNum - 1; j < curColSites.size();
                 j++) // j start from heightPURow because PU0 must occupy 1+ site(s)
            {
                float curHPWLChange = getHPWLChange(curColPU[i], curColSites[j - heightPURow + 1]);
                if (curColSites[j]->getSiteY() - curColSites[j - heightPURow + 1]->getSiteY() != heightPURow - 1)
                {
                    // we need to ensure that there is no occpupied sites in this range
                    curHPWLChange = 1100000000.0;
                }
                if (f[i][j - 1] > f[i - 1][j - heightPURow] + curHPWLChange)
                {
                    fChoice[i][j] = 1;
                    if (i == numPUs - 1)
                    {
                        if (f[i - 1][j - heightPURow] + curHPWLChange < minVal)
                        {
                            minVal = f[i - 1][j - heightPURow] + curHPWLChange;
                            minLastPUTop = j;
                        }
                    }
                }

                f[i][j] = std::min(f[i][j - 1], f[i - 1][j - heightPURow] + curHPWLChange);
            }
        }

        assert(minLastPUTop >= 0);

        std::vector<DeviceInfo::DeviceSite *> PUBottom2Site_reverse(0);

        for (int i = numPUs - 1; i >= 0; i--)
        {
            assert(minLastPUTop >= 0);
            int heightPURow = getPUSiteNum(curColPU[i]);
            while (!fChoice[i][minLastPUTop])
            {
                minLastPUTop--;
                assert(minLastPUTop >= 0);
            }
            auto curSite = curColSites[minLastPUTop - heightPURow + 1];
            assert(PU2Sites[curColPU[i]].size() == 0);
            assert(curSite);
            PU2Sites[curColPU[i]].push_back(curSite); // CLB PU will only occupy one site
            minLastPUTop -= heightPURow;
            // outfile0 << "col#" << c << ": " << curColPU[i]->getName() << " PUY:" << PU2Y[curColPU[i]]
            //          << " numPUs:" << getPUSiteNum(curColPU[i])
            //          << " netNum:" << curColPU[i]->getNetsSetPtr()->size() << "\n";
            // outfile0 << "    macthed with: " << curSite->getName() << " locX:" << curSite->X()
            //          << " locY:" << curSite->Y() << " HPWLIncrease:" << getHPWLChange(curColPU[i], curSite) << "\n";
        }
    }

    for (auto &PUDeque : Column2PUs)
    {
        for (auto tmpPU : PUDeque)
        {
            assert(PU2Sites.find(tmpPU) != PU2Sites.end());
            auto curSite = PU2Sites[tmpPU][0];
            assert(curSite);
            assert(PU2X.find(tmpPU) == PU2X.end());
            PU2X[tmpPU] = curSite->X();
            PU2Y[tmpPU] = curSite->Y();
            PU2LegalSites[tmpPU] = PU2Sites[tmpPU];
            PULevelMatching.emplace_back(tmpPU, curSite);
            tmpTotalDisplacement += std::fabs(tmpPU->X() - curSite->X()) + std::fabs(tmpPU->Y() - curSite->Y());
        }
    }

    // writeStrToGZip(dumpFile, outfile0);
    // print_status("CLBLegalizer: dumped CLBLegalization archieve to: " + dumpFile);
    return tmpTotalDisplacement;
}

void CLBLegalizer::getPUsToLegalize()
{
    PUsToLegalize.clear();
    PUsToLegalizeSet.clear();
    MCLBPUs.clear();
    LCLBPUs.clear();
    for (auto curPU : placementInfo->getPlacementUnits())
    {
        if (!curPU->isLocked())
        {
            if (enableMCLBLegalization)
            {
                if (curPU->isMCLB())
                {
                    MCLBPUs.insert(curPU);
                    PUsToLegalize.push_back(curPU);
                    PUsToLegalizeSet.insert(curPU);
                }
            }

            if (enableLCLBLegalization)
            {
                if (curPU->isLCLB())
                {
                    LCLBPUs.insert(curPU);
                    PUsToLegalize.push_back(curPU);
                    PUsToLegalizeSet.insert(curPU);
                }
            }
        }
    }
    initialPUsToLegalize = PUsToLegalize;
    for (auto curPU : PUsToLegalize)
    {
        PU2Site2HPWLIncrease[curPU] = std::map<DeviceInfo::DeviceSite *, float>();
    }
}

void CLBLegalizer::findSiteType2AvailableSites()
{
    siteType2Sites.clear();
    for (auto siteTypeToLegalize : siteTypesToLegalize)
    {
        siteType2Sites[siteTypeToLegalize] = std::vector<DeviceInfo::DeviceSite *>(0);
        std::vector<DeviceInfo::DeviceSite *> &sitesInType = deviceInfo->getSitesInType(siteTypeToLegalize);
        for (auto curSite : sitesInType)
        {
            if (!curSite->isOccupied() && !curSite->isMapped())
            {
                // if (matchedSites.find(curSite) == matchedSites.end())
                // {
                assert(!curSite->isMapped());
                siteType2Sites[siteTypeToLegalize].push_back(curSite);
                // }
            }
        }
        if (siteTypeToLegalize == "SLICEL")
        {
            // since LCLB might be allowed to mapped to MCLB but MCLB cannot be mapped to LCLB, we cannot handle them
            // at the same time. We MUST legalize SLICEM and make them occupied before we legalize SLICEL
            std::string tmpSiteType = "SLICEM";
            std::vector<DeviceInfo::DeviceSite *> &sitesInType = deviceInfo->getSitesInType(tmpSiteType);
            for (auto curSite : sitesInType)
            {
                if (!curSite->isOccupied() && !curSite->isMapped())
                {
                    // if (matchedSites.find(curSite) == matchedSites.end())
                    // {
                    siteType2Sites[siteTypeToLegalize].push_back(curSite);
                    // }
                }
            }
        }
    }

    for (auto curSiteType : siteTypesToLegalize)
    {
        if (curSiteType == "SLICEM")
        {
            for (auto curSite : siteType2Sites[curSiteType])
            {
                if (curSite->getSiteY() + 1 > MCLBRowNum)
                    MCLBRowNum = curSite->getSiteY() + 1;
                if (curSite->getSiteX() + 1 > MCLBColumnNum)
                    MCLBColumnNum = curSite->getSiteX() + 1;
            }
        }
        if (curSiteType == "SLICEL")
        {
            for (auto curSite : siteType2Sites[curSiteType])
            {
                if (curSite->getSiteY() + 1 > LCLBRowNum)
                    LCLBRowNum = curSite->getSiteY() + 1;
                if (curSite->getSiteX() + 1 > LCLBColumnNum)
                    LCLBColumnNum = curSite->getSiteX() + 1;
            }
        }
    }

    MCLBColumnXs.clear();
    if (enableMCLBLegalization)
    {
        MCLBColumnXs.resize(MCLBColumnNum, -1.0);
        MCLBColumn2Sites.clear();
        MCLBColumn2Sites.resize(MCLBColumnNum, std::vector<DeviceInfo::DeviceSite *>(0));

        for (auto curSite : siteType2Sites["SLICEM"])
        {
            if (curSite->getSiteY() + 1 > MCLBRowNum)
                MCLBRowNum = curSite->getSiteY() + 1;
            if (curSite->getSiteX() + 1 > MCLBColumnNum)
                MCLBColumnNum = curSite->getSiteX() + 1;
            MCLBColumnXs[curSite->getSiteX()] = curSite->X();
            MCLBColumn2Sites[curSite->getSiteX()].push_back(curSite);
        }
    }

    LCLBColumnXs.clear();
    if (enableLCLBLegalization)
    {
        LCLBColumnXs.resize(LCLBColumnNum, -1.0);
        LCLBColumn2Sites.clear();
        LCLBColumn2Sites.resize(LCLBColumnNum, std::vector<DeviceInfo::DeviceSite *>(0));

        for (auto curSite : siteType2Sites["SLICEL"])
        {
            if (curSite->getSiteY() + 1 > LCLBRowNum)
                LCLBRowNum = curSite->getSiteY() + 1;
            if (curSite->getSiteX() + 1 > LCLBColumnNum)
                LCLBColumnNum = curSite->getSiteX() + 1;
            LCLBColumnXs[curSite->getSiteX()] = curSite->X();
            LCLBColumn2Sites[curSite->getSiteX()].push_back(curSite);
        }
    }
}

void CLBLegalizer::findPossibleLegalLocation(bool fixedColumn)
{
    PU2Sites.clear();

    for (auto curPU : PUsToLegalize)
    {
        PU2Sites[curPU] = std::vector<DeviceInfo::DeviceSite *>(0);
    }

    int numPUs = PUsToLegalize.size();

#pragma omp parallel for
    for (int i = 0; i < numPUs; i++)
    {
        auto curPU = PUsToLegalize[i];
        std::string curSiteType = "";
        if (curPU->isLCLB())
            curSiteType = "SLICEL";
        else if (curPU->isMCLB())
            curSiteType = "SLICEM";
        else
            assert(false && "should be LogicCLB or RAMCLB");

        assert(siteType2Sites.find(curSiteType) != siteType2Sites.end());
        std::vector<DeviceInfo::DeviceSite *> *candidateSite = nullptr;
        if (fixedColumn)
        {
            int targetSiteX = -1;
            std::vector<std::vector<DeviceInfo::DeviceSite *>> *column2Sites;
            if (curPU->isLCLB())
            {
                assert(LCLB2Column.find(curPU) != LCLB2Column.end());
                targetSiteX = LCLB2Column[curPU];
                column2Sites = &LCLBColumn2Sites;
            }
            if (curPU->isMCLB())
            {
                assert(MCLB2Column.find(curPU) != MCLB2Column.end());
                targetSiteX = MCLB2Column[curPU];
                column2Sites = &MCLBColumn2Sites;
            }
            assert(targetSiteX >= 0 && "undefine type");

            for (auto &tmpCol : *column2Sites)
            {
                if (tmpCol.size() == 0)
                    continue;
                if (tmpCol[0]->getSiteX() == targetSiteX)
                {
                    candidateSite = &tmpCol;
                }
            }
        }
        else
        {
            candidateSite = PU2SitesInDisplacementThreshold[curPU];
        }

        assert(candidateSite);

        for (auto curSite : *candidateSite)
        {
            float curDisp = getDisplacement(curPU, curSite);
            if ((!fixedColumn && curDisp < displacementThreshold) ||
                (fixedColumn && curDisp < 2 * displacementThreshold))
            {
                if (placementInfo->isLegalLocation(curPU, curSite->X(), curSite->Y()))
                {
                    if (matchedSites.find(curSite) !=
                        matchedSites.end()) // the matched sites should not in the candidate set
                        continue;
                    PU2Sites[curPU].push_back(curSite);
                }
            }
        }
        if (PU2Sites[curPU].size() > 1)
            quick_sort_WLChange(curPU, PU2Sites[curPU], 0, PU2Sites[curPU].size() - 1);
        if (PU2Sites[curPU].size() > (unsigned int)maxNumCandidate)
            PU2Sites[curPU].resize(maxNumCandidate);
    }
    // print_info("#total macro cell = " + std::to_string(PU2Sites.size()));
    // print_info("#total macro candidate site (might be duplicated) = " + std::to_string(totalSiteNum));
}

void CLBLegalizer::createBipartiteGraph()
{
    rightSiteIds.clear();
    adjList.resize(PUsToLegalize.size());
    siteList.clear();

    float minCost = 10000000;

    for (unsigned int leftCellId = 0; leftCellId < PUsToLegalize.size(); leftCellId++)
    {
        auto curPU = PUsToLegalize[leftCellId];
        adjList[leftCellId].clear();
        for (auto curSite : PU2Sites[curPU])
        {
            if (rightSiteIds.find(curSite) == rightSiteIds.end())
            {
                int curSiteCnt = rightSiteIds.size();
                rightSiteIds[curSite] = curSiteCnt;
                siteList.push_back(curSite);
            }
            float tmpCost = getHPWLChange(curPU, curSite);
            adjList[leftCellId].emplace_back(rightSiteIds[curSite], tmpCost);
            if (tmpCost < minCost)
            {
                minCost = tmpCost;
            }
            // adjList[leftCellId].emplace_back(rightSiteIds[curSite],
            //                                  getDisplacement(cellLoc[curCell->getCellId()], curSite));
        }
    }

    if (minCost < 0.0001)
    {
        float compensation = 10 - minCost;
        for (unsigned int leftCellId = 0; leftCellId < PUsToLegalize.size(); leftCellId++)
        {
            for (unsigned int tmpId = 0; tmpId < adjList[leftCellId].size(); tmpId++)
            {
                adjList[leftCellId][tmpId].second += compensation;
            }
        }
    }
}

void CLBLegalizer::updateMatchingAndUnmatchedPUs()
{
    for (unsigned int leftCellId = 0; leftCellId < PUsToLegalize.size(); leftCellId++)
    {
        int rightNode = minCostBipartiteMatcher->getMatchedRightNode(leftCellId);
        auto curPU = PUsToLegalize[leftCellId];
        if (rightNode >= 0)
        {
            assert(matchedPUs.find(curPU) == matchedPUs.end());
            assert(matchedSites.find(siteList[rightNode]) == matchedSites.end());
            matchedPUs.insert(curPU);
            matchedSites.insert(siteList[rightNode]);
            PULevelMatching.emplace_back(curPU, siteList[rightNode]);
        }
    }
    std::vector<PlacementInfo::PlacementUnit *> newPUsToLegalize;
    newPUsToLegalize.clear();
    for (unsigned int leftCellId = 0; leftCellId < PUsToLegalize.size(); leftCellId++)
    {
        auto curPU = PUsToLegalize[leftCellId];
        if (matchedPUs.find(curPU) == matchedPUs.end())
        {
            newPUsToLegalize.push_back(curPU);
        }
    }
    PUsToLegalize = newPUsToLegalize;
}

void CLBLegalizer::dumpMatching(bool fixedColumn, bool enforce)
{
    if (JSONCfg.find("DumpCLBLegalization") != JSONCfg.end() || enforce)
    {
        std::string dumpFile = "";
        if (enforce)
            dumpFile = "DumpCLBLegalization-" + std::to_string(DumpCLBLegalizationCnt) + ".gz";
        else
            dumpFile = JSONCfg["DumpCLBLegalization"] + "-" + std::to_string(DumpCLBLegalizationCnt) + ".gz";

        print_status("CLBLegalizer: dumping CLBLegalization archieve to: " + dumpFile);
        DumpCLBLegalizationCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto matchedPair : PULevelMatching)
            {
                auto matchedSite = matchedPair.second;
                auto curPU = matchedPair.first;
                outfile0 << "name: " << curPU->getName() << " locX:" << curPU->X() << " locY:" << curPU->Y() << "\n";
                outfile0 << "    macthed with: " << matchedSite->getName() << " locX:" << matchedSite->X()
                         << "\n        locY:" << matchedSite->Y()
                         << "\n        dis:" << getDisplacement(curPU, matchedSite)
                         << "\n        HPWLIncrease:" << getHPWLChange(curPU, matchedSite) << "\n";
            }

            if (fixedColumn)
            {
                for (int i = 0; i < MCLBColumnNum; i++)
                {
                    int numPUs = MCLBColumn2PUs[i].size();
                    for (int j = 0; j < numPUs; j++)
                    {
                        auto PU = MCLBColumn2PUs[i][j];
                        outfile0 << "MCLB col#" << i << ": " << PU->getName() << " PUY:" << PU2Y[PU]
                                 << " numPUs:" << getPUSiteNum(PU) << " netNum:" << PU->getNetsSetPtr()->size() << "\n";
                    }
                }
                for (int i = 0; i < LCLBColumnNum; i++)
                {
                    int numPUs = LCLBColumn2PUs[i].size();
                    for (int j = 0; j < numPUs; j++)
                    {
                        auto PU = LCLBColumn2PUs[i][j];
                        outfile0 << "LCLB col#" << i << ": " << PU->getName() << " PUY:" << PU2Y[PU]
                                 << " numPUs:" << getPUSiteNum(PU) << " netNum:" << PU->getNetsSetPtr()->size() << "\n";
                    }
                }
            }

            writeStrToGZip(dumpFile, outfile0);
            print_status("CLBLegalizer: dumped CLBLegalization archieve to: " + dumpFile);
        }
    }
}

int CLBLegalizer::getPUSiteNum(PlacementInfo::PlacementUnit *tmpMacroUnit)
{
    return 1; // currently all CLB should occupy only one site
}

void CLBLegalizer::sortPUsByPU2Y(std::deque<PlacementInfo::PlacementUnit *> &PUs)
{
    int numPUs = PUs.size();
    for (int i = 0; i < numPUs; i++)
        for (int j = i + 1; j < numPUs; j++)
            if (PU2Y[PUs[i]] > PU2Y[PUs[j]])
                swapPU(&PUs[i], &PUs[j]);
}

void CLBLegalizer::sortSitesBySiteY(std::vector<DeviceInfo::DeviceSite *> &sites)
{
    int numSites = sites.size();
    bool ordered = true;
    for (int i = 1; i < numSites; i++)
    {
        if (sites[i - 1]->getSiteY() > sites[i]->getSiteY())
        {
            ordered = false;
            break;
        }
    }
    if (ordered)
        return;
    for (int i = 0; i < numSites; i++)
        for (int j = i + 1; j < numSites; j++)
            if (sites[i]->getSiteY() > sites[j]->getSiteY())
                swapSitePtr(&sites[i], &sites[j]);
}

void CLBLegalizer::updatePUMatchingLocation(bool isRoughLegalization, bool updateDisplacement)
{
    PU2X.clear();
    PU2Y.clear();
    PU2Columns.clear();
    PU2SiteX.clear();

    PU2LegalSites.clear();
    MCLBColumn2PUs.clear();
    if (MCLBColumnNum > 0)
    {
        MCLBColumn2PUs.resize(MCLBColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
    }
    LCLBColumn2PUs.clear();
    if (LCLBColumnNum > 0)
    {
        LCLBColumn2PUs.resize(LCLBColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
    }

    float tmpAverageDisplacement = 0.0;

    for (auto matchedPair : PULevelMatching)
    {
        auto matchedSite = matchedPair.second;
        auto curPU = matchedPair.first;

        if (PU2Y.find(curPU) == PU2Y.end())
        {
            PU2SiteX[curPU] = -1;
            PU2Y[curPU] = 0.0;
            if (curPU->isMCLB())
            {
                MCLBColumn2PUs[matchedSite->getSiteX()].push_back(curPU);
            }
            if (curPU->isLCLB())
            {
                LCLBColumn2PUs[matchedSite->getSiteX()].push_back(curPU);
            }
        }

        float actualPUX = 0;
        float actualPUY = 0;
        // since the matched site is for a single DSP/BRAM cell, we need to find its corresponding placement macro
        // (with multiple DSPs/BRAMs)
        actualPUX = matchedSite->X();
        actualPUY = matchedSite->Y();

        tmpAverageDisplacement += std::fabs(curPU->X() - matchedSite->X()) + std::fabs(curPU->Y() - matchedSite->Y());
        assert(isRoughLegalization || PU2SiteX[curPU] == -1 || PU2SiteX[curPU] == matchedSite->getSiteX());
        PU2SiteX[curPU] = matchedSite->getSiteX();
        PU2X[curPU] += actualPUX;
        PU2Y[curPU] += actualPUY;
        if (PU2Columns.find(curPU) == PU2Columns.end())
        {
            PU2Columns[curPU] = std::vector<int>(0);
        }
        PU2Columns[curPU].push_back(matchedSite->getSiteX());
    }

    tmpAverageDisplacement /= PULevelMatching.size();

    if (verbose)
        print_info("CLBLegalizer Macro Cell Average Displacement = " + std::to_string(tmpAverageDisplacement));

    if (updateDisplacement)
    {
        if (isRoughLegalization)
            roughAverageDisplacement = tmpAverageDisplacement;
        else
            fixedColumnAverageDisplacement = tmpAverageDisplacement;
    }

    for (auto pairPU2X : PU2X)
    {
        int numCells = getPUSiteNum(pairPU2X.first);
        assert(numCells == 1); // currentlt CLB should only cover one site.
        PU2X[pairPU2X.first] /= numCells;
        PU2Y[pairPU2X.first] /= numCells;
    }

#pragma omp parallel for
    for (int i = 0; i < MCLBColumnNum; i++)
        sortPUsByPU2Y(MCLBColumn2PUs[i]);

#pragma omp parallel for
    for (int i = 0; i < LCLBColumnNum; i++)
        sortPUsByPU2Y(LCLBColumn2PUs[i]);

    placementInfo->setPULegalXY(PU2X, PU2Y);
}

void CLBLegalizer::spreadPUs(int columnNum, std::vector<int> &columnUntilization,
                             std::vector<std::vector<DeviceInfo::DeviceSite *>> &column2Sites,
                             std::vector<std::deque<PlacementInfo::PlacementUnit *>> &column2PUs,
                             std::map<PlacementInfo::PlacementUnit *, int> &cell2Column)
{
    while (true)
    {
        int overflowColId = -1;
        std::vector<int> accumulationUtil(columnNum, 0), accumulationCapacity(columnNum, 0);
        accumulationUtil[0] = columnUntilization[0];
        accumulationCapacity[0] = column2Sites[0].size();
        for (int colId = 1; colId < columnNum; colId++)
        {
            accumulationUtil[colId] = accumulationUtil[colId - 1] + columnUntilization[colId];
            accumulationCapacity[colId] = accumulationCapacity[colId - 1] + column2Sites[colId].size();
        }

        for (int colId = 0; colId < columnNum; colId++)
        {
            if (column2Sites[colId].size() < (unsigned int)columnUntilization[colId])
            {
                overflowColId = colId;
                break;
            }
        }
        if (overflowColId < 0)
        {
            break;
        }
        else
        {
            int leftAvaliableCapacity = 0;
            int rightAvaliableCapacity = 0;
            int leftUtil = 0;
            int rightUtil = 0;

            if (overflowColId > 0)
            {
                leftAvaliableCapacity += accumulationCapacity[overflowColId - 1];
                leftUtil += accumulationUtil[overflowColId - 1];
            }
            if (overflowColId < columnNum - 1)
            {
                rightAvaliableCapacity += accumulationCapacity[columnNum - 1] - accumulationCapacity[overflowColId];
                rightUtil += accumulationUtil[columnNum - 1] - accumulationUtil[overflowColId];
            }

            int overflowNum = (unsigned int)columnUntilization[overflowColId] - column2Sites[overflowColId].size();
            int toLeft = 0;

            int totalAvailableCapacity = rightAvaliableCapacity + leftAvaliableCapacity - leftUtil - rightUtil;
            assert(totalAvailableCapacity > 0);
            float toLeftRatio = (float)(leftAvaliableCapacity - leftUtil) / totalAvailableCapacity;
            int toLeftNum = int((overflowNum * toLeftRatio) + 0.4999);
            int toRightNum = overflowNum - toLeft;

            if (leftAvaliableCapacity - leftUtil > 0)
            {
                while (toLeftNum > 0 && column2PUs[overflowColId].size() > 0)
                {
                    column2PUs[overflowColId - 1].push_back(column2PUs[overflowColId][0]);
                    int macroSize = getPUSiteNum(column2PUs[overflowColId][0]);
                    columnUntilization[overflowColId - 1] += macroSize;
                    columnUntilization[overflowColId] -= macroSize;
                    toLeftNum -= macroSize;
                    column2PUs[overflowColId].pop_front();
                }
            }
            if (rightAvaliableCapacity - rightUtil > 0)
            {
                while (toRightNum > 0 && column2PUs[overflowColId].size() > 0)
                {
                    column2PUs[overflowColId + 1].push_front(
                        column2PUs[overflowColId][column2PUs[overflowColId].size() - 1]);
                    int macroSize = getPUSiteNum(column2PUs[overflowColId][column2PUs[overflowColId].size() - 1]);
                    columnUntilization[overflowColId + 1] += macroSize;
                    columnUntilization[overflowColId] -= macroSize;
                    toRightNum -= macroSize;
                    column2PUs[overflowColId].pop_back();
                }
            }
        }
    }
    cell2Column.clear();
    for (unsigned int colId = 0; colId < column2PUs.size(); colId++)
    {
        auto PUs = column2PUs[colId];
        for (auto curPU : PUs)
        {
            cell2Column[curPU] = colId;
        }
    }
}

void CLBLegalizer::resolveOverflowColumns()
{
    if (enableMCLBLegalization)
    {
        spreadPUs(MCLBColumnNum, MCLBColumnUntilization, MCLBColumn2Sites, MCLBColumn2PUs, MCLB2Column);
    }
    if (enableLCLBLegalization)
    {
        spreadPUs(LCLBColumnNum, LCLBColumnUntilization, LCLBColumn2Sites, LCLBColumn2PUs, LCLB2Column);
    }
}

int CLBLegalizer::findIdMaxWithRecurence(int minId, int maxId, std::vector<int> &ids)
{
    int resId = -1;
    int maxRecurence = 0;
    assert(ids.size() > 0);
    for (int i = minId; i <= maxId; i++)
    {
        int cnt = 0;
        for (auto id : ids)
        {
            if (id == i)
                cnt++;
        }
        if (cnt > maxRecurence)
        {
            resId = i;
            maxRecurence = cnt;
        }
    }
    assert(resId >= 0);
    return resId;
}

void CLBLegalizer::mapPUsToColumns()
{

    MCLBColumn2PUs.clear();
    MCLBColumnUntilization.clear();
    if (enableMCLBLegalization)
    {
        if (MCLBColumnNum > 0)
        {
            MCLBColumn2PUs.resize(MCLBColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
            MCLBColumnUntilization.resize(MCLBColumnNum, 0);

            for (auto &PUCol_pair : PU2Columns)
            {
                auto tmpMacroUnit = PUCol_pair.first;
                if (MCLBPUs.find(tmpMacroUnit) != MCLBPUs.end())
                {
                    int colId = findIdMaxWithRecurence(0, MCLBColumnNum - 1, PUCol_pair.second);

                    MCLBColumn2PUs[colId].push_back(tmpMacroUnit);
                    MCLBColumnUntilization[colId] += getPUSiteNum(tmpMacroUnit);
                }
            }
        }
    }

    LCLBColumn2PUs.clear();
    LCLBColumnUntilization.clear();
    if (enableLCLBLegalization)
    {
        if (LCLBColumnNum > 0)
        {
            LCLBColumn2PUs.resize(LCLBColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
            LCLBColumnUntilization.resize(LCLBColumnNum, 0);

            for (auto &PUCol_pair : PU2Columns)
            {
                auto tmpMacroUnit = PUCol_pair.first;
                if (LCLBPUs.find(tmpMacroUnit) != LCLBPUs.end())
                {
                    int colId = findIdMaxWithRecurence(0, LCLBColumnNum - 1, PUCol_pair.second);

                    LCLBColumn2PUs[colId].push_back(tmpMacroUnit);
                    LCLBColumnUntilization[colId] += getPUSiteNum(tmpMacroUnit);
                }
            }
        }
    }
}

void CLBLegalizer::setSitesMapped()
{
    for (auto matchedPair : PULevelMatching)
    {
        auto targetPU = matchedPair.first;
        for (auto curSite : PU2LegalSites[targetPU])
        {
            assert(!curSite->isMapped());
            curSite->setMapped();
        }
    }
}

void CLBLegalizer::resetSitesMapped()
{

    for (auto matchedPair : PULevelMatching)
    {
        auto targetPU = matchedPair.first;
        for (auto curSite : PU2LegalSites[targetPU])
        {
            assert(curSite->isMapped());
            curSite->resetMapped();
        }
    }
    PULevelMatching.clear();
}