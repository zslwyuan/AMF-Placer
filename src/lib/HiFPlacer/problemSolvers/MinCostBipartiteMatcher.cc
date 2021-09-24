#include "MinCostBipartiteMatcher.h"
#include <omp.h>

void MinCostBipartiteMatcher::solve()
{
    int numSolvers = minCostFlowSolvers.size();
#pragma omp parallel for schedule(dynamic)
    for (int solverId = 0; solverId < numSolvers; solverId++)
    {
        minCostFlowSolvers[solverId]->calcMinCostFlow(srcNode, sinkNode, numExpectedMatches);
        for (int i = 0; i < numLeftNodes; i++)
        {
            for (unsigned int j = 0; j < minCostFlowSolvers[solverId]->resGraph.adj[i].size(); j++)
            {
                int destination = minCostFlowSolvers[solverId]->resGraph.adj[i][j]->destination;
                if (destination >= numLeftNodes && destination < numLeftNodes + numRightNodes)
                {
                    if (minCostFlowSolvers[solverId]->resGraph.adj[i][j]->residualFlow == 0)
                    {
                        assert(left2right[i] < 0);
                        left2right[i] = destination - numLeftNodes;
                        assert(right2left[destination - numLeftNodes] < 0);
                        right2left[destination - numLeftNodes] = i;
                    }
                }
            }
        }
    }
}

void MinCostBipartiteMatcher::getConnectedSubgraphAdjList(std::vector<std::vector<std::pair<int, float>>> &adjList,
                                                          std::vector<int> &leftId2ConnectedSubgraphId,
                                                          int &numConnectedSubgraphs, int maxThreadNum)
{
    std::vector<std::vector<int>> inv_adjList;
    inv_adjList.resize(numRightNodes, std::vector<int>());
    for (int i = 0; i < numLeftNodes; i++)
    {
        for (unsigned int j = 0; j < adjList[i].size(); j++)
        {
            unsigned int v = adjList[i][j].first;
            assert(v < inv_adjList.size());
            inv_adjList[v].push_back(i);
        }
    }

    numConnectedSubgraphs = 0;
    for (unsigned int leftNodeId = 0; leftNodeId < adjList.size(); leftNodeId++)
    {
        if (leftId2ConnectedSubgraphId[leftNodeId] >= 0)
            continue;
        int curNode = leftNodeId;

        bool reachedRight[numRightNodes];
        memset(reachedRight, 0, sizeof(reachedRight));
        std::queue<int> leftNodeInQ;
        leftNodeInQ.push(curNode);
        leftId2ConnectedSubgraphId[curNode] = numConnectedSubgraphs % maxThreadNum;
        while (leftNodeInQ.size())
        {
            curNode = leftNodeInQ.front();
            leftNodeInQ.pop();
            for (auto tmpPair : adjList[curNode])
            {
                int rightNode = tmpPair.first;
                if (reachedRight[rightNode])
                    continue;
                reachedRight[rightNode] = 1;
                for (int nextLeftId : inv_adjList[rightNode])
                {
                    if (leftId2ConnectedSubgraphId[nextLeftId] < 0)
                    {
                        leftId2ConnectedSubgraphId[nextLeftId] = numConnectedSubgraphs % maxThreadNum;
                        leftNodeInQ.push(nextLeftId);
                    }
                }
            }
        }

        numConnectedSubgraphs++;
    }

    // print_info("MinCostBipartiteMatcher finds " + std::to_string(numConnectedSubgraphs) +
    //            " connected subgraphs in the input bipartie graph");
}