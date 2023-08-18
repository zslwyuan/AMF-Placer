/**
 * @file GraphPartitioner.h
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

#ifndef _GRAPHPARTITIONER
#define _GRAPHPARTITIONER

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

#include "PlacementInfo.h"
#include "sysInfo.h"
#include "ProcessFuncInterface.h"
#include "ExternalProcessFunc.h"
#include "PaToH/patoh.h"

#define MULTIPROCESS_PARITION

/**
 * @brief GraphPartitioner will recursively bi-partition the netlist (which could be netlist of clusters) based on
 * connectivitity and the net weights
 *
 * @tparam NodeList given node list type
 * @tparam NetList given net list type
 */
template <class NodeList, class NetList> class GraphPartitioner
{
  public:
    /**
     * @brief Construct a new Graph Partitioner object
     *
     * @param nodeList the node list (it should be a vector)
     * @param netList the net list (it should be a vector)
     * @param minClusterCellNum a constraint to set the minimum size of the cluster
     * @param jobs the number of parallel partitioning workers
     * @param verbose whether dumps detailed information
     */
    GraphPartitioner(NodeList nodeList, NetList netList, int minClusterCellNum, int jobs, bool verbose)
        : nodeList(nodeList), netList(netList), minClusterCellNum(minClusterCellNum), verbose(verbose)
    {
        sem_init(&partitionSem, 0, 0);
        for (int tid = 0; tid < jobs; tid++)
            sem_post(&partitionSem);
    }

    ExternalProcessFunc* partitionHyperGraphByShareMem(const std::vector<int> &inputCluster,
                        GraphPartitioner<NodeList, NetList> *graphPartitioner, const double& final_imbal, 
                        int& numHyperNodes, int& numPlacementNets, int& numPlacementPins, 
                        int* &partvec, int* &partweights, int* &cut);
    ExternalProcessFunc * 
    partitionHyperGraph(const std::vector<int> &inputCluster,
                        GraphPartitioner<NodeList, NetList> *graphPartitioner, const double& final_imbal, 
                        int& numHyperNodes, int& numPlacementNets, int& numPlacementPins, 
                        int* &partvec, int* &partweights, int* &cut);

     
    //int* 
    void partitionHyperGraph(const std::vector<int> &inputCluster,
                        GraphPartitioner<NodeList, NetList> *graphPartitioner, const double& final_imbal, 
                        int& numHyperNodes, int& numPlacementNets, int& numPlacementPins, 
                        int* &partvec, int* &partweights, int* &cut, int* &ptr);

    ~GraphPartitioner()
    {
    }

    /**
     * @brief a caller which will start the partitioning procedure
     *
     * @param eachClusterDSPNum  a constraint to limit the maximum of the DSP in a cluster
     * @param eachClusterBRAMNum a constraint to limit the maximum of the BRAM in a cluster
     */
    void solve(int eachClusterDSPNum, int eachClusterBRAMNum);

    /**
     * @brief Get the clusters after partitioning (each is a vector containing ids for nodes inside the clusters)
     *
     * @return const std::vector<std::vector<int>>&
     */
    inline const std::vector<std::vector<int>> &getClusters()
    {
        return clusters;
    }

    /**
     * @brief Get the clusters after partitioning (each is a set containing ids for nodes inside the clusters)
     *
     * @return const std::vector<std::set<int>>&
     */
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

    /**
     * @brief a recursive function which will recursively bi-partition the input cluster into two based on connectivity
     * and some resouce constraints
     *
     * @param inputCluster a given input cluster
     * @param graphPartitioner the partitioning worker (graphPartitioner) to handle this partitioning task (since this
     * is a static function, we have to pass the worker as arguments to conduct partitioning)
     * @param eachClusterDSPNum  a constraint to limit the maximum of the DSP in a cluster
     * @param eachClusterBRAMNum a constraint to limit the maximum of the BRAM in a cluster
     */
    static void recursiveMinCutPartition(std::vector<int> &inputCluster,
                                         GraphPartitioner<NodeList, NetList> *graphPartitioner, int eachClusterDSPNum,
                                         int eachClusterBRAMNum);

    /**
     * @brief the actual function to call partitioner to conduct partitioning with given clusters and parameters
     *
     * @param inputCluster a given input cluster
     * @param outputClusters the resultant two clusters
     * @param graphPartitioner the partitioning worker (graphPartitioner) to handle this partitioning task (since this
     * is a static function, we have to pass the worker as arguments to conduct partitioning)
     * @param eachClusterDSPNum  a constraint to limit the maximum of the DSP in a cluster
     * @param eachClusterBRAMNum a constraint to limit the maximum of the BRAM in a cluster
     * @return unsigned
     */
    unsigned minCutBipartition(const std::vector<int> &inputCluster, std::array<std::vector<int>, 2> &outputClusters,
                               GraphPartitioner<NodeList, NetList> *graphPartitioner, int eachClusterDSPNum,
                               int eachClusterBRAMNum);

    /**
     * @brief Set the max mincut rate
     *
     * if the partitioning lead to a high mincut rate, the partitioning should be canceled.
     *
     * @param _maxCutRate
     */
    void setMaxCutRate(double _maxCutRate)
    {
        maxCutRate = _maxCutRate;
    }

  private:
    /**
     * @brief the resultant clusters (vectors) after partitioning
     *
     */
    std::vector<std::vector<int>> clusters;

    /**
     * @brief the resultant clusters (set) after partitioning
     *
     */
    std::vector<std::set<int>> ClusterPUIdSets;

    /**
     * @brief used to limit the number of processes for the parallel partitioning
     *
     */
    sem_t partitionSem;
    std::mutex clustersLock, cntLock;

    double maxCutRate = 0.0333;

    NodeList nodeList;
    NetList netList;
    unsigned int minClusterCellNum;

    /**
     * @brief sort clusters by sizes to fix the output clusters' order for later processing and avoid the random factor
     * due to the multi-process procedure
     *
     */
    void sortClustersBySize();

    bool verbose;
};

#ifdef MULTIPROCESS_PARITION
#include "ExternalProcessFunc.h"
#endif

#endif