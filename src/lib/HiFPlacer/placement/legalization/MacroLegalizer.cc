/**
 * @file MacroLegalizer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation of the MacroLegalizer which maps DSP/BRAM/CARRY macros
 * to legal location
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "MacroLegalizer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

MacroLegalizer::MacroLegalizer(std::string legalizerName, PlacementInfo *placementInfo, DeviceInfo *deviceInfo,
                               std::vector<DesignInfo::DesignCellType> &macroTypesToLegalize,
                               std::map<std::string, std::string> &JSONCfg)
    : legalizerName(legalizerName), placementInfo(placementInfo), deviceInfo(deviceInfo),
      compatiblePlacementTable(placementInfo->getCompatiblePlacementTable()),
      macroTypesToLegalize(macroTypesToLegalize), cellLoc(placementInfo->getCellId2location()), JSONCfg(JSONCfg)
{
    macroCellsToLegalize.clear();
    PU2X.clear();
    PU2Y.clear();
    PU2LegalSites.clear();

    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
    {
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);
    }

    if (JSONCfg.find("MacroLegalizationVerbose") != JSONCfg.end())
    {
        verbose = JSONCfg["MacroLegalizationVerbose"] == "true";
    }

    if (JSONCfg.find("jobs") != JSONCfg.end())
    {
        nJobs = std::stoi(JSONCfg["jobs"]);
    }

    clockRegionAware = false;
}

void MacroLegalizer::legalize(bool exactLegalization, bool directLegalization)
{
    if (verbose)
        print_status("MacroLegalizer[" + legalizerName + "] Started Legalization.");

    resetSettings();
    findMacroType2AvailableSites();
    getMacrosToLegalize();

    if (!directLegalization)
    {
        roughlyLegalize();
        updatePUMatchingLocation(true, !exactLegalization);
    }

    // dumpMatching();

    if (exactLegalization)
    {
        if (verbose)
            print_status("MacroLegalizer[" + legalizerName + "] Started Fixed-Column Legalization.");
        resetSettings();
        fixedColumnLegalize(directLegalization);
        updatePUMatchingLocation(false, true);
        finalLegalizeBasedOnDP();
        dumpMatching(exactLegalization);
    }

    setSitesMapped();
    if (verbose)
        print_status("MacroLegalizer[" + legalizerName + "] Finished Legalization.");
}

void MacroLegalizer::roughlyLegalize()
{
    while (macroCellsToLegalize.size())
    {
        findMacroCell2SitesInDistance(clockRegionAware);
        findPossibleLegalLocation(false);
        resetMacroCell2SitesInDistance();

        createBipartiteGraph();
        minCostBipartiteMatcher = new MinCostBipartiteMatcher(macro2Sites.size(), rightSiteIds.size(),
                                                              macro2Sites.size(), adjList, nJobs, verbose);

        minCostBipartiteMatcher->solve();
        updateMatchingAndUnmatchedMacroCells();

        displacementThreshold *= 2;
        if ((int)(maxNumCandidate * 2) > maxNumCandidate + 1)
            maxNumCandidate *= 2;
        else
            maxNumCandidate++;
        delete minCostBipartiteMatcher;
        minCostBipartiteMatcher = nullptr;
    }
}

void MacroLegalizer::fixedColumnLegalize(bool directLegalization)
{
    mapMacrosToColumns(directLegalization);
    resolveOverflowColumns();

    macroCellsToLegalize = initialMacrosToLegalize;

    while (macroCellsToLegalize.size())
    {
        findMacroCell2SitesInDistance(clockRegionAware);
        findPossibleLegalLocation(true);
        resetMacroCell2SitesInDistance();
        createBipartiteGraph();
        minCostBipartiteMatcher = new MinCostBipartiteMatcher(macro2Sites.size(), rightSiteIds.size(),
                                                              macro2Sites.size(), adjList, nJobs, verbose);

        minCostBipartiteMatcher->solve();
        updateMatchingAndUnmatchedMacroCells();

        displacementThreshold *= 2;
        maxNumCandidate *= 2;

        if ((int)(maxNumCandidate * 2) > maxNumCandidate + 1)
            maxNumCandidate *= 2;
        else
            maxNumCandidate++;

        delete minCostBipartiteMatcher;
        minCostBipartiteMatcher = nullptr;
    }
}

void MacroLegalizer::finalLegalizeBasedOnDP()
{
    PU2X.clear();
    PU2Y.clear();
    PU2LegalSites.clear();
    resetSettings();
    float tmpAverageDisplacement = 0.0;
    if (verbose)
        print_status("MacroLegalizer[" + legalizerName + "] Start finalLegalizeBasedOnDP");
    tmpAverageDisplacement += DPForMinHPWL(BRAMColumnNum, BRAMColumn2Sites, BRAMColumn2PUs);
    tmpAverageDisplacement += DPForMinHPWL(DSPColumnNum, DSPColumn2Sites, DSPColumn2PUs);
    tmpAverageDisplacement += DPForMinHPWL(CARRYColumnNum, CARRYColumn2Sites, CARRYColumn2PUs);
    finalAverageDisplacement = tmpAverageDisplacement / PU2X.size();
    if (verbose)
        print_status("MacroLegalizer[" + legalizerName +
                     "] Done finalLegalizeBasedOnDP: Macro Cell Average Final Legalization Displacement = " +
                     std::to_string(finalAverageDisplacement));

    placementInfo->setPULegalSite(PU2LegalSites);
    placementInfo->setPULegalXY(PU2X, PU2Y);
}

float MacroLegalizer::DPForMinHPWL(int colNum, std::vector<std::vector<DeviceInfo::DeviceSite *>> &Column2Sites,
                                   std::vector<std::deque<PlacementInfo::PlacementUnit *>> &Column2PUs)
{
    // final Legalization DP
    // i th macro (start from 0), j th row (start from 0)
    // f[i][j] = min(f[i-1][j-row[i]]+HPWLChange[i][j-row[i]+1],f[i][j-1])

    // std::string dumpFile =
    //     JSONCfg["DumpMacroLegalization"] + "-Exact-" + std::to_string(DumpMacroLegalizationCnt) + ".gz";

    // print_status("MacroLegalizer: dumping MacroLegalization archieve to: " + dumpFile);
    // DumpMacroLegalizationCnt++;
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
        int heightPURow = getMarcroCellNum(curColPU[0]);
        totalMacroCellNum += heightPURow;
        float minHPWLChange = 1100000000.0;
        for (unsigned int j = totalMacroCellNum - 1; j < curColSites.size(); j++)
        {
            float curHPWLChange = getHPWLChange(curColPU[0], curColSites[j - heightPURow + 1]);
            if ((curColSites[j]->getSiteY() - curColSites[j - heightPURow + 1]->getSiteY() != heightPURow - 1) ||
                curColSites[j]->getClockRegionY() != curColSites[j - heightPURow + 1]->getClockRegionY())
            {
                // we need to ensure that there is no occpupied sites in this range
                curHPWLChange = 1100000000.0;
            }
            if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curColPU[0]))
            {
                if (curMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_BRAM)
                {
                    // we need to ensure BRAM macros starts from site with even siteY
                    if (curMacro->getCells().size() > 1 && curColSites[j - heightPURow + 1]->getSiteY() % 2 != 0)
                    {
                        curHPWLChange = 1100000000.0;
                    }
                }
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
            int heightPURow = getMarcroCellNum(curColPU[i]);
            totalMacroCellNum += heightPURow;
            for (unsigned int j = totalMacroCellNum - 1; j < curColSites.size();
                 j++) // j start from heightPURow because PU0 must occupy 1+ site(s)
            {
                float curHPWLChange = getHPWLChange(curColPU[i], curColSites[j - heightPURow + 1]);
                if ((curColSites[j]->getSiteY() - curColSites[j - heightPURow + 1]->getSiteY() != heightPURow - 1) ||
                    curColSites[j]->getClockRegionY() != curColSites[j - heightPURow + 1]->getClockRegionY())
                {
                    // we need to ensure that there is no occpupied sites in this range
                    curHPWLChange = 1100000000.0;
                }
                if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curColPU[i]))
                {
                    if (curMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_BRAM)
                    {
                        // we need to ensure BRAM macros starts from site with even siteY
                        if (curMacro->getCells().size() > 1 && curColSites[j - heightPURow + 1]->getSiteY() % 2 != 0)
                        {
                            curHPWLChange = 1100000000.0;
                        }
                    }
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
            int heightPURow = getMarcroCellNum(curColPU[i]);
            while (!fChoice[i][minLastPUTop])
            {
                minLastPUTop--;
                assert(minLastPUTop >= 0);
            }
            auto curSite = curColSites[minLastPUTop - heightPURow + 1];
            assert(PU2Sites[curColPU[i]].size() == 0);
            assert(curSite);
            for (int curColSiteId = minLastPUTop - heightPURow + 1; curColSiteId <= minLastPUTop; curColSiteId++)
            {
                PU2Sites[curColPU[i]].push_back(curColSites[curColSiteId]);
            }
            minLastPUTop -= heightPURow;
            // outfile0 << "col#" << c << ": " << curColPU[i]->getName() << " PUY:" << PU2Y[curColPU[i]]
            //          << " numPUs:" << getMarcroCellNum(curColPU[i])
            //          << " netNum:" << curColPU[i]->getNetsSetPtr()->size() << "\n";
            // outfile0 << "    macthed with: " << curSite->getName() << " locX:" << curSite->X()
            //          << " locY:" << curSite->Y() << " HPWLIncrease:" << getHPWLChange(curColPU[i], curSite) << "\n";
        }
    }
    // writeStrToGZip(dumpFile, outfile0);
    // print_status("dumped MacroLegalization archieve to: " + dumpFile);
    for (auto &PUDeque : Column2PUs)
    {
        for (auto tmpPU : PUDeque)
        {
            assert(PU2Sites.find(tmpPU) != PU2Sites.end());
            auto &curSites = PU2Sites[tmpPU];
            assert(curSites.size());
            assert(PU2X.find(tmpPU) == PU2X.end());
            auto curSite = curSites[0];
            PU2LegalSites[tmpPU] = curSites;
            PU2X[tmpPU] = curSite->X();
            PU2Y[tmpPU] = curSite->Y();
            PULevelMatching.emplace_back(tmpPU, curSite);
            tmpTotalDisplacement += std::fabs(tmpPU->X() - curSite->X()) + std::fabs(tmpPU->Y() - curSite->Y());
        }
    }

    return tmpTotalDisplacement;
}

void MacroLegalizer::getMacrosToLegalize()
{
    macroCellsToLegalize.clear();
    macroUnitsToLegalizeSet.clear();
    BRAMPUs.clear();
    DSPPUs.clear();
    CARRYPUs.clear();
    for (auto curCell : placementInfo->getCells())
    {
        for (auto curType : macroTypesToLegalize)
        {
            if (curCell->getCellType() == curType)
            {
                auto curPU = placementInfo->getPlacementUnitByCell(curCell);
                if (!curPU->isLocked())
                {
                    macroCellsToLegalize.push_back(curCell);
                    macroUnitsToLegalizeSet.insert(curPU);
                    if (curCell->isBRAM())
                    {
                        BRAMPUs.insert(curPU);
                    }
                    if (curCell->isDSP())
                    {
                        DSPPUs.insert(curPU);
                    }
                    if (curCell->isCarry())
                    {
                        CARRYPUs.insert(curPU);
                    }
                }
            }
        }
    }
    initialMacrosToLegalize = macroCellsToLegalize;
}

void MacroLegalizer::findMacroType2AvailableSites()
{
    macroType2Sites.clear();
    for (auto curCellType : macroTypesToLegalize)
    {
        macroType2Sites[curCellType] = std::vector<DeviceInfo::DeviceSite *>(0);
        for (int SharedBELID : placementInfo->getPotentialBELTypeIDs(curCellType))
        {
            std::string targetSiteType =
                compatiblePlacementTable
                    ->sharedCellType2SiteType[compatiblePlacementTable->sharedCellBELTypes[SharedBELID]];
            std::vector<DeviceInfo::DeviceSite *> &sitesInType = deviceInfo->getSitesInType(targetSiteType);
            for (auto curSite : sitesInType)
            {
                if (!curSite->isOccupied() && !curSite->isMapped())
                {
                    // if (matchedSites.find(curSite) == matchedSites.end())
                    // {
                    macroType2Sites[curCellType].push_back(curSite);
                    // }
                }
            }
        }
    }

    for (auto curCellType : macroTypesToLegalize)
    {
        if (DesignInfo::isBRAM(curCellType))
        {
            enableBRAMLegalization = true;
            for (auto curSite : macroType2Sites[curCellType])
            {
                if (curSite->getSiteY() + 1 > BRAMRowNum)
                    BRAMRowNum = curSite->getSiteY() + 1;
                if (curSite->getSiteX() + 1 > BRAMColumnNum)
                    BRAMColumnNum = curSite->getSiteX() + 1;
            }
        }
        if (DesignInfo::isDSP(curCellType))
        {
            enableDSPLegalization = true;
            for (auto curSite : macroType2Sites[curCellType])
            {
                if (curSite->getSiteY() + 1 > DSPRowNum)
                    DSPRowNum = curSite->getSiteY() + 1;
                if (curSite->getSiteX() + 1 > DSPColumnNum)
                    DSPColumnNum = curSite->getSiteX() + 1;
            }
        }
        if (DesignInfo::isCarry(curCellType))
        {
            enableCARRYLegalization = true;
            for (auto curSite : macroType2Sites[curCellType])
            {
                if (curSite->getSiteY() + 1 > CARRYRowNum)
                    CARRYRowNum = curSite->getSiteY() + 1;
                if (curSite->getSiteX() + 1 > CARRYColumnNum)
                    CARRYColumnNum = curSite->getSiteX() + 1;
            }
        }
    }

    BRAMColumnXs.clear();
    if (enableBRAMLegalization)
    {
        BRAMColumnXs.resize(BRAMColumnNum, -1.0);
        BRAMColumn2Sites.clear();
        BRAMColumn2Sites.resize(BRAMColumnNum, std::vector<DeviceInfo::DeviceSite *>(0));
        for (auto curCellType : macroTypesToLegalize)
        {
            if (DesignInfo::isBRAM(curCellType))
            {
                for (auto curSite : macroType2Sites[curCellType])
                {
                    if (curSite->getSiteY() + 1 > BRAMRowNum)
                        BRAMRowNum = curSite->getSiteY() + 1;
                    if (curSite->getSiteX() + 1 > BRAMColumnNum)
                        BRAMColumnNum = curSite->getSiteX() + 1;
                    BRAMColumnXs[curSite->getSiteX()] = curSite->X();
                    BRAMColumn2Sites[curSite->getSiteX()].push_back(curSite);
                }
                break;
            }
        }
    }

    DSPColumnXs.clear();
    if (enableDSPLegalization)
    {
        DSPColumnXs.resize(DSPColumnNum, -1.0);
        DSPColumn2Sites.clear();
        DSPColumn2Sites.resize(DSPColumnNum, std::vector<DeviceInfo::DeviceSite *>(0));
        for (auto curCellType : macroTypesToLegalize)
        {
            if (DesignInfo::isDSP(curCellType))
            {
                for (auto curSite : macroType2Sites[curCellType])
                {
                    if (curSite->getSiteY() + 1 > DSPRowNum)
                        DSPRowNum = curSite->getSiteY() + 1;
                    if (curSite->getSiteX() + 1 > DSPColumnNum)
                        DSPColumnNum = curSite->getSiteX() + 1;
                    DSPColumnXs[curSite->getSiteX()] = curSite->X();
                    DSPColumn2Sites[curSite->getSiteX()].push_back(curSite);
                }
                break;
            }
        }
    }

    CARRYColumnXs.clear();
    if (enableCARRYLegalization)
    {
        CARRYColumnXs.resize(CARRYColumnNum, -1.0);
        CARRYColumn2Sites.clear();
        CARRYColumn2Sites.resize(CARRYColumnNum, std::vector<DeviceInfo::DeviceSite *>(0));
        for (auto curCellType : macroTypesToLegalize)
        {
            if (DesignInfo::isCarry(curCellType))
            {
                for (auto curSite : macroType2Sites[curCellType])
                {
                    if (curSite->getSiteY() + 1 > CARRYRowNum)
                        CARRYRowNum = curSite->getSiteY() + 1;
                    if (curSite->getSiteX() + 1 > CARRYColumnNum)
                        CARRYColumnNum = curSite->getSiteX() + 1;
                    CARRYColumnXs[curSite->getSiteX()] = curSite->X();
                    CARRYColumn2Sites[curSite->getSiteX()].push_back(curSite);
                }
                break;
            }
        }
    }
}

void MacroLegalizer::findPossibleLegalLocation(bool fixedColumn)
{
    macro2Sites.clear();

    for (auto curCell : macroCellsToLegalize)
    {
        macro2Sites[curCell] = std::vector<DeviceInfo::DeviceSite *>(0);
    }

    int numMacroCells = macroCellsToLegalize.size();

    if (verbose)
    {
        print_status("searching sites for " + std::to_string(numMacroCells) + " macroCells with displacement<" +
                     std::to_string(displacementThreshold));
    }
#pragma omp parallel for
    for (int i = 0; i < numMacroCells; i++)
    {
        auto curCell = macroCellsToLegalize[i];
        auto curCellType = curCell->getCellType();
        int macroLength = -1;
        int cellOffset = -1;

        if (DesignInfo::isDSP(curCellType) || DesignInfo::isBRAM(curCellType))
        {
            auto tmpPU = placementInfo->getPlacementUnitByCellId(curCell->getCellId());
            if (dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
            {
                macroLength = cellOffset = 1;
            }
            else if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
            {

                for (cellOffset = 0; cellOffset < (int)(tmpMacro->getCells().size()); cellOffset++)
                {
                    if (tmpMacro->getCell(cellOffset) == curCell)
                    {
                        macroLength = tmpMacro->getCells().size();
                        break;
                    }
                }
            }
        }

        assert(macroType2Sites.find(curCellType) != macroType2Sites.end());

        std::vector<DeviceInfo::DeviceSite *> *candidateSite = nullptr;
        if (fixedColumn)
        {
            int targetSiteX = -1;
            if (curCell->isDSP())
            {
                assert(DSPCell2Column.find(curCell) != DSPCell2Column.end());
                targetSiteX = DSPCell2Column[curCell];
            }
            if (curCell->isCarry())
            {
                assert(CARRYCell2Column.find(curCell) != CARRYCell2Column.end());
                targetSiteX = CARRYCell2Column[curCell];
            }
            if (curCell->isBRAM())
            {
                assert(BRAMCell2Column.find(curCell) != BRAMCell2Column.end());
                targetSiteX = BRAMCell2Column[curCell];
            }
            assert(targetSiteX >= 0);

            std::vector<std::vector<DeviceInfo::DeviceSite *>> *column2Sites;
            if (DesignInfo::isCarry(curCellType))
            {
                column2Sites = &CARRYColumn2Sites;
            }
            else if (DesignInfo::isDSP(curCellType))
            {
                column2Sites = &DSPColumn2Sites;
            }
            else if (DesignInfo::isBRAM(curCellType))
            {
                column2Sites = &BRAMColumn2Sites;
            }
            else
            {
                assert(false && "undefine type");
            }
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
            candidateSite = macro2SitesInDisplacementThreshold[curCell];
        }

        assert(candidateSite);

        for (auto curSite : *candidateSite)
        {
            float curDisp = getDisplacement(cellLoc[curCell->getCellId()], curSite);
            if ((!fixedColumn && curDisp < displacementThreshold) ||
                (fixedColumn && curDisp < 2 * displacementThreshold))
            {
                // since the cell in a macro, we need to ensure the other parts of the macro can be legalized if the
                // cell is placed to the site
                if (placementInfo->isLegalLocation(curCell, curSite->X(), curSite->Y()) ||
                    displacementThreshold > 2 * initialDisplacementThreshold)
                {
                    if (matchedSites.find(curSite) != matchedSites.end())
                        continue;

                    if ((curCellType == DesignInfo::CellType_RAMB36E2 ||
                         curCellType == DesignInfo::CellType_FIFO36E2) &&
                        curSite->getSiteY() % 2 != 0)
                        continue;
                    if ((curCellType == DesignInfo::CellType_RAMB18E2 ||
                         curCellType == DesignInfo::CellType_FIFO18E2) &&
                        curCell->isVirtualCell() && curSite->getSiteY() % 2 != 1)
                        continue;

                    if (macroLength > 1 && cellOffset >= 0 && clockRegionCasLegalization && fixedColumn)
                    {
                        int headLocation = curSite->getSiteY() - cellOffset;
                        int tailLocation = headLocation + macroLength - 1;
                        if (headLocation / clockRegionHeightOfDSE_BRAM != tailLocation / clockRegionHeightOfDSE_BRAM)
                        {
                            continue;
                        }
                    }

                    macro2Sites[curCell].push_back(curSite);
                }
            }
        }
        if (macro2Sites[curCell].size() > 1)
            quick_sort_WLChange(curCell, macro2Sites[curCell], 0, macro2Sites[curCell].size() - 1,
                                cellLoc[curCell->getCellId()]);
        if (macro2Sites[curCell].size() > (unsigned int)maxNumCandidate)
            macro2Sites[curCell].resize(maxNumCandidate);
    }

    // print_info("#total macro cell = " + std::to_string(macro2Sites.size()));
    // print_info("#total macro candidate site (might be duplicated) = " + std::to_string(totalSiteNum));
}

void MacroLegalizer::createBipartiteGraph()
{
    rightSiteIds.clear();
    adjList.resize(macroCellsToLegalize.size());
    siteList.clear();

    float minCost = 10000000;

    for (unsigned int leftCellId = 0; leftCellId < macroCellsToLegalize.size(); leftCellId++)
    {
        auto curCell = macroCellsToLegalize[leftCellId];
        adjList[leftCellId].clear();
        for (auto curSite : macro2Sites[curCell])
        {
            if (rightSiteIds.find(curSite) == rightSiteIds.end())
            {
                int curSiteCnt = rightSiteIds.size();
                rightSiteIds[curSite] = curSiteCnt;
                siteList.push_back(curSite);
            }
            float tmpCost = getHPWLChange(curCell, curSite);
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
        for (unsigned int leftCellId = 0; leftCellId < macroCellsToLegalize.size(); leftCellId++)
        {
            for (unsigned int tmpId = 0; tmpId < adjList[leftCellId].size(); tmpId++)
            {
                adjList[leftCellId][tmpId].second += compensation;
            }
        }
    }
}

void MacroLegalizer::updateMatchingAndUnmatchedMacroCells()
{

    for (unsigned int leftCellId = 0; leftCellId < macroCellsToLegalize.size(); leftCellId++)
    {
        int rightNode = minCostBipartiteMatcher->getMatchedRightNode(leftCellId);
        auto curCell = macroCellsToLegalize[leftCellId];
        if (rightNode >= 0)
        {
            assert(matchedMacroCells.find(curCell) == matchedMacroCells.end());
            assert(matchedSites.find(siteList[rightNode]) == matchedSites.end());
            matchedMacroCells.insert(curCell);
            matchedSites.insert(siteList[rightNode]);
            cellLevelMatching.emplace_back(curCell, siteList[rightNode]);
        }
    }
    std::vector<DesignInfo::DesignCell *> newMacrosToLegalize;
    newMacrosToLegalize.clear();
    for (unsigned int leftCellId = 0; leftCellId < macroCellsToLegalize.size(); leftCellId++)
    {
        auto curCell = macroCellsToLegalize[leftCellId];
        if (matchedMacroCells.find(curCell) == matchedMacroCells.end())
        {
            newMacrosToLegalize.push_back(curCell);
        }
    }
    macroCellsToLegalize = newMacrosToLegalize;
}

void MacroLegalizer::dumpMatching(bool fixedColumn, bool enforce)
{
    if (JSONCfg.find("DumpMacroLegalization") != JSONCfg.end() || enforce)
    {
        std::string dumpFile = "";
        if (enforce)
            dumpFile = JSONCfg["dumpDirectory"] + legalizerName + "DumpMacroLegalization-" +
                       std::to_string(DumpMacroLegalizationCnt) + ".gz";
        else
            dumpFile = JSONCfg["DumpMacroLegalization"] + "-" + legalizerName +
                       std::to_string(DumpMacroLegalizationCnt) + ".gz";

        print_status("MacroLegalizer: dumping MacroLegalization archieve to: " + dumpFile);
        DumpMacroLegalizationCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            if (cellLevelMatching.size() != 0)
            {
                for (auto matchedPair : cellLevelMatching)
                {
                    auto matchedSite = matchedPair.second;
                    auto curCell = matchedPair.first;
                    auto curPU = placementInfo->getPlacementUnitByCell(curCell);
                    outfile0 << "CellLevelMatching name: " << curCell->getName() << " CellId: " << curCell->getCellId()
                             << " PUID: " << curPU->getId() << "\n        locX:" << cellLoc[curCell->getCellId()].X
                             << "\n        locY:" << cellLoc[curCell->getCellId()].Y << "\n";
                    outfile0 << "    macthed with: " << matchedSite->getName() << "\n        locX:" << matchedSite->X()
                             << "\n        locY:" << matchedSite->Y()
                             << "\n        dis:" << getDisplacement(cellLoc[curCell->getCellId()], matchedSite)
                             << "\n        HPWLIncrease:" << getHPWLChange(curCell, matchedSite) << "\n";
                }
            }

            if (PULevelMatching.size() != 0)
            {
                for (auto matchedPair : PULevelMatching)
                {
                    auto matchedSite = matchedPair.second;
                    auto curPU = matchedPair.first;
                    outfile0 << "PULevelMatching name: " << curPU->getName() << " PUID: " << curPU->getId()
                             << "\n        locX:" << curPU->X() << "\n        locY:" << curPU->Y()
                             << "\n        PU2X:" << PU2X[curPU] << "\n        PU2Y:" << PU2Y[curPU] << "\n";
                    assert(std::fabs(matchedSite->X() - PU2X[curPU]) + std::fabs(matchedSite->Y() - PU2Y[curPU]) < 0.1);
                    assert(std::fabs(matchedSite->X() - placementInfo->getPULegalXY().first[curPU]) +
                               std::fabs(matchedSite->Y() - placementInfo->getPULegalXY().second[curPU]) <
                           0.1);
                    outfile0 << "    macthed with: " << matchedSite->getName() << "\n        locX:" << matchedSite->X()
                             << "\n        locY:" << matchedSite->Y()
                             << "\n        dis:" << getDisplacement(curPU, matchedSite)
                             << "\n        HPWLIncrease:" << getHPWLChange(curPU, matchedSite) << "\n";
                }
            }

            if (fixedColumn)
            {
                for (int i = 0; i < BRAMColumnNum; i++)
                {
                    int numPUs = BRAMColumn2PUs[i].size();
                    for (int j = 0; j < numPUs; j++)
                    {
                        auto PU = BRAMColumn2PUs[i][j];
                        outfile0 << "BRAM col#" << i << ": " << PU->getName() << " PUY:" << PU2Y[PU]
                                 << " numPUs:" << getMarcroCellNum(PU) << " netNum:" << PU->getNetsSetPtr()->size()
                                 << "\n";
                    }
                }
                for (int i = 0; i < DSPColumnNum; i++)
                {
                    int numPUs = DSPColumn2PUs[i].size();
                    for (int j = 0; j < numPUs; j++)
                    {
                        auto PU = DSPColumn2PUs[i][j];
                        outfile0 << "DSP col#" << i << ": " << PU->getName() << " PUY:" << PU2Y[PU]
                                 << " numPUs:" << getMarcroCellNum(PU) << " netNum:" << PU->getNetsSetPtr()->size()
                                 << "\n";
                    }
                }
                for (int i = 0; i < CARRYColumnNum; i++)
                {
                    int numPUs = CARRYColumn2PUs[i].size();
                    for (int j = 0; j < numPUs; j++)
                    {
                        auto PU = CARRYColumn2PUs[i][j];
                        outfile0 << "CARRY col#" << i << ": " << PU->getName() << " PUY:" << PU2Y[PU]
                                 << " numPUs:" << getMarcroCellNum(PU) << " netNum:" << PU->getNetsSetPtr()->size()
                                 << "\n";
                    }
                }
            }

            writeStrToGZip(dumpFile, outfile0);
            print_status("MacroLegalizer: dumped MacroLegalization archieve to: " + dumpFile);
        }
    }
}

int MacroLegalizer::getMarcroCellNum(PlacementInfo::PlacementUnit *tmpMacroUnit)
{
    if (dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpMacroUnit))
    {
        return 1;
    }
    else if (auto macroPU = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpMacroUnit))
    {
        if (macroPU->checkHasBRAM())
            return macroPU->getBRAMNum();
        else if (macroPU->checkHasDSP())
            return macroPU->getDSPNum();
        else if (macroPU->checkHasCARRY())
            return macroPU->getCARRYNum();
        else
        {
            assert(false && "undefined legaliation macro");
            return 1;
        }
    }
    assert(false);
    return 1;
}

void MacroLegalizer::sortPUsByPU2Y(std::deque<PlacementInfo::PlacementUnit *> &PUs)
{
    int numPUs = PUs.size();
    for (int i = 0; i < numPUs; i++)
        for (int j = i + 1; j < numPUs; j++)
            if (PU2Y[PUs[i]] > PU2Y[PUs[j]])
                swapPU(&PUs[i], &PUs[j]);
}

void MacroLegalizer::sortSitesBySiteY(std::vector<DeviceInfo::DeviceSite *> &sites)
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

void MacroLegalizer::updatePUMatchingLocation(bool isRoughLegalization, bool updateDisplacement)
{
    PU2X.clear();
    PU2Y.clear();
    PU2Columns.clear();
    PU2SiteX.clear();
    PU2LegalSites.clear();

    BRAMColumn2PUs.clear();
    if (BRAMColumnNum > 0)
    {
        BRAMColumn2PUs.resize(BRAMColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
    }
    DSPColumn2PUs.clear();
    if (DSPColumnNum > 0)
    {
        DSPColumn2PUs.resize(DSPColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
    }
    CARRYColumn2PUs.clear();
    if (CARRYColumnNum > 0)
    {
        CARRYColumn2PUs.resize(CARRYColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
    }

    float tmpAverageDisplacement = 0.0;

    for (auto matchedPair : cellLevelMatching)
    {
        auto matchedSite = matchedPair.second;
        auto curCell = matchedPair.first;

        auto curPU = placementInfo->getPlacementUnitByCell(curCell);

        if (PU2Y.find(curPU) == PU2Y.end())
        {
            PU2SiteX[curPU] = -1;
            PU2Y[curPU] = 0.0;
            if (curCell->isBRAM())
            {
                BRAMColumn2PUs[matchedSite->getSiteX()].push_back(curPU);
            }
            if (curCell->isDSP())
            {
                DSPColumn2PUs[matchedSite->getSiteX()].push_back(curPU);
            }
            if (curCell->isCarry())
            {
                CARRYColumn2PUs[matchedSite->getSiteX()].push_back(curPU);
            }
        }

        float actualPUX = 0;
        float actualPUY = 0;
        // since the matched site is for a single DSP/BRAM/CARRY cell, we need to find its corresponding placement macro
        // (with multiple DSPs/BRAMs/CARRYs)
        placementInfo->getPULocationByCellLocation(curCell, matchedSite->X(), matchedSite->Y(), actualPUX, actualPUY);

        tmpAverageDisplacement += std::fabs(cellLoc[curCell->getCellId()].X - matchedSite->X()) +
                                  std::fabs(cellLoc[curCell->getCellId()].Y - matchedSite->Y());
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

    tmpAverageDisplacement /= cellLevelMatching.size();

    if (verbose)
        print_info("MacroLegalizer[" + legalizerName +
                   "] Macro Cell Average Displacement = " + std::to_string(tmpAverageDisplacement));

    if (updateDisplacement)
    {
        if (isRoughLegalization)
            roughAverageDisplacement = tmpAverageDisplacement;
        else
            fixedColumnAverageDisplacement = tmpAverageDisplacement;
    }

    for (auto pairPU2X : PU2X)
    {
        int numCells = getMarcroCellNum(pairPU2X.first);
        PU2X[pairPU2X.first] /= numCells;
        PU2Y[pairPU2X.first] /= numCells;
    }

#pragma omp parallel for
    for (int i = 0; i < BRAMColumnNum; i++)
        sortPUsByPU2Y(BRAMColumn2PUs[i]);

#pragma omp parallel for
    for (int i = 0; i < DSPColumnNum; i++)
        sortPUsByPU2Y(DSPColumn2PUs[i]);

#pragma omp parallel for
    for (int i = 0; i < CARRYColumnNum; i++)
        sortPUsByPU2Y(CARRYColumn2PUs[i]);

    placementInfo->setPULegalXY(PU2X, PU2Y);
}

void MacroLegalizer::spreadMacros(int columnNum, std::vector<int> &columnUntilization,
                                  std::vector<std::vector<DeviceInfo::DeviceSite *>> &column2Sites,
                                  std::vector<std::deque<PlacementInfo::PlacementUnit *>> &column2PUs,
                                  std::map<DesignInfo::DesignCell *, int> &cell2Column, float budgeRatio)
{
    while (true)
    {
        int overflowColId = -1;
        std::vector<int> accumulationUtil(columnNum, 0), accumulationCapacity(columnNum, 0);
        accumulationUtil[0] = columnUntilization[0];
        accumulationCapacity[0] = column2Sites[0].size() * budgeRatio;
        for (int colId = 1; colId < columnNum; colId++)
        {
            accumulationUtil[colId] = accumulationUtil[colId - 1] + columnUntilization[colId];
            accumulationCapacity[colId] = accumulationCapacity[colId - 1] + column2Sites[colId].size() * budgeRatio;
        }

        for (int colId = 0; colId < columnNum; colId++)
        {
            if (budgeRatio == 1)
            {
                if (column2Sites[colId].size() < (unsigned int)columnUntilization[colId])
                {
                    overflowColId = colId;
                    break;
                }
            }
            else
            {
                if (column2Sites[colId].size() * (budgeRatio + 0.05) < (unsigned int)columnUntilization[colId])
                {
                    overflowColId = colId;
                    break;
                }
            }
        }
        if (overflowColId < 0)
        {
            break;
        }
        else
        {
            print_warning("column overflow resolving");
            for (int colId = 0; colId < columnNum; colId++)
            {
                std::cout << " colId#" << colId << " columnUntilization:" << columnUntilization[colId] << " "
                          << " siteCap:" << column2Sites[colId].size() << "\n";
            }
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

            int overflowNum = (unsigned int)columnUntilization[overflowColId] -
                              column2Sites[overflowColId].size() * budgeRatio; // spread more for redundant space
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
                    int macroSize = getMarcroCellNum(column2PUs[overflowColId][0]);
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
                    int macroSize = getMarcroCellNum(column2PUs[overflowColId][column2PUs[overflowColId].size() - 1]);
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
            if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
            {
                auto curCell = unpackedCell->getCell();
                cell2Column[curCell] = colId;
            }
            else if (auto macroPU = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
            {
                for (auto curCell : macroPU->getCells())
                {
                    cell2Column[curCell] = colId;
                }
            }
        }
    }
}

void MacroLegalizer::resolveOverflowColumns()
{
    if (enableBRAMLegalization)
    {
        spreadMacros(BRAMColumnNum, BRAMColumnUntilization, BRAMColumn2Sites, BRAMColumn2PUs, BRAMCell2Column, 0.9);
    }
    if (enableDSPLegalization)
    {
        spreadMacros(DSPColumnNum, DSPColumnUntilization, DSPColumn2Sites, DSPColumn2PUs, DSPCell2Column);
    }
    if (enableCARRYLegalization)
    {
        spreadMacros(CARRYColumnNum, CARRYColumnUntilization, CARRYColumn2Sites, CARRYColumn2PUs, CARRYCell2Column,
                     0.9);
    }
}

int MacroLegalizer::findIdMaxWithRecurence(int minId, int maxId, std::vector<int> &ids)
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

int MacroLegalizer::findCorrespondingColumn(float curX, std::vector<float> &Xs)
{
    int resId = -1;
    float closeDiff = 100000000;

    for (unsigned int i = 0; i < Xs.size(); i++)
    {
        if (std::fabs(curX - Xs[i]) < closeDiff)
        {
            closeDiff = std::fabs(curX - Xs[i]);
            resId = i;
        }
    }

    assert(resId >= 0);
    return resId;
}

void MacroLegalizer::mapMacrosToColumns(bool directLegalization)
{
    BRAMColumn2PUs.clear();
    BRAMColumnUntilization.clear();
    if (BRAMColumnNum > 0)
    {
        BRAMColumn2PUs.resize(BRAMColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
        BRAMColumnUntilization.resize(BRAMColumnNum, 0);

        if (directLegalization)
        {
            for (auto &tmpMacroUnit : BRAMPUs)
            {
                if (BRAMPUs.find(tmpMacroUnit) != BRAMPUs.end())
                {
                    int colId = findCorrespondingColumn(tmpMacroUnit->X(), BRAMColumnXs);
                    BRAMColumn2PUs[colId].push_back(tmpMacroUnit);
                    BRAMColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
        else
        {
            for (auto &PUCol_pair : PU2Columns)
            {
                auto tmpMacroUnit = PUCol_pair.first;
                if (BRAMPUs.find(tmpMacroUnit) != BRAMPUs.end())
                {
                    int colId = findIdMaxWithRecurence(0, BRAMColumnNum - 1, PUCol_pair.second);

                    BRAMColumn2PUs[colId].push_back(tmpMacroUnit);
                    BRAMColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
    }

    DSPColumn2PUs.clear();
    DSPColumnUntilization.clear();
    if (DSPColumnNum > 0)
    {
        DSPColumn2PUs.resize(DSPColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
        DSPColumnUntilization.resize(DSPColumnNum, 0);

        if (directLegalization)
        {
            for (auto tmpMacroUnit : DSPPUs)
            {
                if (DSPPUs.find(tmpMacroUnit) != DSPPUs.end())
                {
                    int colId = findCorrespondingColumn(tmpMacroUnit->X(), DSPColumnXs);

                    DSPColumn2PUs[colId].push_back(tmpMacroUnit);
                    DSPColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
        else
        {
            for (auto &PUCol_pair : PU2Columns)
            {
                auto tmpMacroUnit = PUCol_pair.first;
                if (DSPPUs.find(tmpMacroUnit) != DSPPUs.end())
                {
                    int colId = findIdMaxWithRecurence(0, DSPColumnNum - 1, PUCol_pair.second);

                    DSPColumn2PUs[colId].push_back(tmpMacroUnit);
                    DSPColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
    }

    CARRYColumn2PUs.clear();
    CARRYColumnUntilization.clear();
    if (CARRYColumnNum > 0)
    {
        CARRYColumn2PUs.resize(CARRYColumnNum, std::deque<PlacementInfo::PlacementUnit *>(0));
        CARRYColumnUntilization.resize(CARRYColumnNum, 0);
        if (directLegalization)
        {
            for (auto tmpMacroUnit : CARRYPUs)
            {
                if (CARRYPUs.find(tmpMacroUnit) != CARRYPUs.end())
                {
                    int colId = findCorrespondingColumn(tmpMacroUnit->X(), CARRYColumnXs);

                    CARRYColumn2PUs[colId].push_back(tmpMacroUnit);
                    CARRYColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
        else
        {
            for (auto &PUCol_pair : PU2Columns)
            {
                auto tmpMacroUnit = PUCol_pair.first;
                if (CARRYPUs.find(tmpMacroUnit) != CARRYPUs.end())
                {
                    int colId = findIdMaxWithRecurence(0, CARRYColumnNum - 1, PUCol_pair.second);

                    CARRYColumn2PUs[colId].push_back(tmpMacroUnit);
                    CARRYColumnUntilization[colId] += getMarcroCellNum(tmpMacroUnit);
                }
            }
        }
    }
}

void MacroLegalizer::setSitesMapped()
{
    if (PULevelMatching.size() == 0)
    {
        for (auto matchedPair : cellLevelMatching)
        {
            auto matchedSite = matchedPair.second;
            assert(!matchedSite->isMapped());
            matchedSite->setMapped();
        }
    }
    else
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
}

void MacroLegalizer::resetSitesMapped()
{
    if (PULevelMatching.size() == 0)
    {
        for (auto matchedPair : cellLevelMatching)
        {
            auto matchedSite = matchedPair.second;
            assert(matchedSite->isMapped());
            matchedSite->resetMapped();
        }
    }
    else
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
}