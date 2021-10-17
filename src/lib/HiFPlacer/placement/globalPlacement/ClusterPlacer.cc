/**
 * @file ClusterPlacer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This implementation file contains APIs' implementation of the ClusterPlacer which cluster nodes in the given
 * netlist and place the clusters on the device based on simulated-annealing as initial placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "ClusterPlacer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

ClusterPlacer::ClusterPlacer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                             float connectionToFixedFactor)
    : placementInfo(placementInfo), JSONCfg(JSONCfg), connectionToFixedFactor(connectionToFixedFactor)
{
    if (JSONCfg.find("ClusterPlacerVerbose") != JSONCfg.end())
        verbose = JSONCfg["ClusterPlacerVerbose"] == "true";
    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);

    if (JSONCfg.find("jobs") != JSONCfg.end())
    {
        jobs = std::stoul(JSONCfg["jobs"]);
    }
    else
    {
        jobs = 1;
    }

    clockRegionXNum = std::stoi(JSONCfg["clockRegionXNum"]);
    clockRegionYNum = std::stoi(JSONCfg["clockRegionYNum"]);

    if (JSONCfg.find("RandomInitialPlacement") != JSONCfg.end())
    {
        randomInitialPlacement = JSONCfg["RandomInitialPlacement"] == "true";
    }
}

void ClusterPlacer::ClusterPlacement()
{
    print_status("Cluster Placement Start.");

    if (!randomInitialPlacement)
    {
        clusterPlacementUnits();
        placeClusters();

        if (JSONCfg.find("drawClusters") != JSONCfg.end())
        {
            if (JSONCfg["drawClusters"] == "true")
                drawClusters();
        }
        if (JSONCfg.find("Dump Cluster file") != JSONCfg.end())
        {
            dumpClusters();
        }
    }
    else
    {
        srandom(20213654);
        for (auto curPU : placementInfo->getPlacementUnits())
        {
            if (!curPU->isFixed())
            {
                float fX = random_float(placementInfo->getGlobalMinX() + 0.1, placementInfo->getGlobalMaxX() - 0.1);
                float fY = random_float(placementInfo->getGlobalMinY() + 0.1, placementInfo->getGlobalMaxY() - 0.1);
                placementInfo->legalizeXYInArea(curPU, fX, fY);
                curPU->setAnchorLocation(fX, fY);
            }
        }
    }

    placementInfo->updateB2BAndGetTotalHPWL();
    placementInfo->checkClockUtilization(true);

    print_status("Cluster Placement Done.");
}

void ClusterPlacer::clusterPlacementUnits()
{
    print_status("Clustering Start.");

    hypergraphPartitioning();

    // refineClustersWithPredefinedClusters();

    setClusterNetsAdjMat();

    print_status("Placement Unit Clustering Done.");
}

void ClusterPlacer::hypergraphPartitioning()
{
    int minClusterCellNum =
        std::min(unsigned(placementInfo->getNumCells() * 0.25),
                 unsigned(avgClusterSizeRequirement)); // 22000 is the rough number of cells in a clock region, Please
                                                       // note that minCellClusterSize is not the number of
                                                       // placementunits but the number of cells

    int eachClusterDSPNum = 96;
    int eachClusterBRAMNum = 24;

    if (JSONCfg.find("clockRegionDSPNum") != JSONCfg.end())
    {
        eachClusterDSPNum = std::stoi(JSONCfg["clockRegionDSPNum"]);
    }
    if (JSONCfg.find("clockRegionBRAMNum") != JSONCfg.end())
    {
        eachClusterBRAMNum = std::stoi(JSONCfg["clockRegionBRAMNum"]);
    }

    clockBasedPartitioning(minClusterCellNum, eachClusterDSPNum, eachClusterBRAMNum);
    if (isClustersToLarges())
    {
        maxMinCutRate = 0.1;
        clockBasedPartitioning(minClusterCellNum, eachClusterDSPNum, eachClusterBRAMNum);
        isClustersToLarges();
    }
}

void ClusterPlacer::createLongPathClusterUnits()
{
    // handle the long paths
    auto &timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo_PathLenSorted();
    auto simpleTimingGraph = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph();
    int pathLengthThr = placementInfo->getLongPathThresholdLevel();
    std::set<int> extractedCellIds;
    extractedCellIds.clear();

    print_status("ClusterPlacer:: create long path cluster units (pathLengthThr=" + std::to_string(pathLengthThr) +
                 ")");
    unsigned int maxSize = 0;
    for (auto timingNode : timingNodes)
    {
        if (timingNode->getLongestPathLength() > pathLengthThr && timingNode->getForwardLevel() > pathLengthThr * 0.333)
        {
            auto curCell = timingNode->getDesignNode();
            auto tmpPU = placementInfo->getPlacementUnitByCell(curCell);
            if (placementUnitId2ClusterUnitId[tmpPU->getId()] < 0)
            {
                std::vector<int> candidateCellIds; //= simpleTimingGraph->BFSFromNode(timingNode->getId(),
                                                   // pathLengthThr, 1000000, extractedCellIds);
                candidateCellIds.clear();

                if (timingNode->getOutEdges().size() >= 16)
                {

                    for (auto outEdge : timingNode->getOutEdges())
                    {
                        if (extractedCellIds.find(outEdge->getSink()->getId()) == extractedCellIds.end())
                            candidateCellIds.push_back(outEdge->getSink()->getId());
                    }

                    // if (candidateCellIds.size() < 16)
                    //     continue;
                }
                else
                {
                    continue;
                }

                std::set<PlacementInfo::PlacementUnit *> PUsInLongPaths;
                PUsInLongPaths.clear();
                for (auto &candidateCellId : candidateCellIds)
                {
                    extractedCellIds.insert(candidateCellId);
                    auto PUInPath = placementInfo->getPlacementUnitByCellId(candidateCellId);
                    // the clusters should not be overlapped with each other
                    if (placementUnitId2ClusterUnitId[PUInPath->getId()] < 0)
                        PUsInLongPaths.insert(PUInPath);
                }

                if (PUsInLongPaths.size() == 0)
                    continue;

                PlacementInfo::ClusterUnit *curClusterUnit = new PlacementInfo::ClusterUnit(clusterUnits.size());
                for (auto PUInPath : PUsInLongPaths)
                {
                    assert(placementUnitId2ClusterUnitId[PUInPath->getId()] < 0);
                    placementUnitId2ClusterUnitId[PUInPath->getId()] = curClusterUnit->getId();
                    curClusterUnit->addPlacementUnit(PUInPath);

                    if (!PUInPath->isFixed())
                    {
                        if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(PUInPath))
                        {
                            int cellId = unpackedCell->getCell()->getCellId();
                            extractedCellIds.insert(cellId);
                        }
                        else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(PUInPath))
                        {
                            for (auto tmpCell : curMacro->getCells())
                            {
                                int cellId = tmpCell->getCellId();
                                extractedCellIds.insert(cellId);
                            }
                        }
                    }
                }

                if (PUsInLongPaths.size() > maxSize)
                    maxSize = PUsInLongPaths.size();

                clusterUnits.push_back(curClusterUnit);
            }
        }
        assert(clusterUnits.size() <= placementInfo->getPlacementUnits().size());
    }
    print_info("ClusterPlacer: largest long-path cluster size=" + std::to_string(maxSize));
}

void ClusterPlacer::createClockBasedClusterUnits()
{
    // add the predefined clusters
    for (auto curClock : placementInfo->getDesignInfo()->getClocksInDesign())
    {
        auto &cellSet = placementInfo->getDesignInfo()->getCellsUnderClock(curClock);
        if (cellSet.size() > 5000 || cellSet.size() < 20)
            continue;

        std::set<PlacementInfo::PlacementUnit *> PUsInuserDefinedClusterCluster;
        PUsInuserDefinedClusterCluster.clear();
        for (auto &curCell : cellSet)
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(curCell);
            // the clusters should not be overlapped with each other
            if (placementUnitId2ClusterUnitId[tmpPU->getId()] < 0)
                PUsInuserDefinedClusterCluster.insert(tmpPU);
        }

        PlacementInfo::ClusterUnit *curClusterUnit = new PlacementInfo::ClusterUnit(clusterUnits.size());
        for (auto tmpPU : PUsInuserDefinedClusterCluster)
        {
            assert(placementUnitId2ClusterUnitId[tmpPU->getId()] < 0);
            placementUnitId2ClusterUnitId[tmpPU->getId()] = curClusterUnit->getId();
            curClusterUnit->addPlacementUnit(tmpPU);
        }
        print_info("build a cluset (size=" + std::to_string(curClusterUnit->getUnits().size()) + ") for clock [" +
                   curClock->getName() + "].");
        clusterUnits.push_back(curClusterUnit);
    }
}

void ClusterPlacer::createUserDefinedClusterBasedClusterUnits()
{
    std::vector<std::vector<DesignInfo::DesignCell *>> &predefinedCellClusters =
        placementInfo->getDesignInfo()->getPredefinedClusters();
    placementUnitId2ClusterUnitId = std::vector<int>(placementInfo->getPlacementUnits().size(), -1);
    clusterUnits.clear();
    clusterNets.clear();

    // add the predefined clusters
    int overlapCnt = 0;
    for (auto &clusterCellSet : predefinedCellClusters)
    {

        bool noOverlap = true;
        std::set<PlacementInfo::PlacementUnit *> PUsInuserDefinedClusterCluster;
        PUsInuserDefinedClusterCluster.clear();
        for (auto tmpCell : clusterCellSet)
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
            // the clusters should not be overlapped with each other
            if (placementUnitId2ClusterUnitId[tmpPU->getId()] >= 0)
            {
                noOverlap = false;
                break;
            }
            PUsInuserDefinedClusterCluster.insert(tmpPU);
        }
        if (noOverlap)
        {
            PlacementInfo::ClusterUnit *curClusterUnit = new PlacementInfo::ClusterUnit(clusterUnits.size());
            for (auto tmpPU : PUsInuserDefinedClusterCluster)
            {
                assert(placementUnitId2ClusterUnitId[tmpPU->getId()] < 0);
                placementUnitId2ClusterUnitId[tmpPU->getId()] = curClusterUnit->getId();
                curClusterUnit->addPlacementUnit(tmpPU);
            }
            clusterUnits.push_back(curClusterUnit);
        }
        else
        {
            overlapCnt++;
        }
    }

    print_info("There are " + std::to_string(predefinedCellClusters.size()) + " predefined userDefinedClusters and " +
               std::to_string(overlapCnt) + " overlapped clusters and totally " + std::to_string(clusterUnits.size()) +
               " userDefinedCluster clusters are identified.");

    // handle the other placement units
    createSinglePUClusterUnits();

    // create the nets between the cluster units
    creaeClusterNets();
}

void ClusterPlacer::createSinglePUClusterUnits()
{
    for (auto tmpPU : placementInfo->getPlacementUnits())
    {
        if (placementUnitId2ClusterUnitId[tmpPU->getId()] < 0)
        {
            PlacementInfo::ClusterUnit *curClusterUnit = new PlacementInfo::ClusterUnit(clusterUnits.size());
            clusterUnits.push_back(curClusterUnit);
            curClusterUnit->addPlacementUnit(tmpPU);
            placementUnitId2ClusterUnitId[tmpPU->getId()] = curClusterUnit->getId();
        }
    }
}

void ClusterPlacer::basicPartitioning(int minClusterCellNum, int eachClusterDSPNum, int eachClusterBRAMNum)
{
    basicGraphPartitioner =
        new GraphPartitioner<std::vector<PlacementInfo::PlacementUnit *>, std::vector<PlacementInfo::PlacementNet *>>(
            placementInfo->getPlacementUnits(), placementInfo->getPlacementNets(), minClusterCellNum, jobs, verbose);
    basicGraphPartitioner->setMaxCutRate(maxMinCutRate);
    basicGraphPartitioner->solve(eachClusterDSPNum, eachClusterBRAMNum);
    clusters = basicGraphPartitioner->getClustersPUIdSets();
    delete basicGraphPartitioner;

    placementUnit2ClusterId.clear();
    placementUnit2ClusterId.resize(placementInfo->getPlacementUnits().size(), -1);
    int clusterId = 0;
    for (auto cluster : clusters)
    {
        for (int placementUnitId : cluster)
        {
            placementUnit2ClusterId[placementUnitId] = clusterId;
        }
        clusterId++;
    }
}

void ClusterPlacer::userDefinedClusterBasedPartitioning(int minClusterCellNum, int eachClusterDSPNum,
                                                        int eachClusterBRAMNum)
{
    createUserDefinedClusterBasedClusterUnits();
    userDefinedClusterBasedGraphPartitioner =
        new GraphPartitioner<std::vector<PlacementInfo::ClusterUnit *>, std::vector<PlacementInfo::ClusterNet *>>(
            clusterUnits, clusterNets, minClusterCellNum, jobs, verbose);
    userDefinedClusterBasedGraphPartitioner->setMaxCutRate(maxMinCutRate);
    userDefinedClusterBasedGraphPartitioner->solve(eachClusterDSPNum, eachClusterBRAMNum);
    clusters = userDefinedClusterBasedGraphPartitioner->getClustersPUIdSets();
    delete userDefinedClusterBasedGraphPartitioner;

    placementUnit2ClusterId.clear();
    placementUnit2ClusterId.resize(placementInfo->getPlacementUnits().size(), -1);
    int clusterId = 0;

    std::vector<std::set<int>> newClusters(0);
    for (auto cluster : clusters)
    {
        std::set<int> curCluster;
        int tmpWeight = 0;
        for (int clusterUnitId : cluster)
        {
            for (auto tmpPU : clusterUnits[clusterUnitId]->getUnits())
            {
                assert(placementUnit2ClusterId[tmpPU->getId()] < 0);
                placementUnit2ClusterId[tmpPU->getId()] = clusterId;
                curCluster.insert(tmpPU->getId());
                tmpWeight += tmpPU->getWeight();
            }
        }
        newClusters.push_back(curCluster);
        clusterId++;
    }

    clusters = newClusters;
}

void ClusterPlacer::clockBasedPartitioning(int minClusterCellNum, int eachClusterDSPNum, int eachClusterBRAMNum)
{
    resetClusterInfo();

    createClockBasedClusterUnits();
    print_info("ClusterPlacer: There are " +
               std::to_string(placementInfo->getDesignInfo()->getClocksInDesign().size()) +
               " global clocks and totally " + std::to_string(clusterUnits.size()) + " clock clusters are identified.");

    int clusterNumRecord = clusterUnits.size();
    createLongPathClusterUnits();
    print_info("ClusterPlacer: There are totally " + std::to_string(clusterUnits.size() - clusterNumRecord) +
               " long-path clusters are identified.");

    clusterNumRecord = clusterUnits.size();
    createSinglePUClusterUnits();
    print_info("ClusterPlacer: There are totally " + std::to_string(clusterUnits.size() - clusterNumRecord) +
               " Single-PU clusters are identified.");

    // handle the other placement units
    creaeClusterNets();

    clockBasedGraphPartitioner =
        new GraphPartitioner<std::vector<PlacementInfo::ClusterUnit *>, std::vector<PlacementInfo::ClusterNet *>>(
            clusterUnits, clusterNets, minClusterCellNum, jobs, verbose);
    clockBasedGraphPartitioner->setMaxCutRate(maxMinCutRate);
    clockBasedGraphPartitioner->solve(eachClusterDSPNum, eachClusterBRAMNum);
    clusters = clockBasedGraphPartitioner->getClustersPUIdSets();
    delete clockBasedGraphPartitioner;

    placementUnit2ClusterId.clear();
    placementUnit2ClusterId.resize(placementInfo->getPlacementUnits().size(), -1);
    int clusterId = 0;

    std::vector<std::set<int>> newClusters(0);
    unsigned int PUCnt = 0;
    for (auto cluster : clusters)
    {
        std::set<int> curCluster;
        int tmpWeight = 0;
        for (int clusterUnitId : cluster)
        {
            for (auto tmpPU : clusterUnits[clusterUnitId]->getUnits())
            {
                assert(placementUnit2ClusterId[tmpPU->getId()] < 0);
                placementUnit2ClusterId[tmpPU->getId()] = clusterId;
                curCluster.insert(tmpPU->getId());
                tmpWeight += tmpPU->getWeight();
                PUCnt++;
            }
        }
        newClusters.push_back(curCluster);
        clusterId++;
    }
    assert(PUCnt == placementUnit2ClusterId.size());
    clusters = newClusters;
}

void ClusterPlacer::refineClustersWithPredefinedClusters()
{
    std::vector<std::vector<DesignInfo::DesignCell *>> &predefinedCellClusters =
        placementInfo->getDesignInfo()->getPredefinedClusters();

    std::set<PlacementInfo::PlacementUnit *> reallocatedPUs;

    int reallocateCnt = 0;
    for (auto &clusterCellSet : predefinedCellClusters)
    {
        std::vector<int> cluster2overlappedCellNum(clusters.size(), 0);
        int maxOverlappedClusterId = 0;
        int maxOverlapCnt = 0;

        int DSPCnt = 0;
        int BRAMCnt = 0;
        for (auto tmpCell : clusterCellSet)
        {
            DSPCnt += tmpCell->isDSP();
            BRAMCnt += tmpCell->isBRAM();
        }

        if (DSPCnt >= 8 || BRAMCnt >= 8)
            continue;

        // find the most overlapped partitioned cluster for the predefined cluster
        for (auto tmpCell : clusterCellSet)
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
            auto tmpPUId = tmpPU->getId();

            for (unsigned int i = 0; i < clusters.size(); i++)
            {
                if (clusters[i].find(tmpPUId) != clusters[i].end())
                {
                    cluster2overlappedCellNum[i]++;
                    if (cluster2overlappedCellNum[i] > maxOverlapCnt)
                    {
                        cluster2overlappedCellNum[i] = cluster2overlappedCellNum[i];
                        maxOverlappedClusterId = i;
                    }
                    break;
                }
            }
        }

        // move cells in predefined Cluster togather
        for (auto tmpCell : clusterCellSet)
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
            auto tmpPUId = tmpPU->getId();

            // don't reallocate PU twice
            if (reallocatedPUs.find(tmpPU) != reallocatedPUs.end())
                continue;
            reallocatedPUs.insert(tmpPU);

            if (placementUnit2ClusterId[tmpPUId] != maxOverlappedClusterId)
            {
                clusters[placementUnit2ClusterId[tmpPUId]].erase(tmpPUId);
                clusters[maxOverlappedClusterId].insert(tmpPUId);
                placementUnit2ClusterId[tmpPUId] = maxOverlappedClusterId;
                reallocateCnt++;
            }
        }
    }

    print_info("reallocate " + std::to_string(reallocateCnt) + " PU(s)");
}

bool ClusterPlacer::isClustersToLarges()
{
    std::string infoStr = "Recursive partitioning generates " + std::to_string(clusters.size()) +
                          " clusters and the numbers of the placement units for them are: ";

    auto &PUs = placementInfo->getPlacementUnits();

    int totalCellsinCluster = 0;
    for (auto cluster : clusters)
    {
        int DSPNum = 0;
        int BRAMNum = 0;
        int cellsinCluster = 0;
        for (int PUId : cluster)
        {
            if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(PUs[PUId]))
            {
                for (auto tmpCell : curMacro->getCells())
                {
                    DSPNum += tmpCell->isDSP();
                    BRAMNum += tmpCell->isBRAM();
                }
            }
            else if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(PUs[PUId]))
            {
                DSPNum += curUnpackedCell->getCell()->isDSP();
                BRAMNum += curUnpackedCell->getCell()->isBRAM();
            }

            cellsinCluster += PUs[PUId]->getWeight();
            totalCellsinCluster += PUs[PUId]->getWeight();
        }
        infoStr += std::to_string(cluster.size()) + "-" + std::to_string(cellsinCluster) +
                   "(DSP:" + std::to_string(DSPNum) + ",BRAM:" + std::to_string(BRAMNum) + "), ";
    }

    print_info(infoStr);

    if ((totalCellsinCluster / clusters.size()) > 0.75 * avgClusterSizeRequirement)
    {
        print_warning("Average cluster size is too large = " + std::to_string((totalCellsinCluster / clusters.size())));
        return true;
    }
    return false;
}

bool containIOCells(PlacementInfo::PlacementUnit *curPU)
{
    if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
    {
        return unpackedCell->getCell()->isIO();
    }
    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
    {
        for (auto tmpCell : curMacro->getCells())
        {
            if (tmpCell->isIO())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void ClusterPlacer::setClusterNetsAdjMat()
{
    clusterAdjMat = std::vector<std::vector<float>>(clusters.size(), std::vector<float>(clusters.size(), 0));
    clusterCLBCellWeights = std::vector<float>(clusters.size(), 0);
    clusterDSPCellWeights = std::vector<float>(clusters.size(), 0);
    clusterBRAMCellWeights = std::vector<float>(clusters.size(), 0);
    cluster2FixedUnitMat = std::vector<std::vector<float>>(
        clusters.size(), std::vector<float>(placementInfo->getFixedPlacementUnits().size(), 0));

    fixedX.clear();
    fixedY.clear();

    float clockRegionW = (placementInfo->getGlobalMaxX() - placementInfo->getGlobalMinX()) / clockRegionXNum;

    std::map<PlacementInfo::PlacementUnit *, int> fixedPlacementUnit2LocId;
    fixedPlacementUnit2LocId.clear();

    for (auto net : placementInfo->getPlacementNets())
    {
        for (auto driveU : net->getDriverUnits())
        {
            if (driveU->isFixed())
            {
                if (fixedPlacementUnit2LocId.find(driveU) == fixedPlacementUnit2LocId.end())
                {
                    int amo = fixedPlacementUnit2LocId.size();
                    fixedPlacementUnit2LocId[driveU] = amo;
                    if (containIOCells(driveU))
                    {
                        fixedX.push_back(driveU->X() + clockRegionW / 2.0);
                        fixedY.push_back(driveU->Y());
                    }
                    else
                    {
                        fixedX.push_back(driveU->X());
                        fixedY.push_back(driveU->Y());
                    }
                }
            }

            for (auto UBeDriven : net->getUnitsBeDriven())
            {
                int A = driveU->getId();
                int B = UBeDriven->getId();
                int clusterA = placementUnit2ClusterId[A];
                int clusterB = placementUnit2ClusterId[B];
                assert(clusterA >= 0 && clusterB >= 0);
                assert((unsigned int)clusterA < cluster2FixedUnitMat.size() &&
                       (unsigned int)clusterB < cluster2FixedUnitMat.size());
                if (UBeDriven->isFixed())
                {
                    if (fixedPlacementUnit2LocId.find(UBeDriven) == fixedPlacementUnit2LocId.end())
                    {
                        int amo = fixedPlacementUnit2LocId.size();
                        fixedPlacementUnit2LocId[UBeDriven] = amo;
                        if (containIOCells(UBeDriven))
                        {
                            fixedX.push_back(UBeDriven->X() + clockRegionW / 2.0);
                            fixedY.push_back(UBeDriven->Y());
                        }
                        else
                        {
                            fixedX.push_back(UBeDriven->X());
                            fixedY.push_back(UBeDriven->Y());
                        }
                    }
                }

                if (net->getPinOffsetsInUnit().size() > 1)
                {
                    if (UBeDriven->isFixed() && !driveU->isFixed())
                    {
                        assert(fixedPlacementUnit2LocId.find(UBeDriven) != fixedPlacementUnit2LocId.end());
                        assert((unsigned int)fixedPlacementUnit2LocId[UBeDriven] <
                               cluster2FixedUnitMat[clusterA].size());
                        cluster2FixedUnitMat[clusterA][fixedPlacementUnit2LocId[UBeDriven]] +=
                            net->getDesignNet()->getOverallEnhanceRatio() / (net->getPinOffsetsInUnit().size() - 1);
                    }
                    else if (!UBeDriven->isFixed() && driveU->isFixed())
                    {
                        assert(fixedPlacementUnit2LocId.find(driveU) != fixedPlacementUnit2LocId.end());
                        assert((unsigned int)fixedPlacementUnit2LocId[driveU] < cluster2FixedUnitMat[clusterB].size());
                        cluster2FixedUnitMat[clusterB][fixedPlacementUnit2LocId[driveU]] +=
                            net->getDesignNet()->getOverallEnhanceRatio() / (net->getPinOffsetsInUnit().size() - 1);
                    }
                    else if (!UBeDriven->isFixed() && !driveU->isFixed())
                    {
                        clusterAdjMat[clusterA][clusterB] +=
                            net->getDesignNet()->getOverallEnhanceRatio() / (net->getPinOffsetsInUnit().size() - 1);
                        clusterAdjMat[clusterB][clusterA] +=
                            net->getDesignNet()->getOverallEnhanceRatio() / (net->getPinOffsetsInUnit().size() - 1);
                    }
                }
            }
        }
    }

    for (auto curPU : placementInfo->getPlacementUnits())
    {
        int PUId = curPU->getId();
        int clusterId = placementUnit2ClusterId[PUId];
        clusterCLBCellWeights[clusterId] += curPU->getWeight();
        clusterDSPCellWeights[clusterId] += curPU->getDSPNum();
        clusterBRAMCellWeights[clusterId] += curPU->getBRAMNum();
    }
}

void ClusterPlacer::placeClusters()
{
    print_status("SA-based Cluster Placement Start.");
    assert(JSONCfg.find("clockRegionXNum") != JSONCfg.end());
    assert(JSONCfg.find("clockRegionYNum") != JSONCfg.end());

    int gridH = std::stoi(JSONCfg["clockRegionYNum"]);
    int gridW = std::stoi(JSONCfg["clockRegionXNum"]);
    int SAIterNum = 100000;
    int restartNum = 10;

    if (JSONCfg.find("Simulated Annealing IterNum") != JSONCfg.end())
        SAIterNum = std::stoi(JSONCfg["Simulated Annealing IterNum"]);
    if (JSONCfg.find("Simulated Annealing restartNum") != JSONCfg.end())
        restartNum = std::stoi(JSONCfg["Simulated Annealing restartNum"]);
    float deviceW = (placementInfo->getGlobalMaxX() - placementInfo->getGlobalMinX());
    float deviceH = (placementInfo->getGlobalMaxY() - placementInfo->getGlobalMinY());

    // SA-based cluster placement
    saPlacer = new SAPlacer("ClusterSA", clusterAdjMat, clusterCLBCellWeights, cluster2FixedUnitMat, fixedX, fixedY,
                            gridH, gridW, deviceH, deviceW, connectionToFixedFactor, y2xRatio * 0.8, SAIterNum, jobs,
                            restartNum, verbose);
    saPlacer->solve();
    cluster2XY = saPlacer->getCluster2XY();

    placeUnitBaseOnClusterPlacement(cluster2XY);

    placementInfo->updateElementBinGrid();
    print_status("SA-based Cluster Placement Done.");
}

void ClusterPlacer::placeUnitBaseOnClusterPlacement(const std::vector<std::pair<int, int>> &cluster2XY)
{
    int gridH = std::stoi(JSONCfg["clockRegionYNum"]);
    int gridW = std::stoi(JSONCfg["clockRegionXNum"]);

    float deviceW = (placementInfo->getGlobalMaxX() - placementInfo->getGlobalMinX());
    float deviceH = (placementInfo->getGlobalMaxY() - placementInfo->getGlobalMinY());

    float regionW = deviceW / gridW;
    float regionH = deviceH / gridH;

    cluster2FP_XY.clear();
    for (auto intXY : cluster2XY)
    {
        float fX = intXY.first * regionW + regionW / 2;
        float fY = intXY.second * regionH + regionH / 2;
        cluster2FP_XY.push_back(std::pair<float, float>(fX, fY));
    }

    assert(!randomInitialPlacement);

    for (auto curPU : placementInfo->getPlacementUnits())
    {
        if (!curPU->isFixed())
        {
            // float fX = random_float(cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].first - regionW / 2,
            //                         cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].first + regionW / 2);
            // float fY = random_float(cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].second - regionH / 2,
            //                         cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].second + regionH / 2);
            float fX = cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].first;
            float fY = cluster2FP_XY[placementUnit2ClusterId[curPU->getId()]].second;
            placementInfo->legalizeXYInArea(curPU, fX, fY);
            curPU->setAnchorLocation(fX, fY);
        }
    }
}

void ClusterPlacer::dumpClusters()
{
    std::string dumpClustersFile = JSONCfg["Dump Cluster file"];
    if (dumpClustersFile != "")
    {
        print_status("dumping cluster information to " + dumpClustersFile);
        std::ofstream outfile0((dumpClustersFile + ".tcl").c_str());
        assert(outfile0.is_open() && outfile0.good() &&
               "The path for cluster result dumping does not exist and please check your path settings");
        for (unsigned int cluster_id = 0; cluster_id < clusters.size(); cluster_id++)
        {
            outfile0 << "# placed at region coordinate: X " << cluster2XY[cluster_id].first << " Y "
                     << cluster2XY[cluster_id].second << "\n";
            outfile0 << "highlight -color_index " << (cluster_id) % 20 + 1 << "  [get_cells {";
            for (int id : clusters[cluster_id])
            {
                if (auto tmpMacro =
                        dynamic_cast<PlacementInfo::PlacementMacro *>(placementInfo->getPlacementUnits()[id]))
                {
                    for (auto cell : tmpMacro->getCells())
                    {
                        outfile0 << cell->getName() << " ";
                    }
                }
                else if (auto tmpUnpacked = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(
                             placementInfo->getPlacementUnits()[id]))
                {
                    outfile0 << tmpUnpacked->getName() << " ";
                }
                else
                {
                    assert(false);
                }
            }
            outfile0 << "}]\n";
        }
        outfile0.close();
        outfile0 = std::ofstream(dumpClustersFile.c_str());
        for (unsigned int cluster_id = 0; cluster_id < clusters.size(); cluster_id++)
        {
            for (int id : clusters[cluster_id])
            {
                if (auto tmpMacro =
                        dynamic_cast<PlacementInfo::PlacementMacro *>(placementInfo->getPlacementUnits()[id]))
                {
                    for (auto cell : tmpMacro->getCells())
                    {
                        outfile0 << cell->getName() << " ";
                    }
                }
                else if (auto tmpUnpacked = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(
                             placementInfo->getPlacementUnits()[id]))
                {
                    outfile0 << tmpUnpacked->getName() << " ";
                }
                else
                {
                    assert(false);
                }
            }
            outfile0 << "\n";
        }
        outfile0.close();
    }
}

void ClusterPlacer::drawClusters()
{
    std::vector<std::pair<int, int>> lines;
    lines.clear();
    for (unsigned int clusterA = 1; clusterA < clusterAdjMat.size(); clusterA++)
        for (unsigned int clusterB = 0; clusterB < clusterA; clusterB++)
        {
            if (clusterAdjMat[clusterA][clusterB] > 0)
            {
                lines.push_back(std::pair<int, int>(clusterA, clusterB));
            }
        }
    // paintClusterNodeLine(cluster2FP_XY, lines, cluster2FixedUnitMat, fixedX, fixedY);
}