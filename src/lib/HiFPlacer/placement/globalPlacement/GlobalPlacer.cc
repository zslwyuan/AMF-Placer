/**
 * @file GlobalPlacer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "GlobalPlacer.h"

#include <cmath>
#include <codecvt>

GlobalPlacer::GlobalPlacer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                           bool resetLegalizationInfo)
    : placementInfo(placementInfo), JSONCfg(JSONCfg)
{
    if (JSONCfg.find("GlobalPlacerVerbose") != JSONCfg.end())
        verbose = JSONCfg["GlobalPlacerVerbose"] == "true";
    if (JSONCfg.find("DumpLUTCoordTrace") != JSONCfg.end() || JSONCfg.find("DumpDSPCoordTrace") != JSONCfg.end() ||
        JSONCfg.find("DumpFFCoordTrace") != JSONCfg.end() || JSONCfg.find("DumpLUTFFCoordTrace") != JSONCfg.end() ||
        JSONCfg.find("DumpCARRYCoordTrace") != JSONCfg.end() || JSONCfg.find("DumpAllCoordTrace") != JSONCfg.end())
        dumpOptTrace = true;
    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);

    if (JSONCfg.find("disableSpreadingConvergeRatio") != JSONCfg.end())
    {
        disableSpreadingForgetRatio = JSONCfg["disableSpreadingConvergeRatio"] == "true";
    }
    if (JSONCfg.find("DirectMacroLegalize") != JSONCfg.end())
    {
        directMacroLegalize = JSONCfg["DirectMacroLegalize"] == "true";
        if (directMacroLegalize)
        {
            print_warning("Direct Macro Legalization is enbaled. It might undermine the HPWL.");
        }
    }

    if (JSONCfg.find("DumpClockUtilization") != JSONCfg.end())
    {
        dumpClockUtilization = JSONCfg["DumpClockUtilization"] == "true";
    }

    clusterPlacer = new ClusterPlacer(placementInfo, JSONCfg, 10.0);
    WLOptimizer = new WirelengthOptimizer(placementInfo, JSONCfg, verbose);

    std::vector<DesignInfo::DesignCellType> macroTypesToLegalize;
    macroTypesToLegalize.clear();
    macroTypesToLegalize.push_back(DesignInfo::CellType_RAMB18E2);
    macroTypesToLegalize.push_back(DesignInfo::CellType_RAMB36E2);
    macroTypesToLegalize.push_back(DesignInfo::CellType_DSP48E2);
    BRAMDSPLegalizer = new MacroLegalizer("BRAMDSPLegalizer", placementInfo, placementInfo->getDeviceInfo(),
                                          macroTypesToLegalize, JSONCfg);
    macroTypesToLegalize.clear();
    macroTypesToLegalize.push_back(DesignInfo::CellType_CARRY8);
    CARRYMacroLegalizer = new MacroLegalizer("CARRYMacroLegalizer", placementInfo, placementInfo->getDeviceInfo(),
                                             macroTypesToLegalize, JSONCfg);

    std::vector<std::string> lclbTypeList(1, "SLICEL");
    lCLBLegalizer =
        new CLBLegalizer("lCLBLegalizer", placementInfo, placementInfo->getDeviceInfo(), lclbTypeList, JSONCfg);

    std::vector<std::string> mclbTypeList(1, "SLICEM");
    mCLBLegalizer =
        new CLBLegalizer("mCLBLegalizer", placementInfo, placementInfo->getDeviceInfo(), mclbTypeList, JSONCfg);

    if (resetLegalizationInfo)
        placementInfo->resetPULegalInformation();

    if (JSONCfg.find("pseudoNetWeightConsiderNetNum") != JSONCfg.end())
    {
        pseudoNetWeightConsiderNetNum = JSONCfg["pseudoNetWeightConsiderNetNum"] == "true";
    }
    else
    {
        pseudoNetWeightConsiderNetNum = true;
    }

    if (pseudoNetWeightConsiderNetNum)
    {
        print_warning("pseudoNetWeightConsiderNetNum option is turn on: " +
                      std::to_string(placementInfo->getPUWithManyNetsRatio()));
    }
    else
    {
        print_warning("pseudoNetWeightConsiderNetNum option is turn off: " +
                      std::to_string(placementInfo->getPUWithManyNetsRatio()));
    }

    minHPWL = placementInfo->getMinHPWL();
    historyHPWLs.clear();
}

void GlobalPlacer::clusterPlacement()
{
    clusterPlacer->ClusterPlacement();
    print_info("ClusterPlacement Total HPWL = " + std::to_string(placementInfo->updateB2BAndGetTotalHPWL()));
}

void GlobalPlacer::GlobalPlacement_CLBElements(int iterNum, bool continuePreviousIteration, int lowerBoundIterNum,
                                               bool enableMacroPseudoNet2Site, bool stopStrictly,
                                               PlacementTimingOptimizer *timingOptimizer)
{

    print_status("GlobalPlacer GlobalPlacement_CLBElements started");

    bool hasUserDefinedClusterInfo = JSONCfg.find("designCluster") != JSONCfg.end();

    bool printHPWL = false;
    if (JSONCfg.find("GlobalPlacerPrintHPWL") != JSONCfg.end())
        printHPWL = JSONCfg["GlobalPlacerPrintHPWL"] == "true";

    WLOptimizer->reloadPlacementInfo();

    pseudoNetWeight = 1.0;
    bool BLEUnpackedYet = !continuePreviousIteration;
    bool shouldLegalize = !stopStrictly;

    if (!continuePreviousIteration || oriPseudoNetWeight < 0)
    {
        if (JSONCfg.find("PseudoNetWeight") != JSONCfg.end())
            pseudoNetWeight = std::stof(JSONCfg["PseudoNetWeight"]);
    }
    else
    {
        pseudoNetWeight = oriPseudoNetWeight;
    }

    historyAverageDisplacement.clear();
    if (placementInfo->getProgress() > 0.1)
        progressRatio = placementInfo->getProgress();
    else
        progressRatio = 0.0;

    spreadingForgetRatio = 0.2;

    if (placementInfo->updateB2BAndGetTotalHPWL() * 100 < minHPWL)
    {
        minHPWL = placementInfo->updateB2BAndGetTotalHPWL() * 100;
    }
    upperBoundHPWL = minHPWL;
    lowerBoundHPWL = 1;

    int iterCntAfterMacrosFixed = 0;

    // global placement iterations
    for (int i = 0; i < iterNum || (!stopStrictly && shouldLegalize); i++)
    {

        // lowerBound: Quadratic Programming based Wirelength Optimization
        lowerBoundIterNum = (placementInfo->getProgress() < 0.965 && !macroCloseToSite) ? 2 : 2;

        for (int j = 0; j < lowerBoundIterNum; j++)
        {
            WLOptimizer->GlobalPlacementQPSolve(pseudoNetWeight, j == 0, true, enableMacroPseudoNet2Site,
                                                pseudoNetWeightConsiderNetNum,
                                                (i > 1 || continuePreviousIteration) && hasUserDefinedClusterInfo &&
                                                        progressRatio<0.6, progressRatio> 0.5 ||
                                                    timingOptEnabled);
            if (progressRatio > 0.5)
                timingOptEnabled = true;
        }

        lowerBoundHPWL = placementInfo->updateB2BAndGetTotalHPWL();
        print_info("HPWL after QP=" + std::to_string(lowerBoundHPWL) +
                   " pseudoNetWeight=" + std::to_string(pseudoNetWeight));
        print_status("WLOptimizer Iteration#" + to_string_align3(i) + " Done HPWL=" + std::to_string(lowerBoundHPWL));

        if (dumpOptTrace)
        {
            dumpCoord();
        }

        // upperBound: Placement Unit Spreading
        spreading(i);

        upperBoundHPWL = placementInfo->updateB2BAndGetTotalHPWL();
        print_status("Spreader Iteration#" + to_string_align3(i) + " Done HPWL=" + std::to_string(upperBoundHPWL));

        if (dumpOptTrace)
        {
            dumpCoord();
        }

        // legalize macros (DSPs/BRAMs)
        if (enableMacroPseudoNet2Site)
        {
            if (BLEUnpackedYet)
            {
                if (i >= 8)
                {
                    macroLegalize(i);
                    print_status("Legalization Iteration#" + to_string_align3(i) + " Done");
                }
            }
            else
            {
                macroLegalize(i);
                print_status("Legalization Iteration#" + to_string_align3(i) + " Done");
            }
        }

        if (printHPWL)
        {
            print_info("HPWL after LG=" + std::to_string(upperBoundHPWL) +
                       " pseudoNetWeight=" + std::to_string(pseudoNetWeight));
        }

        if (minHPWL > upperBoundHPWL)
        {
            minHPWL = upperBoundHPWL;
            if (minHPWL < placementInfo->getMinHPWL())
            {
                placementInfo->setMinHPWL(minHPWL);
            }
        }

        upperBoundHPWL = placementInfo->updateB2BAndGetTotalHPWL();

        placementInfo->checkClockUtilization(dumpClockUtilization);
        if (!updateMinHPWLAfterLegalization && macroCloseToSite)
        {
            print_warning("macroCloseToSite=" + std::to_string(macroCloseToSite) +
                          " and minHPWL is updated to be: " + std::to_string(upperBoundHPWL));
            minHPWL = upperBoundHPWL;
            updateMinHPWLAfterLegalization = true;
            pseudoNetWeight *= 0.5;
            placementInfo->adjustLUTFFUtilization_Routability_Reset();
        }

        // heuristic update pseudoNetWeight
        updatePseudoNetWeight(pseudoNetWeight, i);

        print_info("upperBoundHPWL / lowerBoundHPWL=" + std::to_string(upperBoundHPWL / lowerBoundHPWL));
        print_info("averageMacroLegalDisplacementL=" + std::to_string(averageMacroLegalDisplacement));
        print_info("macroCloseToSite=" + std::to_string(macroCloseToSite));
        print_info("macroLegalizationFixed=" + std::to_string(macroLegalizationFixed));
        print_info("progressRatio=" + std::to_string(progressRatio));
        print_info("upperBoundHPWL=" + std::to_string(upperBoundHPWL));
        print_info("minHPWL=" + std::to_string(minHPWL));

        // converge criteria
        bool criteria0 = upperBoundHPWL / lowerBoundHPWL < 1.02 &&
                         ((averageMacroLegalDisplacement < 1 && macroCloseToSite) || macroLegalizationFixed) &&
                         pseudoNetWeight > 0.02;
        bool criteria1 = (progressRatio > 0.98 || (progressRatio > 0.95 && HPWLChangeLittle)) &&
                         ((macroCloseToSite && averageMacroLegalDisplacement < 1) || macroLegalizationFixed);
        bool criteria2 = upperBoundHPWL < minHPWL * 2 && macroLegalizationFixed && i > 30;
        bool criteria3 = progressRatio > 0.925 && upperBoundHPWL / minHPWL < 1.02 && macroLegalizationFixed;
        bool criteria4 = iterCntAfterMacrosFixed >= 1 && macroLegalizationFixed;

        if (macroLegalizationFixed)
            iterCntAfterMacrosFixed++;
        if (criteria0 || criteria1 || criteria2 || criteria3 || criteria4)
        {
            print_status("Global Placer: B2B converge");
            BRAMDSPLegalizer->dumpMatching(true, true);
            CARRYMacroLegalizer->dumpMatching(true, true);
            mCLBLegalizer->dumpMatching(true, true);
            break;
        }

        if (progressRatio > 0.98 && macroCloseToSite && BLEUnpackedYet)
        {
            print_status("Global Placer: Should do packing now before further optimization");
            break;
        }
    }
    dumpCoord();
    dumpLUTFFCoordinate(true);

    oriPseudoNetWeight = pseudoNetWeight;

    // record the global placer setting
    placementInfo->setPseudoNetWeight(oriPseudoNetWeight);
    placementInfo->setMacroLegalizationParameters(WLOptimizer->getMacroPseudoNetEnhanceCnt(),
                                                  WLOptimizer->getMacroLegalizationWeight());
    // macroSpreader->spreadPlacementUnits();
}

void GlobalPlacer::GlobalPlacement_fixedCLB(int iterNum, float pseudoNetWeight)
{
    print_status("GlobalPlacer GlobalPlacement_fixedCLB started");
    WLOptimizer->reloadPlacementInfo();

    dumpCoord();
    int lowerBoundIterNum = 6;
    for (auto curCell : placementInfo->getCells())
    {
        if (!curCell->isLUT() && !curCell->isFF())
            continue;

        auto curPU = placementInfo->getCellId2PlacementUnit()[curCell->getCellId()];

        if (curPU->isLocked())
            continue;

        curPU->setFixed();
    }
    for (int i = 0; i < iterNum; i++)
    {
        for (int j = 0; j < lowerBoundIterNum; j++)
            WLOptimizer->GlobalPlacementQPSolve(pseudoNetWeight, j == 0, !dumpOptTrace);
        if (dumpOptTrace)
        {
            dumpCoord();
        }

        print_status("WLOptimizer Iteration#" + std::to_string(i) + " Done");

        generalSpreader = nullptr;

        pseudoNetWeight *= 1.05;
    }

    placementInfo->updateElementBinGrid();
    for (auto curCell : placementInfo->getCells())
    {
        if (!curCell->isLUT() && !curCell->isFF())
            continue;

        auto curPU = placementInfo->getCellId2PlacementUnit()[curCell->getCellId()];

        if (curPU->isLocked())
            continue;

        curPU->setUnfixed();
    }
    dumpCoord();
    // dumpDSPCoordinate(true);
    // dumpBRAMCoordinate(true);
    // macroSpreader->spreadPlacementUnits();
}

void GlobalPlacer::dumpCoord()
{
    dumpLUTCoordinate();
    dumpCARRYCoordinate();
    dumpDSPCoordinate();
    dumpFFCoordinate();
    dumpLUTFFCoordinate();
    dumpAllCellsCoordinate();
}

void GlobalPlacer::printPlacedUnits(std::ostream &os)
{
    print_info("Placed Units:");
    for (PlacementInfo::PlacementUnit *curPU : placementInfo->getPlacementUnits())
    {
        if (curPU->isPlaced())
        {
            os << curPU;
        }
    }
}

void GlobalPlacer::dumpLUTCoordinate()
{
    if (JSONCfg.find("DumpLUTCoordTrace") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpLUTCoordTrace"] + "-" + std::to_string(LUTCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        LUTCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isLUT())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isLUT(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::macroLegalize(int curIteration)
{
    // based on the global placement convergence progress and legalization displacement, select different strategies.
    // TODO: make this part more clean and clear for reader!
    if (macroLegalizationFixed)
        return;
    if (macroLocked)
    {
        macroLockedIterCnt++;
        if (macroLockedIterCnt > 3)
            macroLocked = false;
        else
            return;
    }

    if ((BRAMDSPLegalizer->getAverageDisplacementOfRoughLegalization() > 1.5 && progressRatio < 0.9 &&
         curIteration < 10) &&
        !macroLocked && !macrosBindedToSites && !directMacroLegalize)
    {
        // rough legalization phase
        averageMacroLegalDisplacement = BRAMDSPLegalizer->getAverageDisplacementOfRoughLegalization();
        averageMCLBLegalDisplacement = mCLBLegalizer->getAverageDisplacementOfRoughLegalization();
        print_info("DSPBRAM Average Displacement Of Rough Legalization =" +
                   std::to_string(averageMacroLegalDisplacement));
        print_info("MCLB Average Displacement Of Rough Legalization =" + std::to_string(averageMCLBLegalDisplacement));

        placementInfo->getDeviceInfo()->resetAllSiteMapping();
        mCLBLegalizer->legalize();
        BRAMDSPLegalizer->legalize();
        averageMacroLegalDisplacement = BRAMDSPLegalizer->getAverageDisplacementOfRoughLegalization();
        historyAverageDisplacement.push_back(averageMacroLegalDisplacement);
    }
    else
    {
        // exact legalization phase
        averageMacroLegalDisplacement = BRAMDSPLegalizer->getAverageDisplacementOfExactLegalization();
        averageMCLBLegalDisplacement = mCLBLegalizer->getAverageDisplacementOfExactLegalization();
        averageCarryLegalDisplacement = CARRYMacroLegalizer->getAverageDisplacementOfExactLegalization();
        print_info("DSPBRAM Average Displacement Of Exact Legalization =" +
                   std::to_string(averageMacroLegalDisplacement));
        print_info("MCLB Average Displacement Of Exact Legalization =" + std::to_string(averageMCLBLegalDisplacement));
        print_info("CARRY Average Displacement Of Exact Legalization =" +
                   std::to_string(averageCarryLegalDisplacement));
        // gradually reduce the size of initial legalization window
        BRAMDSPLegalizer->setIntitialParameters(40.0, 40);
        mCLBLegalizer->setIntitialParameters(20.0, 20, 2);
        CARRYMacroLegalizer->setIntitialParameters((float)2.0, 3, 2);

        if (((averageMacroLegalDisplacement > 1 || averageMCLBLegalDisplacement > 2 ||
              averageCarryLegalDisplacement > 2)) ||
            !macroCloseToSite)
        {
            if (curIteration >= 10 && averageMacroLegalDisplacement < 7.5 && averageMCLBLegalDisplacement < 7.5 &&
                averageCarryLegalDisplacement < 7.5)
            {
                print_warning("too many times of legalization. enforce the elements to be fixed!");
                macroCloseToSite = true;
                for (auto pair : placementInfo->getPULegalXY().first)
                {
                    auto curPU = pair.first;
                    curPU->setAnchorLocationAndForgetTheOriginalOne(placementInfo->getPULegalXY().first[curPU],
                                                                    placementInfo->getPULegalXY().second[curPU]);
                    curPU->setFixed();
                }
                macroLegalizationFixed = true;
            }
            else
            {
                macroCloseToSite = true;
                if (averageMacroLegalDisplacement > 1)
                {
                    if (averageCarryLegalDisplacement > 2)
                    {
                        if (curIteration % 2 == 0 || averageCarryLegalDisplacement > 5000)
                        {
                            CARRYMacroLegalizer->resetSitesMapped();
                        }
                    }
                    print_status("mCLBLegalizer: Launch.");
                    mCLBLegalizer->resetSitesMapped();
                    mCLBLegalizer->legalize(true);
                    macroCloseToSite = false;
                }
                if (averageMCLBLegalDisplacement > 2)
                {
                    print_status("BRAMDSPLegalizer: Launch.");
                    BRAMDSPLegalizer->resetSitesMapped();
                    BRAMDSPLegalizer->legalize(true, directMacroLegalize);
                    macroCloseToSite = false;
                }
                if (averageCarryLegalDisplacement > 2)
                {
                    if (curIteration % 2 == 0 || averageCarryLegalDisplacement > 5000)
                    {
                        print_status("CARRYMacroLegalizer: Launch.");
                        CARRYMacroLegalizer->resetSitesMapped();
                        CARRYMacroLegalizer->legalize(true, directMacroLegalize);
                    }
                    macroCloseToSite = false;
                }
                averageMacroLegalDisplacement = BRAMDSPLegalizer->getAverageDisplacementOfExactLegalization();
                macrosBindedToSites = true;
                historyAverageDisplacement.push_back(averageMacroLegalDisplacement);
            }
            if (directMacroLegalize)
            {
                for (auto pair : placementInfo->getPULegalXY().first)
                {
                    auto curPU = pair.first;
                    curPU->setAnchorLocationAndForgetTheOriginalOne(placementInfo->getPULegalXY().first[curPU],
                                                                    placementInfo->getPULegalXY().second[curPU]);
                }
            }
            if (macroCloseToSite)
            {
                for (auto pair : placementInfo->getPULegalXY().first)
                {
                    auto curPU = pair.first;
                    curPU->setAnchorLocationAndForgetTheOriginalOne(placementInfo->getPULegalXY().first[curPU],
                                                                    placementInfo->getPULegalXY().second[curPU]);
                    curPU->setFixed();
                }
                macroLegalizationFixed = true;
            }
        }
        else
        {
            if (macroCloseToSite)
            {
                for (auto pair : placementInfo->getPULegalXY().first)
                {
                    auto curPU = pair.first;
                    curPU->setAnchorLocationAndForgetTheOriginalOne(placementInfo->getPULegalXY().first[curPU],
                                                                    placementInfo->getPULegalXY().second[curPU]);
                    curPU->setFixed();
                }
                macroLegalizationFixed = true;
            }
        }
    }

    if (historyAverageDisplacement.size() > 3 && averageMacroLegalDisplacement < 3 && !directMacroLegalize)
    {
        int i = historyAverageDisplacement.size();
        float tmpAvg = (historyAverageDisplacement[i - 3] + historyAverageDisplacement[i - 2] +
                        historyAverageDisplacement[i - 1]) /
                       3.0;

        float err = 0;
        for (unsigned int i = historyAverageDisplacement.size() - 3; i < historyAverageDisplacement.size(); i++)
        {
            err += (historyAverageDisplacement[i] - tmpAvg) * (historyAverageDisplacement[i] - tmpAvg);
        }
        if (std::sqrt(err / 3) < 0.01)
        {
            macroLocked = true;
            macroLockedIterCnt = 0;
            historyAverageDisplacement.clear();
            print_warning("Macros will be locked for a while");
        }
    }
}

void GlobalPlacer::spreading(int currentIteration)
{
    placementInfo->updateElementBinGrid();
    float supplyRatio = (placementInfo->getBinGridW() < 2.5) ? 0.95 : (0.80 + 0.1 * progressRatio);
    if (!macroLegalizationFixed)
    {
        std::string sharedCellType_SLICEL_CARRY8 = "SLICEL_CARRY8";
        generalSpreader = new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_CARRY8, currentIteration,
                                              supplyRatio, verbose);
        generalSpreader->spreadPlacementUnits(spreadingForgetRatio);
        delete generalSpreader;
    }

    std::string sharedCellType_SLICEL_MUXF8 = "SLICEL_MUXF8";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_MUXF8, currentIteration, 0.75, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio);
    delete generalSpreader;

    std::string sharedCellType_SLICEL_MUXF7 = "SLICEL_MUXF7";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_MUXF7, currentIteration, 0.75, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio);
    delete generalSpreader;

    // we gradually increase the shrinkRatio since the area adjustion of LUT/FF will be more accurate.
    // we provide less area so the LUTs/FFs will not be too dense. Too dense placement might be seriously disturbed when
    // some cells are inflatten.
    std::string sharedCellType_SLICEL_LUT = "SLICEL_LUT";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_LUT, currentIteration, supplyRatio, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio);
    delete generalSpreader;

    std::string sharedCellType_SLICEL_FF = "SLICEL_FF";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_FF, currentIteration, supplyRatio, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio);
    delete generalSpreader;

    generalSpreader = nullptr;

    if (progressRatio > 0.4)
    {
        placementInfo->adjustLUTFFUtilization(neighborDisplacementUpperbound);
    }
}

void GlobalPlacer::updatePseudoNetWeight(float &pseudoNetWeight, int curIter)
{
    progressRatio = lowerBoundHPWL / upperBoundHPWL;
    if (progressRatio > 1)
        progressRatio = 0.999;
    if (progressRatio > 0.5)
        spreadingForgetRatio = 1 - 0.8 * progressRatio;
    else // the spreading control is not worthy when the placement is far from convergence.
        spreadingForgetRatio = 1;
    if (disableSpreadingForgetRatio)
        spreadingForgetRatio = 1;
    HPWLChangeLittle = false;
    progressRatio = std::pow(progressRatio, 0.6); // 0.5-1.4;0.6-1.3
    placementInfo->setProgress(progressRatio);

    if (pseudoNetWeight < 0.0020)
        pseudoNetWeight *= 1.825;
    else
        pseudoNetWeight *= 1.5 * (1 - progressRatio) + 1.01 * (progressRatio);

    historyHPWLs.push_back(upperBoundHPWL);

    int numRecorded = historyHPWLs.size();
    if (numRecorded > 5)
    {
        float err = 0;
        for (int vecId = numRecorded - 5; vecId < numRecorded; vecId++)
        {
            err += (minHPWL - historyHPWLs[vecId]) * (minHPWL - historyHPWLs[vecId]);
        }
        if (historyHPWLs[numRecorded - 1] > historyHPWLs[numRecorded - 2] &&
            historyHPWLs[numRecorded - 2] > historyHPWLs[numRecorded - 3])
        {
            if (pseudoNetWeight > 0.03 && progressRatio < 0.95)
            {
                while (historyHPWLs.size() > 1)
                    historyHPWLs.pop_front();
                pseudoNetWeight *= 0.75;
            }
        }
        else if (progressRatio > 0.8 && upperBoundHPWL / minHPWL > 1.333 && updateMinHPWLAfterLegalization)
        {
            print_warning("The upperbound HPWL increases too much, pseudoNetWeight is reduced by 50% to recover.");
            placementInfo->adjustLUTFFUtilization_Routability_Reset();
            pseudoNetWeight *= 0.5;
            historyHPWLs.clear();
        }
        err = std::sqrt(err / 5);
        print_info("err=" + std::to_string(err) + " minHPWL=" + std::to_string(minHPWL) +
                   " err/minHPWL=" + std::to_string(err / minHPWL) + " progressRatio=" + std::to_string(progressRatio));

        if (averageMacroLegalDisplacement > 3 && progressRatio > 0.98)
            pseudoNetWeight *= 1.05;

        if (err / minHPWL < 1.0 / 100.0 && progressRatio > 0.85 &&
            BRAMDSPLegalizer->getAverageDisplacementOfExactLegalization() < 3)
        {
            print_status("Global Placer: B2B MAYBE converge");
            HPWLChangeLittle = true;
            pseudoNetWeight *= 2;
            // break;
        }
    }
    else if (numRecorded > 2 && updateMinHPWLAfterLegalization)
    {
        if (progressRatio > 0.6 && upperBoundHPWL / minHPWL > 1.333 && curIter > 10)
        {
            print_warning("The upperbound HPWL increases too much, pseudoNetWeight is reduced by 50% to recover.");
            placementInfo->adjustLUTFFUtilization_Routability_Reset();
            pseudoNetWeight *= 0.5;
            historyHPWLs.clear();
        }
    }
    else
    {
        print_info("minHPWL=" + std::to_string(minHPWL) + " progressRatio=" + std::to_string(progressRatio));
    }
    // if (pseudoNetWeight > 1)
    // {
    //     pseudoNetWeight = 1;
    // }
}

void GlobalPlacer::dumpCARRYCoordinate()
{
    if (JSONCfg.find("DumpCARRYCoordTrace") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpCARRYCoordTrace"] + "-" + std::to_string(CARRYCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        CARRYCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isCarry())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isCarry(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::dumpFFCoordinate()
{
    if (JSONCfg.find("DumpFFCoordTrace") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpFFCoordTrace"] + "-" + std::to_string(FFCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        FFCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isFF())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isFF(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::dumpAllCellsCoordinate()
{
    if (JSONCfg.find("DumpAllCoordTrace") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpAllCoordTrace"] + "-" + std::to_string(allCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        allCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (curMacro->getCell(vId))
                            outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                        else
                            outfile0 << cellX << " " << cellY << "\n";
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::dumpLUTFFCoordinate(bool enforced)
{
    if (!enforced)
    {
        if (JSONCfg.find("DumpLUTFFCoordTrace") != JSONCfg.end())
        {
            std::string dumpFile =
                JSONCfg["DumpLUTFFCoordTrace"] + "-" + std::to_string(LUTFFCoordinateDumpCnt) + ".gz";
            print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
            LUTFFCoordinateDumpCnt++;
            if (dumpFile != "")
            {
                std::stringstream outfile0;
                for (auto curPU : placementInfo->getPlacementUnits())
                {
                    if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                    {
                        float cellX = curUnpackedCell->X();
                        float cellY = curUnpackedCell->Y();
                        DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                        if (curCell->isFF() || curCell->isLUT())
                        {
                            outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                        }
                    }
                    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                    {
                        for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                        {
                            float offsetX_InMacro, offsetY_InMacro;
                            DesignInfo::DesignCellType cellType;
                            curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                            float cellX = curMacro->X() + offsetX_InMacro;
                            float cellY = curMacro->Y() + offsetY_InMacro;

                            if (DesignInfo::isFF(cellType) || DesignInfo::isLUT(cellType))
                            {
                                if (curMacro->getCell(vId))
                                    outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName()
                                             << "\n";
                                else
                                    outfile0 << cellX << " " << cellY << "\n";
                            }
                        }
                    }
                }
                writeStrToGZip(dumpFile, outfile0);
                print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
            }
        }
    }
    else
    {
        std::string dumpFile = +"FinalLUTFF-" + std::to_string(LUTFFCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        LUTFFCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isFF() || curCell->isLUT())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isFF(cellType) || DesignInfo::isLUT(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::dumpDSPCoordinate(bool enforced)
{
    if (!enforced)
    {
        if (JSONCfg.find("DumpDSPCoordTrace") != JSONCfg.end())
        {
            std::string dumpFile = JSONCfg["DumpDSPCoordTrace"] + "-" + std::to_string(DSPCoordinateDumpCnt) + ".gz";
            print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
            DSPCoordinateDumpCnt++;
            if (dumpFile != "")
            {
                std::stringstream outfile0;
                for (auto curPU : placementInfo->getPlacementUnits())
                {
                    if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                    {
                        float cellX = curUnpackedCell->X();
                        float cellY = curUnpackedCell->Y();
                        DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                        if (curCell->isDSP())
                        {
                            outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                        }
                    }
                    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                    {
                        for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                        {
                            float offsetX_InMacro, offsetY_InMacro;
                            DesignInfo::DesignCellType cellType;
                            curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                            float cellX = curMacro->X() + offsetX_InMacro;
                            float cellY = curMacro->Y() + offsetY_InMacro;

                            if (DesignInfo::isDSP(cellType))
                            {
                                if (curMacro->getCell(vId))
                                    outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName()
                                             << "\n";
                                else
                                    outfile0 << cellX << " " << cellY << "\n";
                            }
                        }
                    }
                }
                writeStrToGZip(dumpFile, outfile0);
                print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
            }
        }
    }
    else
    {
        std::string dumpFile = +"FinalDSP-" + std::to_string(DSPCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        DSPCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isDSP())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isDSP(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

void GlobalPlacer::dumpBRAMCoordinate(bool enforced)
{
    if (!enforced)
    {
        if (JSONCfg.find("DumpBRAMCoordTrace") != JSONCfg.end())
        {
            std::string dumpFile = JSONCfg["DumpBRAMCoordTrace"] + "-" + std::to_string(BRAMCoordinateDumpCnt) + ".gz";
            print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
            BRAMCoordinateDumpCnt++;
            if (dumpFile != "")
            {
                std::stringstream outfile0;
                for (auto curPU : placementInfo->getPlacementUnits())
                {
                    if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                    {
                        float cellX = curUnpackedCell->X();
                        float cellY = curUnpackedCell->Y();
                        DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                        if (curCell->isBRAM())
                        {
                            outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                        }
                    }
                    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                    {
                        for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                        {
                            float offsetX_InMacro, offsetY_InMacro;
                            DesignInfo::DesignCellType cellType;
                            curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                            float cellX = curMacro->X() + offsetX_InMacro;
                            float cellY = curMacro->Y() + offsetY_InMacro;

                            if (DesignInfo::isBRAM(cellType))
                            {
                                if (curMacro->getCell(vId))
                                    outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName()
                                             << "\n";
                                else
                                    outfile0 << cellX << " " << cellY << "\n";
                            }
                        }
                    }
                }
                writeStrToGZip(dumpFile, outfile0);
                print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
            }
        }
    }
    else
    {
        std::string dumpFile = +"FinalBRAM-" + std::to_string(BRAMCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        BRAMCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isBRAM())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isBRAM(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}