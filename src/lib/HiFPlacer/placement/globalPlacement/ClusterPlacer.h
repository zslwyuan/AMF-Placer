/**
 * @file ClusterPlacer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of ClusterPlacer class and its internal modules and APIs which
 * cluster nodes in the given netlist and place the clusters on the device based on simulated-annealing as initial
 * placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _CLUSTERPLACER
#define _CLUSTERPLACER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "GraphPartitioner.h"
#include "PaToH/patoh.h"
#include "PlacementInfo.h"
#include "SAPlacer.h"
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

/**
 * @brief ClusterPlacer will cluster nodes in the given netlist and place the clusters on the device based on
 * simulated-annealing as initial placement.
 *
 */
class ClusterPlacer
{
  public:
    /**
     * @brief Construct a new Cluster Placer object
     *
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg  the user-defined placement configuration
     * @param connectionToFixedFactor the enhancement ratio of the net between elements and I/Os
     */
    ClusterPlacer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                  float connectionToFixedFactor = 5.0);
    ~ClusterPlacer()
    {
        if (saPlacer)
            delete saPlacer;
        if (basicGraphPartitioner)
            delete basicGraphPartitioner;
        if (userDefinedClusterBasedGraphPartitioner)
            delete userDefinedClusterBasedGraphPartitioner;
        for (auto tmpClusterUnit : clusterUnits)
        {
            delete tmpClusterUnit;
        }
        for (auto tmpClusterNet : clusterNets)
        {
            delete tmpClusterNet;
        }
    }

    /**
     * @brief conduct cluster placement, cluster nodes in the given netlist and place the clusters on the device as
     * initial placement.
     *
     */
    void ClusterPlacement();

  private:
    /**
     * @brief the PlacementInfo for this placer to handle
     *
     */
    PlacementInfo *placementInfo;

    /**
     * @brief the resultant clusters of PlacementUnit (id)
     *
     * a vector of the sets of PlacementUnit Id ==> clusters of PlacementUnit id
     *
     */
    std::vector<std::set<int>> clusters;
    std::map<std::string, std::string> &JSONCfg;

    /**
     * @brief the enhancement ratio of the net between elements and I/Os
     *
     */
    float connectionToFixedFactor;

    /**
     * @brief the Y/X coordinate calibration factor
     *
     */
    float y2xRatio = 1.0;
    bool verbose;

    bool randomInitialPlacement = false;

    /**
     * @brief the requirement of the average size of clusters (the number of cells)
     *
     */
    int avgClusterSizeRequirement = 40000;

    /**
     * @brief the maximum cut rate for netlist min-cut partitioning
     *
     */
    double maxMinCutRate = 0.0333;

    /**
     * @brief mapping cluster id to X/Y location in the cluster bin
     *
     */
    std::vector<std::pair<int, int>> cluster2XY;

    /**
     * @brief mapping cluster id to floating-point X/Y location on device
     *
     */
    std::vector<std::pair<float, float>> cluster2FP_XY;

    //  the cluster-level netlist information

    std::vector<std::vector<float>> clusterAdjMat;
    std::vector<float> clusterCLBCellWeights;
    std::vector<float> clusterDSPCellWeights;
    std::vector<float> clusterBRAMCellWeights;

    std::vector<std::vector<float>> cluster2FixedUnitMat;
    std::vector<float> fixedX;
    std::vector<float> fixedY;

    std::vector<int> placementUnit2ClusterId;

    std::vector<int> placementUnitId2ClusterUnitId;
    std::vector<PlacementInfo::ClusterUnit *> clusterUnits;
    std::vector<PlacementInfo::ClusterNet *> clusterNets;

    /**
     * @brief the parallel (multi-threading) worker number
     *
     */
    int jobs;

    /**
     * @brief simulated-annealing placer for the cluster placement.
     *
     * It will map the clusters to different cluster bins and try to minimize the rough total wirelength between the
     * clusters.
     *
     */
    SAPlacer *saPlacer = nullptr;

    /**
     * @brief basicGraphPartitioner will conduct partitioning at the PlacementUnit level considering connectivity and
     * resource demand
     *
     */
    GraphPartitioner<std::vector<PlacementInfo::PlacementUnit *>, std::vector<PlacementInfo::PlacementNet *>>
        *basicGraphPartitioner = nullptr;

    /**
     * @brief userDefinedClusterBasedGraphPartitioner will conduct partitioning at the Cluster level considering
     * connectivity and resource demand. The input clusters are obtained by user-defined information
     *
     */
    GraphPartitioner<std::vector<PlacementInfo::ClusterUnit *>, std::vector<PlacementInfo::ClusterNet *>>
        *userDefinedClusterBasedGraphPartitioner = nullptr;

    /**
     * @brief clockBasedGraphPartitioner will conduct partitioning at the Cluster level considering connectivity and
     * resource demand. The input clusters are obtained by clock domain.
     *
     */
    GraphPartitioner<std::vector<PlacementInfo::ClusterUnit *>, std::vector<PlacementInfo::ClusterNet *>>
        *clockBasedGraphPartitioner = nullptr;

    /**
     * @brief clean all information of the clusters (including PlacementUnit-Cluster mapping and the cluster-level
     * netlist)
     *
     */
    void resetClusterInfo()
    {
        placementUnitId2ClusterUnitId = std::vector<int>(placementInfo->getPlacementUnits().size(), -1);
        clusterUnits.clear();
        clusterNets.clear();
    }

    /**
     * @brief construct the netlist of the generated clusters based on the PlacementNet in PlacementInfo
     *
     */
    void creaeClusterNets()
    {
        // create the nets between the cluster units
        for (auto tmpNet : placementInfo->getPlacementNets())
        {
            PlacementInfo::ClusterNet *curClusterNet = new PlacementInfo::ClusterNet(clusterNets.size());
            clusterNets.push_back(curClusterNet);
            for (auto tmpPU : tmpNet->getUnits())
            {
                curClusterNet->addClusterUnit(clusterUnits[placementUnitId2ClusterUnitId[tmpPU->getId()]]);
            }
        }

        print_info("ClusterPlacer: #clusterNet=" + std::to_string(clusterNets.size()));
    }

    /**
     * @brief construct the cluster nets adjacent matrix for simulated-annealing cluster placement
     *
     */
    void setClusterNetsAdjMat();

    /**
     * @brief call specific partitioners to partition the design netlist into clusters for initial placement
     *
     */
    void hypergraphPartitioning();

    /**
     * @brief pure connectivity-based partitioning based on PaToH
     *
     * @param minClusterCellNum minimum number of cells in a cluster
     * @param eachClusterDSPNum  maximum number of DSP cells in a cluster
     * @param eachClusterBRAMNum  maximum number of BRAM cells in a cluster
     */
    void basicPartitioning(int minClusterCellNum, int eachClusterDSPNum, int eachClusterBRAMNum);

    /**
     * @brief initially cluster cells based on clock domains and conduct connectivity-based partitioning based on PaToH
     *
     * @param minClusterCellNum minimum number of cells in a cluster
     * @param eachClusterDSPNum  maximum number of DSP cells in a cluster
     * @param eachClusterBRAMNum  maximum number of BRAM cells in a cluster
     */
    void clockBasedPartitioning(int minClusterCellNum, int eachClusterDSPNum, int eachClusterBRAMNum);

    /**
     * @brief initially cluster cells based on user-defined clusters and conduct connectivity-based partitioning based
     * on PaToH
     *
     * @param minClusterCellNum minimum number of cells in a cluster
     * @param eachClusterDSPNum  maximum number of DSP cells in a cluster
     * @param eachClusterBRAMNum  maximum number of BRAM cells in a cluster
     */
    void userDefinedClusterBasedPartitioning(int minClusterCellNum, int eachClusterDSPNum, int eachClusterBRAMNum);

    /**
     * @brief initially cluster cells based on user-defined clusters
     *
     */
    void createUserDefinedClusterBasedClusterUnits();

    /**
     * @brief initially cluster cells based on user-defined clusters
     *
     */
    void createLongPathClusterUnits();

    /**
     * @brief initially cluster cells based on clock domains
     *
     */
    void createClockBasedClusterUnits();

    /**
     * @brief wrap single PlacementUnits into ClusterUnit for later uniform processing
     *
     */
    void createSinglePUClusterUnits();

    /**
     * @brief refine the elements in obtained clusters to ensure the cells in user-defined clusters in the same cluster
     *
     * It seems that this refining solution work poorly. Might be abandoned in the future.
     *
     */
    void refineClustersWithPredefinedClusters();

    /**
     * @brief cluster the netlist and create ClusterUnit level netlist for SA placement
     *
     */
    void clusterPlacementUnits();

    /**
     * @brief conduct the ClusterUnit level SA placement and then map PlacementUnit to cluster location
     *
     */
    void placeClusters();

    /**
     * @brief map PlacementUnit to cluster location
     *
     * @param cluster2XY the cluster location at grid level
     */
    void placeUnitBaseOnClusterPlacement(const std::vector<std::pair<int, int>> &cluster2XY);

    /**
     * @brief check whether the average size of clusters is greater than the given threshold
     *
     * @return true if the granularity is too high
     * @return false if the granularity is under constraints
     */
    bool isClustersToLarges();

    inline float random_float(float min, float max)
    {
        return ((float)random() / RAND_MAX) * (max - min) + min;
    }

    void dumpClusters();

    void drawClusters();
};

#endif