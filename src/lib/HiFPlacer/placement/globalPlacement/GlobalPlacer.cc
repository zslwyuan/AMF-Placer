/**
 * @file GlobalPlacer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This implementation file contains APIs' implementation of the GlobalPlacer which organizes/configures other
 * modules to handle global placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
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

    if (JSONCfg.find("DSPCritical") != JSONCfg.end())
        DSPCritical = JSONCfg["DSPCritical"] == "true";

    if (JSONCfg.find("DumpClockUtilization") != JSONCfg.end())
    {
        dumpClockUtilization = JSONCfg["DumpClockUtilization"] == "true";
    }

    if (JSONCfg.find("GlobalPlacerPrintHPWL") != JSONCfg.end())
        printHPWL = JSONCfg["GlobalPlacerPrintHPWL"] == "true";

    hasUserDefinedClusterInfo = JSONCfg.find("designCluster") != JSONCfg.end();

    clusterPlacer = new ClusterPlacer(placementInfo, JSONCfg, 10.0);
    WLOptimizer = new WirelengthOptimizer(placementInfo, JSONCfg, verbose);

    std::vector<DesignInfo::DesignCellType> macroTypesToLegalize;
    macroTypesToLegalize.clear();
    macroTypesToLegalize.push_back(DesignInfo::CellType_RAMB18E2);
    macroTypesToLegalize.push_back(DesignInfo::CellType_RAMB36E2);
    macroTypesToLegalize.push_back(DesignInfo::CellType_FIFO18E2);
    macroTypesToLegalize.push_back(DesignInfo::CellType_FIFO36E2);
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
                                               unsigned int spreadRegionBinNumLimit,
                                               PlacementTimingOptimizer *timingOptimizer)
{
    print_status("GlobalPlacer GlobalPlacement_CLBElements started");

    WLOptimizer->reloadPlacementInfo();
    for (auto tmpNet : placementInfo->getDesignInfo()->getNets())
        tmpNet->setOverallTimingNetEnhancement(1.0);

    pseudoNetWeight = 1.0;

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
    for (int i = 0; i < iterNum || (!stopStrictly); i++)
    {

        float displacementLimit = -10;
        if (timingOptimizer)
        {
            if (timingOptimizer->getEffectFactor() >= 1)
                displacementLimit = 10;

            if (timingOptimizer->getEffectFactor() > 0.5)
                placementInfo->enhanceRiskyClockNet();
            placementInfo->enhanceHighFanoutNet();

            timingOptimizer->conductStaticTimingAnalysis();
            if (timingOptimizer->getEffectFactor() > 1)
                timingDrivenDetailedPlacement_shortestPath_intermediate(timingOptimizer);
        }

        // lowerBound: Quadratic Programming based Wirelength Optimization
        lowerBoundIterNum = (placementInfo->getProgress() < 0.965 && !macroCloseToSite) ? 2 : 2;

        for (int j = 0; j < lowerBoundIterNum; j++)
        {
            WLOptimizer->GlobalPlacementQPSolve(
                pseudoNetWeight, j == 0, true, enableMacroPseudoNet2Site, pseudoNetWeightConsiderNetNum,
                (i > 1 || continuePreviousIteration) && hasUserDefinedClusterInfo, displacementLimit, timingOptimizer);
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

        spreading(i, spreadRegionBinNumLimit, displacementLimit);

        upperBoundHPWL = placementInfo->updateB2BAndGetTotalHPWL();
        print_status("Spreader Iteration#" + to_string_align3(i) + " Done HPWL=" + std::to_string(upperBoundHPWL));

        if (dumpOptTrace)
        {
            dumpCoord();
        }

        // legalize macros (DSPs/BRAMs)
        if (enableMacroPseudoNet2Site)
        {
            bool timingDrivenLegalization = false;
            if (timingOptimizer)
            {
                if (timingOptimizer->getEffectFactor() > 0.6)
                    timingDrivenLegalization = true;
            }
            if (!continuePreviousIteration && !DSPCritical)
            {
                if (i >= 8)
                {
                    macroLegalize(i, timingDrivenLegalization);
                    print_status("Legalization Iteration#" + to_string_align3(i) + " Done");
                }
            }
            else
            {
                macroLegalize(i, timingDrivenLegalization);
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

        bool clockLegal = placementInfo->checkClockUtilization(dumpClockUtilization);
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
        print_info("clockLegal=" + std::to_string(clockLegal));

        // converge criteria
        bool criteria0 = upperBoundHPWL / lowerBoundHPWL < 1.02 &&
                         ((averageMacroLegalDisplacement < 1 && macroCloseToSite) || macroLegalizationFixed) &&
                         pseudoNetWeight > 0.02;
        bool criteria1 = (progressRatio > 0.98 || (progressRatio > 0.95 && HPWLChangeLittle)) &&
                         ((macroCloseToSite && averageMacroLegalDisplacement < 1) || macroLegalizationFixed);
        bool criteria2 = upperBoundHPWL < minHPWL * 2 && macroLegalizationFixed && i > 30;
        bool criteria3 = iterCntAfterMacrosFixed >= 2 && progressRatio > 0.925 && upperBoundHPWL / minHPWL < 1.02 &&
                         macroLegalizationFixed;
        bool criteria4 = iterCntAfterMacrosFixed >= 2 && macroLegalizationFixed;

        if (macroLegalizationFixed)
            iterCntAfterMacrosFixed++;
        // criteria0 || criteria1 || criteria2 ||
        if (criteria3 || criteria4)
        {
            print_status("Global Placer: B2B converge");
            BRAMDSPLegalizer->dumpMatching(true, true);
            CARRYMacroLegalizer->dumpMatching(true, true);
            mCLBLegalizer->dumpMatching(true, true);
            break;
        }

        if (progressRatio > 0.98 && macroCloseToSite && !continuePreviousIteration)
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
            WLOptimizer->GlobalPlacementQPSolve(pseudoNetWeight, j == 0, false);
        if (dumpOptTrace)
        {
            dumpCoord();
        }

        print_status("WLOptimizer Iteration#" + std::to_string(i) + " Done");
        print_info("HPWL after QP=" + std::to_string(placementInfo->updateB2BAndGetTotalHPWL()));
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

void GlobalPlacer::macroLegalize(int curIteration, bool timingDriven, PlacementTimingOptimizer *timingOptimizer)
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

    CARRYMacroLegalizer->setClockRegionAware(enableClockRegionAware);

    if ((BRAMDSPLegalizer->getAverageDisplacementOfRoughLegalization() > 3 && progressRatio < 0.9 &&
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
        BRAMDSPLegalizer->legalize(false, false, timingDriven);
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

        if (((averageMacroLegalDisplacement > 1 || averageMCLBLegalDisplacement > 3 ||
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
                    print_status("BRAMDSPLegalizer: Launch.");
                    BRAMDSPLegalizer->resetSitesMapped();
                    BRAMDSPLegalizer->legalize(true, directMacroLegalize, timingDriven);
                    macroCloseToSite = false;
                }
                if (averageCarryLegalDisplacement > 2 || averageMCLBLegalDisplacement > 3)
                {
                    if (curIteration % 2 == 0 || averageCarryLegalDisplacement > 5000)
                    {
                        mCLBLegalizer->resetSitesMapped();
                        CARRYMacroLegalizer->resetSitesMapped();
                        print_status("mCLBLegalizer: Launch.");
                        mCLBLegalizer->legalize(true);
                        print_status("CARRYMacroLegalizer: Launch.");
                        CARRYMacroLegalizer->legalize(true, directMacroLegalize, timingDriven);
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
    placementInfo->updateElementBinGrid();
}

void GlobalPlacer::spreading(int currentIteration, int spreadRegionSizeLimit, float displacementLimit)
{
    placementInfo->updateElementBinGrid();
    float supplyRatio = (placementInfo->getBinGridW() < 2.5) ? 0.95 : (0.80 + 0.1 * progressRatio);

    if (!macroLegalizationFixed)
    {
        std::string sharedCellType_SLICEL_CARRY8 = "SLICEL_CARRY8";
        generalSpreader = new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_CARRY8, currentIteration,
                                              supplyRatio, verbose);
        generalSpreader->spreadPlacementUnits(spreadingForgetRatio, enableClockRegionAware, displacementLimit);
        delete generalSpreader;
    }

    std::string sharedCellType_SLICEL_MUXF8 = "SLICEL_MUXF8";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_MUXF8, currentIteration, 0.75, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio, enableClockRegionAware, displacementLimit);
    delete generalSpreader;

    std::string sharedCellType_SLICEL_MUXF7 = "SLICEL_MUXF7";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_MUXF7, currentIteration, 0.75, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio, enableClockRegionAware, displacementLimit);
    delete generalSpreader;

    // we gradually increase the shrinkRatio since the area adjustion of LUT/FF will be more accurate.
    // we provide less area so the LUTs/FFs will not be too dense. Too dense placement might be seriously disturbed when
    // some cells are inflatten.
    std::string sharedCellType_SLICEL_LUT = "SLICEL_LUT";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_LUT, currentIteration, supplyRatio, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio, enableClockRegionAware, displacementLimit);
    delete generalSpreader;

    std::string sharedCellType_SLICEL_FF = "SLICEL_FF";
    generalSpreader =
        new GeneralSpreader(placementInfo, JSONCfg, sharedCellType_SLICEL_FF, currentIteration, supplyRatio, verbose);
    generalSpreader->spreadPlacementUnits(spreadingForgetRatio, enableClockRegionAware, displacementLimit);
    delete generalSpreader;

    generalSpreader = nullptr;

    placementInfo->updateElementBinGrid();
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
    if (std::pow(progressRatio, 0.6) > 0.4 && spreadingForgetRatio < 1)
        spreadingForgetRatio = 0.5; // 1 - 0.4 * progressRatio;
    else                            // the spreading control is not worthy when the placement is far from convergence.
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

    if (progressRatio > 0.6 && !placementInfo->isDensePlacement())
    {
        print_warning("GlobalPlacer: clock region aware optimization is enabled.");
        enableClockRegionAware = true;
    }

    if (progressRatio > 0.75)
    {
        if (progressRatio > 0.85 && placementInfo->isClockLegalizationRisky())
        {
            enableClockRegionAware = false;
        }
        BRAMDSPLegalizer->setClockRegionCasLegalization(true);
    }
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
            print_warning(
                "GlobalPlacer: The upperbound HPWL increases too much, pseudoNetWeight is reduced by 50% to recover.");
            placementInfo->adjustLUTFFUtilization_Routability_Reset();
            pseudoNetWeight *= 0.5;
            historyHPWLs.clear();
        }
        err = std::sqrt(err / 5);
        print_info("GlobalPlacer: err=" + std::to_string(err) + " minHPWL=" + std::to_string(minHPWL) +
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
            print_warning(
                "GlobalPlacer: The upperbound HPWL increases too much, pseudoNetWeight is reduced by 50% to recover.");
            placementInfo->adjustLUTFFUtilization_Routability_Reset();
            pseudoNetWeight *= 0.5;
            historyHPWLs.clear();
        }
    }
    else
    {
        print_info("GlobalPlacer: minHPWL=" + std::to_string(minHPWL) +
                   " progressRatio=" + std::to_string(progressRatio));
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
        std::string dumpFile =
            JSONCfg["dumpDirectory"] + "FinalLUTFF-" + std::to_string(LUTFFCoordinateDumpCnt) + ".gz";
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

int GlobalPlacer::timingDrivenDetailedPlacement_shortestPath_intermediate(PlacementTimingOptimizer *timingOptimizer)
{
    float range = 0.75;
    print_status("ParallelCLBPacker: conducting timing-driven detailed placement based on shortest path.");
    auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(0.9);
    std::set<PlacementInfo::PlacementUnit *> PUsTouched;
    PUsTouched.clear();
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();

    int replaceCnt = 0;
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        std::map<int, std::vector<PlacementInfo::Location>> cellId2CandidateLocation;
        cellId2CandidateLocation.clear();

        std::vector<int> cellIdsInCriticalPath;
        PlacementInfo::PlacementUnit *lastPU = nullptr;
        std::set<PlacementInfo::PlacementUnit *> PUsInCriticalPathSet;
        for (auto cellId : oriCellIdsInCriticalPath)
        {
            if (PUsInCriticalPathSet.find(placementInfo->getPlacementUnitByCellId(cellId)) ==
                PUsInCriticalPathSet.end())
            {
                PUsInCriticalPathSet.insert(placementInfo->getPlacementUnitByCellId(cellId));
                cellIdsInCriticalPath.push_back(cellId);
            }
        }

        bool CellUnmapped = false;
        for (auto cellId : cellIdsInCriticalPath)
        {
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);

            cellId2CandidateLocation[cellId] = std::vector<PlacementInfo::Location>(1, cellLoc[cellId]);
        }

        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);

            if (PUsTouched.find(curPU) != PUsTouched.end())
            {
                continue;
            }
            // std::cout << curCell << " has following candidates: \n";
            if (!curPU->isFixed() && !curPU->isLocked() && !curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() &&
                !curPU->checkHasBRAM() && !curPU->checkHasDSP())
            {
                auto curX = cellLoc[cellId].X;
                auto curY = cellLoc[cellId].Y;
                for (float cX = curX - 1.0 * range; cX < curX + 1.1 * range; cX += 0.5 * range)
                {
                    for (float cY = curY - 2.0 * range; cY < curY + 2.1 * range; cY += 1 * range)
                    {
                        if (std::fabs(cX - curX) + std::fabs(cY - curY) < 0.1)
                            continue;
                        PlacementInfo::Location newLoc;
                        newLoc.X = cX;
                        newLoc.Y = cY;
                        cellId2CandidateLocation[cellId].push_back(newLoc);
                    }
                }
            }
        }
        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            PUsTouched.insert(curPU);
        }
        // calculate the shortest paths
        std::vector<std::vector<float>> shortestPath_LayerSite;
        std::vector<std::vector<int>> shortestPath_LayerSite_backtrace;
        shortestPath_LayerSite.push_back(
            std::vector<float>(cellId2CandidateLocation[cellIdsInCriticalPath[0]].size(), 0.0));
        shortestPath_LayerSite_backtrace.push_back(
            std::vector<int>(cellId2CandidateLocation[cellIdsInCriticalPath[0]].size(), -1));

        int bestEndChoice = -1;
        float bestChoiceDelay = 100000;

        for (int i = 1; i < cellIdsInCriticalPath.size(); i++)
        {
            auto predCell = cellIdsInCriticalPath[i - 1];
            auto curCell = cellIdsInCriticalPath[i];
            auto &predCandidates = cellId2CandidateLocation[predCell];
            auto &curCandidates = cellId2CandidateLocation[curCell];
            shortestPath_LayerSite.push_back(std::vector<float>(cellId2CandidateLocation[curCell].size(), 100000.0));
            shortestPath_LayerSite_backtrace.push_back(std::vector<int>(cellId2CandidateLocation[curCell].size(), -1));

            for (int j = 0; j < predCandidates.size(); j++)
            {
                for (int k = 0; k < curCandidates.size(); k++)
                {
                    float predX = predCandidates[j].X, predY = predCandidates[j].Y;
                    float curX = curCandidates[k].X, curY = curCandidates[k].Y;

                    float delay = timingOptimizer->getDelayByModel(predX, predY, curX, curY);

                    if (shortestPath_LayerSite[i][k] > shortestPath_LayerSite[i - 1][j] + delay)
                    {
                        shortestPath_LayerSite[i][k] = shortestPath_LayerSite[i - 1][j] + delay;
                        shortestPath_LayerSite_backtrace[i][k] = j;

                        if (i == cellIdsInCriticalPath.size() - 1)
                        {
                            if (shortestPath_LayerSite[i][k] < bestChoiceDelay)
                            {
                                bestChoiceDelay = shortestPath_LayerSite[i][k];
                                bestEndChoice = k;
                            }
                        }
                    }
                }
            }
        }

        for (int i = shortestPath_LayerSite.size() - 1; i >= 0; i--)
        {
            if (bestEndChoice)
            {
                auto curCellId = cellIdsInCriticalPath[i];
                auto curPU = placementInfo->getPlacementUnitByCellId(curCellId);
                auto curCell = placementInfo->getCells()[curCellId];

                auto &curCandidates = cellId2CandidateLocation[curCellId];
                if (!curPU->isFixed() && !curPU->isLocked() && !curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() &&
                    !curPU->checkHasBRAM() && !curPU->checkHasDSP())
                {
                    if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                    {
                        float offsetX_InMacro = curMacro->getCellOffsetXInMacro(curCell),
                              offsetY_InMacro = curMacro->getCellOffsetYInMacro(curCell);

                        float resX = curCandidates[bestEndChoice].X - offsetX_InMacro;
                        float resY = curCandidates[bestEndChoice].Y - offsetY_InMacro;

                        placementInfo->legalizeXYInArea(curPU, resX, resY);
                        curPU->setAnchorLocationAndForgetTheOriginalOne(resX, resY);
                    }
                    else
                    {
                        placementInfo->legalizeXYInArea(curPU, curCandidates[bestEndChoice].X,
                                                        curCandidates[bestEndChoice].Y);
                        curPU->setAnchorLocationAndForgetTheOriginalOne(curCandidates[bestEndChoice].X,
                                                                        curCandidates[bestEndChoice].Y);
                    }
                    replaceCnt++;
                }
            }
            bestEndChoice = shortestPath_LayerSite_backtrace[i][bestEndChoice];
        }
    }

    placementInfo->updateElementBinGrid();
    print_status("ParallelCLBPacker: conducted timing-driven detailed placement (shortest path) and " +
                 std::to_string(replaceCnt) + " PlacementUnits are replaced.");
    return replaceCnt;
}
