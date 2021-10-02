/**
 * @file GeneralSpreader.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This implementation file contains APIs' implementation of the GeneralSpreader which accounts for the cell
 * spreading, which controls the cell density of specific resource type, under the device constraints for specific
 * regions.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "GeneralSpreader.h"

#include <cmath>
#include <omp.h>
#include <queue>
#include <thread>

GeneralSpreader::GeneralSpreader(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                                 std::string &sharedCellType, int currentIteration, float capacityShrinkRatio,
                                 bool verbose)
    : placementInfo(placementInfo), JSONCfg(JSONCfg), sharedCellType(sharedCellType),
      currentIteration(currentIteration), capacityShrinkRatio(capacityShrinkRatio), verbose(verbose),
      binGrid(placementInfo->getBinGrid(placementInfo->getSharedBELTypeId(sharedCellType)))
{
    overflowBins.clear();
    if (JSONCfg.find("jobs") != JSONCfg.end())
    {
        nJobs = std::stoi(JSONCfg["jobs"]);
    }
    if (JSONCfg.find("SpreaderSimpleExpland") != JSONCfg.end())
    {
        useSimpleExpland = JSONCfg["SpreaderSimpleExpland"] == "true";
    }
}

void GeneralSpreader::spreadPlacementUnits(float forgetRatio)
{
    if (verbose) // usually commented for debug
        print_status("GeneralSpreader: starts to spreadPlacementUnits for type: [" + sharedCellType + "]");

    std::deque<int> overflowBinNumQ;

    int loopCnt = 0;
    // int nthreads = omp_get_num_threads();
    // float totalBinNum = binGrid.size() * binGrid[0].size();

    std::vector<int> historyTotalCellNum(0);
    while (true)
    {
        if (loopCnt % 20 == 0)
        {
            for (auto &row : binGrid)
            {
                for (auto curBin : row)
                {
                    curBin->resetBinShrinkRatio();
                    curBin->resetNoOverflowCounter();
                    curBin->resetOverflowCounter();
                }
            }
        }
        loopCnt++;
        if (JSONCfg.find("DumpLUTFFCoordTrace-GeneralSpreader") != JSONCfg.end())
        {
            std::string dumpFile = JSONCfg["DumpLUTFFCoordTrace-GeneralSpreader"];
        }

        if (verbose)
            print_status("GeneralSpreader: finding overflow regions");
        findOverflowBins(capacityShrinkRatio);
        if (overflowBins.size() == 0)
            break;
        int totalCellNum = 0;
        for (auto curBin : overflowBins)
            totalCellNum += curBin->getCells().size();
        if (loopCnt > 400)
        {
            useSimpleExpland = true;
        }
        if (loopCnt > 1000)
        {
            print_info("found " + std::to_string(overflowBins.size()) + " overflowed bins");
            print_info("found " + std::to_string(totalCellNum) + " cells in them");
            print_warning("failed to solve the overflow bins with better result.");
            print_info("spread for " + std::to_string(loopCnt) + " iterations");
            break;
        }
        historyTotalCellNum.push_back(totalCellNum);
        if (historyTotalCellNum.size() > 20)
        {
            float improved = std::fabs((historyTotalCellNum[historyTotalCellNum.size() - 1] +
                                        historyTotalCellNum[historyTotalCellNum.size() - 2] +
                                        historyTotalCellNum[historyTotalCellNum.size() - 3]) /
                                           3 -
                                       (historyTotalCellNum[historyTotalCellNum.size() - 20] +
                                        historyTotalCellNum[historyTotalCellNum.size() - 19] +
                                        historyTotalCellNum[historyTotalCellNum.size() - 18]) /
                                           3);
            if ((improved < 30 && historyTotalCellNum[historyTotalCellNum.size() - 1] < 1000 && loopCnt > 300) ||
                loopCnt > 500)
            {
                break;
            }
            improved /= (float)historyTotalCellNum[historyTotalCellNum.size() - 1];
            if (improved < 0.01)
            {
                print_info("found " + std::to_string(overflowBins.size()) + " overflowed bins");
                print_info("found " + std::to_string(totalCellNum) + " cells in them");
                print_info("spread for " + std::to_string(loopCnt) + " iterations");
                break;
            }
        }
        if (verbose) // usually commented for debug
        {
            print_info("found " + std::to_string(overflowBins.size()) + " overflowed bins");
            print_info("found " + std::to_string(totalCellNum) + " cells in them");
            print_info("spread for " + std::to_string(loopCnt) + " iterations");
        }
        coveredBinSet.clear();
        expandedRegions.clear();
        std::set<PlacementInfo::PlacementUnit *> involvedPUs;
        std::set<DesignInfo::DesignCell *> involvedCells;
        std::vector<PlacementInfo::PlacementUnit *> involvedPUVec;
        involvedPUVec.clear();
        involvedPUs.clear();
        involvedCells.clear();
        for (auto curBin : overflowBins)
        {
            if (coveredBinSet.find(curBin) != coveredBinSet.end())
                continue;
            GeneralSpreader::SpreadRegion *newRegion = expandFromABin(curBin, capacityShrinkRatio);
            coveredBinSet.insert(curBin);
            // if (loopCnt % 20 == 0)
            // {
            //     std::cout.flush();
            //     for (auto curbin0 : newRegion->getBinsInRegion())
            //     {
            //         std::cout << "-----------------\n";
            //         std::cout << curbin0 << " sharedBELStr:" << curbin0->getSharedCellType() << " X: " <<
            //         curbin0->X()
            //                   << " Y: " << curbin0->Y() << " left:" << curbin0->left() << " right:" <<
            //                   curbin0->right()
            //                   << " top:" << curbin0->top() << " bottom:" << curbin0->bottom() << "\n";
            //         std::cout << "shrinkRatio:" << curbin0->getBinShrinkRatio() << "\n";
            //         std::cout << "capacity:" << curbin0->getCapacity() << "\n";
            //         std::cout << "util:" << curbin0->getUtilization() << "\n";
            //     }
            //     std::cout << "\n\n";
            // }
            bool overlappedWithPreviousRegion = false;
            for (auto curRegion : expandedRegions)
            {
                if (curRegion->isRegionOverlap(newRegion))
                {
                    overlappedWithPreviousRegion = true;
                    std::cout << "newRegion: "
                              << " left:" << newRegion->left() << " right:" << newRegion->right()
                              << " top:" << newRegion->top() << " bottom:" << newRegion->bottom() << "\n";
                    std::cout << "====================================================\n";
                    for (auto curbin0 : newRegion->getBinsInRegion())
                    {
                        std::cout << "-----------------\n";
                        std::cout << curbin0 << " sharedBELStr:" << curbin0->getSharedCellType()
                                  << " X: " << curbin0->X() << " Y: " << curbin0->Y() << " left:" << curbin0->left()
                                  << " right:" << curbin0->right() << " top:" << curbin0->top()
                                  << " bottom:" << curbin0->bottom() << "\n";
                    }
                    std::cout << "\n\n\nexistingRegion: "
                              << " left:" << curRegion->left() << " right:" << curRegion->right()
                              << " top:" << curRegion->top() << " bottom:" << curRegion->bottom() << "\n";
                    std::cout << "====================================================\n";
                    for (auto curbin0 : curRegion->getBinsInRegion())
                    {
                        std::cout << "-----------------\n";
                        std::cout << curbin0 << " sharedBELStr:" << curbin0->getSharedCellType()
                                  << " X: " << curbin0->X() << " Y: " << curbin0->Y() << " left:" << curbin0->left()
                                  << " right:" << curbin0->right() << " top:" << curbin0->top()
                                  << " bottom:" << curbin0->bottom() << "\n";
                    }
                    break;
                }
            }
            if (!overlappedWithPreviousRegion)
            {
                coveredBinSet.insert(curBin);
                expandedRegions.push_back(newRegion);
            }
            else
            {
                assert(false && "should not overlap");
                delete newRegion;
            }
        }

        if (nJobs > 4)
            omp_set_num_threads(
                4); // we don't need that high parallelism here since regionNum will be small in later iterations
#pragma omp parallel sections
        {
#pragma omp section
            {

                if (verbose) // usually commented for debug
                    print_status("involved regions cover " + std::to_string(coveredBinSet.size()) + " bins.");
                if (verbose)
                {
                    print_status("GeneralSpreader: spreading cells in the regions");
                }

                int regionNum = expandedRegions.size();
                if (coveredBinSet.size() > 256 && regionNum > 32)
                {
#pragma omp parallel for schedule(dynamic, 4)
                    for (int regionId = 0; regionId < regionNum; regionId++)
                    {
                        GeneralSpreader::SpreadRegion *curRegion = expandedRegions[regionId];
                        assert(curRegion);
                        assert(curRegion->getCells().size() > 0);
                        SpreadRegion::SubBox *newBox =
                            new SpreadRegion::SubBox(placementInfo, curRegion, binGrid, capacityShrinkRatio, 100, true);
                        newBox->spreadAndPartition();
                        delete newBox;
                    }
                }
                else
                {
                    for (int regionId = 0; regionId < regionNum; regionId++)
                    {
                        GeneralSpreader::SpreadRegion *curRegion = expandedRegions[regionId];
                        assert(curRegion);
                        assert(curRegion->getCells().size() > 0);
                        SpreadRegion::SubBox *newBox =
                            new SpreadRegion::SubBox(placementInfo, curRegion, binGrid, capacityShrinkRatio, 100, true);
                        newBox->spreadAndPartition();
                        delete newBox;
                    }
                }
            }
#pragma omp section
            {
                if (verbose)
                    print_status("GeneralSpreader: loading involved placement units");

                for (auto curRegion : expandedRegions)
                {
                    assert(curRegion);
                    assert(curRegion->getCells().size() > 0);
                    for (auto curCell : curRegion->getCells())
                    {
                        if (involvedCells.find(curCell) == involvedCells.end())
                        {
                            auto tmpPU = placementInfo->getPlacementUnitByCell(curCell);
                            if (involvedPUs.find(tmpPU) == involvedPUs.end())
                            {
                                involvedPUs.insert(tmpPU);
                                involvedPUVec.push_back(tmpPU);
                            }
                            involvedCells.insert(curCell);
                        }
                    }
                }
            }
        }

        omp_set_num_threads(nJobs);

        for (auto curRegion : expandedRegions)
        {
            delete curRegion;
        }

        if (verbose)
            print_status("GeneralSpreader: updating Placement Units With Spreaded Cell Locations");
        updatePlacementUnitsWithSpreadedCellLocations(involvedPUs, involvedCells, involvedPUVec, forgetRatio);
        if (verbose)
            print_status("GeneralSpreader: updated Placement Units With Spreaded Cell Locations");
        dumpLUTFFCoordinate();
    }

    if (verbose)
        print_status("GeneralSpreader: processed all overflow regions");

    recordSpreadedCellLocations();

    if (JSONCfg.find("Dump Cell Density") != JSONCfg.end())
    {
        dumpSiteGridDensity(JSONCfg["Dump Cell Density"]);
    }
    dumpLUTFFCoordinate();

    print_status("GeneralSpreader: accomplished spreadPlacementUnits for type: [" + sharedCellType + "]");
}

bool siteSortCmp(PlacementInfo::PlacementBinInfo *a, PlacementInfo::PlacementBinInfo *b)
{
    return (a->getUtilizationRate() > b->getUtilizationRate());
}

void GeneralSpreader::findOverflowBins(float overflowThreshold)
{
    overflowBins.clear();
    overflowBinSet.clear();
    for (auto &row : binGrid)
    {
        for (auto curBin : row)
        {
            if (curBin->isOverflow(overflowThreshold))
            {
                overflowBins.push_back(curBin);
                overflowBinSet.insert(curBin);
                curBin->countOverflow();
                if (curBin->getOverflowCounter() > 5)
                {
                    if (curBin->getBinShrinkRatio() > 0.8)
                    {
                        curBin->shrinkBinBy(0.015);
                    }
                    else
                    {
                        curBin->resetBinShrinkRatio();
                    }
                    curBin->resetOverflowCounter();
                }
            }
            else
            {
                curBin->countNoOverflow();
                curBin->resetOverflowCounter();
                if (curBin->getNoOverflowCounter() > 5)
                {
                    curBin->resetBinShrinkRatio();
                }
            }
        }
    }
    sort(overflowBins.begin(), overflowBins.end(), siteSortCmp);
}

void GeneralSpreader::updatePlacementUnitsWithSpreadedCellLocationsWorker(
    PlacementInfo *placementInfo, std::set<PlacementInfo::PlacementUnit *> &involvedPUs,
    std::set<DesignInfo::DesignCell *> &involvedCells, std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec,
    float forgetRatio, int startId, int endId)
{
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
    for (int curPUID = startId; curPUID < endId; curPUID++)
    {
        assert((unsigned int)curPUID < involvedPUVec.size());
        auto curPU = involvedPUVec[curPUID];
        if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
        {
            float cellX = curUnpackedCell->X();
            float cellY = curUnpackedCell->Y();
            DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
            if (curPU->isFixed() || curPU->isLocked())
            {
                cellLoc[curCell->getCellId()].X = cellX;
                cellLoc[curCell->getCellId()].Y = cellY;
            }
            else
            {
                makeCellInLegalArea(placementInfo, cellLoc[curCell->getCellId()].X, cellLoc[curCell->getCellId()].Y);
                curPU->setSpreadLocation(cellLoc[curCell->getCellId()].X, cellLoc[curCell->getCellId()].Y, forgetRatio);
                placementInfo->transferCellBinInfo(curCell->getCellId(), curPU->X(), curPU->Y());
                cellLoc[curCell->getCellId()].X = curPU->X();
                cellLoc[curCell->getCellId()].Y = curPU->Y();
            }
        }
        else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
        {
            if (curPU->isFixed() || curPU->isLocked())
            {
                for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                {
                    float offsetX_InMacro, offsetY_InMacro;
                    DesignInfo::DesignCellType cellType;
                    curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);

                    float cellX = curMacro->X() + offsetX_InMacro;
                    float cellY = curMacro->Y() + offsetY_InMacro;

                    DesignInfo::DesignCell *curCell = curMacro->getCell(vId);

                    cellLoc[curCell->getCellId()].X = cellX;
                    cellLoc[curCell->getCellId()].Y = cellY;
                }
            }
            else
            {
                double tmpTotalX = 0.0;
                double tmpTotalY = 0.0;

                int numCellsInvolvedInSpreading = 0;
                for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                {
                    DesignInfo::DesignCell *curCell = curMacro->getCell(vId);
                    if (involvedCells.find(curCell) != involvedCells.end())
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        makeCellInLegalArea(placementInfo, cellLoc[curCell->getCellId()].X,
                                            cellLoc[curCell->getCellId()].Y);
                        tmpTotalX += cellLoc[curCell->getCellId()].X - offsetX_InMacro;
                        tmpTotalY += cellLoc[curCell->getCellId()].Y - offsetY_InMacro;
                        numCellsInvolvedInSpreading++;
                    }
                }
                tmpTotalX /= (double)numCellsInvolvedInSpreading;
                tmpTotalY /= (double)numCellsInvolvedInSpreading;

                float curNewPUX = tmpTotalX;
                float curNewPUY = tmpTotalY;
                placementInfo->legalizeXYInArea(curPU, curNewPUX, curNewPUY);
                curPU->setSpreadLocation(curNewPUX, curNewPUY, forgetRatio);
                placementInfo->enforceLegalizeXYInArea(curPU);
                for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                {
                    float offsetX_InMacro, offsetY_InMacro;
                    DesignInfo::DesignCellType cellType;
                    curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);

                    float cellX = curMacro->X() + offsetX_InMacro;
                    float cellY = curMacro->Y() + offsetY_InMacro;

                    DesignInfo::DesignCell *curCell = curMacro->getCell(vId);
                    placementInfo->transferCellBinInfo(curCell->getCellId(), cellX, cellY);
                    cellLoc[curCell->getCellId()].X = cellX;
                    cellLoc[curCell->getCellId()].Y = cellY;
                }
            }
        }
    }
}

void GeneralSpreader::updatePlacementUnitsWithSpreadedCellLocations(
    std::set<PlacementInfo::PlacementUnit *> &involvedPUs, std::set<DesignInfo::DesignCell *> &involvedCells,
    std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec, float forgetRatio)
{
    if (involvedPUs.size() > 100)
    {
        std::vector<std::thread *> threadsVec;
        threadsVec.clear();
        int eachThreadPUNum = involvedPUVec.size() / nJobs;
        std::vector<std::pair<int, int>> startEndPairs;
        startEndPairs.clear();

        for (int threadId = 0, startId = 0, endId = eachThreadPUNum; threadId < nJobs;
             threadId++, startId += eachThreadPUNum, endId += eachThreadPUNum)
        {
            if (threadId == nJobs - 1)
            {
                endId = involvedPUVec.size();
            }
            startEndPairs.emplace_back(startId, endId);
        }
        for (int threadId = 0; threadId < nJobs; threadId++)
        {
            std::thread *newThread = new std::thread(
                GeneralSpreader::updatePlacementUnitsWithSpreadedCellLocationsWorker, placementInfo,
                std::ref(involvedPUs), std::ref(involvedCells), std::ref(involvedPUVec), std::ref(forgetRatio),
                std::ref(startEndPairs[threadId].first), std::ref(startEndPairs[threadId].second));
            threadsVec.push_back(newThread);
        }
        for (int threadId = 0; threadId < nJobs; threadId++)
        {
            threadsVec[threadId]->join();
            delete threadsVec[threadId];
        }
    }
    else
    {
        std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
        for (auto curPU : involvedPUVec)
        {
            if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
            {
                float cellX = curUnpackedCell->X();
                float cellY = curUnpackedCell->Y();
                DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                if (curPU->isFixed() || curPU->isLocked())
                {
                    cellLoc[curCell->getCellId()].X = cellX;
                    cellLoc[curCell->getCellId()].Y = cellY;
                }
                else
                {
                    makeCellInLegalArea(placementInfo, cellLoc[curCell->getCellId()].X,
                                        cellLoc[curCell->getCellId()].Y);
                    curPU->setSpreadLocation(cellLoc[curCell->getCellId()].X, cellLoc[curCell->getCellId()].Y,
                                             forgetRatio);
                    placementInfo->transferCellBinInfo(curCell->getCellId(), curPU->X(), curPU->Y());
                    cellLoc[curCell->getCellId()].X = curPU->X();
                    cellLoc[curCell->getCellId()].Y = curPU->Y();
                }
            }
            else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
            {
                if (curPU->isFixed() || curPU->isLocked())
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);

                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        DesignInfo::DesignCell *curCell = curMacro->getCell(vId);

                        cellLoc[curCell->getCellId()].X = cellX;
                        cellLoc[curCell->getCellId()].Y = cellY;
                    }
                }
                else
                {
                    double tmpTotalX = 0.0;
                    double tmpTotalY = 0.0;

                    int numCellsInvolvedInSpreading = 0;
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        DesignInfo::DesignCell *curCell = curMacro->getCell(vId);
                        if (involvedCells.find(curCell) != involvedCells.end())
                        {
                            float offsetX_InMacro, offsetY_InMacro;
                            DesignInfo::DesignCellType cellType;
                            curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                            makeCellInLegalArea(placementInfo, cellLoc[curCell->getCellId()].X,
                                                cellLoc[curCell->getCellId()].Y);
                            tmpTotalX += cellLoc[curCell->getCellId()].X - offsetX_InMacro;
                            tmpTotalY += cellLoc[curCell->getCellId()].Y - offsetY_InMacro;
                            numCellsInvolvedInSpreading++;
                        }
                    }
                    tmpTotalX /= (double)numCellsInvolvedInSpreading;
                    tmpTotalY /= (double)numCellsInvolvedInSpreading;
                    float curNewPUX = tmpTotalX;
                    float curNewPUY = tmpTotalY;
                    placementInfo->legalizeXYInArea(curPU, curNewPUX, curNewPUY);
                    curPU->setSpreadLocation(curNewPUX, curNewPUY, forgetRatio);
                    placementInfo->enforceLegalizeXYInArea(curPU);
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);

                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        DesignInfo::DesignCell *curCell = curMacro->getCell(vId);
                        placementInfo->transferCellBinInfo(curCell->getCellId(), cellX, cellY);
                        cellLoc[curCell->getCellId()].X = cellX;
                        cellLoc[curCell->getCellId()].Y = cellY;
                    }
                }
            }
        }
    }
}

GeneralSpreader::SpreadRegion *GeneralSpreader::expandFromABin(PlacementInfo::PlacementBinInfo *curBin,
                                                               float capacityShrinkRatio)
{ // Our Region Expanding (1.4x faster)
    GeneralSpreader::SpreadRegion *resRegion =
        new GeneralSpreader::SpreadRegion(curBin, placementInfo, binGrid, capacityShrinkRatio);

    if (!useSimpleExpland)
    {
        while (resRegion->getOverflowRatio() > capacityShrinkRatio &&
               resRegion->smartFindExpandDirection(coveredBinSet))
        {
            resRegion->smartExpand(coveredBinSet);
        }
    }
    else
    {
        while (resRegion->getOverflowRatio() > capacityShrinkRatio &&
               resRegion->simpleFindExpandDirection(coveredBinSet))
        {
            resRegion->simpleExpand(coveredBinSet);
        }
    }

    // assert(!resRegion->isOverflow() && "TODO: how to handle the situation that the resource is not enough.");
    return resRegion;
}

// GeneralSpreader::SpreadRegion *GeneralSpreader::expandFromABin(PlacementInfo::PlacementBinInfo *curBin,
//                                                                float capacityShrinkRatio)
// { // RippleFPGA Region Expanding
//     GeneralSpreader::SpreadRegion *resRegion =
//         new GeneralSpreader::SpreadRegion(curBin, placementInfo, binGrid, capacityShrinkRatio);

//     while (resRegion->getOverflowRatio() > capacityShrinkRatio &&
//     resRegion->simpleFindExpandDirection(coveredBinSet))
//     {
//         resRegion->simpleExpand(coveredBinSet);
//     }
//     // assert(!resRegion->isOverflow() && "TODO: how to handle the situation that the resource is not enough.");
//     return resRegion;
// }

void GeneralSpreader::SpreadRegion::addBinRegion(int newRegionTopBinY, int newRegionBottomBinY, int newRegionLeftBinX,
                                                 int newRegionRightBinX,
                                                 std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
{
    assert(!isRegionOverlap(newRegionTopBinY, newRegionBottomBinY, newRegionLeftBinX, newRegionRightBinX));
    if (newRegionTopBinY > topBinY)
        topBinY = newRegionTopBinY;
    if (newRegionBottomBinY < bottomBinY)
        bottomBinY = newRegionBottomBinY;
    if (newRegionLeftBinX < leftBinX)
        leftBinX = newRegionLeftBinX;
    if (newRegionRightBinX > rightBinX)
        rightBinX = newRegionRightBinX;

    assert(0 <= newRegionBottomBinY);
    assert(0 <= newRegionTopBinY);
    assert((unsigned int)newRegionBottomBinY < binGrid.size());
    assert((unsigned int)newRegionTopBinY < binGrid.size());
    assert(0 <= newRegionLeftBinX);
    assert(0 <= newRegionRightBinX);
    assert((unsigned int)newRegionLeftBinX < binGrid[0].size());
    assert((unsigned int)newRegionRightBinX < binGrid[0].size());
    for (int i = newRegionBottomBinY; i <= newRegionTopBinY; i++)
        for (int j = newRegionLeftBinX; j <= newRegionRightBinX; j++)
        {
            assert(binSetInRegion.find(binGrid[i][j]) == binSetInRegion.end());
            assert(binGrid[i][j]->X() == j && binGrid[i][j]->Y() == i);
            binsInRegion.push_back(binGrid[i][j]);
            binSetInRegion.insert(binGrid[i][j]);
            assert(coveredBinSet.find(binGrid[i][j]) == coveredBinSet.end());
            coveredBinSet.insert(binGrid[i][j]);
            for (auto curCell : binGrid[i][j]->getCells())
            {
                cellsInRegion.insert(curCell);
                cellsInRegionVec.push_back(curCell);
            }
            totalCapacity += binGrid[i][j]->getCapacity();
            totalUtilization += binGrid[i][j]->getUtilization();
        }
    overflowRatio = totalUtilization / totalCapacity;
}

void GeneralSpreader::recordSpreadedCellLocations()
{
    for (auto tmpPU : placementInfo->getPlacementUnits())
    {
        tmpPU->recordSpreadLocatin();
    }
}

void GeneralSpreader::SpreadRegion::SubBox::spreadAndPartition()
{
    if (level == 0)
        return;
    if (topBinY - bottomBinY < minExpandSize - 1 && rightBinX - leftBinX < minExpandSize - 1)
    {
        return;
    }

    SubBox *boxA = nullptr, *boxB = nullptr;
    if (dirIsH)
    {
        if (rightBinX - leftBinX >= minExpandSize - 1)
        {
            spreadCellsH(&boxA, &boxB);
        }
        if (!(boxA || boxB) && topBinY - bottomBinY >= minExpandSize - 1)
        {
            spreadCellsV(&boxA, &boxB);
        }
    }
    else
    {
        if (topBinY - bottomBinY >= minExpandSize - 1)
        {
            spreadCellsV(&boxA, &boxB);
        }
        if (!(boxA || boxB) && rightBinX - leftBinX >= minExpandSize - 1)
        {
            spreadCellsH(&boxA, &boxB);
        }
    }
    if (boxA)
    {
        boxA->spreadAndPartition();
    }
    if (boxB)
    {
        boxB->spreadAndPartition();
    }
    if (boxA)
        delete boxA;
    if (boxB)
        delete boxB;
}

void GeneralSpreader::SpreadRegion::SubBox::spreadCellsH(SubBox **boxA, SubBox **boxB)
{
    // refer to paper of POLAR and RippleFPGA

    if (cellIds.size() == 0)
        return;
    if (cellIds.size() > 1)
        quick_sort(cellIds, 0, cellIds.size() - 1, true);

    std::vector<float> colCapacity(rightBinX - leftBinX + 1, 0.0);
    float totalCapacity = 0;

    assert(leftBinX >= 0);
    assert(bottomBinY >= 0);
    assert((unsigned int)topBinY < binGrid.size());
    assert((unsigned int)rightBinX < binGrid[topBinY].size());
    // calculate capacity
    for (int binX = leftBinX; binX <= rightBinX; binX++)
    {
        for (int y = bottomBinY; y <= topBinY; y++)
        {
            float binCapacity = capacityShrinkRatio * binGrid[y][binX]->getCapacity();
            colCapacity[binX - leftBinX] += binCapacity;
            totalCapacity += binCapacity;
        }
    }

    // get the boundary of the sub boxes
    //      @        |  |         @
    //      @  boxA  |  |   boxB  @
    //      @        |  |         @
    int boxALeft = leftBinX;
    int boxBRight = rightBinX;
    for (int binX = leftBinX; binX <= rightBinX; binX++, boxALeft++)
    {
        if (colCapacity[binX - leftBinX] > 0)
            break;
    }
    for (int binX = rightBinX; binX >= leftBinX; binX--, boxBRight--)
    {
        if (colCapacity[binX - leftBinX] > 0)
            break;
    }
    if (boxBRight <= boxALeft)
    {
        return;
        assert(false && "should not happen");
    }

    //      |        @  @         |
    //      |  boxA  @  @   boxB  |
    //      |        @  @         |
    int boxARight = boxALeft;
    int boxBLeft = boxBRight;
    float leftCapacity = colCapacity[boxARight - leftBinX];
    float rightCapacity = colCapacity[boxBLeft - leftBinX];
    while (boxARight < boxBLeft - 1)
    {
        if (leftCapacity <= rightCapacity)
        {
            boxARight++;
            leftCapacity += colCapacity[boxARight - leftBinX];
        }
        else
        {
            boxBLeft--;
            rightCapacity += colCapacity[boxBLeft - leftBinX];
        }
    }

    // splits the cells into two part
    float leftUtilRatio = leftCapacity / totalCapacity;
    float totalUtilization = 0;
    for (auto cellId : cellIds)
    {
        totalUtilization += placementInfo->getActualOccupationByCellId(cellId);
    }
    float leftUtil = totalUtilization * leftUtilRatio;

    int cutLineIdX = -1;
    if (leftUtil > eps)
    {
        for (auto cellId : cellIds)
        {
            leftUtil -= placementInfo->getActualOccupationByCellId(cellId);
            cutLineIdX++;
            if (leftUtil <= eps)
                break;
        }
    }

    // assign cells into two subbox A and B
    *boxA = new SubBox(this, topBinY, bottomBinY, boxALeft, boxARight, 0, cutLineIdX);
    *boxB = new SubBox(this, topBinY, bottomBinY, boxBLeft, boxBRight, cutLineIdX + 1, cellIds.size() - 1);
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();

    std::vector<int> &boxACellIds = (*boxA)->cellIds;
    if (boxACellIds.size() > 0)
    {
        int cellInBinHead = boxACellIds.size() - 1;
        int cellInBinTail = boxACellIds.size() - 1;

        // spread cells' location in boxA

        for (int binX = boxARight; binX >= boxALeft; binX--)
        {
            if (cellInBinTail < 0)
                break;

            float cArea = 0;
            int num = 0;
            float oldLoX = cellLoc[boxACellIds[cellInBinTail]].X;
            float oldHiX = oldLoX;
            for (cellInBinHead = cellInBinTail;
                 cellInBinHead >= 0 && (binX == boxALeft || (cArea <= colCapacity[binX - leftBinX] + eps));
                 --cellInBinHead)
            {
                cArea += placementInfo->getActualOccupationByCellId(boxACellIds[cellInBinHead]);
                oldLoX = cellLoc[boxACellIds[cellInBinHead]].X;
                num++;
            }
            cellInBinHead++;
            if (num > 0)
            {
                assert(binX >= 0);
                assert((unsigned int)binX < binGrid[bottomBinY].size());
                float newLoX = binGrid[bottomBinY][binX]->left() + placementInfo->getBinGridW() / (num + 1.0);
                float newHiX = binGrid[bottomBinY][binX]->right() - placementInfo->getBinGridW() / (num + 1.0);
                float rangeold = oldHiX - oldLoX;
                float rangenew = newHiX - newLoX;
                assert(rangeold > -1e-5);
                assert(rangenew > -1e-5);
                if (fabs(rangeold) < 1e-5)
                {
                    float newLoc = (newLoX + newHiX) / 2;
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxACellIds[ci]].X = newLoc;
                    }
                }
                else
                {
                    float scale = rangenew / rangeold;
                    assert(scale > -1e-5);
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxACellIds[ci]].X = newLoX + (cellLoc[boxACellIds[ci]].X - oldLoX) * scale;
                    }
                }
            }
            cellInBinTail = cellInBinHead - 1;
        }
    }
    else
    {
        delete *boxA;
        *boxA = nullptr;
    }

    std::vector<int> &boxBCellIds = (*boxB)->cellIds;

    if (boxBCellIds.size() > 0)
    {
        int cellInBinHead = 0;
        int cellInBinTail = 0;

        // spread cells' location in boxB
        for (int binX = boxBLeft; binX <= boxBRight; binX++)
        {
            if ((unsigned int)cellInBinHead >= boxBCellIds.size())
                break;

            float cArea = 0;
            int num = 0;
            float oldLoX = cellLoc[boxBCellIds[cellInBinHead]].X;
            float oldHiX = oldLoX;

            for (cellInBinTail = cellInBinHead; (unsigned int)cellInBinTail < boxBCellIds.size() &&
                                                (binX == boxBRight || (cArea <= colCapacity[binX - leftBinX] + eps));
                 cellInBinTail++)
            {
                cArea += placementInfo->getActualOccupationByCellId(boxBCellIds[cellInBinTail]);
                oldHiX = cellLoc[boxBCellIds[cellInBinTail]].X;
                num++;
            }
            cellInBinTail--;

            assert(cellLoc[boxBCellIds[cellInBinTail]].X <= oldHiX + 1e-5);
            if (num > 0)
            {
                assert(binX >= 0);
                assert((unsigned int)binX < binGrid[bottomBinY].size());
                float newLoX = binGrid[bottomBinY][binX]->left() + placementInfo->getBinGridW() / (num + 1.0);
                float newHiX = binGrid[bottomBinY][binX]->right() - placementInfo->getBinGridW() / (num + 1.0);
                float rangeold = oldHiX - oldLoX;
                float rangenew = newHiX - newLoX;
                assert(rangeold > -1e-5);
                assert(rangenew > -1e-5);
                if (fabs(rangeold) < 1e-5)
                {
                    float newLoc = (newLoX + newHiX) / 2;
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxBCellIds[ci]].X = newLoc;
                    }
                }
                else
                {
                    float scale = rangenew / rangeold;
                    assert(scale > -1e-5);
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxBCellIds[ci]].X = newLoX + (cellLoc[boxBCellIds[ci]].X - oldLoX) * scale;
                    }
                }
            }
            cellInBinHead = cellInBinTail + 1;
        }
    }
    else
    {
        delete *boxB;
        *boxB = nullptr;
    }

    return;
}

void GeneralSpreader::SpreadRegion::SubBox::spreadCellsV(SubBox **boxA, SubBox **boxB)
{
    // refer to paper of POLAR and RippleFPGA
    if (cellIds.size() == 0)
        return;

    if (cellIds.size() > 1)
        quick_sort(cellIds, 0, cellIds.size() - 1, false);

    std::vector<float> colCapacity(topBinY - bottomBinY + 1, 0.0);
    float totalCapacity = 0;

    assert(leftBinX >= 0);
    assert(bottomBinY >= 0);
    assert((unsigned int)topBinY < binGrid.size());
    assert((unsigned int)rightBinX < binGrid[topBinY].size());
    // calculate capacity
    for (int binY = bottomBinY; binY <= topBinY; binY++)
    {
        for (int binX = leftBinX; binX <= rightBinX; binX++)
        {
            float binCapacity = capacityShrinkRatio * binGrid[binY][binX]->getCapacity();
            colCapacity[binY - bottomBinY] += binCapacity;
            totalCapacity += binCapacity;
        }
    }

    // get the boundary of the sub boxes
    //      @ @ @ @ @ @
    //
    //         boxB
    //
    //      ----------
    //      ----------
    //
    //         boxA
    //
    //      @ @ @ @ @ @
    int boxABottom = bottomBinY;
    int boxBTop = topBinY;
    for (int binY = bottomBinY; binY <= topBinY; binY++, boxABottom++)
    {
        if (colCapacity[binY - bottomBinY] > 0)
            break;
    }
    for (int binY = topBinY; binY >= bottomBinY; binY--, boxBTop--)
    {
        if (colCapacity[binY - bottomBinY] > 0)
            break;
    }
    if (boxBTop <= boxABottom)
    {
        return;
        assert(false && "should not happen");
    }

    //      ----------
    //
    //         boxB
    //
    //      @ @ @ @ @ @
    //      @ @ @ @ @ @
    //
    //
    //         boxA
    //
    //      ----------
    int boxATop = boxABottom;
    int boxBBottom = boxBTop;
    float bottomCapacity = colCapacity[boxATop - bottomBinY];
    float topCapacity = colCapacity[boxBBottom - bottomBinY];
    while (boxATop < boxBBottom - 1)
    {
        if (bottomCapacity <= topCapacity)
        {
            boxATop++;
            bottomCapacity += colCapacity[boxATop - bottomBinY];
        }
        else
        {
            boxBBottom--;
            topCapacity += colCapacity[boxBBottom - bottomBinY];
        }
    }
    // if (leftBinX == 26 && rightBinX == 26 && topBinY == 180 && bottomBinY == 179)
    // {
    //     std::cout << "boxATop=" << boxATop << "\n";
    //     std::cout << "boxABottom=" << boxABottom << "\n";
    //     std::cout << "boxBTop=" << boxBTop << "\n";
    //     std::cout << "boxBBottom=" << boxBBottom << "\n";
    //     std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
    //     for (auto cellId : cellIds)
    //     {
    //         std::cout << "cell#" << cellId << " locX:" << cellLoc[cellId].X << " locY:" << cellLoc[cellId].Y << "\n";
    //     }
    // }
    // splits the cells into two part
    float bottomUtilRatio = bottomCapacity / totalCapacity;
    float totalUtilization = 0;
    for (auto cellId : cellIds)
    {
        totalUtilization += placementInfo->getActualOccupationByCellId(cellId);
    }
    float bottomUtil = totalUtilization * bottomUtilRatio;

    int cutLineIdX = -1;
    if (bottomUtil > eps)
    {
        for (auto cellId : cellIds)
        {
            bottomUtil -= placementInfo->getActualOccupationByCellId(cellId);
            cutLineIdX++;
            if (bottomUtil <= eps)
                break;
        }
    }

    // assign cells into two subbox A and B
    *boxA = new SubBox(this, boxATop, boxABottom, leftBinX, rightBinX, 0, cutLineIdX);
    *boxB = new SubBox(this, boxBTop, boxBBottom, leftBinX, rightBinX, cutLineIdX + 1, cellIds.size() - 1);
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();

    std::vector<int> &boxACellIds = (*boxA)->cellIds;

    if (boxACellIds.size() > 0)
    {
        int cellInBinHead = boxACellIds.size() - 1;
        int cellInBinTail = boxACellIds.size() - 1;
        // spread cells' location in boxA

        for (int binY = boxATop; binY >= boxABottom; binY--)
        {
            if (cellInBinTail < 0)
                break;
            float cArea = 0;
            int num = 0;
            float oriBottomY = cellLoc[boxACellIds[cellInBinTail]].Y;
            float oriTopY = oriBottomY;
            for (cellInBinHead = cellInBinTail;
                 cellInBinHead >= 0 && (binY == boxABottom || (cArea <= colCapacity[binY - bottomBinY] + eps));
                 --cellInBinHead)
            {
                cArea += placementInfo->getActualOccupationByCellId(boxACellIds[cellInBinHead]);
                oriBottomY = cellLoc[boxACellIds[cellInBinHead]].Y;
                num++;
            }
            cellInBinHead++;
            if (num > 0)
            {
                assert(binY >= 0);
                assert((unsigned int)binY < binGrid.size());
                float newBottomY = binGrid[binY][leftBinX]->bottom() + placementInfo->getBinGridH() / (num + 1.0);
                float newTopY = binGrid[binY][leftBinX]->top() - placementInfo->getBinGridH() / (num + 1.0);
                assert(newTopY >= newBottomY);
                float rangeold = oriTopY - oriBottomY;
                float rangenew = newTopY - newBottomY;
                assert(rangeold > -1e-5);
                assert(rangenew > -1e-5);
                if (fabs(rangeold) < 1e-5)
                {
                    float newBottomc = (newBottomY + newTopY) / 2;
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxACellIds[ci]].Y = newBottomc;
                    }
                }
                else
                {
                    float scale = rangenew / rangeold;
                    assert(scale > -1e-5);
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxACellIds[ci]].Y = newBottomY + (cellLoc[boxACellIds[ci]].Y - oriBottomY) * scale;
                    }
                }
            }
            cellInBinTail = cellInBinHead - 1;
        }
    }
    else
    {
        delete *boxA;
        *boxA = nullptr;
    }

    std::vector<int> &boxBCellIds = (*boxB)->cellIds;

    if (boxBCellIds.size() > 0)
    {
        int cellInBinHead = 0;
        int cellInBinTail = 0;

        // spread cells' location in boxB
        for (int binY = boxBBottom; binY <= boxBTop; binY++)
        {
            if ((unsigned int)cellInBinHead >= boxBCellIds.size())
                break;

            float cArea = 0;
            int num = 0;
            float oriBottomY = cellLoc[boxBCellIds[cellInBinHead]].Y;
            float oriTopY = oriBottomY;

            for (cellInBinTail = cellInBinHead; (unsigned int)cellInBinTail < boxBCellIds.size() &&
                                                (binY == boxBTop || (cArea <= colCapacity[binY - bottomBinY] + eps));
                 cellInBinTail++)
            {
                cArea += placementInfo->getActualOccupationByCellId(boxBCellIds[cellInBinTail]);
                oriTopY = cellLoc[boxBCellIds[cellInBinTail]].Y;
                num++;
            }
            cellInBinTail--;
            if (num > 0)
            {
                assert(binY >= 0);
                assert((unsigned int)binY < binGrid.size());
                float newBottomY = binGrid[binY][leftBinX]->bottom() + placementInfo->getBinGridH() / (num + 1.0);
                float newTopY = binGrid[binY][leftBinX]->top() - placementInfo->getBinGridH() / (num + 1.0);
                float rangeold = oriTopY - oriBottomY;
                float rangenew = newTopY - newBottomY;
                assert(rangeold > -1e-5);
                assert(rangenew > -1e-5);
                if (fabs(rangeold) < 1e-5)
                {
                    float newBottomc = (newBottomY + newTopY) / 2;
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxBCellIds[ci]].Y = newBottomc;
                    }
                }
                else
                {
                    float scale = rangenew / rangeold;
                    assert(scale > -1e-5);
                    for (int ci = cellInBinHead; ci <= cellInBinTail; ++ci)
                    {
                        cellLoc[boxBCellIds[ci]].Y = newBottomY + (cellLoc[boxBCellIds[ci]].Y - oriBottomY) * scale;
                    }
                }
            }
            cellInBinHead = cellInBinTail + 1;
        }
    }
    else
    {
        delete *boxB;
        *boxB = nullptr;
    }

    return;
}

const std::string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void GeneralSpreader::dumpSiteGridDensity(std::string dumpFileName)
{
    dumpFileName =
        dumpFileName + "-" + sharedCellType + "-" + currentDateTime() + "-" + std::to_string(dumpSiteGridDensityCnt);
    print_status("GeneralSpreader: dumping density to: " + dumpFileName);
    std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &curBinGrid =
        placementInfo->getBinGrid(placementInfo->getSharedBELTypeId(sharedCellType));

    std::ofstream outfile0(dumpFileName.c_str());
    for (auto &row : curBinGrid)
    {
        for (auto curBin : row)
        {
            outfile0 << curBin->getRealUtilizationRate() << " ";
        }
        outfile0 << "\n";
    }
    outfile0.close();
    dumpSiteGridDensityCnt++;
}

void GeneralSpreader::dumpLUTFFCoordinate()
{
    if (JSONCfg.find("DumpLUTFFCoordTrace-GeneralSpreader") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpLUTFFCoordTrace-GeneralSpreader"] + "-" + sharedCellType + "-" +
                               std::to_string(LUTFFCoordinateDumpCnt) + ".gz";
        print_status("GeneralSpreader: dumping coordinate archieve to: " + dumpFile);
        LUTFFCoordinateDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPU : placementInfo->getPlacementUnits())
            {
                if (auto curUnpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    float cellX = curUnpackedCell->X();
                    float cellY = curUnpackedCell->Y();
                    DesignInfo::DesignCell *curCell = curUnpackedCell->getCell();
                    if (curCell->isFF() || curCell->isLUT())
                    {
                        outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
                    }
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    for (int vId = 0; vId < curMacro->getNumOfCells(); vId++)
                    {
                        float offsetX_InMacro, offsetY_InMacro;
                        DesignInfo::DesignCellType cellType;
                        curMacro->getVirtualCellInfo(vId, offsetX_InMacro, offsetY_InMacro, cellType);
                        float cellX = curMacro->X() + offsetX_InMacro;
                        float cellY = curMacro->Y() + offsetY_InMacro;

                        if (DesignInfo::isFF(cellType) || DesignInfo::isLUT(cellType))
                        {
                            if (curMacro->getCell(vId))
                                outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                            else
                                outfile0 << cellX << " " << cellY << "\n";
                        }
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GeneralSpreader: dumped coordinate archieve to: " + dumpFile);
        }
    }
}

std::ostream &operator<<(std::ostream &os, GeneralSpreader::SpreadRegion::SubBox *curBox)
{
    os << "Box: Top:" << curBox->top() << " Bottom:" << curBox->bottom() << " Left:" << curBox->left()
       << " Right:" << curBox->right();
    return os;
}

void GeneralSpreader::DumpCellsCoordinate(std::string dumpFileName, GeneralSpreader::SpreadRegion *curRegion)
{
    if (dumpFileName != "")
    {
        dumpCnt++;
        dumpFileName = dumpFileName + "-" + std::to_string(dumpCnt) + ".gz";
        // print_status("GeneralSpreader: dumping coordinate archieve to: " + dumpFileName);
        std::stringstream outfile0;
        std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
        for (auto tmpCell : curRegion->getCells())
        {
            outfile0 << cellLoc[tmpCell->getCellId()].X << " " << cellLoc[tmpCell->getCellId()].Y << tmpCell << "\n";
        }
        writeStrToGZip(dumpFileName, outfile0);
        // print_status("GeneralSpreader: dumped coordinate archieve to: " + dumpFileName);
    }
}

void GeneralSpreader::DumpPUCoordinate(std::string dumpFileName,
                                       std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec)
{
    if (dumpFileName != "")
    {
        dumpCnt++;
        dumpFileName = dumpFileName + "-" + std::to_string(dumpCnt) + ".gz";
        // print_status("GeneralSpreader: dumping coordinate archieve to: " + dumpFileName);
        std::stringstream outfile0;
        for (auto curPU : involvedPUVec)
        {
            outfile0 << curPU->X() << " " << curPU->Y() << " " << curPU << "\n";
        }
        writeStrToGZip(dumpFileName, outfile0);
        // print_status("GeneralSpreader: dumped coordinate archieve to: " + dumpFileName);
    }
}