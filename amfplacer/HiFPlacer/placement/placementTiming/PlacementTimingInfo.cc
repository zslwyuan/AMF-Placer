/**
 * @file PlacementTimingInfo.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation for the timing information related to placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "PlacementTimingInfo.h"
#include "PlacementInfo.h"
#include "stringCheck.h"
#include <algorithm>
#include <cmath>
#include <codecvt>
#include <queue>
#include <set>
#include <stack>

PlacementTimingInfo::PlacementTimingInfo(DesignInfo *designInfo, DeviceInfo *deviceInfo,
                                         std::map<std::string, std::string> JSONCfg)
    : designInfo(designInfo), deviceInfo(deviceInfo), JSONCfg(JSONCfg)
{
    if (JSONCfg.find("PlacementTimingInfoVerbose") != JSONCfg.end())
        verbose = JSONCfg["PlacementTimingInfoVerbose"] == "true";

    if (JSONCfg.find("ClockPeriod") != JSONCfg.end())
        clockPeriod = std::stof(JSONCfg["ClockPeriod"]);

    if (JSONCfg.find("DSPCritical") != JSONCfg.end())
        DSPCritical = JSONCfg["DSPCritical"] == "true";

    clockNet2Period.clear();
    cellId2Period.clear();
    for (auto pair : JSONCfg)
    {
        if (pair.first.find("ClockPeriod:") != std::string::npos)
        {
            auto clockDriver = pair.first;
            std::string from = "ClockPeriod:";
            std::string to = "";
            replaceAll(clockDriver, from, to);
            auto driverNet = designInfo->getNet(clockDriver);
            float tmpClkPeriod = std::stof(pair.second);

            clockNet2Period[driverNet] = tmpClkPeriod;
            for (auto succPin : driverNet->getPinsBeDriven())
            {
                auto cell = succPin->getCell();
                if (cell)
                {
                    cellId2Period[cell->getCellId()] = tmpClkPeriod;
                }
            }
        }
    }
}

void PlacementTimingInfo::setDSPInnerDelay()
{
    if (!DSPCritical)
        return;
    auto &nodes = simpleTimingGraph->getNodes();
    for (auto curCell : designInfo->getCells())
    {

        if (curCell->isDSP())
        {
            auto newNode = nodes[curCell->getCellId()];
            newNode->setInnerDelay(1.3);
        }
    }

    print_status("PlacementTimingInfo: set DSP inner delay");
}

void PlacementTimingInfo::buildSimpleTimingGraph()
{
    print_status("PlacementTimingInfo: building simple timing graph (TimingNode is DesignCell)");
    simpleTimingGraph = new TimingGraph<DesignInfo::DesignCell>(this);
    simpleTimingGraph->setClockPeriod(clockPeriod);

    for (auto curCell : designInfo->getCells())
    {
        auto newNode =
            new TimingGraph<DesignInfo::DesignCell>::TimingNode(curCell, simpleTimingGraph->getNodes().size());

        if (curCell->isTimingEndPoint())
        {
            if (!curCell->isDSP())
            {
                newNode->setIsRegister();
                if (cellId2Period.find(curCell->getCellId()) != cellId2Period.end())
                {
                    newNode->setClockPeriod(cellId2Period[curCell->getCellId()]);
                }
                else
                {
                    newNode->setClockPeriod(clockPeriod);
                }
            }
            else if (!DSPCritical || curCell->checkHasDSPReg())
            {
                newNode->setIsRegister();
                if (cellId2Period.find(curCell->getCellId()) != cellId2Period.end())
                {
                    newNode->setClockPeriod(cellId2Period[curCell->getCellId()]);
                }
                else
                {
                    newNode->setClockPeriod(clockPeriod);
                }
            }
        }
        simpleTimingGraph->insertTimingNode(newNode);
    }

    float defaultNetDelay = 1;
    for (auto curCell : designInfo->getCells())
    {
        assert(curCell);
        int curId = curCell->getCellId();
        for (auto srcPin : curCell->getOutputPins())
        {
            assert(srcPin);
            auto curNet = srcPin->getNet();
            if (!curNet)
                continue;
            if (curNet->checkIsGlobalClock() || curNet->checkIsPowerNet())
                continue;
            for (auto pinBeDriven : curNet->getPinsBeDriven())
            {
                if (auto cellBeDriven = pinBeDriven->getCell())
                {
                    assert(srcPin);
                    assert(pinBeDriven);
                    simpleTimingGraph->addEdgeBetween(curId, cellBeDriven->getCellId(), srcPin, pinBeDriven, curNet,
                                                      defaultNetDelay);
                }
            }
        }
    }

    simpleTimingGraph->forwardLevelization();
    simpleTimingGraph->backwardLevelization();
    simpleTimingGraph->setLongestPathLength();

    // std::string cellName = "chip/tile0/g_ariane_core.core/ariane/id_stage_i/operand_a_q[21]_i_8";
    // auto debugCell = designInfo->getCell(cellName);
    // std::cout << debugCell << "\n";
    // assert(!simpleTimingGraph->getNodes()[debugCell->getCellId()]->checkIsRegister());
    // auto listA = simpleTimingGraph->traceBackFromNode(debugCell->getCellId());
    // auto listB = simpleTimingGraph->traceForwardFromNode(debugCell->getCellId());
    // std::cout << "\n\n";
    // for (auto id : listA)
    // {
    //     std::cout << " " << designInfo->getCells()[id]->getName()
    //               << " level:" << simpleTimingGraph->getNodes()[id]->getForwardLevel()
    //               << " path:" << simpleTimingGraph->getNodes()[id]->getLongestPathLength() << "\n";
    // }
    // std::cout << "\n\n";
    // for (auto id : listB)
    // {
    //     std::cout << " " << designInfo->getCells()[id]->getName()
    //               << " level:" << simpleTimingGraph->getNodes()[id]->getForwardLevel()
    //               << " path:" << simpleTimingGraph->getNodes()[id]->getLongestPathLength() << "\n";
    // }
    print_status("PlacementTimingInfo: built simple timing graph");
}

template <typename nodeType> void PlacementTimingInfo::TimingGraph<nodeType>::forwardLevelization()
{
    print_status("PlacementTimingInfo: Timing graph starts forward levalization");
    forwardlevel2NodeIds.clear();

    std::vector<int> curLevelIds;
    curLevelIds.clear();
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->checkIsRegister())
        {
            curLevelIds.push_back(i);
            nodes[i]->setForwardLevel(0);
        }
    }

    int curLevel = 0;
    while (curLevelIds.size())
    {
        // print_info("level#" + std::to_string(curLevel) + " has " + std::to_string(curLevelIds.size()) + " nodes");
        forwardlevel2NodeIds.push_back(curLevelIds);

        curLevelIds.clear();

        int nextLevel = curLevel + 1;
        for (auto curId : forwardlevel2NodeIds[curLevel])
        {
            for (auto outEdge : nodes[curId]->getOutEdges())
            {
                int nextId = outEdge->getSink()->getId();

                if (!nodes[nextId]->checkIsRegister())
                {
                    if (nodes[nextId]->getForwardLevel() < nextLevel)
                    {
                        nodes[nextId]->setForwardLevel(nextLevel);
                        curLevelIds.push_back(nextId);
                    }
                }
            }
        }
        curLevel++;

        if (curLevel > 100)
        {
            print_warning("Reach level#" + std::to_string(curLevel) + " has " + std::to_string(curLevelIds.size()) +
                          " nodes. It might mean that there are loops in timing graph.");
            std::vector<int> nodeInPath;
            for (auto tmpId : curLevelIds)
            {
                nodeInPath.clear();
                nodeInPath.push_back(tmpId);
                findALoopFromNode(nodeInPath, tmpId, tmpId, 0);
            }
        }
    }

    // some nodes are duplicated in some lower levels, remove them
    for (unsigned int level = 0; level < forwardlevel2NodeIds.size(); level++)
    {
        std::vector<int> filteredList;
        filteredList.clear();
        for (auto id : forwardlevel2NodeIds[level])
        {
            assert(nodes[id]->getForwardLevel() >= 0);
            if ((unsigned int)nodes[id]->getForwardLevel() == level)
                filteredList.push_back(id);
        }
        forwardlevel2NodeIds[level] = filteredList;
    }

    // set the destination level of the nodes, in reversed topological order
    for (int level = forwardlevel2NodeIds.size() - 2; level >= 0; level--)
    {
        for (auto curId : forwardlevel2NodeIds[level])
        {
            for (auto outEdge : nodes[curId]->getOutEdges())
            {
                int nextId = outEdge->getSink()->getId();

                if (!nodes[nextId]->checkIsRegister())
                {
                    if (nodes[nextId]->getDestLevel() > nodes[curId]->getDestLevel())
                    {
                        nodes[curId]->setDestLevel(nodes[nextId]->getDestLevel());
                    }
                }
            }
        }
    }

    longPathThresholdLevel = 1;

    int thresholdLevelNum = nodes.size() * longPathThrRatio;
    int mediumThresholdLevelNum = nodes.size() * mediumPathThrRatio;
    int cntNodes = 0;
    std::string levelInfoStr = " details: ";
    for (unsigned int i = 0; i < forwardlevel2NodeIds.size(); i++)
    {
        if (cntNodes < thresholdLevelNum)
        {
            longPathThresholdLevel = i;
        }
        if (cntNodes < mediumThresholdLevelNum)
        {
            mediumPathThresholdLevel = i;
        }
        cntNodes += forwardlevel2NodeIds[i].size();
        levelInfoStr += std::to_string(i) + "(" + std::to_string(forwardlevel2NodeIds[i].size()) + ", " +
                        std::to_string((float)cntNodes / nodes.size()) + "), ";
    }

    for (unsigned int i = 0; i < nodes.size(); i++)
    {
        nodes[i]->sortInEdgesByForwardLevel();
    }

    print_info("PlacementTimingInfo: total level = " + std::to_string(forwardlevel2NodeIds.size()) + levelInfoStr);
    print_info("PlacementTimingInfo: long path threshold level = " + std::to_string(longPathThresholdLevel));
    print_info("PlacementTimingInfo: medium path threshold level = " + std::to_string(mediumPathThresholdLevel));
    print_status("PlacementTimingInfo: Timing graph finished forward levalization");
}

template <typename nodeType> void PlacementTimingInfo::TimingGraph<nodeType>::backwardLevelization()
{
    print_status("PlacementTimingInfo: Timing graph starts backward levalization");
    backwardlevel2NodeIds.clear();

    std::vector<int> curLevelIds;
    curLevelIds.clear();
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->checkIsRegister())
        {
            curLevelIds.push_back(i);
            nodes[i]->setBackwardLevel(0);
        }
    }

    int curLevel = 0;
    while (curLevelIds.size())
    {
        // print_info("level#" + std::to_string(curLevel) + " has " + std::to_string(curLevelIds.size()) + " nodes");
        backwardlevel2NodeIds.push_back(curLevelIds);

        curLevelIds.clear();

        int nextLevel = curLevel + 1;
        for (auto curId : backwardlevel2NodeIds[curLevel])
        {
            for (auto inEdge : nodes[curId]->getInEdges())
            {
                int nextId = inEdge->getSource()->getId();

                if (!nodes[nextId]->checkIsRegister())
                {
                    if (nodes[nextId]->getBackwardLevel() < nextLevel)
                    {
                        nodes[nextId]->setBackwardLevel(nextLevel);
                        curLevelIds.push_back(nextId);
                    }
                }
            }
        }
        curLevel++;
    }

    // some nodes are duplicated in some lower levels, remove them
    for (unsigned int level = 0; level < backwardlevel2NodeIds.size(); level++)
    {
        std::vector<int> filteredList;
        filteredList.clear();
        for (auto id : backwardlevel2NodeIds[level])
        {
            assert(nodes[id]->getBackwardLevel() >= 0);
            if ((unsigned int)nodes[id]->getBackwardLevel() == level)
                filteredList.push_back(id);
        }
        backwardlevel2NodeIds[level] = filteredList;
    }

    for (unsigned int level = 0; level < backwardlevel2NodeIds.size() - 1; level++)
    {
        for (auto curId : backwardlevel2NodeIds[level])
        {
            assert(curId < nodes.size());
            float curClockPeriod = nodes[curId]->getClockPeriod();
            if (curClockPeriod < 0)
            {
                continue;
            }
            for (auto inEdge : nodes[curId]->getInEdges())
            {
                int nextId = inEdge->getSource()->getId();

                assert(nextId < nodes.size());
                if (!nodes[nextId]->checkIsRegister())
                {
                    nodes[nextId]->setClockPeriod(curClockPeriod);
                }
            }
        }
    }

    for (unsigned int i = 0; i < nodes.size(); i++)
    {
        nodes[i]->sortOutEdgesByBackwardLevel();
    }

    print_status("PlacementTimingInfo: Timing graph finished backward levalization");
}

template <typename nodeType>
std::vector<int> PlacementTimingInfo::TimingGraph<nodeType>::traceBackFromNode(int targetId)
{
    std::vector<int> resPath;
    resPath.clear();
    resPath.push_back(targetId);

    int curId = targetId;
    while (nodes[curId]->getForwardLevel() != 0)
    {
        int maxLevelInLastId = -1;
        int cellInMaxLevelInLastId = -1;
        for (auto inEdge : nodes[curId]->getInEdges())
        {
            int lastId = inEdge->getSource()->getId();

            if (nodes[lastId]->getForwardLevel() > maxLevelInLastId && !nodes[lastId]->checkIsRegister())
            {
                maxLevelInLastId = nodes[lastId]->getForwardLevel();
                cellInMaxLevelInLastId = lastId;
            }
        }
        if (cellInMaxLevelInLastId < 0)
            break;
        curId = cellInMaxLevelInLastId;
        resPath.push_back(curId);
    }
    return resPath;
}

template <typename nodeType>
std::vector<int> PlacementTimingInfo::TimingGraph<nodeType>::traceForwardFromNode(int targetId)
{
    std::vector<int> resPath;
    resPath.clear();
    resPath.push_back(targetId);

    int targetPathLength = nodes[targetId]->getLongestPathLength();
    int curId = targetId;
    while (curId >= 0)
    {
        int maxLevelInLastId = -1;
        int cellInMaxLevelInLastId = -1;
        for (auto outEdge : nodes[curId]->getOutEdges())
        {
            int lastId = outEdge->getSink()->getId();
            if (nodes[lastId]->getBackwardLevel() > maxLevelInLastId &&
                nodes[lastId]->getLongestPathLength() >= targetPathLength && !nodes[lastId]->checkIsRegister())
            {
                maxLevelInLastId = nodes[lastId]->getBackwardLevel();
                cellInMaxLevelInLastId = lastId;
            }
        }
        if (cellInMaxLevelInLastId < 0)
            break;
        curId = cellInMaxLevelInLastId;
        resPath.push_back(curId);
    }
    return resPath;
}

template <typename nodeType>
std::vector<int> PlacementTimingInfo::TimingGraph<nodeType>::DFSFromNode(int startNodeId, int pathLenThr,
                                                                         unsigned int sizeThr,
                                                                         std::set<int> &exceptionCells, int fanoutThr)
{
    std::vector<int> resSucessors;
    std::stack<int> nodeStack;
    std::set<int> nodeSet;
    nodeSet.clear();
    resSucessors.clear();
    nodeSet.insert(startNodeId);
    resSucessors.push_back(startNodeId);
    nodeStack.push(startNodeId);
    int targetPathLen = nodes[startNodeId]->getLongestPathLength();

    if (nodes[startNodeId]->getForwardLevel() > targetPathLen * 0.2)
    {
        return resSucessors;
    }

    resSucessors.push_back(startNodeId);
    while (nodeStack.size() && nodeSet.size() < sizeThr)
    {
        int curNode = nodeStack.top();
        nodeStack.pop();

        for (auto outEdge : nodes[curNode]->getOutEdges())
        {
            int nextId = outEdge->getSink()->getId();

            if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
            {
                if (nodeSet.find(nextId) == nodeSet.end())
                {
                    resSucessors.push_back(nextId);
                    nodeSet.insert(nextId);
                    nodeStack.push(nextId);
                }
            }
        }

        for (auto inEdge : nodes[curNode]->getInEdges())
        {
            int nextId = inEdge->getSource()->getId();

            if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
            {
                if (nodeSet.find(nextId) == nodeSet.end())
                {
                    resSucessors.push_back(nextId);
                    nodeSet.insert(nextId);
                    nodeStack.push(nextId);
                }
            }
        }
    }
    return resSucessors;
}

template <typename nodeType>
std::vector<int> PlacementTimingInfo::TimingGraph<nodeType>::BFSFromNode(int startNodeId, int pathLenThr,
                                                                         unsigned int sizeThr,
                                                                         std::set<int> &exceptionCells)
{
    std::vector<int> resSucessors;
    std::queue<int> nodeQ;
    std::set<int> nodeSet;
    nodeSet.clear();
    resSucessors.clear();
    resSucessors.push_back(startNodeId);
    nodeSet.insert(startNodeId);
    nodeQ.push(startNodeId);
    // int targetPathLen = nodes[startNodeId]->getLongestPathLength();

    int targetPathLen = nodes[startNodeId]->getLongestPathLength();

    if (nodes[startNodeId]->getForwardLevel() > targetPathLen * 0.2)
    {
        return resSucessors;
    }

    while (nodeQ.size() && nodeSet.size() < sizeThr)
    {
        int curNode = nodeQ.front();
        nodeQ.pop();

        for (auto outEdge : nodes[curNode]->getOutEdges())
        {
            int nextId = outEdge->getSink()->getId();

            if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
            {
                if (nodeSet.find(nextId) == nodeSet.end() && exceptionCells.find(nextId) == exceptionCells.end())
                {
                    resSucessors.push_back(nextId);
                    nodeSet.insert(nextId);
                    nodeQ.push(nextId);
                }
            }
        }

        for (auto inEdge : nodes[curNode]->getInEdges())
        {
            int nextId = inEdge->getSource()->getId();

            if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
            {
                if (nodeSet.find(nextId) == nodeSet.end() && exceptionCells.find(nextId) == exceptionCells.end())
                {
                    resSucessors.push_back(nextId);
                    nodeSet.insert(nextId);
                    nodeQ.push(nextId);
                }
            }
        }
    }
    return resSucessors;
}

template <typename nodeType> void PlacementTimingInfo::TimingGraph<nodeType>::propogateArrivalTime()
{
    int nodeNum = nodes.size();
#pragma omp parallel for
    for (int j = 0; j < nodeNum; j++)
    {
        auto curNode = nodes[j];
        curNode->setLatestInputArrival(0.0);
        curNode->setLatestOutputArrival(0.0);
        curNode->setSlowestPredecessorId(-1);
    }
    for (unsigned int i = 1; i < forwardlevel2NodeIds.size(); i++)
    {
        int numNodeInLayer = forwardlevel2NodeIds[i].size();
#pragma omp parallel for
        for (int j = 0; j < numNodeInLayer; j++)
        {
            auto curNodeId = forwardlevel2NodeIds[i][j];
            auto curNode = nodes[curNodeId];
            for (auto inEdge : curNode->getInEdges())
            {
                int predId = inEdge->getSource()->getId();
                float predDelay = inEdge->getSource()->getLatestInputArrival();
                float edgeDelay = inEdge->getDelay();
                float newDelay;

                if (inEdge->getSource()->getInnerDelay() < 1.0 || inEdge->getSource()->getForwardLevel() > 0)
                    newDelay = predDelay + edgeDelay + inEdge->getSource()->getInnerDelay();
                else
                    newDelay = predDelay + edgeDelay;
                if (newDelay > curNode->getLatestInputArrival())
                {
                    curNode->setLatestInputArrival(newDelay);
                    curNode->setSlowestPredecessorId(predId);
                    // curNode->setLatestOutputArrival(newDelay + curNode->getInnerDelay());
                    curNode->setLatestOutputArrival(newDelay);
                }
            }
        }
    }

    int numNodeInLayer = forwardlevel2NodeIds[0].size();
#pragma omp parallel for
    for (int j = 0; j < numNodeInLayer; j++)
    {
        auto curNodeId = forwardlevel2NodeIds[0][j];
        assert(curNodeId < nodes.size());
        auto curNode = nodes[curNodeId];
        if (curNode->getDesignNode()->isVirtualCell())
            continue;

        for (auto inEdge : curNode->getInEdges())
        {
            if (inEdge->getSource())
            {
                if (inEdge->getSource()->getForwardLevel() > 0)
                {
                    int predId = inEdge->getSource()->getId();
                    float predDelay = inEdge->getSource()->getLatestInputArrival();
                    float edgeDelay = inEdge->getDelay();
                    float newDelay = predDelay + edgeDelay + inEdge->getSource()->getInnerDelay();
                    if (newDelay > curNode->getLatestInputArrival())
                    {
                        curNode->setLatestInputArrival(newDelay);
                        curNode->setSlowestPredecessorId(predId);
                    }
                }
            }
        }
    }
}

template <typename nodeType> void PlacementTimingInfo::TimingGraph<nodeType>::backPropogateRequiredArrivalTime()
{
    int nodeNum = nodes.size();
#pragma omp parallel for
    for (int j = 0; j < nodeNum; j++)
    {
        auto curNode = nodes[j];
        curNode->setInitialRequiredArrivalTime(clockPeriod);
        curNode->setEarlestSuccessorId(-1);
    }
    for (unsigned int i = 1; i < backwardlevel2NodeIds.size(); i++)
    {
        int numNodeInLayer = backwardlevel2NodeIds[i].size();
#pragma omp parallel for
        for (int j = 0; j < numNodeInLayer; j++)
        {
            auto curNodeId = backwardlevel2NodeIds[i][j];
            auto curNode = nodes[curNodeId];
            for (auto outEdge : curNode->getOutEdges())
            {
                int succId = outEdge->getSink()->getId();
                float succRequiredArrival = outEdge->getSink()->getRequiredArrivalTime();
                float edgeDelay = outEdge->getDelay();
                float newRequiredArrival = succRequiredArrival - edgeDelay - outEdge->getSink()->getInnerDelay();
                if (newRequiredArrival < curNode->getRequiredArrivalTime())
                {
                    curNode->setRequiredArrivalTime(newRequiredArrival);
                    curNode->setEarlestSuccessorId(succId);
                }
            }
        }
    }
}

template <typename nodeType>
std::vector<int> PlacementTimingInfo::TimingGraph<nodeType>::backTraceDelayLongestPathFromNode(int curNodeId)
{
    int slowestPredecessorId = curNodeId;
    std::vector<int> resPath;
    resPath.clear();
    resPath.push_back(slowestPredecessorId);
    while (slowestPredecessorId != -1)
    {
        slowestPredecessorId = nodes[slowestPredecessorId]->getSlowestPredecessorId();
        resPath.push_back(slowestPredecessorId);

        if (slowestPredecessorId == -1 || nodes[slowestPredecessorId]->getForwardLevel() == 0)
            break;
    }
    return resPath;
}

template <typename nodeType>
bool PlacementTimingInfo::TimingGraph<nodeType>::backTraceDelayLongestPathFromNode(int curNodeId,
                                                                                   std::vector<int> &isCovered,
                                                                                   std::vector<int> &resPath,
                                                                                   int converThr)
{
    int slowestPredecessorId = curNodeId;
    resPath.clear();
    resPath.push_back(slowestPredecessorId);
    while (slowestPredecessorId != -1)
    {
        slowestPredecessorId = nodes[slowestPredecessorId]->getSlowestPredecessorId();
        resPath.push_back(slowestPredecessorId);

        if (isCovered[slowestPredecessorId] > 30 && nodes[slowestPredecessorId]->getForwardLevel() > 5 &&
            nodes[slowestPredecessorId]->getOutEdges().size() > 1)
        {
            // if (nodes[slowestPredecessorId]->getDesignNode()->isDSP() ||
            //     nodes[slowestPredecessorId]->getDesignNode()->isBRAM() ||
            //     nodes[slowestPredecessorId]->getDesignNode()->isCarry())
            // {
            //     if (isCovered[slowestPredecessorId] > 60)
            //         return false;
            // }
            // else
            {
                return false;
            }
        }
        if (slowestPredecessorId == -1 || nodes[slowestPredecessorId]->getForwardLevel() == 0)
            break;
    }
    return true;
}

template class PlacementTimingInfo::TimingGraph<DesignInfo::DesignCell>;