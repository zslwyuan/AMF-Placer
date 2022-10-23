/**
 * @file PlacementTimingOptimizer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "PlacementTimingOptimizer.h"

#include <cmath>
#include <codecvt>

PlacementTimingOptimizer::PlacementTimingOptimizer(PlacementInfo *placementInfo,
                                                   std::map<std::string, std::string> &JSONCfg)
    : placementInfo(placementInfo), timingInfo(placementInfo->getTimingInfo()), JSONCfg(JSONCfg)
{
    if (JSONCfg.find("PlacementTimingOptimizerVerbose") != JSONCfg.end())
        verbose = JSONCfg["PlacementTimingOptimizerVerbose"] == "true";
    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);
    if (JSONCfg.find("DSPCritical") != JSONCfg.end())
        DSPCritical = JSONCfg["DSPCritical"] == "true";

    designInfo = placementInfo->getDesignInfo();
    deviceInfo = placementInfo->getDeviceInfo();
    initPois();
}

void PlacementTimingOptimizer::setPinsLocation()
{
    auto &cellLoc = placementInfo->getCellId2location();
    auto &pinLoc = placementInfo->getPinId2location();
    pinLoc = std::vector<PlacementInfo::Location>(designInfo->getPins().size());
    for (auto tmpCell : placementInfo->getCells())
    {
        int cellId = tmpCell->getCellId();
        auto tmpCellLoc = cellLoc[cellId];
        for (auto tmpPin : tmpCell->getPins())
        {
            float pinX = tmpCellLoc.X + tmpPin->getOffsetXInCell();
            float pinY = tmpCellLoc.Y + tmpPin->getOffsetYInCell();
            assert((unsigned int)tmpPin->getElementIdInType() < pinLoc.size());
            pinLoc[tmpPin->getElementIdInType()].X = pinX;
            pinLoc[tmpPin->getElementIdInType()].Y = pinY;
        }
    }
}

float PlacementTimingOptimizer::getWorstSlackOfCell(DesignInfo::DesignCell *srcCell)
{
    auto &timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
    auto &cellLoc = placementInfo->getCellId2location();

    unsigned int srcCellId = srcCell->getCellId();
    auto srcNode = timingNodes[srcCellId];
    auto srcLoc = cellLoc[srcCellId];

    float worstSlack = 100;
    for (auto outputNet : srcCell->getOutputNets())
    {
        for (auto pinBeDriven : outputNet->getPinsBeDriven())
        {
            // get the sinkPin information
            auto sinkCell = pinBeDriven->getCell();
            unsigned int sinkCellId = sinkCell->getCellId();
            auto sinkNode = timingNodes[sinkCellId];
            auto sinkLoc = cellLoc[sinkCellId];
            int succPathLen = sinkNode->getLongestPathLength();
            float netDelay = getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
            float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;

            if (slack < worstSlack)
                worstSlack = slack;
        }
    }

    return worstSlack;
}

std::vector<int> PlacementTimingOptimizer::findCriticalPath()
{

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    float maxDelay = 0;
    int maxDelayId = -1;
    for (unsigned int i = 0; i < timingGraph->getNodes().size(); i++)
    {
        if (timingGraph->getNodes()[i]->getLatestInputArrival() > maxDelay)
        {
            maxDelay = timingGraph->getNodes()[i]->getLatestInputArrival();
            maxDelayId = i;
        }
    }

    std::vector<std::vector<int>> resPaths;
    auto resPath = timingGraph->backTraceDelayLongestPathFromNode(maxDelayId);

    return resPath;
}

std::vector<std::vector<int>> PlacementTimingOptimizer::findCriticalPaths(float criticalRatio, bool checkOverlap,
                                                                          int pathNumThr, int converThr)
{

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    timingGraph->sortedEndpointByDelay();
    std::vector<int> isCovered(timingGraph->getNodes().size(), 0);
    std::vector<std::vector<int>> resPaths;

    for (auto curEndpoint : timingGraph->getSortedTimingEndpoints())
    {
        if (isCovered[curEndpoint->getId()])
            continue;
        if (curEndpoint->getLatestInputArrival() / timingGraph->getClockPeriod() < criticalRatio)
            break;
        std::vector<int> resPath;
        bool findSuccessfully =
            timingGraph->backTraceDelayLongestPathFromNode(curEndpoint->getId(), isCovered, resPath, converThr);
        if (findSuccessfully)
        {
            // std::cout << "find endpoint [" << curEndpoint->getDesignNode()
            //           << "] delay=" << curEndpoint->getLatestInputArrival() << " with " << resPath.size()
            //           << " nodes in path.\n";
            resPaths.push_back(resPath);
            if (checkOverlap)
            {
                for (auto cellId : resPath)
                {
                    auto PU = placementInfo->getPlacementUnitByCellId(cellId);
                    if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(PU))
                    {
                        isCovered[unpackedCell->getCell()->getCellId()]++;
                    }
                    else if (auto macro = dynamic_cast<PlacementInfo::PlacementMacro *>(PU))
                    {
                        for (auto cell : macro->getCells())
                        {
                            isCovered[cell->getCellId()]++;
                        }
                    }
                }
            }
        }

        if (resPaths.size() > pathNumThr)
            break;
    }

    return resPaths;
}

std::vector<std::vector<int>>
PlacementTimingOptimizer::findCriticalPaths(float criticalRatio, std::vector<bool> &FFDirectlyDrivenButNotInOneSlot)
{

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    timingGraph->sortedEndpointByDelay();
    std::vector<int> isCovered(timingGraph->getNodes().size(), 0);
    std::vector<std::vector<int>> resPaths;

    for (auto curEndpoint : timingGraph->getSortedTimingEndpoints())
    {
        if (isCovered[curEndpoint->getId()] || !FFDirectlyDrivenButNotInOneSlot[curEndpoint->getId()])
            continue;
        if (curEndpoint->getLatestInputArrival() / timingGraph->getClockPeriod() < criticalRatio)
            break;
        std::vector<int> resPath;
        bool findSuccessfully =
            timingGraph->backTraceDelayLongestPathFromNode(curEndpoint->getId(), isCovered, resPath, 100);
        if (findSuccessfully)
        {
            // std::cout << "find endpoint [" << curEndpoint->getDesignNode()
            //           << "] delay=" << curEndpoint->getLatestInputArrival() << " with " << resPath.size()
            //           << " nodes in path.\n";
            resPath.resize(2);
            resPaths.push_back(resPath);
            for (auto cellId : resPath)
            {
                auto PU = placementInfo->getPlacementUnitByCellId(cellId);
                if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(PU))
                {
                    isCovered[unpackedCell->getCell()->getCellId()] = 1;
                }
                else if (auto macro = dynamic_cast<PlacementInfo::PlacementMacro *>(PU))
                {
                    for (auto cell : macro->getCells())
                    {
                        isCovered[cell->getCellId()] = 1;
                    }
                }
            }
        }
    }

    return resPaths;
}

float PlacementTimingOptimizer::conductStaticTimingAnalysis(bool disableOptimisticTiming)
{
    print_status("PlacementTimingOptimizer: conducting Static Timing Analysis");

    unsigned int highFanoutThr = 10000;

    if (placementInfo->getNetDistributionByDensity(512) < 200)
    {
        highFanoutThr = 1000;
        print_warning("highFanoutThr is set to 1000");
    }

    if (enableCounter)
        STA_Cnt++;

    increaseLowDelayVal = false;
    if (disableOptimisticTiming)
        increaseLowDelayVal = true;

    effectFactor = (STA_Cnt / 30.0);
    if (effectFactor < 1)
        effectFactor = std::pow(effectFactor, 1);
    else
        effectFactor = (effectFactor - 1) * 0.3 + 1;

    bool printOut = false;
    std::string dumpFileName = "optNetDelayInfo.txt";
    std::ofstream outfile0;
    if (JSONCfg.find("PlacementTimingOptimizer_EdgesDelayLog") != JSONCfg.end())
    {
        printOut = true;
        std::string dumpFileName = JSONCfg["PlacementTimingOptimizer_EdgesDelayLog"];
        print_status("PlacementTimingOptimizer: dumping PlacementTimingOptimizer_EdgesDelayLog to: " + dumpFileName);
        outfile0.open(dumpFileName.c_str());
        assert(outfile0.is_open() && outfile0.good() &&
               "The path for PlacementTimingOptimizer_EdgesDelayLog does not exist and please check "
               "your path settings");
    }

    // std::string cellName =
    // "chip/tile0/g_ariane_core.core/ariane/ex_stage_i/i_mult/i_div/mem_q[4][sbe][result][55]_i_8"; int targetCellId =
    // placementInfo->getDesignInfo()->getCell(cellName)->getCellId(); auto targetCellLoc =
    // placementInfo->getCellId2location()[targetCellId]; print_warning("targetCellLoc X:" +
    // std::to_string(targetCellLoc.X) + " Y:" + std::to_string(targetCellLoc.Y));

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    setPinsLocation();

    auto &pinLoc = placementInfo->getPinId2location();
    auto &cellLoc = placementInfo->getCellId2location();

    auto &edges = timingGraph->getEdges();
    int numEdges = edges.size();

#pragma omp parallel for
    for (int i = 0; i < numEdges; i++)
    {
        auto edge = edges[i];
        auto &pin1Loc = pinLoc[edge->getSourcePin()->getElementIdInType()];
        auto &pin2Loc = pinLoc[edge->getSinkPin()->getElementIdInType()];
        if (pin1Loc.X < -5 && pin1Loc.Y < -5)
            continue;
        if (pin2Loc.X < -5 && pin2Loc.Y < -5)
            continue;

        edge->setDelay(getDelayByModel(edge->getSink(), edge->getSource(), pin1Loc.X, pin1Loc.Y, pin2Loc.X, pin2Loc.Y));
    }

    timingGraph->propogateArrivalTime();
    timingGraph->backPropogateRequiredArrivalTime();
    float maxDelay = 0;
    int maxDelayId = -1;
    for (unsigned int i = 0; i < timingGraph->getNodes().size(); i++)
    {
        if (timingGraph->getNodes()[i]->getLatestInputArrival() > maxDelay)
        {
            maxDelay = timingGraph->getNodes()[i]->getLatestInputArrival();
            maxDelayId = i;
        }
    }

    auto resPath = timingGraph->backTraceDelayLongestPathFromNode(maxDelayId);

    std::cout << "An example of long delay path for the current placement:\n";
    for (auto id : resPath)
    {
        std::cout << designInfo->getCells()[id] << " X:" << cellLoc[id].X << " Y:" << cellLoc[id].Y
                  << "   [delay]: " << timingGraph->getNodes()[id]->getLatestInputArrival()
                  << "   [required]: " << timingGraph->getNodes()[id]->getRequiredArrivalTime() << "\n";
    }

    // // important:::  chip/tile1/g_ariane_core.core/ariane/id_stage_i/operand_b_q[63]_i_31__0
    // print_warning("===========================================================================\n");
    // std::string cellNames[24] = {"design_1_i/DigitRec_0/inst/ap_CS_fsm_reg[9]_replica_1",
    //                              "design_1_i/DigitRec_0/inst/knn_set_311_0_reg_21661[31]_i_2",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_84",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_18",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799_reg[31]_i_8",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_132",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_37",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799_reg[31]_i_9",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_182",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_55",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799_reg[31]_i_10",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_310_1_reg_22001[31]_i_66",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_310_1_reg_22001[31]_i_14",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_310_1_reg_22001_reg[31]_i_4",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_314_1_reg_22175[31]_i_40",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_314_1_reg_22175[31]_i_28",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_314_1_reg_22175_reg[31]_i_5",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_314_1_reg_22175_reg[31]_i_3",
    //                              "design_1_i/DigitRec_0/inst/knn_set_307_3_reg_21799[31]_i_5",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_310_1_reg_22001[31]_i_1",
    //                              "design_1_i/DigitRec_0/inst/knn_set_306_343_reg_21841[8]_i_5",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_316_1_reg_21919[31]_i_3",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_316_1_reg_21919[31]_i_1",
    //                              "design_1_i/DigitRec_0/inst/ap_phi_reg_pp2_iter25_knn_set_316_1_reg_21919_reg[1]"};

    // std::vector<std::string> cellNameVec(cellNames, cellNames + 24);
    // float totalDelay = 0;
    // for (int i = 0; i < cellNameVec.size() - 1; i++)
    // {
    //     auto id = designInfo->getCell(cellNameVec[i])->getCellId();
    //     auto curNode = timingGraph->getNodes()[id];
    //     for (auto edge : curNode->getOutEdges())
    //     {
    //         auto &pin1Loc = pinLoc[edge->getSourcePin()->getElementIdInType()];
    //         auto &pin2Loc = pinLoc[edge->getSinkPin()->getElementIdInType()];
    //         if (pin1Loc.X < -5 && pin1Loc.Y < -5)
    //             continue;
    //         if (pin2Loc.X < -5 && pin2Loc.Y < -5)
    //             continue;
    //         float delay = getDelayByModel(pin1Loc.X, pin1Loc.Y, pin2Loc.X, pin2Loc.Y);
    //         if (edge->getSinkPin()->getCell()->getName() == cellNameVec[i + 1])
    //         {
    //             std::cout << edge->getSinkPin()->getCell() << "X:" << pin1Loc.X << " Y:" << pin1Loc.Y
    //                       << "  [InputTotalDelay]: " << totalDelay << "   [ToNextDelay]: " << delay << "\n";
    //             totalDelay += curNode->getInnerDelay() + delay;
    //             break;
    //         }
    //     }
    // }
    if (printOut)
    {
        for (auto node : timingInfo->getSimplePlacementTimingInfo())
        {
            if (node->getOutEdges().size() > 32)
                continue;
            for (auto edge : node->getOutEdges())
            {
                outfile0 << " src:" << edge->getSourcePin()->getName() << " sink:" << edge->getSinkPin()->getName()
                         << " delay:" << edge->getDelay() << "\n";
            }
        }
        outfile0.close();
    }

    return maxDelay;
}

float PlacementTimingOptimizer::getSlackThr()
{
    assert(placementInfo->getTimingInfo());

    // float maxEnhanceRatio = 0;
    auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
    auto &cellLoc = placementInfo->getCellId2location();
    assert(cellLoc.size() == timingNodes.size());

    // std::ofstream outfile0("timingOptProc.log");

    // auto deviceInfo = placementInfo->getDeviceInfo();

    std::vector<int> slackCntVec(200, 0);
    int totalSlackChecked = 0;

    netActualSlackPinNum.clear();

    for (auto curNet : placementInfo->getPlacementNets())
    {
        auto designNet = curNet->getDesignNet();
        if (designNet->checkIsPowerNet() || designNet->checkIsGlobalClock())
            continue;

        if (curNet->getDriverUnits().size() != 1 || curNet->getUnits().size() <= 1 || curNet->getUnits().size() >= 1000)
            continue;
        auto &pins = designNet->getPins();
        int pinNum = pins.size();

        assert(curNet->getUnits().size() == (unsigned int)pinNum);

        int driverPinInNet = -1;

        for (int i = 0; i < pinNum; i++)
        {
            if (pins[i]->isOutputPort())
            {
                driverPinInNet = i;
                break;
            }
        }

        assert(driverPinInNet >= 0);

        // get the srcPin information
        auto srcCell = pins[driverPinInNet]->getCell();
        unsigned int srcCellId = srcCell->getCellId();
        auto srcNode = timingNodes[srcCellId];
        auto srcLoc = cellLoc[srcCellId];
        // iterate the sinkPin for evaluation and enhancement

        netActualSlackPinNum[curNet] = 0;
        for (int pinBeDriven = 0; pinBeDriven < pinNum; pinBeDriven++)
        {
            if (pinBeDriven == driverPinInNet)
                continue;

            // get the sinkPin information
            auto sinkCell = pins[pinBeDriven]->getCell();
            unsigned int sinkCellId = sinkCell->getCellId();
            auto sinkNode = timingNodes[sinkCellId];
            auto sinkLoc = cellLoc[sinkCellId];
            float netDelay = getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
            float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;

            if (slack > 0)
                continue;

            netActualSlackPinNum[curNet]++;
            int slotId = (int)(-slack / 0.1);
            if (slotId >= 100)
                slotId = 100;
            if (slotId < 0)
                slotId = 0;
            slackCntVec[slotId] += 1;
            totalSlackChecked++;
        }
    }

    float slackThr = 0;
    int slackUnderThrCnt = 0;
    float careRatio = 0.333;

    if (careRatio <= 0.9 && totalSlackChecked > 0)
    {
        for (unsigned int i = 0; i < slackCntVec.size(); i++)
        {
            slackUnderThrCnt += slackCntVec[i];
            if (slackUnderThrCnt > totalSlackChecked * (1 - careRatio))
                break;
            slackThr -= 0.1;
        }
    }

    std::string outputStr = "";
    for (unsigned int i = 0; i < slackCntVec.size(); i++)
    {
        outputStr += " " + std::to_string(slackCntVec[i]);
    }
    print_info("Slack distribution:" + outputStr);
    print_info("slackThr = " + std::to_string(slackThr));
    return slackThr;
}

void PlacementTimingOptimizer::incrementalStaticTimingAnalysis_forPUWithLocation(PlacementInfo::PlacementUnit *curPU,
                                                                                 float targetX, float targetY)
{
    print_status("PlacementTimingOptimizer: conducting incremental Static Timing Analysis");

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    setPinsLocation();

    auto &pinLoc = placementInfo->getPinId2location();

    auto &edges = timingGraph->getEdges();
    int numEdges = edges.size();

#pragma omp parallel for
    for (int i = 0; i < numEdges; i++)
    {
        auto edge = edges[i];
        auto &pin1Loc = pinLoc[edge->getSourcePin()->getElementIdInType()];
        auto &pin2Loc = pinLoc[edge->getSinkPin()->getElementIdInType()];
        if (pin1Loc.X < -5 && pin1Loc.Y < -5)
            continue;
        if (pin2Loc.X < -5 && pin2Loc.Y < -5)
            continue;

        edge->setDelay(getDelayByModel(edge->getSink(), edge->getSource(), pin1Loc.X, pin1Loc.Y, pin2Loc.X, pin2Loc.Y));
    }

    timingGraph->propogateArrivalTime();
    timingGraph->backPropogateRequiredArrivalTime();
    float maxDelay = 0;
    int maxDelayId = -1;
    for (unsigned int i = 0; i < timingGraph->getNodes().size(); i++)
    {
        if (timingGraph->getNodes()[i]->getLatestInputArrival() > maxDelay)
        {
            maxDelay = timingGraph->getNodes()[i]->getLatestInputArrival();
            maxDelayId = i;
        }
    }

    auto resPath = timingGraph->backTraceDelayLongestPathFromNode(maxDelayId);

    std::cout << "An example of long delay path for the current placement:\n";
    for (auto id : resPath)
    {
        std::cout << designInfo->getCells()[id]
                  << "   [delay]: " << timingGraph->getNodes()[id]->getLatestInputArrival()
                  << "   [required]: " << timingGraph->getNodes()[id]->getRequiredArrivalTime() << "\n";
    }
}

void PlacementTimingOptimizer::clusterLongPathInOneClockRegion(int pathLenThr, float clusterThrRatio)
{
    // if (DSPCritical)
    //     return;
    placementInfo->updateElementBinGrid();
    conductStaticTimingAnalysis();
    print_warning("PlacementTimingOptimizer: clustering long path in one clock region");
    placementInfo->updateElementBinGrid();
    auto &timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo_PathLenSorted();
    auto simpleTimingGraph = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph();
    auto &cellLoc = placementInfo->getCellId2location();
    auto deviceInfo = placementInfo->getDeviceInfo();
    auto YX2ClockRegion = deviceInfo->getClockRegions();
    auto &PU2ClockRegionCenter = placementInfo->getPU2ClockRegionCenters();
    auto &PU2ClockRegionColumn = placementInfo->getPU2ClockRegionColumn();

    PU2ClockRegionCenter.clear();
    PU2ClockRegionColumn.clear();

    std::set<int> extractedCellIds;
    std::set<PlacementInfo::PlacementUnit *> extractedPUs;

    extractedCellIds.clear();
    extractedPUs.clear();
    clockRegionclusters.clear();

    unsigned int fanoutThr = 512; // placementInfo->getHighFanOutThr();
    int sizeThr = 20000;
    if (placementInfo->isDensePlacement() || clockRegionClusterTooLarge)
        sizeThr = 2000;
    for (unsigned int nodeId = 0; nodeId < timingNodes.size() * 0.1; nodeId++)
    {
        auto timingNode = timingNodes[nodeId];
        if (timingNode->getLongestPathLength() > pathLenThr)
        {
            if (extractedCellIds.find(nodeId) != extractedCellIds.end())
                continue;
            auto candidateCellIds =
                simpleTimingGraph->DFSFromNode(timingNode->getId(), pathLenThr, sizeThr, extractedCellIds, 1000000);

            if (candidateCellIds.size() >= pathLenThr * 0.8)
            {
                std::set<PlacementInfo::PlacementUnit *> PUsInLongPaths;
                PUsInLongPaths.clear();

                float totalWeight = 0;
                for (auto &candidateCellId : candidateCellIds)
                {
                    auto PUInPath = placementInfo->getPlacementUnitByCellId(candidateCellId);
                    if (extractedPUs.find(PUInPath) == extractedPUs.end() &&
                        PUsInLongPaths.find(PUInPath) == PUsInLongPaths.end())
                    {
                        PUsInLongPaths.insert(PUInPath);
                        totalWeight += PUInPath->getWeight();
                    }
                }

                if (totalWeight >= pathLenThr)
                {
                    std::map<std::pair<int, int>, float> clockRegionYX2Cnt;
                    clockRegionYX2Cnt.clear();

                    float maxClockRegionWeight = 0;
                    float totalClockRegionWeight = 0;
                    std::pair<int, int> optClockLocYX;

                    for (auto tmpPU : PUsInLongPaths)
                    {
                        if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                        {
                            int cellId = unpackedCell->getCell()->getCellId();
                            int clockRegionX, clockRegionY;
                            auto tmpLoc = cellLoc[cellId];
                            deviceInfo->getClockRegionByLocation(tmpLoc.X, tmpLoc.Y, clockRegionX, clockRegionY);
                            std::pair<int, int> tmpClockLocYX(-1, clockRegionX);
                            // if (clockRegionX < 2)
                            //     clockRegionX = 1;
                            // if (clockRegionX > 2)
                            //     clockRegionX = 3;
                            if (clockRegionYX2Cnt.find(tmpClockLocYX) == clockRegionYX2Cnt.end())
                            {
                                clockRegionYX2Cnt[tmpClockLocYX] = 0;
                            }
                            clockRegionYX2Cnt[tmpClockLocYX] += 1;
                            totalClockRegionWeight += 1;

                            if (clockRegionYX2Cnt[tmpClockLocYX] > maxClockRegionWeight)
                            {
                                maxClockRegionWeight = clockRegionYX2Cnt[tmpClockLocYX];
                                optClockLocYX = tmpClockLocYX;
                            }
                        }
                        else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                        {
                            for (auto tmpCell : curMacro->getCells())
                            {
                                int cellId = tmpCell->getCellId();
                                int clockRegionX, clockRegionY;
                                auto tmpLoc = cellLoc[cellId];
                                deviceInfo->getClockRegionByLocation(tmpLoc.X, tmpLoc.Y, clockRegionX, clockRegionY);

                                std::pair<int, int> tmpClockLocYX(-1, clockRegionX);
                                if (clockRegionYX2Cnt.find(tmpClockLocYX) == clockRegionYX2Cnt.end())
                                {
                                    clockRegionYX2Cnt[tmpClockLocYX] = 0;
                                }
                                clockRegionYX2Cnt[tmpClockLocYX] += 1;
                                totalClockRegionWeight += 1;

                                if (clockRegionYX2Cnt[tmpClockLocYX] > maxClockRegionWeight)
                                {
                                    maxClockRegionWeight = clockRegionYX2Cnt[tmpClockLocYX];
                                    optClockLocYX = tmpClockLocYX;
                                }
                            }
                        }
                    }

                    if ((maxClockRegionWeight > totalClockRegionWeight * clusterThrRatio) &&
                        maxClockRegionWeight >= pathLenThr * 0.333)
                    {
                        // if (optClockLocYX.second < 2)
                        //     optClockLocYX.second = 1;
                        // if (optClockLocYX.second > 2)
                        //     optClockLocYX.second = 3;
                        auto optClockRegion = YX2ClockRegion[0][optClockLocYX.second];
                        float cX = (optClockRegion->getLeft() + optClockRegion->getRight()) / 2;
                        std::vector<int> PUIdsInLongPaths;
                        PUIdsInLongPaths.clear();
                        bool abandon = false;
                        for (auto curPU : PUsInLongPaths)
                        {
                            if (curPU->checkHasDSP())
                                abandon = true;
                        }
                        if (abandon)
                            break;
                        for (auto curPU : PUsInLongPaths)
                        {
                            if (!curPU->isFixed() && !curPU->checkHasBRAM() && !curPU->checkHasDSP())
                            {
                                float fX = cX;
                                float fY = curPU->Y();
                                placementInfo->legalizeXYInArea(curPU, fX, fY);
                                // curPU->setAnchorLocationAndForgetTheOriginalOne(fX, fY);
                                extractedPUs.insert(curPU);

                                PU2ClockRegionCenter[curPU] = std::pair<float, float>(fX, fY);
                                PU2ClockRegionColumn[curPU] = optClockLocYX.second;

                                if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                                {
                                    int cellId = unpackedCell->getCell()->getCellId();
                                    if (timingNodes[cellId]->getOutEdges().size() < fanoutThr)
                                        extractedCellIds.insert(cellId);
                                }
                                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                                {
                                    for (auto tmpCell : curMacro->getCells())
                                    {
                                        int cellId = tmpCell->getCellId();
                                        if (timingNodes[cellId]->getOutEdges().size() < fanoutThr)
                                            extractedCellIds.insert(cellId);
                                    }
                                }
                                PUIdsInLongPaths.push_back(curPU->getId());
                            }
                        }

                        clockRegionclusters.push_back(PUIdsInLongPaths);
                        std::cout << "maxClockRegionWeight: " << maxClockRegionWeight
                                  << " totalClockRegionWeight:" << totalClockRegionWeight
                                  << " #extractedCellIds=" << extractedCellIds.size()
                                  << " #extractedPUs=" << extractedPUs.size()
                                  << " pathLength=" << timingNode->getLongestPathLength() << "\n";
                    }

                    else if (totalClockRegionWeight >= 20000)
                    {
                        for (auto tmpCellId : candidateCellIds)
                        {
                            if (timingNodes[tmpCellId]->getOutEdges().size() < fanoutThr)
                                extractedCellIds.insert(tmpCellId);
                        }
                    }
                }
            }
        }

        if (extractedCellIds.size() > timingNodes.size() * 0.2 && sizeThr > 20000)
        {
            PU2ClockRegionCenter.clear();
            PU2ClockRegionColumn.clear();

            extractedCellIds.clear();
            extractedPUs.clear();
            clockRegionclusters.clear();

            sizeThr = 2000;
            clockRegionClusterTooLarge = true;
        }
    }
    dumpClockRegionClusters();
    // if (!placementInfo->isDensePlacement())
    stretchClockRegionColumns();
}

void PlacementTimingOptimizer::stretchClockRegionColumns()
{
    auto &binGrid = placementInfo->getGlobalBinGrid();
    auto &PU2ClockRegionColumn = placementInfo->getPU2ClockRegionColumn();
    auto YX2ClockRegion = deviceInfo->getClockRegions();
    int clockRegionXNum = YX2ClockRegion[0].size();
    std::vector<int> curClockRegionX2cellNum(clockRegionXNum, 0);
    std::vector<std::vector<PlacementInfo::PlacementUnit *>> clockRegionX2PUs(
        clockRegionXNum, std::vector<PlacementInfo::PlacementUnit *>(0));
    std::vector<float> curClockRegionX2MinY(clockRegionXNum, placementInfo->getGlobalBinMaxLocY());
    std::vector<float> curClockRegionX2MaxY(clockRegionXNum, placementInfo->getGlobalBinMinLocY());

    for (auto PU : placementInfo->getPlacementUnits())
    {
        int clockRegionX, clockRegionY;
        deviceInfo->getClockRegionByLocation(PU->X(), PU->X(), clockRegionX, clockRegionY);
        curClockRegionX2cellNum[clockRegionX] += PU->getWeight();
        clockRegionX2PUs[clockRegionX].push_back(PU);

        int binIdX, binIdY;
        placementInfo->getGridXY(PU->X(), PU->Y(), binIdX, binIdY);

        if (binGrid[binIdY][binIdX]->getCells().size() > 16)
        {
            if (PU->Y() > curClockRegionX2MaxY[clockRegionX])
            {
                curClockRegionX2MaxY[clockRegionX] = PU->Y();
            }
            if (PU->Y() < curClockRegionX2MinY[clockRegionX])
            {
                curClockRegionX2MinY[clockRegionX] = PU->Y();
            }
        }
    }

    std::vector<int> newClockRegionX2cellNum = curClockRegionX2cellNum;
    for (auto pair : PU2ClockRegionColumn)
    {
        auto PU = pair.first;
        int oriClockRegionX, oriClockRegionY;
        deviceInfo->getClockRegionByLocation(PU->X(), PU->X(), oriClockRegionX, oriClockRegionY);
        newClockRegionX2cellNum[oriClockRegionX] -= PU->getWeight();
        newClockRegionX2cellNum[pair.second] += PU->getWeight();
    }

    for (unsigned int colX = 0; colX < curClockRegionX2cellNum.size(); colX++)
    {
        float stretchRatio = (float)newClockRegionX2cellNum[colX] / (float)curClockRegionX2cellNum[colX];

        float bottomLimit = placementInfo->getGlobalBinMinLocY();
        float topLimit = placementInfo->getGlobalBinMaxLocY();
        float completeH = topLimit - bottomLimit;
        float oriTopY = curClockRegionX2MaxY[colX];
        float oriBottomY = curClockRegionX2MinY[colX];
        float oriH = std::abs(oriTopY - oriBottomY);
        bool maybeCongestion =
            completeH * 0.625 > oriH && (oriBottomY < 0.075 * completeH || oriTopY > 0.925 * completeH);

        if (oriBottomY < 0.075 * completeH)
            bottomLimit = std::max(std::max(oriBottomY, bottomLimit), (float)0.05 * completeH);
        if (oriTopY > 0.925 * completeH)
            topLimit = std::min(std::min(oriTopY, topLimit), (float)0.95 * completeH);

        if (std::abs(oriTopY - oriBottomY) < 0.1)
            continue;
        if (stretchRatio > 1 || (maybeCongestion && stretchRatio > 0.95))
        {
            if (maybeCongestion)
            {
                print_warning("PlacementTimingOptimizer: " + std::to_string(colX) +
                              "th column is stretched more to avoid potential congestion");
                stretchRatio *= 1.1;
            }
            else
                stretchRatio *= 1.05;
            float newH = stretchRatio * oriH;
            float deltaH = newH - oriH;
            float newTopY = oriTopY + deltaH / 2;
            float newBottomY = oriBottomY - deltaH / 2;

            if (newBottomY < bottomLimit)
            {
                newTopY += (bottomLimit - newBottomY);
                newBottomY = bottomLimit;
            }

            if (newTopY > topLimit)
            {
                newBottomY = std::max(bottomLimit, newBottomY - (newTopY - topLimit));
                newTopY = topLimit;
            }
            newH = newTopY - newBottomY;
            if (oriH < 0.01)
                continue;
            stretchRatio = newH / oriH;
            for (auto PU : clockRegionX2PUs[colX])
            {
                if (!PU->isFixed())
                {
                    float fY = (PU->Y() - oriBottomY) * stretchRatio + newBottomY;
                    float fX = PU->X();
                    placementInfo->legalizeXYInArea(PU, fX, fY);
                    PU->setAnchorLocationAndForgetTheOriginalOne(fX, fY);
                }
            }
            print_warning("PlacementTimingOptimizer: " + std::to_string(colX) + "th column is stretched by " +
                          std::to_string(stretchRatio) + " oriBottomY=" + std::to_string(oriBottomY) +
                          " oriTopY=" + std::to_string(oriTopY) + " newBottomY=" + std::to_string(newBottomY) +
                          " newTopY=" + std::to_string(newTopY));
        }
    }

    placementInfo->updateElementBinGrid();
}

void PlacementTimingOptimizer::dumpClockRegionClusters()
{
    std::string dumpClockRegionClustersFile = JSONCfg["Dump Cluster file"] + "-clockRegion";
    if (dumpClockRegionClustersFile != "")
    {
        print_status("dumping cluster information to " + dumpClockRegionClustersFile);
        std::ofstream outfile0((dumpClockRegionClustersFile + ".tcl").c_str());
        assert(outfile0.is_open() && outfile0.good() &&
               "The path for cluster result dumping does not exist and please check your path settings");
        for (unsigned int cluster_id = 0; cluster_id < clockRegionclusters.size(); cluster_id++)
        {
            outfile0 << "highlight -color_index " << (cluster_id) % 20 + 1 << "  [get_cells {";
            for (int id : clockRegionclusters[cluster_id])
            {
                if (auto tmpMacro =
                        dynamic_cast<PlacementInfo::PlacementMacro *>(placementInfo->getPlacementUnits()[id]))
                {
                    for (auto cell : tmpMacro->getCells())
                    {
                        outfile0 << cell->getName() << " ";
                    }
                }
                else if (auto tmpUnpacked = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(
                             placementInfo->getPlacementUnits()[id]))
                {
                    outfile0 << tmpUnpacked->getName() << " ";
                }
                else
                {
                    assert(false);
                }
            }
            outfile0 << "}]\n";
        }
        outfile0.close();
    }
}

void PlacementTimingOptimizer::propogateArrivalTime()
{
    assert(timingInfo);
    // auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
}

std::vector<float> &PlacementTimingOptimizer::getPUId2Slack(bool update)
{
    if (update)
    {
        auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
        // float clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
        auto &cellLoc = placementInfo->getCellId2location();
        assert(cellLoc.size() == timingNodes.size());

        unsigned int highFanoutThr = 1000;

        int PUNum = placementInfo->getPlacementUnits().size();
        PUId2Slack.clear();
        PUId2Slack.resize(PUNum, 0);

#pragma omp parallel for
        for (int PUId = 0; PUId < PUNum; PUId++)
        {
            auto &curPU = placementInfo->getPlacementUnits()[PUId];

            for (auto curNet : *(curPU->getNetsSetPtr()))
            {
                if (curNet->getDriverUnits().size() == 0)
                    continue;
                if (curNet->getDriverUnits()[0] != curPU)
                    continue;

                auto designNet = curNet->getDesignNet();
                assert(designNet);
                if (designNet->checkIsPowerNet() || designNet->checkIsGlobalClock())
                    continue;

                if (curNet->getDriverUnits().size() != 1 || curNet->getUnits().size() <= 1 ||
                    curNet->getUnits().size() >= highFanoutThr)
                    continue;
                auto &PUs = curNet->getUnits();
                auto &pins = designNet->getPins();
                int pinNum = pins.size();

                assert(curNet->getUnits().size() == (unsigned int)pinNum);

                int driverPinInNet = -1;

                for (int i = 0; i < pinNum; i++)
                {
                    if (pins[i]->isOutputPort())
                    {
                        driverPinInNet = i;
                        break;
                    }
                }

                assert(driverPinInNet >= 0);

                // get the srcPin information
                auto srcCell = pins[driverPinInNet]->getCell();
                unsigned int srcCellId = srcCell->getCellId();
                auto srcNode = timingNodes[srcCellId];
                auto srcLoc = cellLoc[srcCellId];

                for (int pinBeDriven = 0; pinBeDriven < pinNum; pinBeDriven++)
                {
                    if (pinBeDriven == driverPinInNet)
                        continue;

                    // get the sinkPin information
                    auto sinkCell = pins[pinBeDriven]->getCell();
                    unsigned int sinkCellId = sinkCell->getCellId();
                    auto sinkNode = timingNodes[sinkCellId];
                    auto sinkLoc = cellLoc[sinkCellId];

                    float netDelay = getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
                    float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;
                    if (slack < 0)
                    {
                        if (slack < PUId2Slack[PUId])
                            PUId2Slack[PUId] = slack;
                    }
                }
            }
        }
    }
    return PUId2Slack;
}
