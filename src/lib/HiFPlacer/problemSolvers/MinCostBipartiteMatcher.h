/**
 * @file MinCostBipartiteMatcher.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
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

#ifndef _MinCostBipartiteMatcher
#define _MinCostBipartiteMatcher

#include "PlacementInfo.h"
#include "minCostFlow/MinCostFlow.h"
#include "sysInfo.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class MinCostBipartiteMatcher
{
  public:
    MinCostBipartiteMatcher(int numLeftNodes, int numRightNodes, int numExpectedMatches,
                            std::vector<std::vector<std::pair<int, float>>> &adjList, int maxThreadNum, bool verbose)
        : numLeftNodes(numLeftNodes), numRightNodes(numRightNodes), numExpectedMatches(numExpectedMatches),
          adjList(adjList), maxThreadNum(maxThreadNum), verbose(verbose)
    {

        std::vector<int> leftId2ConnectedSubgraphId(0);
        leftId2ConnectedSubgraphId.resize(numLeftNodes, -1);
        int numConnectedSubgraphs = -1;
        getConnectedSubgraphAdjList(adjList, leftId2ConnectedSubgraphId, numConnectedSubgraphs, maxThreadNum);
        if (maxThreadNum < numConnectedSubgraphs)
            threadNum = maxThreadNum;
        else
            threadNum = numConnectedSubgraphs;
        minCostFlowSolvers.clear();
        minCostFlowSolvers.resize(threadNum, nullptr);

        int totalNumEdges = 0;
        for (auto &curList : adjList)
            totalNumEdges += curList.size();
        totalNumEdges += numLeftNodes + numRightNodes;

        srcNode = numLeftNodes + numRightNodes;
        sinkNode = srcNode + 1;
        for (int i = 0; i < threadNum; i++)
            minCostFlowSolvers[i] = new MinCostFlow(numLeftNodes + numRightNodes + 2, numLeftNodes, srcNode, sinkNode);

        if (verbose)
        {
            print_info("#ConnectedSubgraphs: " + std::to_string(numConnectedSubgraphs));
            print_info("#leftNodes: " + std::to_string(numLeftNodes));
            print_info("#rightNodes: " + std::to_string(numRightNodes));
        }

        assert(adjList.size() == (unsigned int)numLeftNodes);
        for (int i = 0; i < numLeftNodes; i++)
        {
            for (unsigned int j = 0; j < adjList[i].size(); j++)
            {
                assert(leftId2ConnectedSubgraphId[i] >= 0);
                int v = adjList[i][j].first + numLeftNodes;
                float w = adjList[i][j].second;
                assert(w > 0.00001);
                minCostFlowSolvers[leftId2ConnectedSubgraphId[i]]->addEdge(i, v, 1, w);
            }
        }

        for (int solverId = 0; solverId < threadNum; solverId++)
        {
            for (int i = 0; i < numLeftNodes; i++)
            {
                minCostFlowSolvers[solverId]->addEdge(srcNode, i, 1, 0);
            }
            for (int i = numLeftNodes; i < numLeftNodes + numRightNodes; i++)
            {
                minCostFlowSolvers[solverId]->addEdge(i, sinkNode, 1, 0);
            }
        }

        left2right.clear();
        right2left.clear();
        left2right.resize(numLeftNodes, -1);
        right2left.resize(numRightNodes, -1);
    }

    ~MinCostBipartiteMatcher()
    {
        for (auto minCostFlowSolver : minCostFlowSolvers)
            delete minCostFlowSolver;
    }

    void solve();

    inline int getMatchedRightNode(int x)
    {
        return left2right[x];
    }

  private:
    int numLeftNodes;
    int numRightNodes;
    int numExpectedMatches;
    std::vector<std::vector<std::pair<int, float>>> &adjList;
    std::vector<std::vector<std::vector<std::pair<int, float>>>> connectedSubgraphAdjList;
    void getConnectedSubgraphAdjList(std::vector<std::vector<std::pair<int, float>>> &adjList,
                                     std::vector<int> &leftId2ConnectedSubgraphId, int &numConnectedSubgraphs,
                                     int maxThreadNum);
    int maxThreadNum;
    int threadNum = 1;
    bool verbose;

    std::vector<MinCostFlow *> minCostFlowSolvers;
    int srcNode;
    int sinkNode;
    std::vector<int> left2right;
    std::vector<int> right2left;
};

#endif