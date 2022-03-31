/**
 * @file IncrementalBELPacker.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation of the IncrementalBELPacker which  incrementally packs
 * some LUTs/FFs during global placement based on their distance, interconnection density and compatibility
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "IncrementalBELPacker.h"
#include "dumpZip.h"
#include "readZip.h"
#include "strPrint.h"
#include "stringCheck.h"
#include <assert.h>
#include <cmath>
#include <queue>

void IncrementalBELPacker::isLUTsPackable(PlacementInfo::PlacementUnpackedCell *LUTA,
                                          PlacementInfo::PlacementUnpackedCell *LUTB)
{
    assert(LUTA);
    assert(LUTB);
    assert(LUTA->getCell()->isLUT() && !LUTA->getCell()->isLUT6());
    assert(LUTB->getCell()->isLUT() && !LUTB->getCell()->isLUT6());
}

void IncrementalBELPacker::LUTFFPairing(float disThreshold)
{
    print_status("IncrementalBELPacker Pairing LUTs and FFs.");
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
    LUTFFPairs.clear();
    std::vector<bool> packed(placementUnpackedCells.size(), false);
    std::vector<PlacementInfo::PlacementUnpackedCell *> oriPlacementUnpackedCells = placementUnpackedCells;

    placementUnits.resize(placementMacros.size());

    int LUTTO1FFPackedCnt = 0;
    int LUTTOMultiFFPackedCnt = 0;
    for (auto unpackedCell : oriPlacementUnpackedCells)
    {
        DesignInfo::DesignCell *curCell = unpackedCell->getCell();
        if (curCell->isLUT())
        {
            assert(curCell->getOutputPins().size() > 0);
            if (curCell->getOutputPins().size() == 1)
            {
                if (curCell->getOutputPins()[0]
                        ->isUnconnected()) // interestingly, some LUTs generated by Vivado might have no output
                    continue;
                assert(curCell->getOutputPins()[0]->getNet());
                if (curCell->getOutputPins()[0]->getNet()->getPinsBeDriven().size() == 1)
                {
                    auto pinBeDriven = curCell->getOutputPins()[0]->getNet()->getPinsBeDriven()[0];
                    assert(pinBeDriven->getCell());
                    auto FFBeDriven = pinBeDriven->getCell();
                    if (FFBeDriven->isFF() && cellInMacros.find(FFBeDriven) == cellInMacros.end())
                    {
                        if (getCellDistance(cellLoc[FFBeDriven->getCellId()], cellLoc[curCell->getCellId()]) <
                            disThreshold)
                        {
                            LUTFFPairs.emplace_back(curCell, FFBeDriven);
                            PlacementInfo::PlacementUnpackedCell *FFPU =
                                static_cast<PlacementInfo::PlacementUnpackedCell *>(
                                    cellId2PlacementUnitVec[FFBeDriven->getCellId()]);
                            FFPU->setPacked();
                            unpackedCell->setPacked();

                            PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
                                curCell->getName(), placementUnits.size(),
                                PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair);

                            curMacro->addOccupiedSite(0.0, 0.0625);
                            curMacro->addCell(curCell, curCell->getCellType(), 0, 0.0);
                            curMacro->addCell(FFBeDriven, FFBeDriven->getCellType(), 0, 0.0);
                            float newX = (cellLoc[curCell->getCellId()].X * curCell->getPins().size() +
                                          cellLoc[FFBeDriven->getCellId()].X * FFBeDriven->getPins().size()) /
                                         (curCell->getPins().size() + FFBeDriven->getPins().size());
                            float newY = (cellLoc[curCell->getCellId()].Y * curCell->getPins().size() +
                                          cellLoc[FFBeDriven->getCellId()].Y * FFBeDriven->getPins().size()) /
                                         (curCell->getPins().size() + FFBeDriven->getPins().size());
                            placementInfo->legalizeXYInArea(curMacro, newX, newY);
                            curMacro->setAnchorLocationAndForgetTheOriginalOne(newX, newY);
                            cellLoc[curCell->getCellId()].X = cellLoc[FFBeDriven->getCellId()].X = newX;
                            cellLoc[curCell->getCellId()].Y = cellLoc[FFBeDriven->getCellId()].Y = newY;
                            curMacro->setWeight(
                                compatiblePlacementTable->cellType2sharedBELTypeOccupation[FFBeDriven->getCellType()] +
                                compatiblePlacementTable->cellType2sharedBELTypeOccupation[curCell->getCellType()]);
                            placementMacros.push_back(curMacro);
                            placementUnits.push_back(curMacro);

                            cellId2PlacementUnit[curCell->getCellId()] = curMacro;
                            cellId2PlacementUnit[FFBeDriven->getCellId()] = curMacro;

                            cellInMacros.insert(curCell);
                            cellInMacros.insert(FFBeDriven);

                            LUTTO1FFPackedCnt++;
                        }
                    }
                }
                else if (curCell->getOutputPins()[0]->getNet()->getPinsBeDriven().size() <= 4)
                {
                    float minDis = 1000000000;
                    DesignInfo::DesignCell *FFToPack = nullptr;
                    int drivenFFNum = 0;
                    for (auto pinBeDriven : curCell->getOutputPins()[0]->getNet()->getPinsBeDriven())
                    {
                        assert(pinBeDriven->getCell());
                        auto FFBeDriven = pinBeDriven->getCell();
                        if (FFBeDriven->isFF() && cellInMacros.find(FFBeDriven) == cellInMacros.end())
                        {
                            float curDis =
                                getCellDistance(cellLoc[FFBeDriven->getCellId()], cellLoc[curCell->getCellId()]);
                            if (curDis < disThreshold)
                            {
                                if (curDis < minDis)
                                {
                                    minDis = curDis;
                                    FFToPack = FFBeDriven;
                                }
                            }
                            drivenFFNum++;
                        }
                    }

                    if (FFToPack && drivenFFNum <= 4)
                    {
                        if (FFToPack->isFF())
                        {
                            if (getCellDistance(cellLoc[FFToPack->getCellId()], cellLoc[curCell->getCellId()]) <
                                disThreshold)
                            {
                                LUTFFPairs.emplace_back(curCell, FFToPack);
                                PlacementInfo::PlacementUnpackedCell *FFPU =
                                    static_cast<PlacementInfo::PlacementUnpackedCell *>(
                                        cellId2PlacementUnitVec[FFToPack->getCellId()]);
                                FFPU->setPacked();
                                unpackedCell->setPacked();

                                PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
                                    curCell->getName(), placementUnits.size(),
                                    PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair);

                                curMacro->addOccupiedSite(0.0, 0.0625);
                                curMacro->addCell(curCell, curCell->getCellType(), 0, 0.0);
                                curMacro->addCell(FFToPack, FFToPack->getCellType(), 0, 0.0);
                                float newX = (cellLoc[curCell->getCellId()].X * curCell->getPins().size() +
                                              cellLoc[FFToPack->getCellId()].X * FFToPack->getPins().size()) /
                                             (curCell->getPins().size() + FFToPack->getPins().size());
                                float newY = (cellLoc[curCell->getCellId()].Y * curCell->getPins().size() +
                                              cellLoc[FFToPack->getCellId()].Y * FFToPack->getPins().size()) /
                                             (curCell->getPins().size() + FFToPack->getPins().size());
                                placementInfo->legalizeXYInArea(curMacro, newX, newY);
                                curMacro->setAnchorLocationAndForgetTheOriginalOne(newX, newY);
                                cellLoc[curCell->getCellId()].X = cellLoc[FFToPack->getCellId()].X = newX;
                                cellLoc[curCell->getCellId()].Y = cellLoc[FFToPack->getCellId()].Y = newY;
                                curMacro->setWeight(
                                    compatiblePlacementTable
                                        ->cellType2sharedBELTypeOccupation[FFToPack->getCellType()] +
                                    compatiblePlacementTable->cellType2sharedBELTypeOccupation[curCell->getCellType()]);
                                placementMacros.push_back(curMacro);
                                placementUnits.push_back(curMacro);

                                cellId2PlacementUnit[curCell->getCellId()] = curMacro;
                                cellId2PlacementUnit[FFToPack->getCellId()] = curMacro;

                                cellInMacros.insert(curCell);
                                cellInMacros.insert(FFToPack);

                                LUTTOMultiFFPackedCnt++;
                            }
                        }
                    }
                }
            }
            else
            {
                // this is a LUT6_2, has two output pins and we don't pack them temporarily.
            }
        }
    }

    print_info("IncrementalBELPacker: LUTTO1FFPackedCnt=" + std::to_string(LUTTO1FFPackedCnt));
    print_info("IncrementalBELPacker: LUTTOMultiFFPackedCnt=" + std::to_string(LUTTOMultiFFPackedCnt));
    placementUnpackedCells.clear();
    for (auto unpackedCell : oriPlacementUnpackedCells)
    {
        if (!unpackedCell->isPacked())
        {
            unpackedCell->renewId(placementUnits.size());
            placementUnits.push_back(unpackedCell);
            placementUnpackedCells.push_back(unpackedCell);
        }
        else
        {
            delete unpackedCell;
        }
    }

    print_status("IncrementalBELPacker Updating Cell-PlacementUnit Mapping");
    placementInfo->updateCells2PlacementUnits();
    placementInfo->updateElementBinGrid();
    // since new placement units might be generated, we need to update the net information
    print_status("IncrementalBELPacker Loading Nets");
    placementInfo->reloadNets();

    std::map<DesignInfo::DesignCell *, PlacementInfo::PlacementUnit *> cell2PUMap;

    cell2PUMap.clear();
    for (auto unpackedCell : placementUnpackedCells)
    {
        DesignInfo::DesignCell *curCell = unpackedCell->getCell();
        assert(cell2PUMap.find(curCell) == cell2PUMap.end());
        cell2PUMap[curCell] = unpackedCell;
    }

    for (auto curMacro : placementMacros)
    {
        for (auto curCell : curMacro->getCells())
        {
            if (cell2PUMap.find(curCell) != cell2PUMap.end())
            {
                std::cout << "recorded PU:" << cell2PUMap[curCell] << "\n";
                std::cout << "another PU:" << curMacro << "\n";
                std::cout.flush();
            }
            assert(cell2PUMap.find(curCell) == cell2PUMap.end());
            cell2PUMap[curCell] = curMacro;
        }
    }

    print_status("IncrementalBELPacker Paired LUTs and FFs (#Pairs = " + std::to_string(LUTFFPairs.size()) + ")");
    dumpPairedLUTFF();
}

void IncrementalBELPacker::FFPairing(float disThreshold)
{
    print_status("IncrementalBELPacker Pairing FFs.");
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
    FF_FFPairs.clear();
    std::vector<bool> packed(placementUnpackedCells.size(), false);
    std::vector<PlacementInfo::PlacementUnpackedCell *> oriPlacementUnpackedCells = placementUnpackedCells;

    placementUnits.resize(placementMacros.size());
    std::set<PlacementInfo::PlacementUnpackedCell *> packedCells;
    int FFPairedCnt = 0;
    int FFCouldBePackedCnt = 0;

    for (auto CS : designInfo->getControlSets())
    {
        auto &FFs = CS->getFFs();
        std::vector<FFLocation> FFpoints;
        FFpoints.clear();

        for (auto curFF : FFs)
        {
            auto tmpPU = cellId2PlacementUnit[curFF->getCellId()];
            if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
            {
                FFpoints.emplace_back(unpackedCell);
                FFCouldBePackedCnt++;
            }
        }

        // build k-d tree
        kdt::KDTree<FFLocation> kdtree(FFpoints, y2xRatio);

        for (auto &FFTmpPoint : FFpoints)
        {
            auto FF0 = FFTmpPoint.getUnpackedCell();

            if (packedCells.find(FF0) != packedCells.end())
                continue;
            // K-nearest neighbor search (gets indices to neighbors)
            int k = 10;
            FFLocation query(FF0);
            std::vector<int> indices = kdtree.knnSearch(query, k);
            int closestInd = -1;
            float closetDis = 10000000;
            for (auto tmpInd : indices)
            {
                auto tmpPU1 = FFpoints[tmpInd].getUnpackedCell();
                if (auto FF1 = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU1))
                {
                    if (packedCells.find(FF1) != packedCells.end())
                        continue;
                    if (FF0 == FF1)
                        continue;
                    DesignInfo::DesignCell *FFCell0 = FF0->getCell();
                    DesignInfo::DesignCell *FFCell1 = FF1->getCell();
                    float dis = getCellDistance(cellLoc[FFCell0->getCellId()], cellLoc[FFCell1->getCellId()]);
                    if (dis < disThreshold)
                    {
                        if (dis < closetDis)
                        {
                            closetDis = dis;
                            closestInd = tmpInd;
                        }
                    }
                }
            }

            if (closestInd >= 0)
            {
                auto FF1 = FFpoints[closestInd].getUnpackedCell();
                DesignInfo::DesignCell *FFCell0 = FF0->getCell();
                DesignInfo::DesignCell *FFCell1 = FF1->getCell();
                FF_FFPairs.emplace_back(FFCell0, FFCell1);
                FF0->setPacked();
                FF1->setPacked();

                PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
                    FF0->getName(), placementUnits.size(), PlacementInfo::PlacementMacro::PlacementMacroType_FFFFPair);

                curMacro->addOccupiedSite(0.0, 0.0625);
                curMacro->addCell(FFCell0, FFCell0->getCellType(), 0, 0.0);
                curMacro->addCell(FFCell1, FFCell1->getCellType(), 0, 0.0);
                float newX = (cellLoc[FFCell0->getCellId()].X * FFCell0->getPins().size() +
                              cellLoc[FFCell1->getCellId()].X * FFCell1->getPins().size()) /
                             (FFCell0->getPins().size() + FFCell1->getPins().size());
                float newY = (cellLoc[FFCell0->getCellId()].Y * FFCell0->getPins().size() +
                              cellLoc[FFCell1->getCellId()].Y * FFCell1->getPins().size()) /
                             (FFCell0->getPins().size() + FFCell1->getPins().size());
                placementInfo->legalizeXYInArea(curMacro, newX, newY);
                curMacro->setAnchorLocationAndForgetTheOriginalOne(newX, newY);
                cellLoc[FFCell0->getCellId()].X = cellLoc[FFCell1->getCellId()].X = newX;
                cellLoc[FFCell0->getCellId()].Y = cellLoc[FFCell1->getCellId()].Y = newY;
                curMacro->setWeight(compatiblePlacementTable->cellType2sharedBELTypeOccupation[FFCell1->getCellType()] +
                                    compatiblePlacementTable->cellType2sharedBELTypeOccupation[FFCell0->getCellType()]);
                placementMacros.push_back(curMacro);
                placementUnits.push_back(curMacro);

                cellId2PlacementUnit[FFCell0->getCellId()] = curMacro;
                cellId2PlacementUnit[FFCell1->getCellId()] = curMacro;

                assert(cellInMacros.find(FFCell0) == cellInMacros.end());
                assert(cellInMacros.find(FFCell1) == cellInMacros.end());
                cellInMacros.insert(FFCell0);
                cellInMacros.insert(FFCell1);

                packedCells.insert(FF0);
                packedCells.insert(FF1);

                FFPairedCnt++;
            }
        }
    }

    print_info("IncrementalBELPacker: 2FFPairedCnt=" + std::to_string(FFPairedCnt));
    print_info("IncrementalBELPacker: FFCouldBePackedCnt=" + std::to_string(FFCouldBePackedCnt));
    placementUnpackedCells.clear();
    for (auto unpackedCell : oriPlacementUnpackedCells)
    {
        if (!unpackedCell->isPacked())
        {
            unpackedCell->renewId(placementUnits.size());
            placementUnits.push_back(unpackedCell);
            placementUnpackedCells.push_back(unpackedCell);
        }
        else
        {
            delete unpackedCell;
        }
    }

    print_status("IncrementalBELPacker Updating Cell-PlacementUnit Mapping");
    placementInfo->updateCells2PlacementUnits();
    placementInfo->updateElementBinGrid();
    // since new placement units might be generated, we need to update the net information
    print_status("IncrementalBELPacker Loading Nets");
    placementInfo->reloadNets();

    std::map<DesignInfo::DesignCell *, PlacementInfo::PlacementUnit *> cell2PUMap;

    cell2PUMap.clear();
    for (auto unpackedCell : placementUnpackedCells)
    {
        DesignInfo::DesignCell *curCell = unpackedCell->getCell();
        assert(cell2PUMap.find(curCell) == cell2PUMap.end());
        cell2PUMap[curCell] = unpackedCell;
    }

    for (auto curMacro : placementMacros)
    {
        for (auto curCell : curMacro->getCells())
        {
            if (cell2PUMap.find(curCell) != cell2PUMap.end())
            {
                std::cout << "recorded PU:" << cell2PUMap[curCell] << "\n";
                std::cout << "another PU:" << curMacro << "\n";
                std::cout.flush();
            }
            assert(cell2PUMap.find(curCell) == cell2PUMap.end());
            cell2PUMap[curCell] = curMacro;
        }
    }

    print_status("IncrementalBELPacker Paired LUTs and FFs (#Pairs = " + std::to_string(LUTFFPairs.size()) + ")");
    dumpPairedLUTFF();
}

void IncrementalBELPacker::dumpPairedLUTFF()
{
    if (JSONCfg.find("DumpLUTFFPair") != JSONCfg.end())
    {
        std::string dumpFile = JSONCfg["DumpLUTFFPair"] + "-" + std::to_string(LUTFFPairDumpCnt) + ".gz";
        print_status("IncrementalBELPacker: dumping LUTFFPair archieve to: " + dumpFile);
        LUTFFPairDumpCnt++;
        if (dumpFile != "")
        {
            std::stringstream outfile0;
            for (auto curPair : LUTFFPairs)
            {
                outfile0 << curPair.first->getName() << " " << curPair.second->getName() << "\n";
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("IncrementalBELPacker: dumped LUTFFPair archieve to: " + dumpFile);
        }
    }
}