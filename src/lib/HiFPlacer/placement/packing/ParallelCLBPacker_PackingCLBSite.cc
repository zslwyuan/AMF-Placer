/**
 * @file ParallelCLBPacker_PackingCLBSite.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This implementation file contains APIs' implementation related to the CLBSite-level processes in the
 * ParallelCLBPacker which finally packs LUT/FF/MUX/CARRY elements into legal CLB sites in a parallel approach.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "ParallelCLBPacker.h"

void ParallelCLBPacker::PackingCLBSite::refreshPrioryQueue()
{
    std::sort(priorityQueue.begin(), priorityQueue.end(),
              [](const PackingCLBCluster *a, const PackingCLBCluster *b) -> bool {
                  return a->getScoreInSite() > b->getScoreInSite();
              });
    for (unsigned int i = 1; i < priorityQueue.size(); i++)
    {
        if (!(priorityQueue[i - 1]->getScoreInSite() >= priorityQueue[i]->getScoreInSite()))
        {
            std::cout << "priorityQueue[" << i - 1 << "]:\n" << priorityQueue[i - 1] << "\n";
            std::cout << "priorityQueue[i]:\n" << priorityQueue[i] << "\n";
            for (unsigned int j = 0; j < priorityQueue.size(); j++)
            {
                std::cout << "#" << j << ": " << priorityQueue[j]->getScoreInSite() << "\n";
            }
        }
        assert(priorityQueue[i - 1]->getScoreInSite() >= priorityQueue[i]->getScoreInSite());
    }
}

void ParallelCLBPacker::PackingCLBSite::removeInvalidClustersFromPQ()
{
    int validCnt = 0;
    for (unsigned int i = 0; i < priorityQueue.size(); i++)
    {
        if (priorityQueue[i]->areAllPUsValidForThisSite(PUId2PackingCLBSite, this))
        {
            priorityQueue[validCnt] = priorityQueue[i];
            validCnt++;
        }
        else
        {
            delete priorityQueue[i];
            priorityQueue[i] = nullptr;
        }
    }
    if (determinedClusterInSite)
        assert(determinedClusterInSite->areAllPUsValidForThisSite(PUId2PackingCLBSite, this));
    priorityQueue.resize(validCnt);
}

void ParallelCLBPacker::PackingCLBSite::removeClustersIncompatibleWithDetClusterFromPQ()
{
    if (determinedClusterInSite)
    {
        int validCnt = 0;
        for (unsigned int i = 0; i < priorityQueue.size(); i++)
        {
            bool changed = false;
            for (auto tmpPU : determinedClusterInSite->getPUs())
            {
                if (!priorityQueue[i]->contains(tmpPU))
                {
                    if (!priorityQueue[i]->addPU(tmpPU, true))
                    {
                        delete priorityQueue[i];
                        priorityQueue[i] = nullptr;
                        break;
                    }
                    else
                    {
                        changed = true;
                    }
                }
            }
            if (priorityQueue[i])
            {
                if (changed)
                {
                    priorityQueue[i]->refreshId();
                    priorityQueue[i]->updateScoreInSite();
                }
                priorityQueue[validCnt] = priorityQueue[i];
                validCnt++;
            }
        }
        priorityQueue.resize(validCnt);
    }
}

void ParallelCLBPacker::PackingCLBSite::removeInvalidPUsFromNeighborPUs()
{
    std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> newNbr;
    newNbr.clear();
    for (auto tmpPU : neighborPUs)
    {
        bool PUValid = true;
        if (PUId2PackingCLBSite[tmpPU->getId()])
        {
            if (PUId2PackingCLBSite[tmpPU->getId()] != this)
            {
                PUValid = false;
            }
        }
        if (PUValid)
        {
            bool detCompatible = true;
            if (determinedClusterInSite)
            {
                if (!determinedClusterInSite->contains(tmpPU))
                {
                    PackingCLBCluster *testDetCluster = new PackingCLBCluster(determinedClusterInSite);
                    detCompatible = (testDetCluster->addPU(tmpPU, true));
                    delete testDetCluster;
                }
            }
            if (detCompatible)
                newNbr.insert(tmpPU);
        }
    }
    neighborPUs = newNbr;
}

void ParallelCLBPacker::PackingCLBSite::updateConsistentPUsInTop()
{
    if (priorityQueue.size()) // if there are possible condidates
    {
        PackingCLBCluster *topCluster = priorityQueue[0];
        std::vector<PlacementInfo::PlacementUnit *> countedPUs;
        countedPUs.clear();
        for (auto pair : PU2TopCnt)
            countedPUs.push_back(pair.first);

        std::vector<PlacementInfo::PlacementUnit *> PUsToAdd;
        PUsToAdd.clear();
        for (auto PU : countedPUs)
        {
            if (topCluster->contains(PU) && PUId2PackingCLBSite[PU->getId()] == this)
            {
                assert(PU2TopCnt.find(PU) != PU2TopCnt.end());
                PU2TopCnt[PU]++;
                if (PU2TopCnt[PU] >= unchangedIterationThr)
                {
                    if (determinedClusterInSite) // if there is determinedClusterInSite, only add PUs not in in
                    {
                        if (!determinedClusterInSite->contains(PU))
                        {
                            PUsToAdd.push_back(PU);
                        }
                    }
                    else // if there is no determinedClusterInSite yet, all PUs meeting requirement can add
                    {
                        PUsToAdd.push_back(PU);
                    }
                }
            }
            else
            {
                PU2TopCnt.erase(PU); // erase the records of the PUs not in top
            }
        }

        // add the PUs into record if they are not recorded yet
        for (auto PU : topCluster->getPUs())
        {
            if (PU2TopCnt.find(PU) == PU2TopCnt.end())
            {
                PU2TopCnt[PU] = 1;
            }
        }

        if (PUsToAdd.size()) // some new consistent PUs found in top
        {

            // remove candidates in PQ which is not compatible with added PUs
            int validCnt = 0;
            for (unsigned int i = 0; i < priorityQueue.size(); i++)
            {
                bool changed = false;
                for (auto tmpPU : PUsToAdd)
                {
                    if (!priorityQueue[i]->contains(tmpPU))
                    {
                        if (!priorityQueue[i]->addPU(tmpPU, true))
                        {
                            delete priorityQueue[i];
                            priorityQueue[i] = nullptr;
                            break;
                        }
                        else
                        {
                            changed = true;
                        }
                    }
                }
                if (priorityQueue[i])
                {
                    if (changed)
                    {
                        priorityQueue[i]->refreshId();
                        priorityQueue[i]->updateScoreInSite();
                    }
                    priorityQueue[validCnt] = priorityQueue[i];
                    validCnt++;
                }
            }
            priorityQueue.resize(validCnt);

            if (!determinedClusterInSite) // if there is determinedClusterInSite
            {
                delete determinedClusterInSite;
                // priorityQueue.push_back(determinedClusterInSite);
            }

            determinedClusterInSite = new PackingCLBCluster(topCluster);

            for (auto countedPair : PU2TopCnt)
            {
                if (countedPair.second < unchangedIterationThr ||
                    !placementInfo->checkClockColumnLegalization(countedPair.first, CLBSite))
                {
                    determinedClusterInSite->removePUToConstructDetCluster(countedPair.first);
                }
                else
                {
                    placementInfo->addPUIntoClockColumn(countedPair.first, CLBSite);
                }
            }

            determinedClusterInSite->clusterHash();
            determinedClusterInSite->refreshId();
            determinedClusterInSite->updateScoreInSite();
            detScore = determinedClusterInSite->getScoreInSite();
        }
    }
}

void ParallelCLBPacker::PackingCLBSite::findNewClustersWithNeighborPUs()
{

    std::set<int> hashIdSet;
    hashIdSet.clear();
    for (auto tmpSeedCluster : seedClusters)
    {
        hashIdSet.insert(tmpSeedCluster->getHash());
    }
    for (auto tmpSeedCluster : seedClusters)
    {
        for (auto tmpPU : neighborPUs)
        {
            // if (hashIdSet.find(tmpSeedCluster->clusterHashWithAdditionalPU(tmpPU)) != hashIdSet.end())
            //     continue;
            PackingCLBCluster *tmpCluster = new PackingCLBCluster(tmpSeedCluster);

            if (tmpCluster->addPU(tmpPU))
            {
                tmpCluster->refreshId();
                tmpCluster->incrementalUpdateScoreInSite(tmpPU);
                tmpCluster->clusterHash();
                if (hashIdSet.find(tmpCluster->getHash()) != hashIdSet.end())
                {
                    delete tmpCluster;
                    tmpCluster = nullptr;
                }
                else
                {
                    hashIdSet.insert(tmpCluster->getHash());
                    priorityQueue.push_back(tmpCluster);
                }
            }
            else
            {
                // if (tmpSeedCluster->getPUs().size() == 0 && !isCarrySite && !isLUTRAMSite &&
                //     (tmpSeedCluster->isPUTypeCompatibleWithSiteType(tmpPU)))
                // {
                //     PackingCLBCluster *tmpCluster = new PackingCLBCluster(tmpSeedCluster);
                //     tmpCluster->addPUFailReason(tmpPU);
                //     delete tmpCluster;
                //     std::cout.flush();
                //     assert(false);
                // }
                // assert(tmpCluster->getPUs().size() || isCarrySite || isLUTRAMSite ||
                //        !(tmpSeedCluster->isPUTypeCompatibleWithSiteType(tmpPU)));
                delete tmpCluster;
                tmpCluster = nullptr;
            }
        }
    }
}

std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> *
ParallelCLBPacker::PackingCLBSite::findNeiborPUsFromBinGrid(
    DesignInfo::DesignCellType curCellType, float targetX, float targetY, float displacementLowerbound,
    float displacementUpperbound, int PUNumThreshold, const std::vector<PackingCLBSite *> &PUId2PackingCLBSite,
    float y2xRatio, std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> *res, bool clockRegionAware)
{
    assert(displacementLowerbound < displacementUpperbound);
    // please note that the input DesignCell is only used to find the corresponding binGrid for site search.
    if (!res)
    {
        std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> *res =
            new std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare>();
        res->clear();
    }

    int binIdX, binIdY, clockRegionX, clockRegionY;
    placementInfo->getGridXY(targetX, targetY, binIdX, binIdY);
    placementInfo->getDeviceInfo()->getClockRegionByLocation(targetX, targetY, clockRegionX, clockRegionY);

    auto sharedTypeIds = placementInfo->getPotentialBELTypeIDs(curCellType);

    for (auto sharedTypeId : sharedTypeIds)
    {
        std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &curBinGrid =
            placementInfo->getBinGrid(sharedTypeId);
        assert(binIdY >= 0);
        assert((unsigned int)binIdY < curBinGrid.size());
        assert(binIdX >= 0);
        assert((unsigned int)binIdX < curBinGrid[binIdY].size());

        std::queue<std::pair<int, int>> binXYqueue;
        std::set<std::pair<int, int>> reachedBinXYs;
        binXYqueue.emplace(binIdX, binIdY);
        reachedBinXYs.emplace(binIdX, binIdY);

        while (binXYqueue.size() > 0)
        {
            std::pair<int, int> curXY = binXYqueue.front();
            binXYqueue.pop();
            int curbinIdX = curXY.first, curbinIdY = curXY.second;

            PlacementInfo::PlacementBinInfo *curBin = curBinGrid[curbinIdY][curbinIdX];
            float bin2TargetXYDistance = curBin->getManhattanDistanceTo(targetX, targetY);
            if (bin2TargetXYDistance > displacementUpperbound)
                continue;
            for (auto tmpCell : curBin->getCells())
            {
                if (tmpCell->isLUT() || tmpCell->isFF())
                {
                    auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);

                    if (tmpPU->isFixed() || tmpPU->isPacked())
                        continue;
                    if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                    {
                        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_CARRY ||
                            tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MCLB)
                        {
                            continue;
                        }
                    }
                    if (!PUId2PackingCLBSite[tmpPU->getId()])
                    {
                        float tmpPUDis = fabs(targetX - tmpPU->X()) + y2xRatio * fabs(targetY - tmpPU->Y());
                        if (tmpPUDis > displacementLowerbound && tmpPUDis <= displacementUpperbound)
                        {
                            int PUClockRegionX, PUClockRegionY;
                            placementInfo->getDeviceInfo()->getClockRegionByLocation(tmpPU->X(), targetY,
                                                                                     PUClockRegionX, PUClockRegionY);
                            if (PUClockRegionX == clockRegionX)
                                res->insert(tmpPU);
                        }
                    }
                    else
                    {
                        if (PUId2PackingCLBSite[tmpPU->getId()] == this)
                        {
                            float tmpPUDis = fabs(targetX - tmpPU->X()) + y2xRatio * fabs(targetY - tmpPU->Y());
                            if (tmpPUDis > displacementLowerbound && tmpPUDis <= displacementUpperbound)
                            {
                                int PUClockRegionX, PUClockRegionY;
                                placementInfo->getDeviceInfo()->getClockRegionByLocation(
                                    tmpPU->X(), targetY, PUClockRegionX, PUClockRegionY);
                                if (PUClockRegionX == clockRegionX)
                                    res->insert(tmpPU);
                            }
                        }
                    }
                }
            }

            // if (res->size() < (unsigned int)PUNumThreshold)
            {
                for (int nextY = curbinIdY - 1; nextY <= curbinIdY + 1; nextY++)
                {
                    for (int nextX = curbinIdX - 1; nextX <= curbinIdX + 1; nextX++)
                    {
                        if (!(nextY >= 0))
                            continue;
                        if (!((unsigned int)nextY < curBinGrid.size()))
                            continue;
                        if (!(nextX >= 0))
                            continue;
                        if (!((unsigned int)nextX < curBinGrid[binIdY].size()))
                            continue;
                        PlacementInfo::PlacementBinInfo *nextBin = curBinGrid[nextY][nextX];
                        float nextBin2TargetXYDistance = nextBin->getManhattanDistanceTo(targetX, targetY);
                        if (nextBin2TargetXYDistance > displacementUpperbound)
                            continue;
                        std::pair<int, int> nextXY(nextX, nextY);
                        if (reachedBinXYs.find(nextXY) == reachedBinXYs.end())
                        {
                            reachedBinXYs.insert(nextXY);
                            binXYqueue.push(nextXY);
                        }
                    }
                }
            }
        }
    }

    return res;
}

// Node-centric DL algorithm flow at each computation node
void ParallelCLBPacker::PackingCLBSite::updateStep(bool initial, bool debug)
{
    if (debug)
        setDebug();
    removeInvalidClustersFromPQ();
    removeClustersIncompatibleWithDetClusterFromPQ();
    removeInvalidPUsFromNeighborPUs();
    updateConsistentPUsInTop(); //  the PQ top might be kept updated but some of its PUs might be consistent

    seedClusters = priorityQueue;
    if (seedClusters.size() == 0)
    {
        if (determinedClusterInSite)
        {
            seedClusters.push_back(new PackingCLBCluster(determinedClusterInSite));
        }
        else
        {
            seedClusters.push_back(new PackingCLBCluster(this));
        }
        seedClusters[0]->clusterHash();
    }

    if (neighborPUs.size() < numNeighbor && curD < maxD)
    {
        if (initial)
        {
            findNeiborPUsFromBinGrid(DesignInfo::CellType_LUT6, CLBSite->X(), CLBSite->Y(), 0, curD, numNeighbor,
                                     PUId2PackingCLBSite, y2xRatio, &neighborPUs);
            findNeiborPUsFromBinGrid(DesignInfo::CellType_FDCE, CLBSite->X(), CLBSite->Y(), 0, curD, numNeighbor,
                                     PUId2PackingCLBSite, y2xRatio, &neighborPUs);
        }
        else
        {
            float newD = std::min(curD + deltaD, maxD);
            findNeiborPUsFromBinGrid(DesignInfo::CellType_LUT6, CLBSite->X(), CLBSite->Y(), curD, newD, numNeighbor,
                                     PUId2PackingCLBSite, y2xRatio, &neighborPUs);
            findNeiborPUsFromBinGrid(DesignInfo::CellType_FDCE, CLBSite->X(), CLBSite->Y(), curD, newD, numNeighbor,
                                     PUId2PackingCLBSite, y2xRatio, &neighborPUs);
            curD = newD;
        }
    }

    findNewClustersWithNeighborPUs();

    refreshPrioryQueue();
    if (priorityQueue.size() > PQSize)
    {
        int finalPQSize = PQSize;
        float scoreThreshold = priorityQueue[PQSize - 1]->getScoreInSite() * 0.99;
        bool extendable = true;
        for (unsigned int i = PQSize; i < priorityQueue.size(); i++)
        {
            if (extendable && priorityQueue[i]->getScoreInSite() > scoreThreshold)
            {
                finalPQSize = i + 1;
                if (finalPQSize > 1.5 * PQSize)
                {
                    extendable = false;
                }
            }
            else
            {
                extendable = false;
                assert(priorityQueue[i]);
                delete priorityQueue[i];
                priorityQueue[i] = nullptr;
            }
        }
        priorityQueue.resize(finalPQSize);
    }
}

void ParallelCLBPacker::PackingCLBSite::finalMapToSlotsForCarrySite()
{
    assert(isCarrySite);

    assert(fixedPairedLUTs.size() + determinedClusterInSite->getSingleLUTs().size() +
               determinedClusterInSite->getPairedLUTs().size() <=
           8);
    // map LUT connected to carry and their paired LUT
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.LUTs[i][j][k])
                {
                    if (slotMapping.LUTs[i][1 - j][k])
                    {
                        if (conflictLUTsContain(slotMapping.LUTs[i][1 - j][k]))
                        {
                            for (auto pair : determinedClusterInSite->getPairedLUTs())
                            {
                                if (pair.first == slotMapping.LUTs[i][1 - j][k])
                                {
                                    slotMapping.LUTs[i][1 - j][k] = pair.first;
                                    slotMapping.LUTs[i][j][k] = pair.second;
                                    mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                    mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                    mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                    mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                    break;
                                }
                                if (pair.second == slotMapping.LUTs[i][1 - j][k])
                                {
                                    slotMapping.LUTs[i][1 - j][k] = pair.second;
                                    slotMapping.LUTs[i][j][k] = pair.first;
                                    mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                    mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                    mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                    mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // mapped paired LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto pair : determinedClusterInSite->getPairedLUTs())
                {
                    if (mappedLUTs.find(pair.first) == mappedLUTs.end())
                    {
                        if (mappedLUTs.find(pair.second) != mappedLUTs.end())
                        {
                            for (int i = 0; i < 2; i++)
                            {
                                for (int j = 0; j < 2; j++)
                                {
                                    for (int k = 0; k < 4; k++)
                                    {
                                        if (slotMapping.LUTs[i][j][k])
                                        {
                                            std::cout << "i,j,k:" << i << "," << j << "," << k << "   "
                                                      << slotMapping.LUTs[i][j][k] << "\n";
                                        }
                                    }
                                }
                            }
                            determinedClusterInSite->printMyself();
                            std::cout.flush();
                        }
                        assert(mappedLUTs.find(pair.second) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = pair.first;
                        slotMapping.LUTs[i][1][k] = pair.second;
                        mappedLUTs.insert(slotMapping.LUTs[i][0][k]);
                        mappedLUTs.insert(slotMapping.LUTs[i][1][k]);
                        mappedCells.insert(slotMapping.LUTs[i][0][k]);
                        mappedCells.insert(slotMapping.LUTs[i][1][k]);
                        break;
                    }
                }
            }
        }
    }

    // mapped single LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto tmpLUT : determinedClusterInSite->getSingleLUTs())
                {
                    if (mappedLUTs.find(tmpLUT) == mappedLUTs.end())
                    {
                        assert(mappedLUTs.find(tmpLUT) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = tmpLUT;
                        mappedLUTs.insert(tmpLUT);
                        mappedCells.insert(tmpLUT);
                        break;
                    }
                }
            }
        }
    }

    // mapped FFs
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.FFs[i][j][k])
                {
                    int halfCLBId = i * 2 + j;
                    auto &CSFF = determinedClusterInSite->getFFControlSets()[halfCLBId];
                    // if (slotMapping.LUTs[i][j][k])
                    // {
                    //     if (slotMapping.LUTs[i][j][k]->isLUT6())
                    //     {
                    //         if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(
                    //                 placementInfo->getPlacementUnitByCell(slotMapping.LUTs[i][0][k])))
                    //         {
                    //             if (tmpMacro->getMacroType() ==
                    //                 PlacementInfo::PlacementMacro::PlacementMacroType_BEL)
                    //             {
                    //                 for (auto tmpCell : tmpMacro->getCells())
                    //                 {
                    //                     if (tmpCell->isFF())
                    //                     {
                    //                         if (mappedFFs.find(tmpCell) == mappedFFs.end())
                    //                         {
                    //                             for (auto tmpFF : CSFF.getFFs())
                    //                             {
                    //                                 if (mappedFFs.find(tmpFF) == mappedFFs.end() &&
                    //                                     tmpFF == tmpCell)
                    //                                 {
                    //                                     mappedFFs.insert(tmpFF);
                    //                                     mappedCells.insert(tmpFF);
                    //                                     slotMapping.FFs[i][j][k] = tmpFF;
                    //                                 }
                    //                             }
                    //                         }
                    //                     }
                    //                 }
                    //             }
                    //         }
                    //         continue;
                    //     }
                    // }
                    for (auto tmpFF : CSFF.getFFs())
                    {
                        if (mappedFFs.find(tmpFF) == mappedFFs.end())
                        {
                            mappedFFs.insert(tmpFF);
                            mappedCells.insert(tmpFF);
                            slotMapping.FFs[i][j][k] = tmpFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    assert(fixedPairedLUTs.size() + determinedClusterInSite->getSingleLUTs().size() +
               determinedClusterInSite->getPairedLUTs().size() <=
           8);

    if (determinedClusterInSite)
    {

        unsigned int FFCnt = 0;
        for (auto &CSFF : determinedClusterInSite->getFFControlSets())
            FFCnt += CSFF.getFFs().size();

        if (FFCnt != mappedFFs.size())
        {
            std::cout << "FFCnt: " << FFCnt << " mappedFFs.size():" << mappedFFs.size() << "\n";
            determinedClusterInSite->printMyself();
            for (int i = 0; i < 2; i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        std::cout << "i,k,j:" << i << "," << k << "," << j << ":\n";
                        if (slotMapping.LUTs[i][j][k])
                            std::cout << slotMapping.LUTs[i][j][k] << "\n";
                        if (slotMapping.FFs[i][j][k])
                            std::cout << slotMapping.FFs[i][j][k] << "\n";
                    }
                }
            }
            assert(FFCnt == mappedFFs.size());
        }
    }
}

bool isLUT6(DesignInfo::DesignCell *cell)
{
    if (!cell)
        return false;
    return (cell->getOriCellType() == DesignInfo::CellType_LUT6) ||
           (cell->getOriCellType() == DesignInfo::CellType_LUT6_2);
}

void ParallelCLBPacker::PackingCLBSite::greedyMapForCommonLUTFFInSite()
{
    std::map<DesignInfo::DesignCell *, DesignInfo::DesignCell *> FF2LUT;
    auto &singleLUTs = determinedClusterInSite->getSingleLUTs();
    auto &pairedLUTs = determinedClusterInSite->getPairedLUTs();
    for (auto &FFSet : determinedClusterInSite->getFFControlSets())
    {
        for (auto curFF : FFSet.getFFs())
        {
            PlacementInfo::PlacementMacro *pairMacro =
                dynamic_cast<PlacementInfo::PlacementMacro *>(placementInfo->getPlacementUnitByCell(curFF));
            if (pairMacro)
            {
                if (pairMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair)
                {
                    assert(pairMacro->getCells().size() == 2);
                    assert(pairMacro->getCells()[0]->isLUT());
                    assert(pairMacro->getCells()[1]->isFF());
                    FF2LUT[pairMacro->getCells()[1]] = pairMacro->getCells()[0];
                }
            }
        }
    }

    assert(singleLUTs.size() + pairedLUTs.size() <= 8);
    // map LUT connected to carry and their paired LUT
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.LUTs[i][j][k])
                {
                    if (slotMapping.LUTs[i][1 - j][k])
                    {
                        for (auto pair : pairedLUTs)
                        {
                            if (pair.first == slotMapping.LUTs[i][1 - j][k])
                            {
                                slotMapping.LUTs[i][1 - j][k] = pair.first;
                                slotMapping.LUTs[i][j][k] = pair.second;
                                mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                break;
                            }
                            if (pair.second == slotMapping.LUTs[i][1 - j][k])
                            {
                                slotMapping.LUTs[i][1 - j][k] = pair.second;
                                slotMapping.LUTs[i][j][k] = pair.first;
                                mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // mapped paired LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto pair : pairedLUTs)
                {
                    if (mappedLUTs.find(pair.first) == mappedLUTs.end())
                    {
                        if (mappedLUTs.find(pair.second) != mappedLUTs.end())
                        {
                            for (int i = 0; i < 2; i++)
                            {
                                for (int j = 0; j < 2; j++)
                                {
                                    for (int k = 0; k < 4; k++)
                                    {
                                        if (slotMapping.LUTs[i][j][k])
                                        {
                                            std::cout << "i,j,k:" << i << "," << j << "," << k << "   "
                                                      << slotMapping.LUTs[i][j][k] << "\n";
                                        }
                                    }
                                }
                            }
                            determinedClusterInSite->printMyself();
                            std::cout.flush();
                        }
                        assert(mappedLUTs.find(pair.second) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = pair.first;
                        slotMapping.LUTs[i][1][k] = pair.second;
                        mappedLUTs.insert(slotMapping.LUTs[i][0][k]);
                        mappedLUTs.insert(slotMapping.LUTs[i][1][k]);
                        mappedCells.insert(slotMapping.LUTs[i][0][k]);
                        mappedCells.insert(slotMapping.LUTs[i][1][k]);
                        break;
                    }
                }
            }
        }
    }

    // mapped single LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto tmpLUT : singleLUTs)
                {
                    if (mappedLUTs.find(tmpLUT) == mappedLUTs.end())
                    {
                        assert(mappedLUTs.find(tmpLUT) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = tmpLUT;
                        mappedLUTs.insert(tmpLUT);
                        mappedCells.insert(tmpLUT);
                        break;
                    }
                }
            }
        }
    }

    // assert(determinedClusterInSite->getSingleLUTs().size() + determinedClusterInSite->getPairedLUTs().size() * 2 ==
    //        mappedLUTs.size());

    // mapped FFs
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.FFs[i][j][k])
                {
                    int halfCLBId = i * 2 + j;
                    auto &CSFF = determinedClusterInSite->getFFControlSets()[halfCLBId];

                    for (auto tmpFF : CSFF.getFFs())
                    {
                        if (mappedFFs.find(tmpFF) == mappedFFs.end())
                        {
                            mappedFFs.insert(tmpFF);
                            mappedCells.insert(tmpFF);
                            slotMapping.FFs[i][j][k] = tmpFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    assert(singleLUTs.size() + pairedLUTs.size() <= 8);
    if (determinedClusterInSite)
    {

        unsigned int FFCnt = 0;
        for (auto &CSFF : determinedClusterInSite->getFFControlSets())
            FFCnt += CSFF.getFFs().size();

        if (FFCnt != mappedFFs.size())
        {
            std::cout << "FFCnt: " << FFCnt << " mappedFFs.size():" << mappedFFs.size() << "\n";
            determinedClusterInSite->printMyself();
            for (int i = 0; i < 2; i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        std::cout << "i,k,j:" << i << "," << k << "," << j << ":\n";
                        if (slotMapping.LUTs[i][j][k])
                            std::cout << slotMapping.LUTs[i][j][k] << "\n";
                        if (slotMapping.FFs[i][j][k])
                            std::cout << slotMapping.FFs[i][j][k] << "\n";
                    }
                }
            }
            assert(FFCnt == mappedFFs.size());
        }
    }

    for (int i0 = 0; i0 < 2; i0++)
    {
        for (int k0 = 0; k0 < 4; k0++)
        {
            if (!isLUT6(slotMapping.LUTs[i0][0][k0]) && !isLUT6(slotMapping.LUTs[i0][1][k0]))
            {
                int oriDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][0][k0]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][1][k0]);

                // switch locations
                int newDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][1][k0]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][0][k0]);

                if (oriDirectInternalRouteNum < newDirectInternalRouteNum)
                {
                    DesignInfo::DesignCell *tmpLUT;
                    tmpLUT = slotMapping.LUTs[i0][0][k0];
                    slotMapping.LUTs[i0][0][k0] = slotMapping.LUTs[i0][1][k0];
                    slotMapping.LUTs[i0][1][k0] = tmpLUT;
                    assert(!isLUT6(tmpLUT));
                }
            }
        }
    }

    int LUTSwapOptions[4][4] = {{0, 1, 0, 1}, {1, 0, 0, 1}, {1, 0, 1, 0}, {0, 1, 1, 0}};

    for (int i0 = 0; i0 < 2; i0++)
    {
        for (int k0 = 0; k0 < 4; k0++)
        {
            for (int i1 = 0; i1 < 2; i1++)
            {
                for (int k1 = 0; k1 < 4; k1++)
                {
                    if (i0 == i1 && k0 == k1)
                    {
                        continue;
                    }
                    int oriDirectInternalRouteNum =
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][0][k0]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][1][k0]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][0][k1], slotMapping.FFs[i1][0][k1]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][1][k1], slotMapping.FFs[i1][1][k1]);

                    int optDirectInternalRouteNum = -1;
                    int optimalOption = -1;
                    for (int optionId = 0; optionId < 4; optionId++)
                    {
                        // switch locations
                        int newDirectInternalRouteNum =
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][LUTSwapOptions[optionId][0]][k1],
                                                    slotMapping.FFs[i0][0][k0]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][LUTSwapOptions[optionId][1]][k1],
                                                    slotMapping.FFs[i0][1][k0]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][LUTSwapOptions[optionId][2]][k0],
                                                    slotMapping.FFs[i1][0][k1]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][LUTSwapOptions[optionId][3]][k0],
                                                    slotMapping.FFs[i1][1][k1]);
                        if (isLUT6(slotMapping.LUTs[i1][LUTSwapOptions[optionId][1]][k1]) ||
                            isLUT6(slotMapping
                                       .LUTs[i0][LUTSwapOptions[optionId][3]][k0])) // illegal to put LUT6 at LUT5 slot
                            continue;
                        if (newDirectInternalRouteNum > optDirectInternalRouteNum)
                        {
                            optimalOption = optionId;
                            optDirectInternalRouteNum = newDirectInternalRouteNum;
                        }
                    }

                    if (oriDirectInternalRouteNum < optDirectInternalRouteNum)
                    {
                        DesignInfo::DesignCell *tmpLUT0 = slotMapping.LUTs[i1][LUTSwapOptions[optimalOption][0]][k1];
                        DesignInfo::DesignCell *tmpLUT1 = slotMapping.LUTs[i1][LUTSwapOptions[optimalOption][1]][k1];
                        DesignInfo::DesignCell *tmpLUT2 = slotMapping.LUTs[i0][LUTSwapOptions[optimalOption][2]][k0];
                        DesignInfo::DesignCell *tmpLUT3 = slotMapping.LUTs[i0][LUTSwapOptions[optimalOption][3]][k0];
                        slotMapping.LUTs[i0][0][k0] = tmpLUT0;
                        assert(!isLUT6(tmpLUT1));
                        slotMapping.LUTs[i0][1][k0] = tmpLUT1;
                        slotMapping.LUTs[i1][0][k1] = tmpLUT2;
                        assert(!isLUT6(tmpLUT3));
                        slotMapping.LUTs[i1][1][k1] = tmpLUT3;
                        continue;
                    }
                }
            }
        }
    }

    int FFSwapOption[24][4] = {{0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
                               {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
                               {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 0, 1}, {2, 3, 1, 0},
                               {3, 0, 1, 2}, {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}};

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            int oriDirectInternalRouteNum =
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][0], slotMapping.FFs[i][j][0]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][1], slotMapping.FFs[i][j][1]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][2], slotMapping.FFs[i][j][2]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][3], slotMapping.FFs[i][j][3]);
            int optDirectInternalRouteNum = -1;
            int optimalOption = -1;
            for (int optionId = 0; optionId < 24; optionId++)
            {
                int newDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][0],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][0]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][1],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][1]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][2],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][2]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][3],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][3]]);
                if (newDirectInternalRouteNum > optDirectInternalRouteNum)
                {
                    optimalOption = optionId;
                    optDirectInternalRouteNum = newDirectInternalRouteNum;
                }
            }
            if (oriDirectInternalRouteNum < optDirectInternalRouteNum)
            {
                DesignInfo::DesignCell *tmpFF0 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][0]];
                DesignInfo::DesignCell *tmpFF1 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][1]];
                DesignInfo::DesignCell *tmpFF2 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][2]];
                DesignInfo::DesignCell *tmpFF3 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][3]];
                slotMapping.FFs[i][j][0] = tmpFF0;
                slotMapping.FFs[i][j][1] = tmpFF1;
                slotMapping.FFs[i][j][2] = tmpFF2;
                slotMapping.FFs[i][j][3] = tmpFF3;
                continue;
            }
        }
    }
}

void ParallelCLBPacker::PackingCLBSite::mapMuxF8Macro(int muxF8Offset, PlacementInfo::PlacementMacro *MUXF8Macro)
{
    assert(muxF8Offset < 2);
    slotMapping.MuxF8[muxF8Offset] = MUXF8Macro->getCells()[0];
    assert(slotMapping.MuxF8[muxF8Offset]->getOriCellType() == DesignInfo::CellType_MUXF8);
    auto curMUXF8 = slotMapping.MuxF8[muxF8Offset];
    std::vector<DesignInfo::DesignCell *> virtualLUTs;
    std::vector<DesignInfo::DesignCell *> virtualFFs;
    std::vector<DesignInfo::DesignCell *> virtualMUXs;
    std::vector<DesignInfo::DesignCell *> LUTsInMacro;
    std::vector<DesignInfo::DesignCell *> FFsInMacro;
    std::vector<DesignInfo::DesignCell *> MUXsInMacro;
    for (auto curCell : MUXF8Macro->getCells())
    {
        if (curCell->isVirtualCell())
        {
            if (curCell->isMux())
                virtualMUXs.push_back(curCell);
            if (curCell->isFF())
                virtualFFs.push_back(curCell);
            if (curCell->isLUT())
                virtualLUTs.push_back(curCell);
        }
        else
        {
            if (curCell->isMux())
                MUXsInMacro.push_back(curCell);
            if (curCell->isFF())
                FFsInMacro.push_back(curCell);
            if (curCell->isLUT())
                LUTsInMacro.push_back(curCell);
        }
    }

    // map MuxF7
    for (DesignInfo::DesignPin *pinBeDriven : curMUXF8->getInputPins())
    {
        if (!pinBeDriven->getDriverPin())
            continue;
        if (pinBeDriven->getRefPinName() == "I0")
        {
            auto I0MuxF7 = pinBeDriven->getDriverPin()->getCell();
            if (MUXF8Macro->hasCell(I0MuxF7))
            {
                assert(I0MuxF7->getCellType() == DesignInfo::CellType_MUXF7);
                slotMapping.MuxF7[muxF8Offset][1] = I0MuxF7; // I0 is for the upper one
            }
        }
        else if (pinBeDriven->getRefPinName() == "I1")
        {
            auto I1MuxF7 = pinBeDriven->getDriverPin()->getCell();
            if (MUXF8Macro->hasCell(I1MuxF7))
            {
                assert(I1MuxF7->getCellType() == DesignInfo::CellType_MUXF7);
                slotMapping.MuxF7[muxF8Offset][0] = I1MuxF7; // I1 is for the lower one
            }
        }
    }

    // fill route-thru MuxF7
    unsigned int fillVirtualCellCnt = 0;
    for (int i = 0; i < 2; i++)
    {
        if (!slotMapping.MuxF7[muxF8Offset][i])
        {
            assert(fillVirtualCellCnt < virtualMUXs.size());
            slotMapping.MuxF7[muxF8Offset][i] = virtualMUXs[fillVirtualCellCnt];
            fillVirtualCellCnt++;
        }
    }
    assert(fillVirtualCellCnt == virtualMUXs.size());

    // fill LUTs into slots
    for (int i = 0; i < 2; i++)
    {
        auto curMuxF7 = slotMapping.MuxF7[muxF8Offset][i];
        for (DesignInfo::DesignPin *pinBeDriven : curMuxF7->getInputPins())
        {
            if (!pinBeDriven->getDriverPin())
                continue;
            if (pinBeDriven->getRefPinName() == "I0")
            {
                auto I0LUT = pinBeDriven->getDriverPin()->getCell();
                if (MUXF8Macro->hasCell(I0LUT))
                {
                    assert(I0LUT->isLUT6());
                    slotMapping.LUTs[muxF8Offset][0][i * 2 + 1] = I0LUT; // I0 is for the upper one
                }
            }
            else if (pinBeDriven->getRefPinName() == "I1")
            {
                auto I1LUT = pinBeDriven->getDriverPin()->getCell();
                if (MUXF8Macro->hasCell(I1LUT))
                {
                    assert(I1LUT->isLUT6());
                    slotMapping.LUTs[muxF8Offset][0][i * 2] = I1LUT; // I1 is for the lower one
                }
            }
        }
    }

    // fill route-thru LUTs
    fillVirtualCellCnt = 0;
    for (int i = 0; i < 4; i++)
    {
        if (!slotMapping.LUTs[muxF8Offset][0][i])
        {
            assert(fillVirtualCellCnt < virtualLUTs.size());
            slotMapping.LUTs[muxF8Offset][0][i] = virtualLUTs[fillVirtualCellCnt];
            fillVirtualCellCnt++;
        }
    }
    assert(fillVirtualCellCnt == virtualLUTs.size());

    // fill selection signal FFs
    if (3 == virtualFFs.size())
    {
        slotMapping.FFs[muxF8Offset][0][0] = virtualFFs[0];
        slotMapping.FFs[muxF8Offset][0][1] = virtualFFs[1];
        slotMapping.FFs[muxF8Offset][0][2] = virtualFFs[2];
    }
    else if (2 == virtualFFs.size())
    {
        assert(FFsInMacro.size() == 1);
        slotMapping.FFs[muxF8Offset][0][0] = virtualFFs[0];
        slotMapping.FFs[muxF8Offset][0][1] = virtualFFs[1];
        slotMapping.FFs[muxF8Offset][0][2] = FFsInMacro[0];
    }
    else
    {
        assert(false && "undefined situation");
    }

    for (auto tmpCell : MUXF8Macro->getCells())
    {
        mappedCells.insert(tmpCell);
        if (tmpCell->isLUT())
            mappedLUTs.insert(tmpCell);
        else if (tmpCell->isFF())
            mappedFFs.insert(tmpCell);
    }
}

void ParallelCLBPacker::PackingCLBSite::mapMuxF7Macro(int halfCLBOffset, PlacementInfo::PlacementMacro *MUXF7Macro)
{
    assert(halfCLBOffset < 2);
    unsigned int F7Offset = 0;
    if (slotMapping.MuxF7[halfCLBOffset][F7Offset])
        F7Offset++;
    if (slotMapping.MuxF7[halfCLBOffset][F7Offset])
    {
        std::cout << determinedClusterInSite << "\n";
        std::cout.flush();
    }
    assert(!slotMapping.MuxF7[halfCLBOffset][F7Offset]);
    slotMapping.MuxF7[halfCLBOffset][F7Offset] = MUXF7Macro->getCells()[0];
    assert(slotMapping.MuxF7[halfCLBOffset][F7Offset]->getOriCellType() == DesignInfo::CellType_MUXF7);
    auto curMUXF7 = slotMapping.MuxF7[halfCLBOffset][F7Offset];
    std::vector<DesignInfo::DesignCell *> virtualLUTs;
    std::vector<DesignInfo::DesignCell *> virtualFFs;
    std::vector<DesignInfo::DesignCell *> virtualMUXs;
    std::vector<DesignInfo::DesignCell *> LUTsInMacro;
    std::vector<DesignInfo::DesignCell *> FFsInMacro;
    std::vector<DesignInfo::DesignCell *> MUXsInMacro;
    for (auto curCell : MUXF7Macro->getCells())
    {
        if (curCell->isVirtualCell())
        {
            if (curCell->isMux())
                virtualMUXs.push_back(curCell);
            if (curCell->isFF())
                virtualFFs.push_back(curCell);
            if (curCell->isLUT())
                virtualLUTs.push_back(curCell);
        }
        else
        {
            if (curCell->isMux())
                MUXsInMacro.push_back(curCell);
            if (curCell->isFF())
                FFsInMacro.push_back(curCell);
            if (curCell->isLUT())
                LUTsInMacro.push_back(curCell);
        }
    }

    // fill LUTs into slots
    for (DesignInfo::DesignPin *pinBeDriven : curMUXF7->getInputPins())
    {
        if (!pinBeDriven->getDriverPin())
        {
            continue;
        }

        assert(pinBeDriven->getDriverPin());
        if (pinBeDriven->getRefPinName() == "I0")
        {
            auto I0LUT = pinBeDriven->getDriverPin()->getCell();
            if (MUXF7Macro->hasCell(I0LUT))
            {
                assert(I0LUT->isLUT6());
                slotMapping.LUTs[halfCLBOffset][0][F7Offset * 2 + 1] = I0LUT; // I0 is for the upper one
            }
        }
        else if (pinBeDriven->getRefPinName() == "I1")
        {
            if (!pinBeDriven->getDriverPin())
            {
                continue;
            }

            assert(pinBeDriven->getDriverPin());
            auto I1LUT = pinBeDriven->getDriverPin()->getCell();
            if (MUXF7Macro->hasCell(I1LUT))
            {
                assert(I1LUT->isLUT6());
                slotMapping.LUTs[halfCLBOffset][0][F7Offset * 2] = I1LUT; // I1 is for the lower one
            }
        }
    }

    // fill route-thru LUTs
    unsigned int fillVirtualCellCnt = 0;
    assert(virtualLUTs.size() <= 2 && "There should be no more than two Virtual LUTs");
    for (unsigned int i = F7Offset * 2; i < F7Offset * 2 + 2; i++)
    {
        if (!slotMapping.LUTs[halfCLBOffset][0][i])
        {
            assert(fillVirtualCellCnt < virtualLUTs.size());
            slotMapping.LUTs[halfCLBOffset][0][i] = virtualLUTs[fillVirtualCellCnt];
            fillVirtualCellCnt++;
        }
    }
    assert(fillVirtualCellCnt == virtualLUTs.size());

    // fill selection signal FFs
    if (1 == virtualFFs.size())
    {
        slotMapping.FFs[halfCLBOffset][0][F7Offset * 2] = virtualFFs[0];
    }
    else
    {
        assert(virtualFFs.size() <= 1 && "MUXF7 macro should only have one virtual FF.");
    }

    if (1 == FFsInMacro.size())
    {
        assert(FFsInMacro.size() == 1);
        slotMapping.FFs[halfCLBOffset][0][F7Offset * 2 + 1] = FFsInMacro[0];
    }
    else
    {
        assert(FFsInMacro.size() <= 1 && "undefined situation");
    }

    for (auto tmpCell : MUXF7Macro->getCells())
    {
        mappedCells.insert(tmpCell);
        if (tmpCell->isLUT())
            mappedLUTs.insert(tmpCell);
        else if (tmpCell->isFF())
            mappedFFs.insert(tmpCell);
    }
}

int ParallelCLBPacker::PackingCLBSite::findMuxFromHalfCLB(PlacementInfo::PlacementMacro *MUXF8Macro)
{
    assert(determinedClusterInSite);
    int resHalfCLB = -1;
    for (int i = 0; i < 4; i++)
    {
        for (auto tmpFF : determinedClusterInSite->getFFControlSets()[i].getFFs())
        {
            if (MUXF8Macro->hasCell(tmpFF))
            {
                if (resHalfCLB == -1)
                    resHalfCLB = i;
                else
                    assert(resHalfCLB == i);
            }
        }
    }
    return resHalfCLB;
}

void ParallelCLBPacker::PackingCLBSite::greedyMapMuxForCommonLUTFFInSite()
{

    std::map<DesignInfo::DesignCell *, DesignInfo::DesignCell *> FF2LUT;
    auto &singleLUTs = determinedClusterInSite->getSingleLUTs();
    auto &pairedLUTs = determinedClusterInSite->getPairedLUTs();
    for (auto &FFSet : determinedClusterInSite->getFFControlSets())
    {
        for (auto curFF : FFSet.getFFs())
        {
            PlacementInfo::PlacementMacro *pairMacro =
                dynamic_cast<PlacementInfo::PlacementMacro *>(placementInfo->getPlacementUnitByCell(curFF));
            if (pairMacro)
            {
                if (pairMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair)
                {
                    assert(pairMacro->getCells().size() == 2);
                    assert(pairMacro->getCells()[0]->isLUT());
                    assert(pairMacro->getCells()[1]->isFF());
                    FF2LUT[pairMacro->getCells()[1]] = pairMacro->getCells()[0];
                }
            }
        }
    }

    std::vector<PlacementInfo::PlacementMacro *> MUXF7Macros;
    std::vector<PlacementInfo::PlacementMacro *> MUXF8Macros;
    for (auto tmpPU : determinedClusterInSite->getPUs())
    {
        if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
        {
            if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX7)
            {
                MUXF7Macros.push_back(tmpMacro);
            }
            else if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX8)
            {
                MUXF8Macros.push_back(tmpMacro);
            }
        }
    }
    assert((MUXF7Macros.size() + 1) / 2 + MUXF8Macros.size() <= 2);

    for (unsigned int i = 0; i < MUXF8Macros.size(); i++)
    {
        assert(MUXF8Macros[i]->getCells().size() == 10);
        assert(MUXF8Macros[i]->getCells()[0]->getCellType() == DesignInfo::CellType_MUXF8);
        int targetHalfCLB = findMuxFromHalfCLB(MUXF8Macros[i]);
        assert(targetHalfCLB % 2 == 0);
        mapMuxF8Macro(targetHalfCLB / 2, MUXF8Macros[i]);
    }

    for (unsigned int i = 0; i < MUXF7Macros.size(); i++)
    {
        assert(MUXF7Macros[i]->getCells().size() >= 4 && MUXF7Macros[i]->getCells().size() <= 5);
        assert(MUXF7Macros[i]->getCells()[0]->getCellType() == DesignInfo::CellType_MUXF7);
        int targetHalfCLB = findMuxFromHalfCLB(MUXF7Macros[i]);
        assert(targetHalfCLB % 2 == 0);
        mapMuxF7Macro(targetHalfCLB / 2, MUXF7Macros[i]);
    }

    assert(determinedClusterInSite->getSingleLUTs().size() + determinedClusterInSite->getPairedLUTs().size() <= 8);
    // map LUT connected to carry and their paired LUT
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.LUTs[i][j][k])
                {
                    if (slotMapping.LUTs[i][1 - j][k])
                    {
                        for (auto pair : determinedClusterInSite->getPairedLUTs())
                        {
                            if (pair.first == slotMapping.LUTs[i][1 - j][k])
                            {
                                slotMapping.LUTs[i][1 - j][k] = pair.first;
                                slotMapping.LUTs[i][j][k] = pair.second;
                                mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                break;
                            }
                            if (pair.second == slotMapping.LUTs[i][1 - j][k])
                            {
                                slotMapping.LUTs[i][1 - j][k] = pair.second;
                                slotMapping.LUTs[i][j][k] = pair.first;
                                mappedLUTs.insert(slotMapping.LUTs[i][j][k]);
                                mappedLUTs.insert(slotMapping.LUTs[i][1 - j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][j][k]);
                                mappedCells.insert(slotMapping.LUTs[i][1 - j][k]);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // mapped paired LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto pair : determinedClusterInSite->getPairedLUTs())
                {
                    if (mappedLUTs.find(pair.first) == mappedLUTs.end())
                    {
                        if (mappedLUTs.find(pair.second) != mappedLUTs.end())
                        {
                            for (int i = 0; i < 2; i++)
                            {
                                for (int j = 0; j < 2; j++)
                                {
                                    for (int k = 0; k < 4; k++)
                                    {
                                        if (slotMapping.LUTs[i][j][k])
                                        {
                                            std::cout << "i,j,k:" << i << "," << j << "," << k << "   "
                                                      << slotMapping.LUTs[i][j][k] << "\n";
                                        }
                                    }
                                }
                            }
                            determinedClusterInSite->printMyself();
                            std::cout.flush();
                        }
                        assert(mappedLUTs.find(pair.second) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = pair.first;
                        slotMapping.LUTs[i][1][k] = pair.second;
                        mappedLUTs.insert(slotMapping.LUTs[i][0][k]);
                        mappedLUTs.insert(slotMapping.LUTs[i][1][k]);
                        mappedCells.insert(slotMapping.LUTs[i][0][k]);
                        mappedCells.insert(slotMapping.LUTs[i][1][k]);
                        break;
                    }
                }
            }
        }
    }

    // mapped single LUTs
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (!slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                for (auto tmpLUT : determinedClusterInSite->getSingleLUTs())
                {
                    if (mappedLUTs.find(tmpLUT) == mappedLUTs.end())
                    {
                        assert(mappedLUTs.find(tmpLUT) == mappedLUTs.end());
                        slotMapping.LUTs[i][0][k] = tmpLUT;
                        mappedLUTs.insert(tmpLUT);
                        mappedCells.insert(tmpLUT);
                        break;
                    }
                }
            }
        }
    }

    // mapped FFs
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (!slotMapping.FFs[i][j][k])
                {
                    int halfCLBId = i * 2 + j;
                    auto &CSFF = determinedClusterInSite->getFFControlSets()[halfCLBId];
                    for (auto tmpFF : CSFF.getFFs())
                    {
                        if (mappedFFs.find(tmpFF) == mappedFFs.end())
                        {
                            mappedFFs.insert(tmpFF);
                            mappedCells.insert(tmpFF);
                            slotMapping.FFs[i][j][k] = tmpFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    assert(determinedClusterInSite->getSingleLUTs().size() + determinedClusterInSite->getPairedLUTs().size() <= 8);

    for (int i0 = 0; i0 < 2; i0++)
    {
        for (int k0 = 0; k0 < 4; k0++)
        {
            if (!isLUT6(slotMapping.LUTs[i0][0][k0]) && !isLUT6(slotMapping.LUTs[i0][1][k0]))
            {
                int oriDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][0][k0]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][1][k0]);

                // switch locations
                int newDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][1][k0]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][0][k0]);

                if (oriDirectInternalRouteNum < newDirectInternalRouteNum)
                {
                    DesignInfo::DesignCell *tmpLUT;
                    tmpLUT = slotMapping.LUTs[i0][0][k0];
                    slotMapping.LUTs[i0][0][k0] = slotMapping.LUTs[i0][1][k0];
                    slotMapping.LUTs[i0][1][k0] = tmpLUT;
                    assert(!isLUT6(tmpLUT));
                }
            }
        }
    }

    int LUTSwapOptions[4][4] = {{0, 1, 0, 1}, {1, 0, 0, 1}, {1, 0, 1, 0}, {0, 1, 1, 0}};

    for (int i0 = 0; i0 < 2; i0++)
    {
        for (int k0 = 0; k0 < 4; k0++)
        {
            for (int i1 = 0; i1 < 2; i1++)
            {
                for (int k1 = 0; k1 < 4; k1++)
                {
                    if (i0 == i1 && k0 == k1)
                    {
                        continue;
                    }
                    if ((isMuxMacro(slotMapping.LUTs[i0][0][k0]) || isMuxMacro(slotMapping.FFs[i0][0][k0]) ||
                         isMuxMacro(slotMapping.LUTs[i0][1][k0]) || isMuxMacro(slotMapping.FFs[i0][1][k0]) ||
                         isMuxMacro(slotMapping.LUTs[i1][0][k1]) || isMuxMacro(slotMapping.FFs[i1][0][k1]) ||
                         isMuxMacro(slotMapping.LUTs[i1][1][k1]) || isMuxMacro(slotMapping.FFs[i1][1][k1])))
                        continue;
                    int oriDirectInternalRouteNum =
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][0][k0], slotMapping.FFs[i0][0][k0]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][1][k0], slotMapping.FFs[i0][1][k0]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][0][k1], slotMapping.FFs[i1][0][k1]) +
                        checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][1][k1], slotMapping.FFs[i1][1][k1]);

                    int optDirectInternalRouteNum = -1;
                    int optimalOption = -1;
                    for (int optionId = 0; optionId < 4; optionId++)
                    {
                        // switch locations
                        int newDirectInternalRouteNum =
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][LUTSwapOptions[optionId][0]][k1],
                                                    slotMapping.FFs[i0][0][k0]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i1][LUTSwapOptions[optionId][1]][k1],
                                                    slotMapping.FFs[i0][1][k0]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][LUTSwapOptions[optionId][2]][k0],
                                                    slotMapping.FFs[i1][0][k1]) +
                            checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i0][LUTSwapOptions[optionId][3]][k0],
                                                    slotMapping.FFs[i1][1][k1]);
                        if (isLUT6(slotMapping.LUTs[i1][LUTSwapOptions[optionId][1]][k1]) ||
                            isLUT6(slotMapping
                                       .LUTs[i0][LUTSwapOptions[optionId][3]][k0])) // illegal to put LUT6 at LUT5 slot
                            continue;
                        if (newDirectInternalRouteNum > optDirectInternalRouteNum)
                        {
                            optimalOption = optionId;
                            optDirectInternalRouteNum = newDirectInternalRouteNum;
                        }
                    }

                    if (oriDirectInternalRouteNum < optDirectInternalRouteNum)
                    {
                        DesignInfo::DesignCell *tmpLUT0 = slotMapping.LUTs[i1][LUTSwapOptions[optimalOption][0]][k1];
                        DesignInfo::DesignCell *tmpLUT1 = slotMapping.LUTs[i1][LUTSwapOptions[optimalOption][1]][k1];
                        DesignInfo::DesignCell *tmpLUT2 = slotMapping.LUTs[i0][LUTSwapOptions[optimalOption][2]][k0];
                        DesignInfo::DesignCell *tmpLUT3 = slotMapping.LUTs[i0][LUTSwapOptions[optimalOption][3]][k0];
                        slotMapping.LUTs[i0][0][k0] = tmpLUT0;
                        assert(!isLUT6(tmpLUT1));
                        slotMapping.LUTs[i0][1][k0] = tmpLUT1;
                        slotMapping.LUTs[i1][0][k1] = tmpLUT2;
                        assert(!isLUT6(tmpLUT3));
                        slotMapping.LUTs[i1][1][k1] = tmpLUT3;
                        continue;
                    }
                }
            }
        }
    }

    int FFSwapOption[24][4] = {{0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
                               {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
                               {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 0, 1}, {2, 3, 1, 0},
                               {3, 0, 1, 2}, {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}};

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if ((isMuxMacro(slotMapping.FFs[i][j][0]) || isMuxMacro(slotMapping.FFs[i][j][1]) ||
                 isMuxMacro(slotMapping.FFs[i][j][2]) || isMuxMacro(slotMapping.FFs[i][j][3])))
                continue;
            int oriDirectInternalRouteNum =
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][0], slotMapping.FFs[i][j][0]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][1], slotMapping.FFs[i][j][1]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][2], slotMapping.FFs[i][j][2]) +
                checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][3], slotMapping.FFs[i][j][3]);
            int optDirectInternalRouteNum = -1;
            int optimalOption = -1;
            for (int optionId = 0; optionId < 24; optionId++)
            {
                int newDirectInternalRouteNum =
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][0],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][0]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][1],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][1]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][2],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][2]]) +
                    checkDirectLUTFFConnect(FF2LUT, slotMapping.LUTs[i][j][3],
                                            slotMapping.FFs[i][j][FFSwapOption[optionId][3]]);
                if (newDirectInternalRouteNum > optDirectInternalRouteNum)
                {
                    optimalOption = optionId;
                    optDirectInternalRouteNum = newDirectInternalRouteNum;
                }
            }
            if (oriDirectInternalRouteNum < optDirectInternalRouteNum)
            {
                DesignInfo::DesignCell *tmpFF0 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][0]];
                DesignInfo::DesignCell *tmpFF1 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][1]];
                DesignInfo::DesignCell *tmpFF2 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][2]];
                DesignInfo::DesignCell *tmpFF3 = slotMapping.FFs[i][j][FFSwapOption[optimalOption][3]];
                slotMapping.FFs[i][j][0] = tmpFF0;
                slotMapping.FFs[i][j][1] = tmpFF1;
                slotMapping.FFs[i][j][2] = tmpFF2;
                slotMapping.FFs[i][j][3] = tmpFF3;
                continue;
            }
        }
    }

    if (determinedClusterInSite)
    {

        unsigned int FFCnt = 0;
        for (auto &CSFF : determinedClusterInSite->getFFControlSets())
            FFCnt += CSFF.getFFs().size();

        if (FFCnt != mappedFFs.size())
        {
            std::cout << "FFCnt: " << FFCnt << " mappedFFs.size():" << mappedFFs.size() << "\n";
            determinedClusterInSite->printMyself();
            for (int i = 0; i < 2; i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        std::cout << "i,k,j:" << i << "," << k << "," << j << ":\n";
                        if (slotMapping.LUTs[i][j][k])
                            std::cout << slotMapping.LUTs[i][j][k] << "\n";
                        if (slotMapping.FFs[i][j][k])
                            std::cout << slotMapping.FFs[i][j][k] << "\n";
                    }
                }
            }
            assert(FFCnt == mappedFFs.size());
        }
    }
}

void ParallelCLBPacker::PackingCLBSite::finalMapToSlotsForCommonLUTFFInSite()
{
    assert(!checkIsPrePackedSite() && !checkIsMuxSite());
    assert(fixedPairedLUTs.size() == 0 && conflictLUTs.size() == 0);
    assert(determinedClusterInSite->getSingleLUTs().size() + determinedClusterInSite->getPairedLUTs().size() <= 8);

    greedyMapForCommonLUTFFInSite();
}

void ParallelCLBPacker::PackingCLBSite::mapLUTRAMRelatedCellsToSlots(PlacementInfo::PlacementMacro *_LUTRAMMacro)
{
    LUTRAMMacro = _LUTRAMMacro;
    isLUTRAMSite = true;
}

void ParallelCLBPacker::PackingCLBSite::mapCarryRelatedCellsToSlots(PlacementInfo::PlacementMacro *_CARRYChain,
                                                                    float siteOffset)
{
    isCarrySite = true;
    CARRYChain = _CARRYChain;
    CARRYChainSiteOffset = siteOffset;

    mappedCells.clear();
    mappedLUTs.clear();
    mappedFFs.clear();

    float siteHeight = 1.0;
    float lowerBound = siteHeight * CARRYChainSiteOffset - (1e-2);
    float uppwerBound = siteHeight * CARRYChainSiteOffset + (1e-2);
    for (auto curCell : CARRYChain->getCells())
    {
        float offsetY = CARRYChain->getCellOffsetYInMacro(curCell);
        if (offsetY > lowerBound && offsetY < uppwerBound)
        {
            if (curCell->isCarry())
            {
                auto curCarry = curCell;
                slotMapping.Carry = curCarry;
                mappedCells.insert(curCarry);
                for (DesignInfo::DesignPin *pinBeDriven : curCarry->getInputPins())
                {
                    if (pinBeDriven->isUnconnected())
                        continue;
                    if (!pinBeDriven->getDriverPin()) // pin connect to GND/VCC which has no specifc driver pin
                        continue;

                    if (pinBeDriven->getRefPinName().find("S[") == 0)
                    {
                        if (placementInfo->getPlacementUnitByCell(pinBeDriven->getDriverPin()->getCell()) ==
                                CARRYChain &&
                            pinBeDriven->getDriverPin()->getCell()->isLUT())
                        {
                            char SPinCellId =
                                pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                            slotMapping.LUTs[SPinCellId / 4][0][SPinCellId % 4] =
                                pinBeDriven->getDriverPin()->getCell();
                            mappedCells.insert(pinBeDriven->getDriverPin()->getCell());
                            mappedLUTs.insert(pinBeDriven->getDriverPin()->getCell());
                            // char LUTCode = SPinCellId + 'A';
                            // std::string LUTSiteName = std::string(1, LUTCode) + "6LUT";
                            // outfile0 << "  " << pinBeDriven->getDriverPin()->getCell()->getName() << " "
                            //          << CLBSite->getName() << "/" + LUTSiteName << "\n";
                        }
                    }
                    else if (pinBeDriven->getRefPinName().find("DI[") == 0)
                    {
                        if (placementInfo->getPlacementUnitByCell(pinBeDriven->getDriverPin()->getCell()) ==
                                CARRYChain &&
                            pinBeDriven->getDriverPin()->getCell()->isLUT())
                        {
                            char DIPinCellId =
                                pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                            slotMapping.LUTs[DIPinCellId / 4][1][DIPinCellId % 4] =
                                pinBeDriven->getDriverPin()->getCell();
                            mappedCells.insert(pinBeDriven->getDriverPin()->getCell());
                            mappedLUTs.insert(pinBeDriven->getDriverPin()->getCell());
                            // char LUTCode = DIPinCellId + 'A';
                            // std::string LUTSiteName = std::string(1, LUTCode) + "5LUT";
                            // outfile0 << "  " << pinBeDriven->getDriverPin()->getCell()->getName() << " "
                            //          << CLBSite->getName() << "/" + LUTSiteName << "\n";
                        }
                    }
                }
                std::vector<std::string> checkFFRefPins{"O[", "CO["};
                for (DesignInfo::DesignPin *driverPin : curCarry->getOutputPins())
                {
                    if (driverPin->isUnconnected())
                        continue;
                    DesignInfo::DesignNet *curOutputNet = driverPin->getNet();
                    bool findMatchedInputPin = false;
                    for (auto patternPin : checkFFRefPins)
                    {
                        if (driverPin->getRefPinName().find(patternPin) == 0)
                        {
                            findMatchedInputPin = true;
                            break;
                        }
                    }

                    if (findMatchedInputPin)
                    {
                        int FFcnt = 0;
                        DesignInfo::DesignCell *theFF = nullptr;
                        for (auto pinBeDriven : curOutputNet->getPinsBeDriven())
                        {
                            if (pinBeDriven->getCell()->isFF())
                            {
                                FFcnt++;
                                if (pinBeDriven->getRefPinName().find("D") != std::string::npos)
                                {
                                    if (placementInfo->getPlacementUnitByCell(pinBeDriven->getCell()) == CARRYChain)
                                        theFF = pinBeDriven->getCell();
                                }
                            }
                        }
                        if (FFcnt == 1 && theFF)
                        {
                            char FFPinCellId =
                                driverPin->getRefPinName()[driverPin->getRefPinName().find("[") + 1] - '0';
                            if (driverPin->getRefPinName().find("CO[") != std::string::npos)
                            {
                                assert(!slotMapping.FFs[FFPinCellId / 4][1][FFPinCellId % 4]);
                                slotMapping.FFs[FFPinCellId / 4][1][FFPinCellId % 4] = theFF;
                                mappedCells.insert(theFF);
                                mappedFFs.insert(theFF);
                                // std::string FFSiteName = std::string(1, FFCode) + "FF2";
                                // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                                //          << "\n";
                            }
                            else if (driverPin->getRefPinName().find("O[") != std::string::npos)
                            {
                                assert(!slotMapping.FFs[FFPinCellId / 4][0][FFPinCellId % 4]);
                                slotMapping.FFs[FFPinCellId / 4][0][FFPinCellId % 4] = theFF;
                                mappedCells.insert(theFF);
                                mappedFFs.insert(theFF);
                                // std::string FFSiteName = std::string(1, FFCode) + "FF";
                                // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                                //          << "\n";
                            }
                        }
                    }
                }
            }

            if (curCell->isVirtualCell())
            {
                assert(mappedCells.find(curCell) == mappedCells.end());
                if (curCell->isLUT())
                {
                    assert(curCell->isLUT6());
                    assert(curCell->getName().find('(') != std::string::npos);
                    int slotId = curCell->getName()[curCell->getName().find('(') - 1] - '0';
                    assert(slotId < 8 && slotId >= 0);
                    assert(!slotMapping.LUTs[slotId / 4][0][slotId % 4]);
                    mappedCells.insert(curCell);
                    mappedLUTs.insert(curCell);
                    slotMapping.LUTs[slotId / 4][0][slotId % 4] = curCell;
                }
                else
                {
                    assert(curCell->isFF());
                    assert(curCell->getName().find('(') != std::string::npos);
                    int strOffset = curCell->getName().find('(');
                    int slotId = curCell->getName()[strOffset - 1] - '0';
                    assert(curCell->getName()[strOffset - 2] == '2' || curCell->getName()[strOffset - 2] == 'F');
                    int oddCLB = curCell->getName()[strOffset - 2] == '2';
                    assert(slotId < 8 && slotId >= 0);
                    assert(!slotMapping.FFs[slotId / 4][oddCLB][slotId % 4]);
                    mappedCells.insert(curCell);
                    mappedFFs.insert(curCell);
                    slotMapping.FFs[slotId / 4][oddCLB][slotId % 4] = curCell;
                }
            }
        }
    }

    fixedPairedLUTs.clear();
    conflictLUTs.clear();
    fixedLUTsInPairs.clear();

    for (int i = 0; i < 2; i++)
        for (int k = 0; k < 4; k++)
        {
            if (slotMapping.LUTs[i][0][k] && !slotMapping.LUTs[i][1][k])
            {
                conflictLUTs.insert(slotMapping.LUTs[i][0][k]);
            }
            else if (slotMapping.LUTs[i][1][k] && !slotMapping.LUTs[i][0][k])
            {
                conflictLUTs.insert(slotMapping.LUTs[i][1][k]);
            }
            else if (slotMapping.LUTs[i][1][k] && slotMapping.LUTs[i][0][k])
            {
                if (slotMapping.LUTs[i][1][k]->getCellId() < slotMapping.LUTs[i][0][k]->getCellId())
                {
                    fixedPairedLUTs.emplace(slotMapping.LUTs[i][1][k], slotMapping.LUTs[i][0][k]);
                    fixedLUTsInPairs.insert(slotMapping.LUTs[i][1][k]);
                    fixedLUTsInPairs.insert(slotMapping.LUTs[i][0][k]);
                }
                else
                {
                    fixedPairedLUTs.emplace(slotMapping.LUTs[i][0][k], slotMapping.LUTs[i][1][k]);
                    fixedLUTsInPairs.insert(slotMapping.LUTs[i][1][k]);
                    fixedLUTsInPairs.insert(slotMapping.LUTs[i][0][k]);
                }
            }
        }
}