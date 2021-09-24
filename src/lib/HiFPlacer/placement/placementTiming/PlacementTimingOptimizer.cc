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
                float overflowRatio = std::pow((float)targetPathLen / levelThr, 1.75);
                // if (overflowRatio > 10)
                //     overflowRatio = 10;
                if (pinNum < 600)
                    enhanceRatio = 1.5 * (overflowRatio + 0.0025 * pinNum);
                else
                    enhanceRatio = 1.5 * (overflowRatio + 1.5);
                if (enhanceRatio > maxEnhanceRatio)
                    maxEnhanceRatio = enhanceRatio;
                curPinA->getNet()->enhanceOverallNetEnhancement(enhanceRatio);
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

void PlacementTimingOptimizer::propogateArrivalTime()
{
    assert(timingInfo);
    auto timingGraph = timingInfo->getSimplePlacementTimingGraph();
}