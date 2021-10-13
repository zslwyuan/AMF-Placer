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
 */

#include "PlacementTimingInfo.h"
#include "PlacementInfo.h"

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <queue>
#include <set>
#include <stack>

PlacementTimingInfo::PlacementTimingInfo(DesignInfo *designInfo, DeviceInfo *deviceInfo,
                                         std::map<std::string, std::string> &JSONCfg)
    : designInfo(designInfo), deviceInfo(deviceInfo), JSONCfg(JSONCfg)
{
    if (JSONCfg.find("PlacementTimingInfoVerbose") != JSONCfg.end())
        verbose = JSONCfg["PlacementTimingInfoVerbose"] == "true";
}

void PlacementTimingInfo::buildSimpleTimingGraph()
{
    print_status("PlacementTimingInfo: building simple timing graph (TimingNode is DesignCell)");
    simpleTimingGraph = new TimingGraph<DesignInfo::DesignCell>();
    for (auto curCell : designInfo->getCells())
    {
        auto newNode =
            new TimingGraph<DesignInfo::DesignCell>::TimingNode(curCell, simpleTimingGraph->getNodes().size());

        if (curCell->isTimingEndPoint())
        {
            newNode->setIsRegister();
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
            nodeInPath.clear();
            nodeInPath.push_back(curLevelIds[0]);
            findALoopFromNode(nodeInPath, curLevelIds[0], curLevelIds[0], 0);
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
                                                                         std::set<int> &exceptionCells)
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

    bool forwarding = true;
    while (nodeStack.size() && nodeSet.size() < sizeThr)
    {
        int curNode = nodeStack.top();
        resSucessors.push_back(curNode);
        nodeStack.pop();

        if (forwarding)
        {
            bool findNext = false;
            for (auto outEdge : nodes[curNode]->getOutEdges())
            {
                int nextId = outEdge->getSink()->getId();

                if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
                {
                    if (nodeSet.find(nextId) == nodeSet.end() && exceptionCells.find(nextId) == exceptionCells.end())
                    {
                        nodeSet.insert(nextId);
                        nodeStack.push(nextId);
                        findNext = true;
                    }
                }
            }
            if (!findNext)
            {
                forwarding = false;
                return resSucessors;
            }
        }

        if (!forwarding)
        {
            bool findNext = false;
            for (auto inEdge : nodes[curNode]->getInEdges())
            {
                int nextId = inEdge->getSource()->getId();

                if (!nodes[nextId]->checkIsRegister() && nodes[nextId]->getLongestPathLength() > pathLenThr)
                {
                    if (nodeSet.find(nextId) == nodeSet.end() && exceptionCells.find(nextId) == exceptionCells.end())
                    {
                        nodeSet.insert(nextId);
                        nodeStack.push(nextId);
                        findNext = true;
                    }
                }
            }
            if (!findNext)
                forwarding = true;
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

    bool forwarding = true;
    while (nodeQ.size() && nodeSet.size() < sizeThr)
    {
        int curNode = nodeQ.front();
        nodeQ.pop();

        if (forwarding)
        {
            bool findNext = false;
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
                        findNext = true;
                    }
                }
            }
            if (!findNext)
                forwarding = false;
        }

        if (!forwarding)
        {
            bool findNext = false;
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
                        findNext = true;
                    }
                }
            }
            if (!findNext)
                forwarding = true;
        }
    }
    return resSucessors;
}

template <typename nodeType> void PlacementTimingInfo::TimingGraph<nodeType>::propogateArrivalTime()
{
    // for (unsigned int i = 1; i < forwardlevel2NodeIds.size(); i++)
    // {
    //     for (auto curNode : forwardlevel2NodeIds[i])
    //     {
    //     }
    // }
}

template class PlacementTimingInfo::TimingGraph<DesignInfo::DesignCell>;