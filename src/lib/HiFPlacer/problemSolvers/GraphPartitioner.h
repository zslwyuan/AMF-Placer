#ifndef _GRAPHPARTITIONER
#define _GRAPHPARTITIONER

#include "PaToH/patoh.h"
#include "PlacementInfo.h"
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

#define MULTIPROCESS_PARITION

template <class NodeList, class NetList> class GraphPartitioner
{
  public:
    GraphPartitioner(NodeList &nodeList, NetList &netList, int minClusterCellNum, int jobs, bool verbose)
        : nodeList(nodeList), netList(netList), minClusterCellNum(minClusterCellNum), verbose(verbose)
    {
        sem_init(&partitionSem, 0, 0);
        for (int tid = 0; tid < jobs; tid++)
            sem_post(&partitionSem);
    }

    ~GraphPartitioner()
    {
    }

    void solve(int eachClusterDSPNum, int eachClusterBRAMNum);

    inline const std::vector<std::vector<int>> &getClusters()
    {
        return clusters;
    }

    inline const std::vector<std::set<int>> &getClustersPUIdSets()
    {
        ClusterPUIdSets.clear();
        int i = 0;
        for (auto &tmpCluster : clusters)
        {
            ClusterPUIdSets.push_back(std::set<int>());
            for (auto id : tmpCluster)
                ClusterPUIdSets[i].insert(id);
            i++;
        }
        return ClusterPUIdSets;
    }

    static void recursiveMinCutPartition(std::vector<int> &inputCluster,
                                         GraphPartitioner<NodeList, NetList> *graphPartitioner, int eachClusterDSPNum,
                                         int eachClusterBRAMNum);
    unsigned minCutBipartition(const std::vector<int> &inputCluster, std::array<std::vector<int>, 2> &outputClusters,
                               GraphPartitioner<NodeList, NetList> *graphPartitioner, int eachClusterDSPNum,
                               int eachClusterBRAMNum);

    void setMaxCutRate(double _maxCutRate)
    {
        maxCutRate = _maxCutRate;
    }

  private:
    std::vector<std::vector<int>> clusters;
    std::vector<std::set<int>> ClusterPUIdSets;
    sem_t partitionSem;
    std::mutex clustersLock, cntLock;

    double maxCutRate = 0.0333;

    NodeList &nodeList;
    NetList &netList;
    unsigned int minClusterCellNum;

    void sortClustersBySize();

    bool verbose;
};

#ifdef MULTIPROCESS_PARITION
#include "ExternalProcessFunc.h"
#endif

#endif