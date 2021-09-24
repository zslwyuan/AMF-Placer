#ifndef _MINCostFlow
#define _MINCostFlow

#include "strPrint.h"
#include <algorithm>
#include <vector>
#include <queue>
#include <assert.h>

/**
 * @brief Min-Cost Flow algorithm for simple bipartite graph
 *
 */
class MinCostFlow
{
  public:
    /**
     * @brief An edge is represented as a struct
     *      destination   -  denotes the ending node of an edge. For example, 'v' in u-->v
     *      capacity      -  the maximum capacity of an edge
     *      residualFlow  -  the residual amount of flow that can flow through the edge
     *      counterEdge   -  a pointer to the counter edge in residual graph for performance optimization
     */
    struct Edge
    {
        int destination;
        int capacity;
        int residualFlow;
        float cost;
        Edge *counterEdge;
    };

    /**
     * @brief A graph is represented as a struct
     *
     * numVertices - denotes the number of vertices in the graph
     * adj         - Adjacency list : Collection of unordered lists one for each vertex
     */
    struct Graph
    {
        int numVertices;
        std::vector<Edge *> *adj;
    };

    MinCostFlow(int numVertices, int flow, int source, int destination)
        : numVertices(numVertices), flow(flow), source(source), destination(destination)
    {

        // Initialize the graphs
        resGraph.numVertices = numVertices;
        resGraph.adj = new std::vector<Edge *>[numVertices];
    }

    ~MinCostFlow()
    {
        for (int u = 0; u < numVertices; u++)
        {
            for (unsigned int e = 0; e < resGraph.adj[u].size(); e++)
            {
                delete resGraph.adj[u][e];
            }
        }
        delete[] resGraph.adj;
    }

    void addEdge(int tu, int tv, int tcap, float tcost)
    {
        Edge *tmpEdge1 = genEdge(tv, tcap, tcap, tcost + 0.01); // avoid edge cycle
        Edge *tmpEdge2 = genEdge(tu, tcap, 0, -tcost);
        tmpEdge1->counterEdge = tmpEdge2;
        tmpEdge2->counterEdge = tmpEdge1;
        resGraph.adj[tu].push_back(tmpEdge1);
        resGraph.adj[tv].push_back(tmpEdge2);
    }

    /**
     * @brief Calculates the cost of flow 'requiredFlow' from 's' to 't'
     *
     * @param s source
     * @param t sink
     * @param requiredFlow
     * @return float Returns 'MAX_VAL' if such a flow is not possible
     */
    float calcMinCostFlow(int s, int t, int requiredFlow)
    {
        // int u = -1;
        int v = -1, currFlow = 0;
        float runningCost = 0;
        Edge *te1, *te2;

        // Run shortest path augmentation
        int parent[resGraph.numVertices];
        Edge *parentEdge[resGraph.numVertices];
        int stepProgress = requiredFlow / 100;
        if (stepProgress < 1)
            stepProgress = 1;
        while (SPFA(resGraph, s, t, parent, parentEdge))
        {
            int path_flow = MAX_VAL;
            for (v = t; v != s; v = parent[v])
            {
                assert(v >= 0);
                assert(v < numVertices);
                assert(v != parent[v]);
                // u = parent[v];
                te1 = parentEdge[v];
                path_flow = std::min(path_flow, te1->residualFlow);
            }
            path_flow = std::min(path_flow, requiredFlow - currFlow);
            for (v = t; v != s; v = parent[v])
            {
                // u = parent[v];
                te1 = parentEdge[v];
                te2 = te1->counterEdge;
                te1->residualFlow -= path_flow;
                te2->residualFlow += path_flow;
                runningCost += path_flow * (te1->cost);
            }
            currFlow += path_flow;
            assert(path_flow > 0);
            // if (currFlow % (5 * stepProgress) == 0)
            // {
            //     printProgress((double)currFlow / requiredFlow);
            //     printf(" path_flow: %d", path_flow);
            // }
            if (currFlow == requiredFlow)
            {
                break;
            }
        }
        if (currFlow == requiredFlow)
        {
            return runningCost;
        }
        else
        {
            return MAX_VAL;
        }
    }

    Graph resGraph;

  private:
    int numVertices, flow, source, destination;
    int MAX_VAL = 200000000;

    float eps = 1e-5;

    // Generates a new edge (allocating space dynamically) and returns a pointed to the edge
    Edge *genEdge(int destination, int capacity, int residualFlow, float cost)
    {
        Edge *e1 = new Edge;
        e1->destination = destination;
        e1->capacity = capacity;
        e1->residualFlow = residualFlow;
        e1->cost = cost;
        return e1;
    }

    /**
     * @brief Finds the shortest path from source to sink
     *
     * @param resGraph input graph
     * @param source source node
     * @param sink sink node
     * @param parentVertex parentVertex and parentEdge are updated and can be used to reconstruct the shortest path
     * @param parentEdge parentVertex and parentEdge are updated and can be used to reconstruct the shortest path
     * @return true  if there is a path from source to sink
     * @return false if no path exists from source to sink
     */
    bool SPFA(Graph resGraph, int source, int sink, int parentVertex[], Edge *parentEdge[])
    {
        // Initialize variables that will be needed
        int numVertices = resGraph.numVertices;
        std::vector<Edge *> *adj = resGraph.adj;
        float distance[numVertices];
        int inQueueCnt[numVertices];
        // Initialize visited, parentVertex and distance
        for (int i = 0; i < numVertices; i++)
        {
            parentVertex[i] = -1;
            distance[i] = MAX_VAL;
            inQueueCnt[i] = 0;
        }

        distance[source] = 0;
        std::queue<int> nodeQ;
        std::vector<bool> nodesInQ;
        nodesInQ.resize(numVertices, false);
        while (!nodeQ.empty())
            nodeQ.pop();

        nodeQ.push(source);
        nodesInQ[source] = true;
        inQueueCnt[source] = 1;

        while (!nodeQ.empty())
        {
            int u = nodeQ.front();
            nodeQ.pop();
            int numEdge = adj[u].size();
            //#pragma omp parallel for
            for (int e = 0; e < numEdge; e++)
            {
                if (adj[u][e]->residualFlow > 0)
                {
                    int v = adj[u][e]->destination;
                    float w = adj[u][e]->cost;
                    if (distance[v] > distance[u] + w + eps)
                    {
                        distance[v] = distance[u] + w;
                        parentVertex[v] = u;
                        parentEdge[v] = adj[u][e];
                        //#pragma omp critical
                        {
                            if (!nodesInQ[v])
                            {
                                nodesInQ[v] = true;
                                nodeQ.push(v);
                                inQueueCnt[v]++;
                                assert(inQueueCnt[v] <= numVertices);
                            }
                        }
                        assert(distance[v] > -0.00001);
                    }
                }
            }
            nodesInQ[u] = false;
        }

        if (parentVertex[sink] == -1)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

#endif