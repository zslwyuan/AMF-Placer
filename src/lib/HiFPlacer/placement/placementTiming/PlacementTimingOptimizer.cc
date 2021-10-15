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

    designInfo = placementInfo->getDesignInfo();
    deviceInfo = placementInfo->getDeviceInfo();
    initPois();
}

void PlacementTimingOptimizer::enhanceNetWeight_LevelBased(int levelThr)
{
    bool printOut = false;
    std::string dumpFileName = "optNetInfo.txt";
    std::ofstream outfile0;
    if (JSONCfg.find("PlacementTimingOptimizer_EnhanceNetWeightLevelBasedLog") != JSONCfg.end())
    {
        printOut = true;
        std::string dumpFileName = JSONCfg["PlacementTimingOptimizer_EnhanceNetWeightLevelBasedLog"];
        print_status("PlacementTimingOptimizer: dumping enhanceNetWeight_LevelBased to: " + dumpFileName);
        outfile0.open(dumpFileName.c_str());
        assert(outfile0.is_open() && outfile0.good() &&
               "The path for PlacementTimingOptimizer_EnhanceNetWeightLevelBasedLog does not exist and please check "
               "your path settings");
    }

    print_status("PlacementTimingOptimizer: enhanceNetWeight_LevelBased starts.");
    if (levelThr < 4)
        return;
    assert(timingInfo);

    float maxEnhanceRatio = 0;
    auto timingNodes = timingInfo->getSimplePlacementTimingInfo();
    for (auto cellA : designInfo->getCells())
    {
        if (cellA->isVirtualCell())
            continue;
        int targetPathLen = timingNodes[cellA->getCellId()]->getLongestPathLength();
        if (targetPathLen < levelThr)
            continue;

        // int pinAIdInNet = 0;
        for (DesignInfo::DesignPin *curPinA : cellA->getPins())
        {
            if (curPinA->getNet())
            {
                if (curPinA->isInputPort())
                    continue;
                // int pinBIdInNet = 0;
                auto pins = curPinA->getNet()->getPins();
                int pinNum = pins.size();
                if (pinNum <= 1 || pinNum >= 1000)
                    continue;

                float enhanceRatio;
                float overflowRatio = std::pow((float)0.8 * targetPathLen / levelThr, 1);
                // if (overflowRatio > 10)
                //     overflowRatio = 10;
                if (pinNum < 600)
                    enhanceRatio = 1.5 * (overflowRatio + 0.0025 * pinNum);
                else
                    enhanceRatio = 1.5 * (overflowRatio + 1.5);

                enhanceRatio = std::sqrt(enhanceRatio);
                if (enhanceRatio > maxEnhanceRatio)
                    maxEnhanceRatio = enhanceRatio;
                curPinA->getNet()->enhanceOverallNetEnhancement(enhanceRatio);
                if (curPinA->getName() ==
                    "chip/tile1/g_ariane_core.core/ariane/id_stage_i/issue_q[sbe][pc][63]_i_1__0/O")
                {
                    float tmpV = curPinA->getNet()->getOverallEnhanceRatio();
                    print_warning("THE PIN is enhanced by " + std::to_string(enhanceRatio) + " targetPathLen=" +
                                  std::to_string(targetPathLen) + " totalEnhance=" + std::to_string(tmpV));
                    print_warning("THE PIN is enhanced by " + std::to_string(enhanceRatio) + " targetPathLen=" +
                                  std::to_string(targetPathLen) + " totalEnhance=" + std::to_string(tmpV));
                    print_warning("THE PIN is enhanced by " + std::to_string(enhanceRatio) + " targetPathLen=" +
                                  std::to_string(targetPathLen) + " totalEnhance=" + std::to_string(tmpV));
                    print_warning("THE PIN is enhanced by " + std::to_string(enhanceRatio) + " targetPathLen=" +
                                  std::to_string(targetPathLen) + " totalEnhance=" + std::to_string(tmpV));
                }
                if (printOut)
                {
                    outfile0 << "enhanced net: [ " << curPinA->getName()
                             << "] fanOut:" << curPinA->getNet()->getPins().size() << " by " << enhanceRatio << "\n";
                }
            }
        }
    }
    if (printOut)
    {
        for (auto cellA : designInfo->getCells())
        {
            if (cellA->isVirtualCell())
                continue;
            int targetPathLen = timingNodes[cellA->getCellId()]->getLongestPathLength();

            outfile0 << "cell path len: [ " << cellA->getName() << "] = " << targetPathLen << "\n";
        }
        outfile0.close();
    }

    print_status("PlacementTimingOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=" +
                 std::to_string(maxEnhanceRatio) + ")");
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

void PlacementTimingOptimizer::setEdgesDelay()
{

    bool printOut = false;
    std::string dumpFileName = "optNetDelayInfo.txt";
    std::ofstream outfile0;
    if (JSONCfg.find("PlacementTimingOptimizer_EdgesDelayLog") != JSONCfg.end())
    {
        printOut = true;
        std::string dumpFileName = JSONCfg["PlacementTimingOptimizer_EdgesDelayLog"];
        print_status("PlacementTimingOptimizer: dumping enhanceNetWeight_LevelBased to: " + dumpFileName);
        outfile0.open(dumpFileName.c_str());
        assert(outfile0.is_open() && outfile0.good() &&
               "The path for PlacementTimingOptimizer_EdgesDelayLog does not exist and please check "
               "your path settings");
    }

    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
    setPinsLocation();

    auto &pinLoc = placementInfo->getPinId2location();
    for (auto edge : timingGraph->getEdges())
    {
        auto &pin1Loc = pinLoc[edge->getSourcePin()->getElementIdInType()];
        auto &pin2Loc = pinLoc[edge->getSinkPin()->getElementIdInType()];
        if (pin1Loc.X < -5 && pin1Loc.Y < -5)
            continue;
        if (pin2Loc.X < -5 && pin2Loc.Y < -5)
            continue;
        edge->setDelay(std::fabs(pin1Loc.X - pin2Loc.X) * xDelayUnit + std::fabs(pin1Loc.Y - pin2Loc.Y) * yDelayUnit);
    }

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
}

void PlacementTimingOptimizer::clusterLongPathInOneClockRegion(int pathLenThr, float clusterThrRatio)
{
    print_warning("PlacementTimingOptimizer: clustering long path in one clock region");

    auto &timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo_PathLenSorted();
    auto simpleTimingGraph = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph();
    auto &cellLoc = placementInfo->getCellId2location();
    auto deviceInfo = placementInfo->getDeviceInfo();
    auto YX2ClockRegion = deviceInfo->getClockRegions();
    auto &PU2ClockRegionCenter = placementInfo->getPU2ClockRegionCenters();
    PU2ClockRegionCenter.clear();

    std::set<int> extractedCellIds;
    std::set<PlacementInfo::PlacementUnit *> extractedPUs;
    extractedCellIds.clear();
    extractedPUs.clear();

    unsigned int maxSize = 0;
    for (unsigned int nodeId = 0; nodeId < timingNodes.size() * 0.1; nodeId++)
    {
        auto timingNode = timingNodes[nodeId];
        if (timingNode->getLongestPathLength() > pathLenThr)
        {
            auto candidateCellIds =
                simpleTimingGraph->BFSFromNode(timingNode->getId(), pathLenThr, 20000, extractedCellIds);

            if (candidateCellIds.size() >= pathLenThr * 0.8)
            {
                std::set<PlacementInfo::PlacementUnit *> PUsInLongPaths;
                PUsInLongPaths.clear();
                for (auto &candidateCellId : candidateCellIds)
                {
                    auto PUInPath = placementInfo->getPlacementUnitByCellId(candidateCellId);
                    if (extractedPUs.find(PUInPath) == extractedPUs.end() &&
                        PUsInLongPaths.find(PUInPath) == PUsInLongPaths.end())
                    {
                        PUsInLongPaths.insert(PUInPath);
                    }
                }

                if (PUsInLongPaths.size() >= 8)
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
                        (maxClockRegionWeight < totalClockRegionWeight * 0.95) && maxClockRegionWeight >= 4)
                    {
                        auto optClockRegion = YX2ClockRegion[0][optClockLocYX.second];
                        float cX = (optClockRegion->getLeft() + optClockRegion->getRight()) / 2;
                        float cY = (optClockRegion->getTop() + optClockRegion->getBottom()) / 2;
                        for (auto curPU : PUsInLongPaths)
                        {
                            if (!curPU->isFixed())
                            {
                                float fX = cX;
                                float fY = curPU->Y();
                                placementInfo->legalizeXYInArea(curPU, fX, fY);
                                // curPU->setAnchorLocationAndForgetTheOriginalOne(fX, fY);
                                extractedPUs.insert(curPU);

                                PU2ClockRegionCenter[curPU] = std::pair<float, float>(fX, fY);

                                if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                                {
                                    int cellId = unpackedCell->getCell()->getCellId();
                                    extractedCellIds.insert(cellId);
                                }
                                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                                {
                                    for (auto tmpCell : curMacro->getCells())
                                    {
                                        int cellId = tmpCell->getCellId();
                                        extractedCellIds.insert(cellId);
                                    }
                                }
                            }
                        }
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
                            extractedCellIds.insert(tmpCellId);
                        }
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
    print_info("ClusterPlacer: largest long-path cluster size=" + std::to_string(maxSize));
}

void PlacementTimingOptimizer::propogateArrivalTime()
{
    assert(timingInfo);
    // auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
}