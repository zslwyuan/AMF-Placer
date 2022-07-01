/**
 * @file ParallelCLBPacker.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This implementation file contains APIs' implementation of the ParallelCLBPacker which finally packs
 * LUT/FF/MUX/CARRY elements into legal CLB sites in a parallel approach.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "ParallelCLBPacker.h"
#define TIMINGDP

void ParallelCLBPacker::prePackLegalizedMacros(PlacementInfo::PlacementMacro *tmpMacro)
{
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> &PULegalSite =
        placementInfo->getPULegalSite();
    if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_CARRY)
    {
        assert(PULegalSite.find(tmpMacro) != PULegalSite.end());
        std::vector<DeviceInfo::DeviceSite *> &legalSites = PULegalSite[tmpMacro];
        for (unsigned int i = 0; i < legalSites.size(); i++)
        {
            deviceSite2PackingSite[legalSites[i]]->mapCarryRelatedCellsToSlots(tmpMacro, i);
            deviceSite2PackingSite[legalSites[i]]->addCarry();
            placementInfo->addPUIntoClockColumn(tmpMacro, legalSites[i]);
        }
    }
    else if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MCLB)
    {
        assert(PULegalSite.find(tmpMacro) != PULegalSite.end());
        std::vector<DeviceInfo::DeviceSite *> &legalSites = PULegalSite[tmpMacro];
        for (unsigned int i = 0; i < legalSites.size(); i++)
        {
            deviceSite2PackingSite[legalSites[i]]->mapLUTRAMRelatedCellsToSlots(tmpMacro);
            deviceSite2PackingSite[legalSites[i]]->addLUTRAMMacro();
            placementInfo->addPUIntoClockColumn(tmpMacro, legalSites[i]);
        }
    }
}

ParallelCLBPacker::ParallelCLBPacker(DesignInfo *designInfo, DeviceInfo *deviceInfo, PlacementInfo *placementInfo,
                                     std::map<std::string, std::string> &JSONCfg, int unchangedIterationThr,
                                     int numNeighbor, float deltaD, float curD, float maxD, int PQSize,
                                     float HPWLWeight, std::string packerName,
                                     PlacementTimingOptimizer *timingOptimizer, WirelengthOptimizer *WLOptimizer)
    : designInfo(designInfo), deviceInfo(deviceInfo), placementInfo(placementInfo), JSONCfg(JSONCfg),
      unchangedIterationThr(unchangedIterationThr), numNeighbor(numNeighbor), deltaD(deltaD), curD(curD), maxD(maxD),
      PQSize(PQSize), HPWLWeight(HPWLWeight), packerName(packerName), timingOptimizer(timingOptimizer),
      WLOptimizer(WLOptimizer), PUId2PackingCLBSite(placementInfo->getPlacementUnits().size(), nullptr),
      PUId2PackingCLBSiteCandidate(placementInfo->getPlacementUnits().size(), nullptr),
      placementUnits(placementInfo->getPlacementUnits()),
      placementUnpackedCells(placementInfo->getPlacementUnpackedCells()),
      placementMacros(placementInfo->getPlacementMacros()), cellInMacros(placementInfo->getCellInMacros()),
      cellId2PlacementUnit(placementInfo->getCellId2PlacementUnit())
{
    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
    {
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);
    }
    // PlacementInfo *placementInfo, DeviceInfo::DeviceSite *CLBSite, int unchangedIterationThr,
    //                        int numNeighbor, float deltaD, float curD, float maxD, int PQSize, float y2xRatio,
    //                        std::vector<PackingCLBSite *> &PUId2PackingCLBSite

    int numClockCols = placementInfo->getDeviceInfo()->getClockColumns().size();
    clockColumns2PackingSites =
        std::vector<std::vector<PackingCLBSite *>>(numClockCols, std::vector<PackingCLBSite *>());

    std::string targetSiteType = "SLICEL";
    deviceSite2PackingSite.clear();
    for (auto curSite : deviceInfo->getSitesInType(targetSiteType))
    {
        if (curSite->isOccupied())
            continue;
        PackingCLBSite *tmpPackingSite =
            new PackingCLBSite(placementInfo, curSite, unchangedIterationThr, numNeighbor, deltaD, curD, maxD, PQSize,
                               y2xRatio, HPWLWeight, PUId2PackingCLBSite);
        deviceSite2PackingSite[curSite] = tmpPackingSite;
        packingSites.push_back(tmpPackingSite);
        clockColumns2PackingSites[curSite->getClockHalfColumn()->getId()].push_back(tmpPackingSite);
    }
    targetSiteType = "SLICEM";
    for (auto curSite : deviceInfo->getSitesInType(targetSiteType))
    {
        if (curSite->isOccupied())
            continue;
        PackingCLBSite *tmpPackingSite =
            new PackingCLBSite(placementInfo, curSite, unchangedIterationThr, numNeighbor, deltaD, curD, maxD, PQSize,
                               y2xRatio, HPWLWeight, PUId2PackingCLBSite);
        deviceSite2PackingSite[curSite] = tmpPackingSite;
        packingSites.push_back(tmpPackingSite);
        clockColumns2PackingSites[curSite->getClockHalfColumn()->getId()].push_back(tmpPackingSite);
    }

    for (auto tmpMacro : placementMacros)
    {
        prePackLegalizedMacros(tmpMacro);
    }

    print_status("ParallelCLBPacker: CARRY macros are mapped to sites.");

    int numPackingSites = packingSites.size();
    for (int i = 0; i < numPackingSites; i++)
    {
        auto tmpPackingSite = packingSites[i];
        if (tmpPackingSite->getDeterminedClusterInSite())
        {
            assert(tmpPackingSite->checkIsPrePackedSite());
        }
    }
    print_status("ParallelCLBPacker: initialized.");
}

void ParallelCLBPacker::packCLBsIteration(bool initial, bool debug)
{
    int numClockCols = clockColumns2PackingSites.size();
#pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < numClockCols; i++)
    {
        for (unsigned int j = 0; j < clockColumns2PackingSites[i].size(); j++)
        {
            auto tmpPackingSite = clockColumns2PackingSites[i][j];
            tmpPackingSite->updateStep(initial, debug);
        }
    }

    //     int numPackingSites = packingSites.size();
    // #pragma omp parallel for schedule(dynamic, 16)
    //     for (int i = 0; i < numPackingSites; i++)
    //     {
    //         auto tmpPackingSite = packingSites[i];
    //         tmpPackingSite->updateStep(initial, debug);
    //     }

    // update PU's selection of packing site
    PUId2PackingCLBSite.clear();
    PUId2PackingCLBSite.resize(placementInfo->getPlacementUnits().size(), nullptr);

    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                {
                    assert(!PUId2PackingCLBSite[tmpPU->getId()]);
                    // if (!placementInfo->checkClockColumnLegalization(tmpPU, packingSite->getCLBSite()))
                    // {
                    //     placementInfo->printOutClockColumnLegalization(tmpPU, packingSite->getCLBSite());
                    // }
                    // assert(placementInfo->checkClockColumnLegalization(tmpPU, packingSite->getCLBSite()));
                    PUId2PackingCLBSite[tmpPU->getId()] = packingSite;
                    placementInfo->addPUIntoClockColumn(tmpPU, packingSite->getCLBSite());
                }
            }
        }
    }

    PUId2PackingCLBSiteCandidate.clear();
    PUId2PackingCLBSiteCandidate.resize(placementInfo->getPlacementUnits().size(), nullptr);
    for (auto tmpPackingSite : packingSites)
    {
        if (tmpPackingSite->hasValidPQTop())
        {
            const ParallelCLBPacker::PackingCLBSite::PackingCLBCluster *tmpTop = tmpPackingSite->getPriorityQueueTop();
            for (auto tmpPU : tmpTop->getPUs())
            {
                if (!PUId2PackingCLBSite[tmpPU->getId()])
                {

                    if (!placementInfo->checkClockColumnLegalization(tmpPU, tmpPackingSite->getCLBSite()))
                        continue;

                    if (!PUId2PackingCLBSiteCandidate[tmpPU->getId()])
                    {
                        PUId2PackingCLBSiteCandidate[tmpPU->getId()] = tmpPackingSite;
                        // if (tmpPU->getId() == 117104)
                        // {
                        //     std::ofstream debugFile;
                        //     debugFile.open("OpenPitonSLICE_SLICE_X38Y213", std::ios_base::app);
                        //     assert(debugFile.is_open() && debugFile.good() &&
                        //            "The path for placement Tcl dumping does not exist and please check your path "
                        //            "settings");

                        //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<First Choice >>>>>>>>>>>>>>>\n";
                        //     debugFile << PUId2PackingCLBSiteCandidate[117104]->getCLBSite()->getName() << "\n";
                        //     debugFile << PUId2PackingCLBSiteCandidate[117104]->getPriorityQueueTop();
                        //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>\n";
                        //     debugFile.close();
                        // }
                        // if (tmpPU->getId() == 116959)
                        // {
                        //     std::ofstream debugFile;
                        //     debugFile.open("OpenPitonSLICE_SLICE_X38Y213", std::ios_base::app);
                        //     assert(debugFile.is_open() && debugFile.good() &&
                        //            "The path for placement Tcl dumping does not exist and please check your path "
                        //            "settings");

                        //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<First Choice >>>>>>>>>>>>>>>\n";
                        //     debugFile << PUId2PackingCLBSiteCandidate[116959]->getCLBSite()->getName() << "\n";
                        //     debugFile << PUId2PackingCLBSiteCandidate[116959]->getPriorityQueueTop();
                        //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>\n";
                        //     debugFile.close();
                        // }
                    }
                    else
                    {
                        float oriDeltaScore =
                            PUId2PackingCLBSiteCandidate[tmpPU->getId()]->getPriorityQueueTop()->getScoreInSite() -
                            PUId2PackingCLBSiteCandidate[tmpPU->getId()]->getDetScore();
                        float newDeltaScore =
                            tmpPackingSite->getPriorityQueueTop()->getScoreInSite() - tmpPackingSite->getDetScore();
                        if (newDeltaScore > oriDeltaScore)
                        {
                            PUId2PackingCLBSiteCandidate[tmpPU->getId()] = tmpPackingSite;
                            // if (tmpPU->getId() == 117104)
                            // {
                            //     std::ofstream debugFile;
                            //     debugFile.open("OpenPitonSLICE_SLICE_X38Y213", std::ios_base::app);
                            //     assert(debugFile.is_open() && debugFile.good() &&
                            //            "The path for placement Tcl dumping does not exist and please check your path
                            //            " "settings");

                            //     debugFile
                            //         << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Second Choice >>>>>>>>>>>>>>> :
                            //         newDeltaScore"
                            //         << newDeltaScore << "  oriDeltaScore:" << oriDeltaScore << "\n";
                            //     debugFile << PUId2PackingCLBSiteCandidate[117104]->getCLBSite()->getName() << "\n";
                            //     debugFile << PUId2PackingCLBSiteCandidate[117104]->getPriorityQueueTop();
                            //     debugFile << "-----------------------------------------------------------------\n";
                            //     if (PUId2PackingCLBSiteCandidate[117104]->getDeterminedClusterInSite())
                            //         debugFile << PUId2PackingCLBSiteCandidate[117104]->getDeterminedClusterInSite();
                            //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>\n";
                            //     debugFile.close();
                            // }
                            // if (tmpPU->getId() == 116959)
                            // {
                            //     std::ofstream debugFile;
                            //     debugFile.open("OpenPitonSLICE_SLICE_X38Y213", std::ios_base::app);
                            //     assert(debugFile.is_open() && debugFile.good() &&
                            //            "The path for placement Tcl dumping does not exist and please check your path
                            //            " "settings");

                            //     debugFile
                            //         << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Second Choice >>>>>>>>>>>>>>> :
                            //         newDeltaScore"
                            //         << newDeltaScore << "  oriDeltaScore:" << oriDeltaScore << "\n";
                            //     debugFile << PUId2PackingCLBSiteCandidate[116959]->getCLBSite()->getName() << "\n";
                            //     debugFile << PUId2PackingCLBSiteCandidate[116959]->getPriorityQueueTop();
                            //     debugFile << "-----------------------------------------------------------------\n";
                            //     if (PUId2PackingCLBSiteCandidate[116959]->getDeterminedClusterInSite())
                            //         debugFile << PUId2PackingCLBSiteCandidate[116959]->getDeterminedClusterInSite();
                            //     debugFile << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>\n";
                            //     debugFile.close();
                            // }
                        }
                    }
                }
            }
        }
    }

    for (unsigned int i = 0; i < PUId2PackingCLBSite.size(); i++)
    {
        if (!PUId2PackingCLBSite[i] && PUId2PackingCLBSiteCandidate[i])
        {
            PUId2PackingCLBSite[i] = PUId2PackingCLBSiteCandidate[i];
        }
    }
}

void ParallelCLBPacker::packCLBs(int packIterNum, bool doExceptionHandling, bool debug)
{
    placementInfo->updateB2BAndGetTotalHPWL();
    placementInfo->updateElementBinGrid(); // we don't need utilization information here, we can update LUT/FF
                                           // utilization when needed.
    int iterCnt = 0;
    std::ofstream debugFile;
    debugFile.open("OpenPitonSLICE_SLICE_X38Y213");
    debugFile.close();

    packCLBsIteration(true, debug);
    print_status("ParallelCLBPacker: initial packCLBsIteration done.");
    while (true)
    {
        bool noValidPQTop = true;
        for (auto tmpPackingSite : packingSites)
        {
            if (tmpPackingSite->hasValidPQTop())
            {
                noValidPQTop = false;
            }
        }
        if (noValidPQTop)
        {
            break;
        }

        int mappedPUCnt = 0;
        for (auto packingSite : PUId2PackingCLBSite)
        {
            if (packingSite)
                mappedPUCnt++;
        }

        print_status("ParallelCLBPacker: iter#" + std::to_string(iterCnt) +
                     " #Mapped PU=" + std::to_string(mappedPUCnt));

        int deteminedCnt = 0;
        std::map<int, int> sliceSize2SliceCnt;
        sliceSize2SliceCnt.clear();
        for (auto packingSite : packingSites)
        {
            if (packingSite)
            {
                if (packingSite->getDeterminedClusterInSite())
                {
                    if (packingSite->getDeterminedClusterInSite()->getPUs().size())
                    {
                        deteminedCnt++;
                        int tmpSize = 0;
                        for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                        {
                            if (tmpPU->getType() == PlacementInfo::PlacementUnitType_Macro)
                            {
                                if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                                {
                                    tmpSize += tmpPU->getWeight();
                                    assert((unsigned int)tmpPU->getWeight() >= curMacro->getCells().size());
                                }

                                // tmpSize += tmpPU->getWeight();
                            }
                            else
                                tmpSize += 1;
                        }
                        if (sliceSize2SliceCnt.find(tmpSize) == sliceSize2SliceCnt.end())
                        {
                            sliceSize2SliceCnt[tmpSize] = 1;
                        }
                        else
                        {
                            sliceSize2SliceCnt[tmpSize]++;
                        }
                    }
                }
            }
        }
        setPULocationToPackedSite();
        int numFixed = 0;
        for (auto PU : placementInfo->getPlacementUnits())
            if (PU->isFixed())
                numFixed++;

        float fixedRatio = (float)numFixed / (float)placementInfo->getPlacementUnits().size();
        if (fixedRatio > 0.25 && WLOptimizer && iterCnt < 32)
        {
            placementInfo->updateElementBinGrid();
            timingOptimizer->pauseCounter();
            timingOptimizer->conductStaticTimingAnalysis();
            placementInfo->getPU2ClockRegionCenters().clear();
            print_status("ParallelCLBPacker: Move unfixed elements with WLOptimizer");
            WLOptimizer->GlobalPlacementQPSolve(placementInfo->getPseudoNetWeight() * 2 * (1 + fixedRatio), true, true,
                                                true, true, false, 1, timingOptimizer);
            dumpAllCellsCoordinate();
        }

        print_status("ParallelCLBPacker: current HPWL=" + std::to_string(placementInfo->updateB2BAndGetTotalHPWL()));
        print_status("ParallelCLBPacker: iter#" + std::to_string(iterCnt) +
                     " #determined Slice=" + std::to_string(deteminedCnt));

        std::string distibution = "";
        for (auto tmpPair : sliceSize2SliceCnt)
        {
            distibution += ("(" + std::to_string(tmpPair.first) + "," + std::to_string(tmpPair.second) + "), ");
        }
        print_status("ParallelCLBPacker: iter#" + std::to_string(iterCnt) + " Distribution: " + distibution);

        if (iterCnt > packIterNum)
        {
            break;
        }
        iterCnt++;

        // setPULocationToPackedSite();
        // placementInfo->updateB2BAndGetTotalHPWL();

        packCLBsIteration(false, debug);

        for (auto packingSite : packingSites)
        {
            if (packingSite)
            {
                if (packingSite->getDeterminedClusterInSite())
                {
                    assert(packingSite->getDeterminedClusterInSite()->areAllPUsValidForThisSite(PUId2PackingCLBSite,
                                                                                                packingSite));
                }
            }
        }
        // dumpCLBPacking();
    }
    print_status("ParallelCLBPacker: finish iterative packing");
    if (doExceptionHandling)
    {
        // assert(!incrementalPacking && "for incremental packing, it is not worthy to do exception handling");
        placementInfo->updateElementBinGrid();
        timingOptimizer->conductStaticTimingAnalysis();
        exceptionHandling(true);
        placementInfo->updateElementBinGrid();
        timingOptimizer->conductStaticTimingAnalysis();
        int packNum = packingSites.size();
        addDSPBRAMPackingSites();
        int replaceCnt = 1000;

#ifdef TIMINGDP
        for (int i = 0; i < 120 && replaceCnt > 5; i++)
        {
            cellId2PackingSite = std::vector<PackingCLBSite *>(placementInfo->getCells().size(), nullptr);
            PUId2PackingCLBSite.clear();
            PUId2PackingCLBSite.resize(placementInfo->getPlacementUnits().size(), nullptr);
            for (auto packingSite : packingSites)
            {
                if (packingSite)
                {
                    if (packingSite->getDeterminedClusterInSite())
                    {
                        auto cellSet = packingSite->getDeterminedClusterInSite()->getCellSet();
                        for (auto cell : cellSet)
                        {
                            cellId2PackingSite[cell->getCellId()] = packingSite;
                            auto curPU = placementInfo->getPlacementUnitByCellId(cell->getCellId());
                            PUId2PackingCLBSite[curPU->getId()] = packingSite;
                        }
                    }
                    else if (packingSite->checkIsDSPBRAMSite())
                    {
                        cellId2PackingSite[packingSite->getDSPBRAMCell()->getCellId()] = packingSite;
                        auto curPU =
                            placementInfo->getPlacementUnitByCellId(packingSite->getDSPBRAMCell()->getCellId());
                        PUId2PackingCLBSite[curPU->getId()] = packingSite;
                    }
                }
            }
            i++;
            if (i >= 100)
                replaceCnt = timingDrivenDetailedPlacement_shortestPath(i, 1.0 - 100.0 / 105.0);
            else
                replaceCnt = timingDrivenDetailedPlacement_shortestPath(i, 1.0 - i / 105.0);

            setPULocationToPackedSite();
            timingOptimizer->conductStaticTimingAnalysis();
        }

        replaceCnt = 1000;
        for (int i = 0; i < 40 && replaceCnt > 5; i++)
        {
            cellId2PackingSite = std::vector<PackingCLBSite *>(placementInfo->getCells().size(), nullptr);
            PUId2PackingCLBSite.clear();
            PUId2PackingCLBSite.resize(placementInfo->getPlacementUnits().size(), nullptr);
            for (auto packingSite : packingSites)
            {
                if (packingSite)
                {
                    if (packingSite->getDeterminedClusterInSite())
                    {
                        auto cellSet = packingSite->getDeterminedClusterInSite()->getCellSet();
                        for (auto cell : cellSet)
                        {
                            cellId2PackingSite[cell->getCellId()] = packingSite;
                            auto curPU = placementInfo->getPlacementUnitByCellId(cell->getCellId());
                            PUId2PackingCLBSite[curPU->getId()] = packingSite;
                        }
                    }
                    else if (packingSite->checkIsDSPBRAMSite())
                    {
                        cellId2PackingSite[packingSite->getDSPBRAMCell()->getCellId()] = packingSite;
                        auto curPU =
                            placementInfo->getPlacementUnitByCellId(packingSite->getDSPBRAMCell()->getCellId());
                        PUId2PackingCLBSite[curPU->getId()] = packingSite;
                    }
                }
            }
            replaceCnt = timingDrivenDetailedPlacement_swap(i);
            setPULocationToPackedSite();
            timingOptimizer->conductStaticTimingAnalysis();
        }
#endif
#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < packNum; i++)
            packingSites[i]->finalMapToSlots();
#ifdef TIMINGDP
        timingDrivenDetailedPlacement_LUTFFPairReloacationAfterSlotMapping();
#endif
    }

    // ensure the packing is legal
    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                assert(packingSite->getDeterminedClusterInSite()->checkCellCorrectness(nullptr, true));
            }
        }
    }
    dumpFinalPacking();
}

inline float getAngle(float v1x, float v1y, float v2x, float v2y)
{
    float angle = atan2(v2y, v2x) - atan2(v1y, v1x);
    if (angle > M_PI)
    {
        angle -= 2 * M_PI;
    }
    else if (angle <= -M_PI)
    {
        angle += 2 * M_PI;
    }
    return angle;
}

int ParallelCLBPacker::timingDrivenDetailedPlacement_shortestPath(int iterId, float displacementRatio)
{
    print_status("ParallelCLBPacker: conducting timing-driven detailed placement based on shortest path.");
    auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(0.9);
    std::set<PlacementInfo::PlacementUnit *> PUsTouched;
    PUsTouched.clear();
    std::ofstream outfileTcl("./DetailedPlacementRecord");
    int replaceCnt = 0;
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        std::map<int, std::vector<PackingCLBSite *>> cellId2CandidateSites;
        std::set<PackingCLBSite *> sitesCandidates;
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> site2TrialCluster;
        cellId2CandidateSites.clear();

        std::vector<int> cellIdsInCriticalPath;
        PlacementInfo::PlacementUnit *lastPU = nullptr;
        std::set<PlacementInfo::PlacementUnit *> PUsInCriticalPathSet;
        for (auto cellId : oriCellIdsInCriticalPath)
        {
            if (PUsInCriticalPathSet.find(placementInfo->getPlacementUnitByCellId(cellId)) ==
                PUsInCriticalPathSet.end())
            {
                PUsInCriticalPathSet.insert(placementInfo->getPlacementUnitByCellId(cellId));
                cellIdsInCriticalPath.push_back(cellId);
            }
        }
        // std::cout << "processing endpoint [" << designInfo->getCells()[cellIdsInCriticalPath[0]] << "]  with "
        //           << cellIdsInCriticalPath.size() << " nodes in path.\n";

        bool CellUnmapped = false;
        for (auto cellId : cellIdsInCriticalPath)
        {
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            auto curPackingSite = cellId2PackingSite[cellId];
            auto curCell = designInfo->getCells()[cellId];
            if (!curPackingSite)
            {
                CellUnmapped = true;
                // std::cout << "a cell of the PU does not map to sites: " << curCell << "\n";
                break;
            }
            assert(curPackingSite);
            cellId2CandidateSites[cellId] = std::vector<PackingCLBSite *>(1, curPackingSite);
            sitesCandidates.insert(curPackingSite);
            if (!curPackingSite->checkIsDSPBRAMSite())
                site2TrialCluster[curPackingSite] =
                    new PackingCLBSite::PackingCLBCluster(curPackingSite->getDeterminedClusterInSite());
        }
        if (CellUnmapped)
            continue;
        // find candidate sites for each possible PU

        for (int siteCandidateLimit = 1; siteCandidateLimit <= 10; siteCandidateLimit++)
        {
            float displacementThr = 5.0 * displacementRatio;
            if (displacementThr < 1)
                displacementThr = 1;
            for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
            {
                auto cellId = cellIdsInCriticalPath[orderI];
                auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
                if (PUsTouched.find(curPU) != PUsTouched.end())
                {
                    continue;
                }
                auto curCell = designInfo->getCells()[cellId];
                // std::cout << curCell << " has following candidates: \n";
                if (!curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() && !curPU->checkHasBRAM() &&
                    !curPU->checkHasDSP())
                {
                    float v1x = 0, v1y = 0, v2x = 0, v2y = 0;
                    if (orderI > 0 && orderI < cellIdsInCriticalPath.size() - 1)
                    {
                        auto predSite = cellId2PackingSite[cellIdsInCriticalPath[orderI - 1]];
                        int predSiteOrderId = orderI - 1;
                        auto curSite = cellId2PackingSite[cellId];
                        auto succSite = cellId2PackingSite[cellIdsInCriticalPath[orderI + 1]];
                        int succSiteOrderId = orderI + 1;
                        assert(predSite && curSite && succSite);
                        // while (curSite == predSite && predSiteOrderId - 1 >= 0)
                        // {
                        //     predSiteOrderId--;
                        //     predSite = cellId2PackingSite[cellIdsInCriticalPath[predSiteOrderId]];
                        // }
                        // while (curSite == succSite && succSiteOrderId + 1 <= cellIdsInCriticalPath.size() - 1)
                        // {
                        //     succSiteOrderId++;
                        //     succSite = cellId2PackingSite[cellIdsInCriticalPath[succSiteOrderId]];
                        // }
                        v1x = predSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                        v1y = predSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();
                        v2x = succSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                        v2y = succSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();
                    }
                    else if (orderI == 0)
                    {
                        auto curSite = cellId2PackingSite[cellId];
                        auto succSite = cellId2PackingSite[cellIdsInCriticalPath[orderI + 1]];
                        assert(curSite && succSite);
                        int succSiteOrderId = orderI + 1;
                        while (curSite == succSite && succSiteOrderId + 1 <= cellIdsInCriticalPath.size() - 1)
                        {
                            succSiteOrderId++;
                            succSite = cellId2PackingSite[cellIdsInCriticalPath[succSiteOrderId]];
                        }
                        v1x = v2x = succSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                        v1y = v2y = succSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();
                    }
                    else
                    {
                        auto predSite = cellId2PackingSite[cellIdsInCriticalPath[orderI - 1]];
                        auto curSite = cellId2PackingSite[cellId];
                        assert(predSite && curSite);
                        int predSiteOrderId = orderI - 1;
                        while (curSite == predSite && predSiteOrderId - 1 >= 0)
                        {
                            predSiteOrderId--;
                            predSite = cellId2PackingSite[cellIdsInCriticalPath[predSiteOrderId]];
                        }
                        v1x = v2x = predSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                        v1y = v2y = predSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();
                    }
                    if (std::fabs(v1x) + std::fabs(v1y) + std::fabs(v2x) + std::fabs(v2y) < 0.1)
                        continue;
                    if (std::fabs(getAngle(v1x, v1y, v2x, v2y)) < M_PI / 2)
                    {
                        assert(PUId2PackingCLBSite[curPU->getId()]);
                        auto candidateSitesToPlaceTheCell_cone = findNeiborSitesFromBinGrid(
                            DesignInfo::CellType_LUT4, PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->X(),
                            PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->Y(), 0, displacementThr, y2xRatio, false,
                            v1x, v1y, v2x, v2y, 20);

                        for (auto curDeviceSite : *candidateSitesToPlaceTheCell_cone)
                        {
                            if (deviceSite2PackingSite.find(curDeviceSite) == deviceSite2PackingSite.end())
                                continue;
                            PackingCLBSite *candidatePackingSite = deviceSite2PackingSite[curDeviceSite];
                            if (cellId2CandidateSites[cellId].size() >= siteCandidateLimit)
                                break;
                            bool duplicate = false;
                            for (auto existCandidate : cellId2CandidateSites[cellId])
                            {
                                if (existCandidate == candidatePackingSite)
                                    duplicate = true;
                            }
                            if (duplicate)
                                continue;
                            PackingCLBSite::PackingCLBCluster *trialCluster = nullptr;
                            if (sitesCandidates.find(candidatePackingSite) == sitesCandidates.end())
                            {
                                if (!candidatePackingSite->getDeterminedClusterInSite())
                                {
                                    auto determinedClusterInSite =
                                        new PackingCLBSite::PackingCLBCluster(candidatePackingSite);
                                    candidatePackingSite->setDeterminedClusterInSite(determinedClusterInSite);
                                }
                                trialCluster = new PackingCLBSite::PackingCLBCluster(
                                    candidatePackingSite->getDeterminedClusterInSite());
                                site2TrialCluster[candidatePackingSite] = trialCluster;
                            }
                            else
                            {
                                trialCluster = site2TrialCluster[candidatePackingSite];
                            }

                            if (trialCluster->checkAddPU(curPU))
                            {
                                assert(trialCluster->addPU(curPU));
                                cellId2CandidateSites[cellId].push_back(candidatePackingSite);
                                sitesCandidates.insert(candidatePackingSite);
                            }

                            // }
                        }

                        delete candidateSitesToPlaceTheCell_cone;
                    }
                }
            }

            for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
            {
                auto cellId = cellIdsInCriticalPath[orderI];
                auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
                auto curCell = designInfo->getCells()[cellId];
                if (PUsTouched.find(curPU) != PUsTouched.end())
                {
                    continue;
                }
                // std::cout << curCell << " has following candidates: \n";
                if (!curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() && !curPU->checkHasBRAM() &&
                    !curPU->checkHasDSP())
                {
                    std::vector<DeviceInfo::DeviceSite *> *candidateSitesToPlaceTheCell = findNeiborSitesFromBinGrid(
                        DesignInfo::CellType_LUT4, PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->X(),
                        PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->Y(), 0, 0.8 + displacementRatio, y2xRatio,
                        false);
                    if (cellId2CandidateSites[cellId].size() >= siteCandidateLimit + 1)
                        break;
                    // std::cout << curCell << " has " << candidateSitesToPlaceTheCell->size()
                    //           << " neighbors and candidates are:\n";
                    for (auto curDeviceSite : *candidateSitesToPlaceTheCell)
                    {
                        if (deviceSite2PackingSite.find(curDeviceSite) == deviceSite2PackingSite.end())
                            continue;
                        PackingCLBSite *candidatePackingSite = deviceSite2PackingSite[curDeviceSite];
                        bool duplicate = false;
                        for (auto existCandidate : cellId2CandidateSites[cellId])
                        {
                            if (existCandidate == candidatePackingSite)
                                duplicate = true;
                        }
                        if (duplicate)
                            continue;
                        PackingCLBSite::PackingCLBCluster *trialCluster = nullptr;
                        if (sitesCandidates.find(candidatePackingSite) == sitesCandidates.end())
                        {
                            if (!candidatePackingSite->getDeterminedClusterInSite())
                            {
                                auto determinedClusterInSite =
                                    new PackingCLBSite::PackingCLBCluster(candidatePackingSite);
                                candidatePackingSite->setDeterminedClusterInSite(determinedClusterInSite);
                            }
                            trialCluster = new PackingCLBSite::PackingCLBCluster(
                                candidatePackingSite->getDeterminedClusterInSite());
                            site2TrialCluster[candidatePackingSite] = trialCluster;
                        }
                        else
                        {
                            trialCluster = site2TrialCluster[candidatePackingSite];
                        }

                        if (trialCluster->checkAddPU(curPU))
                        {
                            assert(trialCluster->addPU(curPU));
                            cellId2CandidateSites[cellId].push_back(candidatePackingSite);
                            sitesCandidates.insert(candidatePackingSite);
                        }
                    }

                    delete candidateSitesToPlaceTheCell;
                }

                // for (auto packingSite : cellId2CandidateSites[cellId])
                // {
                //     std::cout << "      " << packingSite->getCLBSite()->getName() << "\n";
                // }
            }
        }

        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            PUsTouched.insert(curPU);
        }
        // calculate the shortest paths
        std::vector<std::vector<float>> shortestPath_LayerSite;
        std::vector<std::vector<int>> shortestPath_LayerSite_backtrace;
        shortestPath_LayerSite.push_back(
            std::vector<float>(cellId2CandidateSites[cellIdsInCriticalPath[0]].size(), 0.0));
        shortestPath_LayerSite_backtrace.push_back(
            std::vector<int>(cellId2CandidateSites[cellIdsInCriticalPath[0]].size(), -1));

        if (iterId > 100)
        {
            outfileTcl << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>\nCellsCandidate:\n";
            for (int i = 0; i < cellIdsInCriticalPath.size(); i++)
            {
                auto curCellId = cellIdsInCriticalPath[i];
                auto curCell = designInfo->getCells()[curCellId];
                auto &curCandidates = cellId2CandidateSites[curCellId];
                outfileTcl << "cell: " << curCell << "\n";
                for (int k = 0; k < curCandidates.size(); k++)
                {
                    outfileTcl << "      " << cellId2CandidateSites[curCellId][k]->getCLBSite()->getName() << "\n";
                }
            }
        }

        int bestEndChoice = -1;
        float bestChoiceDelay = 100000;
        for (int i = 1; i < cellIdsInCriticalPath.size(); i++)
        {
            auto predCell = cellIdsInCriticalPath[i - 1];
            auto curCell = cellIdsInCriticalPath[i];
            auto &predCandidates = cellId2CandidateSites[predCell];
            auto &curCandidates = cellId2CandidateSites[curCell];
            shortestPath_LayerSite.push_back(std::vector<float>(cellId2CandidateSites[curCell].size(), 100000.0));
            shortestPath_LayerSite_backtrace.push_back(std::vector<int>(cellId2CandidateSites[curCell].size(), -1));

            for (int j = 0; j < predCandidates.size(); j++)
            {
                for (int k = 0; k < curCandidates.size(); k++)
                {
                    float delay = timingOptimizer->getDelayByModel(
                        predCandidates[j]->getCLBSite()->X(), predCandidates[j]->getCLBSite()->Y(),
                        curCandidates[k]->getCLBSite()->X(), curCandidates[k]->getCLBSite()->Y());

                    if (shortestPath_LayerSite[i][k] > shortestPath_LayerSite[i - 1][j] + delay)
                    {
                        shortestPath_LayerSite[i][k] = shortestPath_LayerSite[i - 1][j] + delay;
                        shortestPath_LayerSite_backtrace[i][k] = j;

                        if (i == cellIdsInCriticalPath.size() - 1)
                        {
                            if (shortestPath_LayerSite[i][k] < bestChoiceDelay)
                            {
                                bestChoiceDelay = shortestPath_LayerSite[i][k];
                                bestEndChoice = k;
                            }
                        }
                    }
                }
            }
        }

        for (int i = shortestPath_LayerSite.size() - 1; i >= 0; i--)
        {
            assert(bestEndChoice >= 0);
            if (bestEndChoice > 0)
            {
                auto curCellId = cellIdsInCriticalPath[i];
                auto curPU = placementInfo->getPlacementUnitByCellId(curCellId);

                auto &curCandidates = cellId2CandidateSites[curCellId];

                auto targetPackingSite = cellId2CandidateSites[curCellId][bestEndChoice];
                if (!targetPackingSite->getDeterminedClusterInSite()->checkAddPU(curPU))
                    continue;

                cellId2CandidateSites[curCellId][0]->getDeterminedClusterInSite()->removePUToConstructDetCluster(curPU);

                assert(bestEndChoice < cellId2CandidateSites[curCellId].size());
                assert(targetPackingSite->getDeterminedClusterInSite()->addPU(curPU));
                PUId2PackingCLBSite[curPU->getId()] = targetPackingSite;
                placementInfo->addPUIntoClockColumn(curPU, targetPackingSite->getCLBSite());
                auto cellSet = targetPackingSite->getDeterminedClusterInSite()->getCellSet();
                for (auto cell : cellSet)
                {
                    cellId2PackingSite[cell->getCellId()] = targetPackingSite;
                }
                replaceCnt++;
            }
            bestEndChoice = shortestPath_LayerSite_backtrace[i][bestEndChoice];
        }

        for (auto pair : site2TrialCluster)
        {
            delete pair.second;
        }
    }

    print_status("ParallelCLBPacker: conducted timing-driven detailed placement (shortest path) and " +
                 std::to_string(replaceCnt) + " PlacementUnits are replaced.");
    outfileTcl.close();
    return replaceCnt;
}

int ParallelCLBPacker::timingDrivenDetailedPlacement_LUTFFPairReloacationAfterSlotMapping()
{
    print_status("ParallelCLBPacker: re-place some LUT-FF pairs since they are not connected via internal nets.");
    auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(1, false, 100);

    // auto &sortedTimingNodes = timingOptimizer->getSortedTimingNodes();
    // std::vector<bool> FFDirectlyDrivenButNotInOneSlot;
    // FFDirectlyDrivenButNotInOneSlot.clear();
    // FFDirectlyDrivenButNotInOneSlot.resize(designInfo->getCells().size(), 0);
    // for (auto node : sortedTimingNodes)
    // {
    //     int cellId = node->getDesignNode()->getCellId();
    //     bool shouldRelocateLUTFFPair = false;
    //     PackingCLBSite *srcSite = nullptr;
    //     auto LUTFFPair = dynamic_cast<PlacementInfo::PlacementMacro
    //     *>(placementInfo->getPlacementUnitByCellId(cellId)); DesignInfo::DesignCell *targetLUT = nullptr;
    //     DesignInfo::DesignCell *targetFF = nullptr;
    //     if (LUTFFPair)
    //     {
    //         if (LUTFFPair->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair)
    //         {
    //             auto curPackingSite = cellId2PackingSite[cellId];
    //             targetLUT = LUTFFPair->getCells()[0];
    //             targetFF = LUTFFPair->getCells()[1];
    //             auto LUTLoc = curPackingSite->getLUTSlot(targetLUT);
    //             auto FFLoc = curPackingSite->getFFSlot(targetFF);
    //             shouldRelocateLUTFFPair = !(LUTLoc[0] == FFLoc[0] && LUTLoc[1] == FFLoc[1] && LUTLoc[2] == FFLoc[2]);
    //             srcSite = curPackingSite;
    //         }
    //         FFDirectlyDrivenButNotInOneSlot[cellId] = shouldRelocateLUTFFPair;
    //     }
    // }

    // auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(1, FFDirectlyDrivenButNotInOneSlot);
    // oriCellIdsInCriticalPaths.resize(200);
    std::set<PlacementInfo::PlacementUnit *> PUsTouched;
    PUsTouched.clear();
    int replaceCnt = 0;
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        std::map<int, std::vector<PackingCLBSite *>> cellId2CandidateSites;
        std::set<PackingCLBSite *> sitesCandidates;
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> site2TrialCluster;
        cellId2CandidateSites.clear();

        std::vector<int> cellIdsInCriticalPath;
        PlacementInfo::PlacementUnit *lastPU = nullptr;
        std::set<PlacementInfo::PlacementUnit *> PUsInCriticalPathSet;
        for (auto cellId : oriCellIdsInCriticalPath)
        {
            if (PUsInCriticalPathSet.find(placementInfo->getPlacementUnitByCellId(cellId)) ==
                PUsInCriticalPathSet.end())
            {
                PUsInCriticalPathSet.insert(placementInfo->getPlacementUnitByCellId(cellId));
                cellIdsInCriticalPath.push_back(cellId);
            }
        }

        bool shouldRelocateLUTFFPair = false;
        PackingCLBSite *srcSite = nullptr;
        auto LUTFFPair = dynamic_cast<PlacementInfo::PlacementMacro *>(
            placementInfo->getPlacementUnitByCellId(oriCellIdsInCriticalPath[0]));
        DesignInfo::DesignCell *targetLUT = nullptr;
        DesignInfo::DesignCell *targetFF = nullptr;
        if (LUTFFPair)
        {
            if (LUTFFPair->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair)
            {
                auto curPackingSite = cellId2PackingSite[oriCellIdsInCriticalPath[0]];
                targetLUT = LUTFFPair->getCells()[0];
                targetFF = LUTFFPair->getCells()[1];
                auto LUTLoc = curPackingSite->getLUTSlot(targetLUT);
                auto FFLoc = curPackingSite->getFFSlot(targetFF);
                shouldRelocateLUTFFPair = !(LUTLoc[0] == FFLoc[0] && LUTLoc[1] == FFLoc[1] && LUTLoc[2] == FFLoc[2]);
                srcSite = curPackingSite;
            }
        }

        if (!shouldRelocateLUTFFPair)
            continue;

        // std::cout << "handling " << LUTFFPair << "\n";
        int siteCandidateLimit = 9;

        float displacementThr = 1.5;
        int orderI = 0;
        auto cellId = cellIdsInCriticalPath[orderI];
        auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
        if (PUsTouched.find(curPU) != PUsTouched.end())
        {
            continue;
        }

        PUsTouched.insert(curPU);
        if (!curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() && !curPU->checkHasBRAM() && !curPU->checkHasDSP())
        {
            float v1x = 0, v1y = 0, v2x = 0, v2y = 0;

            auto curSite = cellId2PackingSite[cellId];
            auto succSite = cellId2PackingSite[cellIdsInCriticalPath[orderI + 1]];
            assert(curSite && succSite);
            int succSiteOrderId = orderI + 1;
            while (curSite == succSite && succSiteOrderId + 1 <= cellIdsInCriticalPath.size() - 1)
            {
                succSiteOrderId++;
                succSite = cellId2PackingSite[cellIdsInCriticalPath[succSiteOrderId]];
            }
            v1x = v2x = succSite->getCLBSite()->X() - curSite->getCLBSite()->X();
            v1y = v2y = succSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();

            if (std::fabs(v1x) + std::fabs(v1y) + std::fabs(v2x) + std::fabs(v2y) < 0.1)
                continue;
            if (std::fabs(getAngle(v1x, v1y, v2x, v2y)) < M_PI / 2)
            {
                assert(PUId2PackingCLBSite[curPU->getId()]);
                auto candidateSitesToPlaceTheCell_cone = findNeiborSitesFromBinGrid(
                    DesignInfo::CellType_LUT4, PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->X(),
                    PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->Y(), 0, displacementThr, y2xRatio, false, v1x,
                    v1y, v2x, v2y, 20);

                for (auto curDeviceSite : *candidateSitesToPlaceTheCell_cone)
                {
                    if (deviceSite2PackingSite.find(curDeviceSite) == deviceSite2PackingSite.end())
                        continue;
                    PackingCLBSite *candidatePackingSite = deviceSite2PackingSite[curDeviceSite];
                    if (cellId2CandidateSites[cellId].size() >= siteCandidateLimit)
                        break;
                    bool duplicate = false;
                    for (auto existCandidate : cellId2CandidateSites[cellId])
                    {
                        if (existCandidate == candidatePackingSite)
                            duplicate = true;
                    }
                    if (duplicate)
                        continue;
                    PackingCLBSite::PackingCLBCluster *trialCluster = nullptr;
                    if (sitesCandidates.find(candidatePackingSite) == sitesCandidates.end())
                    {
                        if (!candidatePackingSite->getDeterminedClusterInSite())
                        {
                            auto determinedClusterInSite = new PackingCLBSite::PackingCLBCluster(candidatePackingSite);
                            candidatePackingSite->setDeterminedClusterInSite(determinedClusterInSite);
                        }
                        trialCluster =
                            new PackingCLBSite::PackingCLBCluster(candidatePackingSite->getDeterminedClusterInSite());
                        site2TrialCluster[candidatePackingSite] = trialCluster;
                    }
                    else
                    {
                        trialCluster = site2TrialCluster[candidatePackingSite];
                    }

                    if (trialCluster->checkAddPU(curPU))
                    {
                        assert(trialCluster->addPU(curPU));
                        cellId2CandidateSites[cellId].push_back(candidatePackingSite);
                        sitesCandidates.insert(candidatePackingSite);
                    }

                    // }
                }
                delete candidateSitesToPlaceTheCell_cone;
            }
        }

        // std::cout << curCell << " has following candidates: \n";
        if (!curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() && !curPU->checkHasBRAM() && !curPU->checkHasDSP())
        {
            std::vector<DeviceInfo::DeviceSite *> *candidateSitesToPlaceTheCell = findNeiborSitesFromBinGrid(
                DesignInfo::CellType_LUT4, PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->X(),
                PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->Y(), 0, displacementThr, y2xRatio, false);
            if (cellId2CandidateSites[cellId].size() >= siteCandidateLimit + 1)
                break;
            // std::cout << curCell << " has " << candidateSitesToPlaceTheCell->size()
            //           << " neighbors and candidates are:\n";
            for (auto curDeviceSite : *candidateSitesToPlaceTheCell)
            {
                if (deviceSite2PackingSite.find(curDeviceSite) == deviceSite2PackingSite.end())
                    continue;
                PackingCLBSite *candidatePackingSite = deviceSite2PackingSite[curDeviceSite];
                bool duplicate = false;
                for (auto existCandidate : cellId2CandidateSites[cellId])
                {
                    if (existCandidate == candidatePackingSite)
                        duplicate = true;
                }
                if (duplicate)
                    continue;
                PackingCLBSite::PackingCLBCluster *trialCluster = nullptr;
                if (sitesCandidates.find(candidatePackingSite) == sitesCandidates.end())
                {
                    if (!candidatePackingSite->getDeterminedClusterInSite())
                    {
                        auto determinedClusterInSite = new PackingCLBSite::PackingCLBCluster(candidatePackingSite);
                        candidatePackingSite->setDeterminedClusterInSite(determinedClusterInSite);
                    }
                    trialCluster =
                        new PackingCLBSite::PackingCLBCluster(candidatePackingSite->getDeterminedClusterInSite());
                    site2TrialCluster[candidatePackingSite] = trialCluster;
                }
                else
                {
                    trialCluster = site2TrialCluster[candidatePackingSite];
                }

                if (trialCluster->checkAddPU(curPU))
                {
                    assert(trialCluster->addPU(curPU));
                    cellId2CandidateSites[cellId].push_back(candidatePackingSite);
                    sitesCandidates.insert(candidatePackingSite);
                }
            }

            delete candidateSitesToPlaceTheCell;
        }

        // calculate the shortest paths
        int bestEndChoice = -1;
        float bestChoiceDelay = 100000;

        int predCell = cellIdsInCriticalPath[1];
        auto &curCandidates = cellId2CandidateSites[cellId];
        auto predCandidate = cellId2PackingSite[predCell];

        for (int k = 0; k < curCandidates.size(); k++)
        {
            // std::cout << "considering " << curCandidates[k]->getCLBSite()->getName() << "\n";
            if (curCandidates[k] == srcSite)
                continue;
            float delay = timingOptimizer->getDelayByModel(
                predCandidate->getCLBSite()->X(), predCandidate->getCLBSite()->Y(), curCandidates[k]->getCLBSite()->X(),
                curCandidates[k]->getCLBSite()->Y());

            if (bestChoiceDelay > delay)
            {
                // std::cout << "      better delay\n";
                if (curCandidates[k]->getSlotMapping().canDirectConnectInSlot(targetLUT, targetFF))
                {
                    bestChoiceDelay = delay;
                    bestEndChoice = k;
                    //  std::cout << "      has direct connect\n";
                }
            }
        }

        if (bestEndChoice > 0)
        {
            srcSite->getDeterminedClusterInSite()->removePUToConstructDetCluster(LUTFFPair);
            srcSite->getSlotMappingRef().removeLUTFFPair(targetLUT, targetFF);
            assert(curCandidates[bestEndChoice]->getDeterminedClusterInSite()->addPU(LUTFFPair));
            curCandidates[bestEndChoice]->getSlotMappingRef().addLUTFFPair(targetLUT, targetFF);
            PUId2PackingCLBSite[LUTFFPair->getId()] = curCandidates[bestEndChoice];
            placementInfo->addPUIntoClockColumn(LUTFFPair, curCandidates[bestEndChoice]->getCLBSite());
            replaceCnt++;
        }

        for (auto pair : site2TrialCluster)
        {
            delete pair.second;
        }
    }

    print_status("ParallelCLBPacker: conducted timing-driven detailed placement (LUT-FF pairs) and " +
                 std::to_string(replaceCnt) + " PlacementUnits are replaced.");

    return replaceCnt;
}

int ParallelCLBPacker::timingDrivenDetailedPlacement_shortestPath_intermediate()
{
    print_status("ParallelCLBPacker: conducting timing-driven detailed placement based on shortest path.");
    auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(1);
    std::set<PlacementInfo::PlacementUnit *> PUsTouched;
    PUsTouched.clear();
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();

    int replaceCnt = 0;
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        std::map<int, std::vector<PlacementInfo::Location>> cellId2CandidateLocation;
        cellId2CandidateLocation.clear();

        std::vector<int> cellIdsInCriticalPath;
        PlacementInfo::PlacementUnit *lastPU = nullptr;
        std::set<PlacementInfo::PlacementUnit *> PUsInCriticalPathSet;
        for (auto cellId : oriCellIdsInCriticalPath)
        {
            if (PUsInCriticalPathSet.find(placementInfo->getPlacementUnitByCellId(cellId)) ==
                PUsInCriticalPathSet.end())
            {
                PUsInCriticalPathSet.insert(placementInfo->getPlacementUnitByCellId(cellId));
                cellIdsInCriticalPath.push_back(cellId);
            }
        }

        bool CellUnmapped = false;
        for (auto cellId : cellIdsInCriticalPath)
        {
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            auto curPackingSite = cellId2PackingSite[cellId];
            auto curCell = designInfo->getCells()[cellId];

            cellId2CandidateLocation[cellId] = std::vector<PlacementInfo::Location>(1, cellLoc[cellId]);
        }

        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPackingSite = cellId2PackingSite[cellId];
            if (curPackingSite)
                continue;
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            auto curCell = designInfo->getCells()[cellId];
            if (PUsTouched.find(curPU) != PUsTouched.end())
            {
                continue;
            }
            // std::cout << curCell << " has following candidates: \n";
            if (!curPU->isFixed() && !curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() && !curPU->checkHasBRAM() &&
                !curPU->checkHasDSP())
            {
                auto curX = cellLoc[cellId].X;
                auto curY = cellLoc[cellId].Y;
                for (float cX = curX - 1.0; cX < curX + 1.1; cX += 0.5)
                {
                    for (float cY = curY - 1.0; cY < curY + 1.1; cY += 0.5)
                    {
                        if (std::fabs(cX - curX) + std::fabs(cY - curY) < 0.1)
                            continue;
                        PlacementInfo::Location newLoc;
                        newLoc.X = cX;
                        newLoc.Y = cY;
                        cellId2CandidateLocation[cellId].push_back(newLoc);
                    }
                }
            }
        }
        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            PUsTouched.insert(curPU);
        }
        // calculate the shortest paths
        std::vector<std::vector<float>> shortestPath_LayerSite;
        std::vector<std::vector<int>> shortestPath_LayerSite_backtrace;
        shortestPath_LayerSite.push_back(
            std::vector<float>(cellId2CandidateLocation[cellIdsInCriticalPath[0]].size(), 0.0));
        shortestPath_LayerSite_backtrace.push_back(
            std::vector<int>(cellId2CandidateLocation[cellIdsInCriticalPath[0]].size(), -1));

        int bestEndChoice = -1;
        float bestChoiceDelay = 100000;

        for (int i = 1; i < cellIdsInCriticalPath.size(); i++)
        {
            auto predCell = cellIdsInCriticalPath[i - 1];
            auto curCell = cellIdsInCriticalPath[i];
            auto &predCandidates = cellId2CandidateLocation[predCell];
            auto &curCandidates = cellId2CandidateLocation[curCell];
            shortestPath_LayerSite.push_back(std::vector<float>(cellId2CandidateLocation[curCell].size(), 100000.0));
            shortestPath_LayerSite_backtrace.push_back(std::vector<int>(cellId2CandidateLocation[curCell].size(), -1));

            for (int j = 0; j < predCandidates.size(); j++)
            {
                for (int k = 0; k < curCandidates.size(); k++)
                {
                    float predX = predCandidates[j].X, predY = predCandidates[j].Y;
                    float curX = curCandidates[k].X, curY = curCandidates[k].Y;

                    float delay = timingOptimizer->getDelayByModel(predX, predY, curX, curY);

                    if (shortestPath_LayerSite[i][k] > shortestPath_LayerSite[i - 1][j] + delay)
                    {
                        shortestPath_LayerSite[i][k] = shortestPath_LayerSite[i - 1][j] + delay;
                        shortestPath_LayerSite_backtrace[i][k] = j;

                        if (i == cellIdsInCriticalPath.size() - 1)
                        {
                            if (shortestPath_LayerSite[i][k] < bestChoiceDelay)
                            {
                                bestChoiceDelay = shortestPath_LayerSite[i][k];
                                bestEndChoice = k;
                            }
                        }
                    }
                }
            }
        }

        for (int i = shortestPath_LayerSite.size() - 1; i >= 0; i--)
        {
            if (bestEndChoice)
            {
                auto curCellId = cellIdsInCriticalPath[i];
                auto curPU = placementInfo->getPlacementUnitByCellId(curCellId);

                auto &curCandidates = cellId2CandidateLocation[curCellId];
                if (!curPU->isFixed() && !curPU->checkHasCARRY() && !curPU->checkHasLUTRAM() &&
                    !curPU->checkHasBRAM() && !curPU->checkHasDSP())
                {
                    curPU->setAnchorLocationAndForgetTheOriginalOne(curCandidates[bestEndChoice].X,
                                                                    curCandidates[bestEndChoice].Y);
                    replaceCnt++;
                }
            }
            bestEndChoice = shortestPath_LayerSite_backtrace[i][bestEndChoice];
        }
    }

    print_status("ParallelCLBPacker: conducted timing-driven detailed placement (shortest path) and " +
                 std::to_string(replaceCnt) + " PlacementUnits are replaced.");
    return replaceCnt;
}

int ParallelCLBPacker::timingDrivenDetailedPlacement_swap(int iterId)
{
    print_status("ParallelCLBPacker: conducting timing-driven detailed placement based on swaping.");
    auto oriCellIdsInCriticalPaths = timingOptimizer->findCriticalPaths(0.9);
    std::set<PlacementInfo::PlacementUnit *> PUsTouched;
    std::set<PlacementInfo::PlacementUnit *> PUsDontTouch;
    PUsTouched.clear();
    PUsDontTouch.clear();
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        for (int orderI = oriCellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = oriCellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            PUsDontTouch.insert(curPU);
        }
    }

    int replaceCnt = 0;
    for (auto oriCellIdsInCriticalPath : oriCellIdsInCriticalPaths)
    {
        std::map<int, std::vector<PackingCLBSite *>> cellId2CandidateSites;
        std::set<PackingCLBSite *> sitesCandidates;
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> site2TrialCluster;
        cellId2CandidateSites.clear();

        std::vector<int> cellIdsInCriticalPath;
        PlacementInfo::PlacementUnit *lastPU = nullptr;
        std::set<PlacementInfo::PlacementUnit *> PUsInCriticalPathSet;
        for (auto cellId : oriCellIdsInCriticalPath)
        {
            if (PUsInCriticalPathSet.find(placementInfo->getPlacementUnitByCellId(cellId)) ==
                PUsInCriticalPathSet.end())
            {
                PUsInCriticalPathSet.insert(placementInfo->getPlacementUnitByCellId(cellId));
                cellIdsInCriticalPath.push_back(cellId);
            }
        }

        bool CellUnmapped = false;
        for (auto cellId : cellIdsInCriticalPath)
        {
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            auto curPackingSite = cellId2PackingSite[cellId];
            if (!curPackingSite)
            {
                CellUnmapped = true;
                break;
            }
            assert(curPackingSite);
        }
        if (CellUnmapped)
            continue;

        // std::cout << "processing endpoint [" << designInfo->getCells()[cellIdsInCriticalPath[0]] << "]  with "
        //           << cellIdsInCriticalPath.size() << " nodes in path.\n";

        // find candidate sites for each possible PU
        for (int orderI = 1; orderI <= cellIdsInCriticalPath.size() - 2; orderI++)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            auto curCell = designInfo->getCells()[cellId];
            // std::cout << "------ " << curPU << "  \n"
            //           << " cell: " << curCell << "\n";
            if (curPU->getType() == PlacementInfo::PlacementUnitType_Macro)
                continue;
            if (PUsTouched.find(curPU) != PUsTouched.end())
            {
                // std::cout << "bypassing touched\n";
                continue;
            }
            if (curCell->isLUT())
            {
                float v1x = 0, v1y = 0, v2x = 0, v2y = 0;

                auto predSite = cellId2PackingSite[cellIdsInCriticalPath[orderI - 1]];
                int predSiteOrderId = orderI - 1;
                auto curSite = cellId2PackingSite[cellId];
                auto succSite = cellId2PackingSite[cellIdsInCriticalPath[orderI + 1]];
                int succSiteOrderId = orderI + 1;
                assert(predSite && curSite && succSite);

                float oriDelay =
                    timingOptimizer->getDelayByModel(curSite->getCLBSite()->X(), curSite->getCLBSite()->Y(),
                                                     predSite->getCLBSite()->X(), predSite->getCLBSite()->Y()) +
                    timingOptimizer->getDelayByModel(curSite->getCLBSite()->X(), curSite->getCLBSite()->Y(),
                                                     succSite->getCLBSite()->X(), succSite->getCLBSite()->Y());

                if (oriDelay < 0.3)
                    continue;

                v1x = predSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                v1y = predSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();
                v2x = succSite->getCLBSite()->X() - curSite->getCLBSite()->X();
                v2y = succSite->getCLBSite()->Y() - curSite->getCLBSite()->Y();

                // std::cout << curCell << " oriDelay=" << oriDelay << " has following candidates: \n";

                if (std::fabs(v1x) + std::fabs(v1y) < 0.1 || std::fabs(v2x) + std::fabs(v2y) < 0.1)
                    continue;

                assert(PUId2PackingCLBSite[curPU->getId()]);
                auto candidateSitesToPlaceTheCell_cone = findNeiborSitesFromBinGrid(
                    DesignInfo::CellType_LUT4, PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->X(),
                    PUId2PackingCLBSite[curPU->getId()]->getCLBSite()->Y(), 0, 1.1, y2xRatio, false);
                PackingCLBSite *bestCandidatePackingSite = nullptr;
                PlacementInfo::PlacementUnit *bestSwapCandidatePU = nullptr;
                float bestOverheadSlack = -1000000;
                float oriOverhead = timingOptimizer->getWorstSlackOfCell(curCell);
                // std::cout << curCell << " oriOverhead=" << oriOverhead << " has following candidates: \n";
                for (auto curDeviceSite : *candidateSitesToPlaceTheCell_cone)
                {
                    if (deviceSite2PackingSite.find(curDeviceSite) == deviceSite2PackingSite.end())
                        continue;
                    PackingCLBSite *candidatePackingSite = deviceSite2PackingSite[curDeviceSite];
                    if (curSite == candidatePackingSite)
                        continue;

                    if (!candidatePackingSite->getDeterminedClusterInSite())
                    {
                        continue;
                    }

                    float newDelay =
                        timingOptimizer->getDelayByModel(candidatePackingSite->getCLBSite()->X(),
                                                         candidatePackingSite->getCLBSite()->Y(),
                                                         predSite->getCLBSite()->X(), predSite->getCLBSite()->Y()) +
                        timingOptimizer->getDelayByModel(candidatePackingSite->getCLBSite()->X(),
                                                         candidatePackingSite->getCLBSite()->Y(),
                                                         succSite->getCLBSite()->X(), succSite->getCLBSite()->Y());

                    // std::cout << "      " << candidatePackingSite->getCLBSite()->getName() << " newDelay=" <<
                    // newDelay
                    //           << "\n";
                    if (newDelay > oriDelay - 0.05)
                        continue;

                    // find lowest slack LUT
                    for (auto tmpPU : candidatePackingSite->getDeterminedClusterInSite()->getPUs())
                    {
                        if (tmpPU->isLocked() || PUsDontTouch.find(tmpPU) != PUsDontTouch.end())
                            continue;
                        if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                        {
                            auto targetCell = unpackedCell->getCell();

                            // try to swap LUTs
                            if (targetCell->isLUT())
                            {
                                auto trialTargetCluster = new PackingCLBSite::PackingCLBCluster(
                                    candidatePackingSite->getDeterminedClusterInSite());
                                trialTargetCluster->removePUToConstructDetCluster(tmpPU);
                                if (trialTargetCluster->addPU(curPU))
                                {
                                    auto trialCurrentCluster = new PackingCLBSite::PackingCLBCluster(
                                        PUId2PackingCLBSite[curPU->getId()]->getDeterminedClusterInSite());
                                    trialCurrentCluster->removePUToConstructDetCluster(curPU);
                                    if (trialCurrentCluster->addPU(tmpPU))
                                    {
                                        // std::cout << "           " << tmpPU
                                        //           << " worstSlack=" <<
                                        //           timingOptimizer->getWorstSlackOfCell(targetCell)
                                        //           << "\n";
                                        float overhead = timingOptimizer->getWorstSlackOfCell(targetCell);
                                        if (overhead > oriOverhead + 0.2)
                                        {
                                            if (overhead > bestOverheadSlack)
                                            {
                                                bestOverheadSlack = overhead;
                                                bestCandidatePackingSite = candidatePackingSite;
                                                bestSwapCandidatePU = tmpPU;
                                            }
                                        }
                                    }
                                    delete trialCurrentCluster;
                                }
                                delete trialTargetCluster;
                            }
                        }
                    }
                }

                if (bestCandidatePackingSite)
                {
                    auto trialTargetCluster = bestCandidatePackingSite->getDeterminedClusterInSite();
                    trialTargetCluster->removePUToConstructDetCluster(bestSwapCandidatePU);
                    assert(trialTargetCluster->addPU(curPU));
                    auto trialCurrentCluster = PUId2PackingCLBSite[curPU->getId()]->getDeterminedClusterInSite();
                    trialCurrentCluster->removePUToConstructDetCluster(curPU);
                    assert(trialCurrentCluster->addPU(bestSwapCandidatePU));
                    auto unpackedCell_curPU = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU);
                    auto unpackedCell_bestSwapCandidatePU =
                        dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(bestSwapCandidatePU);
                    cellId2PackingSite[unpackedCell_curPU->getCell()->getCellId()] = bestCandidatePackingSite;
                    cellId2PackingSite[unpackedCell_bestSwapCandidatePU->getCell()->getCellId()] =
                        PUId2PackingCLBSite[curPU->getId()];
                    replaceCnt++;
                }
                delete candidateSitesToPlaceTheCell_cone;
            }
        }

        for (int orderI = cellIdsInCriticalPath.size() - 1; orderI >= 0; orderI--)
        {
            auto cellId = cellIdsInCriticalPath[orderI];
            auto curPU = placementInfo->getPlacementUnitByCellId(cellId);
            PUsTouched.insert(curPU);
        }
    }

    print_status("ParallelCLBPacker: conducted timing-driven detailed placement (swaping) and " +
                 std::to_string(replaceCnt) + " PlacementUnits are replaced.");
    return replaceCnt;
}

void ParallelCLBPacker::checkPackedPUsAndUnpackedPUs()
{
    packedPUs.clear();
    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                {
                    assert(packedPUs.find(tmpPU) == packedPUs.end());
                    packedPUs.insert(tmpPU);
                }
            }
        }
    }

    unpackedPUs.clear();
    unpackedPUsVec.clear();
    for (auto tmpCell : placementInfo->getCells())
    {
        if (tmpCell->isLUT() || tmpCell->isFF())
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
            if (packedPUs.find(tmpPU) != packedPUs.end())
                continue;
            if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
            {
                if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_CARRY ||
                    tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MCLB)
                {
                    continue;
                }
            }
            if (unpackedPUs.find(tmpPU) == unpackedPUs.end())
            {
                unpackedPUs.insert(tmpPU);
                unpackedPUsVec.push_back(tmpPU);
            }
        }
    }
}

void ParallelCLBPacker::exceptionHandling(bool verbose)
{
    checkPackedPUsAndUnpackedPUs();
    print_status("ParallelCLBPacker: start exceptionHandling.");

    std::vector<bool> isLegalizedPU(placementUnits.size(), false);

    float Dc = maxD * 0.5;

    auto inputUnpackedPUsVec = unpackedPUsVec;

    std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> processedPUs;
    processedPUs.clear();

    PUPoints.clear();
    for (auto PU : inputUnpackedPUsVec)
    {
        if (!PU->isLocked())
            PUPoints.emplace_back(PU);
    }

    while (PUPoints.size())
    {
        timingOptimizer->getPUId2Slack(true); // update PU slack information
        // build k-d tree
        kdt::KDTree<PULocation> kdtree(PUPoints, y2xRatio);
        processedPUs.clear();
        print_status("ParallelCLBPacker: starting parallel ripping up for " + std::to_string(PUPoints.size()) +
                     " PUs and current displacement threshold for ripping up is " + std::to_string(Dc));
        // loop until there is no unprocessed PU
        while (processedPUs.size() < PUPoints.size())
        {
            std::vector<PlacementInfo::PlacementUnit *> noRipUpOverlapPUs;
            std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> coveredPUs;
            noRipUpOverlapPUs.clear();
            coveredPUs.clear();

            // find unprocessed PUs and try to rip up them with Dc
            for (auto &tmpPUPoint : PUPoints)
            {
                // avoid that the potential ripup site cause conflict so the PUs to be processed should far enough
                // between each othters.
                if (coveredPUs.find(tmpPUPoint.getPU()) != coveredPUs.end())
                    continue;
                if (processedPUs.find(tmpPUPoint.getPU()) != processedPUs.end())
                    continue;
                if (isLegalizedPU[tmpPUPoint.getPU()->getId()])
                    continue;
                noRipUpOverlapPUs.push_back(tmpPUPoint.getPU());
                coveredPUs.insert(tmpPUPoint.getPU());
                processedPUs.insert(tmpPUPoint.getPU());
                std::vector<int> indices = kdtree.radiusSearch(tmpPUPoint, 4 * Dc + 2);
                for (auto tmpInd : indices)
                {
                    assert(std::fabs(PUPoints[tmpInd].getPU()->X() - tmpPUPoint.getPU()->X()) +
                               y2xRatio * std::fabs(PUPoints[tmpInd].getPU()->Y() - tmpPUPoint.getPU()->Y()) <=
                           4 * Dc + 2);
                    coveredPUs.insert(PUPoints[tmpInd].getPU());
                }
            }

            // print_status("ParallelCLBPacker: find " + std::to_string(noRipUpOverlapPUs.size()) +
            //              " relatively independent PUs from " + std::to_string(PUPoints.size()) + " PUs. Total " +
            //              std::to_string(processedPUs.size()) + " have been tried.");

            unsigned int numNoRipUpOverlapPUs = noRipUpOverlapPUs.size();
            // involvedPackingSite2PU.clear();

#pragma omp parallel for schedule(dynamic)
            for (unsigned int i = 0; i < numNoRipUpOverlapPUs; i++)
            {
                isLegalizedPU[noRipUpOverlapPUs[i]->getId()] = exceptionPULegalize(noRipUpOverlapPUs[i], Dc, verbose);
            }

            int successCnt = 0;
            for (unsigned int i = 0; i < numNoRipUpOverlapPUs; i++)
            {
                successCnt += isLegalizedPU[noRipUpOverlapPUs[i]->getId()];
            }
            // print_status("ParallelCLBPacker: successfully rip up " + std::to_string(successCnt) + " PUs of the " +
            //              std::to_string(noRipUpOverlapPUs.size()) + " PUs.");
        }
        // remove those successfully ripup PU from PUPoints
        int unprocessedCnt = 0;
        for (unsigned int i = 0; i < PUPoints.size(); i++)
        {
            if (!isLegalizedPU[PUPoints[i].getPU()->getId()])
            {
                PUPoints[unprocessedCnt] = PUPoints[i];
                unprocessedCnt++;
            }
        }
        PUPoints.resize(unprocessedCnt);
        Dc += 0.3 * maxD;

        if (placementInfo->isDensePlacement())
            clockRegionAware = false;

        if (Dc > 300)
        {
            Dc = maxD * 0.5 - 1;
            for (unsigned int i = 0; i < PUPoints.size(); i++)
            {
                std::cout << PUPoints[i].getPU() << "\n=================================================\n";
            }
            assert("fail to handle exceptions" && false);
            break;
        }
        if (Dc < 1)
            Dc = 1;
        print_status("ParallelCLBPacker: there are " + std::to_string(unprocessedCnt) +
                     " PUs left to be legalized and current displacement threshold for ripping up is " +
                     std::to_string(Dc));
        int tmpFFCnt = 0, tmpLUTCnt = 0;
        float avgFFWeight = 0, avgLUTWeight = 0;
        for (unsigned int i = 0; i < PUPoints.size(); i++)
        {
            auto tmpPU = PUPoints[i].getPU();
            if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
            {
                if (unpackCell->getCell()->isFF())
                {
                    tmpFFCnt++;
                    avgFFWeight += placementInfo->getActualOccupation(unpackCell->getCell());
                }
                else if (unpackCell->getCell()->isLUT())
                {
                    tmpLUTCnt++;
                    avgLUTWeight += placementInfo->getActualOccupation(unpackCell->getCell());
                }
            }
            else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
            {
                assert(curMacro->getMacroType() != PlacementInfo::PlacementMacro::PlacementMacroType_CARRY &&
                       curMacro->getMacroType() != PlacementInfo::PlacementMacro::PlacementMacroType_MCLB);
                assert(!curMacro->isPacked());
                for (auto tmpCell : curMacro->getCells())
                {
                    if (tmpCell->isFF())
                    {
                        tmpFFCnt++;
                        avgFFWeight += placementInfo->getActualOccupation(tmpCell);
                    }
                    else if (tmpCell->isLUT())
                    {
                        tmpLUTCnt++;
                        avgLUTWeight += placementInfo->getActualOccupation(tmpCell);
                    }
                }
            }
        }
        avgFFWeight /= tmpFFCnt;
        avgLUTWeight /= tmpLUTCnt;
        print_info("there are " + std::to_string(tmpFFCnt) + " FFs(avgW=" + std::to_string(avgFFWeight) + ") and " +
                   std::to_string(tmpLUTCnt) + " LUTs(avgW=" + std::to_string(avgLUTWeight) + ") ");
        placementInfo->updateElementBinGrid();
        timingOptimizer->conductStaticTimingAnalysis();
    }

    if (verbose)
    {
        print_status("ParallelCLBPacker::exceptionHandling done!");
    }
}

bool ParallelCLBPacker::exceptionPULegalize(PlacementInfo::PlacementUnit *curPU, float displacementThreshold,
                                            bool verbose)
{

    std::vector<DeviceInfo::DeviceSite *> *candidateSitesToPlaceThePU = nullptr;
    if (displacementThreshold < 4)
        candidateSitesToPlaceThePU = findNeiborSitesFromBinGrid(DesignInfo::CellType_LUT4, curPU->X(), curPU->Y(), 0,
                                                                displacementThreshold, y2xRatio, clockRegionAware);
    else
        candidateSitesToPlaceThePU =
            findNeiborSitesFromBinGrid(DesignInfo::CellType_LUT4, curPU->X(), curPU->Y(), displacementThreshold - 3,
                                       displacementThreshold, y2xRatio, clockRegionAware);
    std::vector<siteWithScore> sitesToRipUp;
    sitesToRipUp.clear();
    for (auto tmpSite : *candidateSitesToPlaceThePU)
    {
        float lambda1 = 0.02;
        float lambda2 = 1.0;
        float lambda3 = 4.0;
        assert(deviceSite2PackingSite.find(tmpSite) != deviceSite2PackingSite.end());
        PackingCLBSite *packingSite = deviceSite2PackingSite[tmpSite];

        // if (involvedPackingSite2PU.find(packingSite) != involvedPackingSite2PU.end())
        // {
        //     if (involvedPackingSite2PU[packingSite] != curPU)
        //     {
        //         std::cout << "displacementThr: " << displacementThreshold << "\n";
        //         std::cout << "curPU: " << curPU << "\n";
        //         std::cout << "prevPU: " << involvedPackingSite2PU[packingSite] << "\n";
        //         std::cout << " packingSite: " << packingSite->getCLBSite()->getName()
        //                   << " X:" << packingSite->getCLBSite()->X() << " Y:" << packingSite->getCLBSite()->Y() <<
        //                   "\n";
        //         assert(false);
        //     }
        // }

        // involvedPackingSite2PU[packingSite] = curPU;

        float changeHPWL = 0;
        for (auto tmpNet : *(curPU->getNetsSetPtr()))
        {
            if (tmpNet->getUnits().size() > 64) // ignore large net
                continue;
            changeHPWL += tmpNet->getNewHPWLByTrying(curPU, tmpSite->X(), tmpSite->Y(), y2xRatio);
            changeHPWL -= tmpNet->getHPWL(y2xRatio);
            assert(!std::isnan(changeHPWL));
        }

        float packedScore = 0;
        float score = -lambda1 * changeHPWL;
        // std::cout << "site: " << packingSite->getCLBSite()->getName() << " changeHPWL:" << changeHPWL << "\n";
        if (packingSite->getDeterminedClusterInSite())
        {
            packingSite->getDeterminedClusterInSite()->updateScoreInSite();
            packedScore = packingSite->getDeterminedClusterInSite()->getScoreInSite();
            score += -lambda2 * packedScore - lambda3 * packingSite->getDeterminedClusterInSite()->getTotalCellWeight();
            // std::cout << "packedScore: " << packedScore << "\n"
            //           << " CellWeight:" << packingSite->getDeterminedClusterInSite()->getTotalCellWeight() << "\n"
            //           << "DeterminedClusterInSite: " << packingSite->getDeterminedClusterInSite() << "\n";
        }

        sitesToRipUp.emplace_back(packingSite, score);
    }
    std::sort(sitesToRipUp.begin(), sitesToRipUp.end(), [](const siteWithScore &a, const siteWithScore &b) -> bool {
        return a.score == b.score
                   ? (a.site->getCLBSite()->getElementIdInParent() > b.site->getCLBSite()->getElementIdInParent())
                   : (a.score > b.score);
    });

    // if (verbose)
    // {
    //     for (auto pair : sitesToRipUp)
    //     {
    //         std::cout << "site: " << pair.site->getCLBSite()->getName() << " X:" << pair.site->getCLBSite()->X()
    //                   << " Y:" << pair.site->getCLBSite()->Y() << " score:" << pair.score << "\n";
    //     }
    //     std::cout << "\n";
    //     std::cout.flush();
    // }
    delete candidateSitesToPlaceThePU;

    // std::cout << "start check sites\n";
    std::cout.flush();
    for (auto pair : sitesToRipUp)
    {

        PackingCLBSite *packingSite = pair.site;
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> packingSite2DeterminedCluster;
        packingSite2DeterminedCluster.clear(); // record the original determined cluster for rolling back

        // std::cout << "start check sites: ripUpAndLegalizae\n";
        if (ripUpAndLegalizae(packingSite, curPU, displacementThreshold, packingSite2DeterminedCluster, verbose))
        {
            return true;
        }
        else
        {
            assert(packingSite->getDeterminedClusterInSite());
            for (auto pair : packingSite2DeterminedCluster)
            {
                if (pair.first->getDeterminedClusterInSite())
                    delete pair.first->getDeterminedClusterInSite();
                pair.first->setDeterminedClusterInSite(pair.second);
            }
        }
    }
    return false;
    // std::cout << "handled unpacked PU: " << curPU << "\n candidateSites:\n";
    // for (auto pair : sitesToRipUp)
    // {
    //     std::cout << "site: " << pair.site->getName() << " X:" << pair.site->X() << " Y:" << pair.site->Y()
    //               << " score:" << pair.score << "\n";
    // }

    // std::cout << "\n";
    // std::cout.flush();
    // assert(false);
}

bool ParallelCLBPacker::ripUpAndLegalizae(
    PackingCLBSite *curTargetPackingSite, PlacementInfo::PlacementUnit *curPU, float displacementThreshold,
    std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> &packingSite2DeterminedCluster, bool verbose)
{
    PackingCLBSite::PackingCLBCluster *backup_determinedCluster = nullptr;

    if (curTargetPackingSite->getDeterminedClusterInSite())
    {

        backup_determinedCluster =
            new PackingCLBSite::PackingCLBCluster(curTargetPackingSite->getDeterminedClusterInSite());

        // if (verbose)
        // {
        //     std::cout << "ripUpAndLegalizae target site: " << curTargetPackingSite->getCLBSite()->getName()
        //               << " X:" << curTargetPackingSite->getCLBSite()->X()
        //               << " Y:" << curTargetPackingSite->getCLBSite()->Y() << "\n";
        //     std::cout << "and its determined cluster is :\n" << backup_determinedCluster << "\n";
        //     std::cout.flush();
        // }
        delete curTargetPackingSite->getDeterminedClusterInSite();
    }

    packingSite2DeterminedCluster[curTargetPackingSite] = backup_determinedCluster;
    curTargetPackingSite->setDeterminedClusterInSite(nullptr);

    if (curTargetPackingSite->checkIsCarrySite())
    {
        curTargetPackingSite->addCarry();
    }
    else if (curTargetPackingSite->checkIsLUTRAMSite())
    {
        curTargetPackingSite->addLUTRAMMacro();
    }
    else
    {
        curTargetPackingSite->setDeterminedClusterInSite(new PackingCLBSite::PackingCLBCluster(curTargetPackingSite));
    }

    // if (verbose)
    // {
    //     std::cout << "start ripUpAndLegalizae\n";
    //     std::cout.flush();
    // }

    assert(curTargetPackingSite->getDeterminedClusterInSite());
    assert(curTargetPackingSite->getDeterminedClusterInSite()->getPUs().size() == 0);
    if (curTargetPackingSite->getDeterminedClusterInSite()->addPU(curPU))
    {
        // if (verbose)
        // {
        //     std::cout << "successfully curTargetPackingSite->getDeterminedClusterInSite()->addPU(curPU)\n";
        //     std::cout.flush();
        // }

        if (backup_determinedCluster)
        {

            std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> evictedPUs = backup_determinedCluster->getPUs();
            std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> notEvictedPUs;
            notEvictedPUs.clear();

            auto &PUId2Slack = timingOptimizer->getPUId2Slack();
            std::vector<PUWithScore> PUsWithSlack;
            PUsWithSlack.clear();
            for (auto evictedPU : evictedPUs)
            {
                PUsWithSlack.emplace_back(evictedPU, PUId2Slack[evictedPU->getId()]);
            }
            std::sort(PUsWithSlack.begin(), PUsWithSlack.end(), [](const PUWithScore &a, const PUWithScore &b) -> bool {
                return a.score == b.score ? (a.PU->getId() > b.PU->getId()) : (a.score < b.score);
            });

            for (auto evictedPU_scorePair : PUsWithSlack)
            {
                auto evictedPU = evictedPU_scorePair.PU;
                assert(curTargetPackingSite->getDeterminedClusterInSite()->getPUs().find(evictedPU) ==
                       curTargetPackingSite->getDeterminedClusterInSite()->getPUs().end());
                // if (verbose)
                // {
                //     std::cout << " curTargetPackingSite->getDeterminedClusterInSite()->tryAddPU(evictedPU)\n";
                //     std::cout.flush();
                // }
                assert(notEvictedPUs.find(evictedPU) == notEvictedPUs.end());
                if (curTargetPackingSite->getDeterminedClusterInSite()->tryAddPU(evictedPU))
                {
                    // if (verbose)
                    // {
                    //     std::cout << " before
                    //     curTargetPackingSite->getDeterminedClusterInSite()->addPU(evictedPU)\n"; std::cout.flush();
                    // }
                    // if (!curTargetPackingSite->getDeterminedClusterInSite()->addPU(evictedPU))
                    // {
                    //     std::cout << "cluster:\n" << curTargetPackingSite->getDeterminedClusterInSite() << "\n";
                    //     curTargetPackingSite->getDeterminedClusterInSite()->addPUFailReason(evictedPU);
                    //     assert(false);
                    // }
                    curTargetPackingSite->getDeterminedClusterInSite()->incrementalUpdateScoreInSite(evictedPU);
                    curTargetPackingSite->getDeterminedClusterInSite()->getHash();
                    notEvictedPUs.insert(evictedPU);

                    // if (verbose)
                    // {
                    //     std::cout
                    //         << " successfully
                    //         curTargetPackingSite->getDeterminedClusterInSite()->addPU(evictedPU)\n";
                    //     std::cout.flush();
                    // }
                }
            }

            for (auto evictedPU : evictedPUs)
            {
                if (notEvictedPUs.find(evictedPU) != notEvictedPUs.end())
                    continue;
                // std::cout << "rip up the evicted PU\n";
                std::vector<DeviceInfo::DeviceSite *> *candidateSitesToPlaceThePU = nullptr;
                if (displacementThreshold < 4)
                    candidateSitesToPlaceThePU = findNeiborSitesFromBinGrid(
                        DesignInfo::CellType_LUT4, curTargetPackingSite->getCLBSite()->X(),
                        curTargetPackingSite->getCLBSite()->Y(), 0, displacementThreshold, y2xRatio, clockRegionAware);
                else
                    candidateSitesToPlaceThePU =
                        findNeiborSitesFromBinGrid(DesignInfo::CellType_LUT4, curTargetPackingSite->getCLBSite()->X(),
                                                   curTargetPackingSite->getCLBSite()->Y(), displacementThreshold - 3,
                                                   displacementThreshold, y2xRatio, clockRegionAware);

                float highestScoreIncrease = -1e5;
                PackingCLBSite::PackingCLBCluster *bestClusterToPack = nullptr;
                for (auto tmpSite : *candidateSitesToPlaceThePU)
                {
                    assert(deviceSite2PackingSite.find(tmpSite) != deviceSite2PackingSite.end());
                    PackingCLBSite *tmpPackingSiteForEvictim = deviceSite2PackingSite[tmpSite];

                    // if (involvedPackingSite2PU.find(tmpPackingSiteForEvictim) != involvedPackingSite2PU.end())
                    // {
                    //     if (involvedPackingSite2PU[tmpPackingSiteForEvictim] != curPU)
                    //     {
                    //         std::cout << "displacementThr: " << displacementThreshold << "\n";
                    //         std::cout << "curPU: " << curPU << "\n";
                    //         std::cout << "prevPU: " << involvedPackingSite2PU[tmpPackingSiteForEvictim] << "\n";
                    //         std::cout << " tmpPackingSiteForEvictim: "
                    //                   << tmpPackingSiteForEvictim->getCLBSite()->getName()
                    //                   << " X:" << tmpPackingSiteForEvictim->getCLBSite()->X()
                    //                   << " Y:" << tmpPackingSiteForEvictim->getCLBSite()->Y() << "\n";
                    //         assert(false);
                    //     }
                    // }
                    // involvedPackingSite2PU[tmpPackingSiteForEvictim] = curPU;

                    PackingCLBSite::PackingCLBCluster *tmpCluster = nullptr;
                    if (tmpPackingSiteForEvictim->getDeterminedClusterInSite())
                    {
                        tmpCluster = new PackingCLBSite::PackingCLBCluster(
                            tmpPackingSiteForEvictim->getDeterminedClusterInSite());
                    }
                    else
                    {
                        tmpCluster = new PackingCLBSite::PackingCLBCluster(tmpPackingSiteForEvictim);
                    }

                    tmpCluster->updateScoreInSite();
                    float oriScore = tmpCluster->getScoreInSite();
                    // std::cout << "adding into tmpCluster \n";
                    if (tmpCluster->addPU(evictedPU))
                    {

                        tmpCluster->incrementalUpdateScoreInSite(evictedPU);
                        tmpCluster->getHash();
                        float newScore = tmpCluster->getScoreInSite();
                        // std::cout << "successfully and oriScore=" << oriScore << " newScore=" << newScore << "\n";
                        if (!bestClusterToPack)
                        {
                            bestClusterToPack = tmpCluster;
                            highestScoreIncrease = newScore - oriScore;
                            // std::cout << "the cluster is better. Update bestClusterToPack\n";
                        }
                        else
                        {
                            if (highestScoreIncrease < newScore - oriScore)
                            {
                                delete bestClusterToPack;
                                bestClusterToPack = tmpCluster;
                                highestScoreIncrease = newScore - oriScore;
                                // std::cout << "the cluster is better. Update bestClusterToPack\n";
                            }
                            else
                            {
                                delete tmpCluster;
                                // std::cout << "the cluster is not good enough.\n";
                            }
                        }
                    }
                    else
                    {
                        // std::cout << "failed\n";
                        delete tmpCluster;
                    }
                }
                delete candidateSitesToPlaceThePU;
                if (bestClusterToPack)
                {
                    PackingCLBSite *evictPUToPackingSite = bestClusterToPack->getParentPackingCLB();
                    if (packingSite2DeterminedCluster.find(evictPUToPackingSite) ==
                        packingSite2DeterminedCluster.end()) // if the determined cluster has no backup yet, backup
                    // the determined cluster of the site might be changed by multiple PU but we only record the most
                    // original determined cluster
                    {
                        if (evictPUToPackingSite->getDeterminedClusterInSite())
                        {
                            auto backUp = new PackingCLBSite::PackingCLBCluster(
                                evictPUToPackingSite->getDeterminedClusterInSite());
                            packingSite2DeterminedCluster[evictPUToPackingSite] =
                                backUp; // back up for later rolling back.
                        }
                        else
                        {
                            // if the site has no determined cluster, it must be not site for CLB with CARRY
                            assert(!evictPUToPackingSite->checkIsPrePackedSite());
                            packingSite2DeterminedCluster[evictPUToPackingSite] =
                                nullptr; // back up for later rolling back.
                        }
                    }
                    if (evictPUToPackingSite->getDeterminedClusterInSite())
                        delete evictPUToPackingSite->getDeterminedClusterInSite();
                    evictPUToPackingSite->setDeterminedClusterInSite(bestClusterToPack);
                }
                else
                {
                    // std::cout << "failed to ripUpAndLegalizae target site: "
                    //           << curTargetPackingSite->getCLBSite()->getName()
                    //           << " X:" << curTargetPackingSite->getCLBSite()->X()
                    //           << " Y:" << curTargetPackingSite->getCLBSite()->Y() << "because evictedPU: " <<
                    //           evictedPU
                    //           << "\n cannot be added into it\n";
                    return false;
                }
            }
        }
    }
    else
    {
        // std::cout << "failed to ripUpAndLegalizae target site: " << curTargetPackingSite->getCLBSite()->getName()
        //           << " X:" << curTargetPackingSite->getCLBSite()->X()
        //           << " Y:" << curTargetPackingSite->getCLBSite()->Y() << "because curPU cannot be added into it\n";
        return false;
    }
    // std::cout << "successfully ripUpAndLegalizae target site: " << curTargetPackingSite->getCLBSite()->getName()
    //           << " X:" << curTargetPackingSite->getCLBSite()->X() << " Y:" << curTargetPackingSite->getCLBSite()->Y()
    //           << "\n";
    return true;
}

std::vector<DeviceInfo::DeviceSite *> *
ParallelCLBPacker::findNeiborSitesFromBinGrid(DesignInfo::DesignCellType curCellType, float targetX, float targetY,
                                              float displacementLowerbound, float displacementUpperbound,
                                              float y2xRatio, bool clockRegionAware, float v1x, float v1y, float v2x,
                                              float v2y, int numLimit)
{
    assert(displacementLowerbound < displacementUpperbound);
    // please note that the input DesignCell is only used to find the corresponding binGrid for site search.
    std::vector<DeviceInfo::DeviceSite *> *res = new std::vector<DeviceInfo::DeviceSite *>();
    res->clear();

    int binIdX, binIdY;
    placementInfo->getGridXY(targetX, targetY, binIdX, binIdY);

    auto sharedTypeIds = placementInfo->getPotentialBELTypeIDs(curCellType);

    int clockRegionX, clockRegionY;
    placementInfo->getDeviceInfo()->getClockRegionByLocation(targetX, targetY, clockRegionX, clockRegionY);

    float vxCom, vyCom;
    vxCom = v1x + v2x;
    vyCom = v1y + v2y;

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
            for (auto tmpSite : curBin->getCorrespondingSites())
            {
                float vx = tmpSite->X() - targetX;
                float vy = tmpSite->Y() - targetY;
                float tmpPUDis = fabs(vx) + y2xRatio * fabs(vy);
                if (tmpPUDis > displacementLowerbound && tmpPUDis <= displacementUpperbound &&
                    std::fabs(getAngle(vx, vy, vxCom, vyCom)) < 0.42 * M_PI)
                {
                    int siteClockRegionX, siteClockRegionY;
                    placementInfo->getDeviceInfo()->getClockRegionByLocation(tmpSite->X(), targetY, siteClockRegionX,
                                                                             siteClockRegionY);
                    if (siteClockRegionX != clockRegionX && clockRegionAware)
                        continue;

                    res->push_back(tmpSite);
                }
            }

            if (res->size() > numLimit)
                break;

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
                    if (reachedBinXYs.find(nextXY) != reachedBinXYs.end())
                    {
                        continue;
                    }

                    std::vector<float> binXB;
                    binXB.push_back(nextBin->left());
                    binXB.push_back(nextBin->right());

                    std::vector<float> binYB;
                    binYB.push_back(nextBin->top());
                    binYB.push_back(nextBin->bottom());

                    bool overlap = false;
                    for (auto bX : binXB)
                    {
                        for (auto bY : binYB)
                        {
                            float vx = bX - targetX;
                            float vy = bY - targetY;
                            if (std::fabs(getAngle(vx, vy, vxCom, vyCom)) < 0.42 * M_PI)
                            {
                                overlap = true;
                            }
                        }
                    }

                    if (reachedBinXYs.find(nextXY) == reachedBinXYs.end())
                    {
                        reachedBinXYs.insert(nextXY);
                        if (overlap)
                            binXYqueue.push(nextXY);
                    }
                }
            }
        }
    }

    return res;
}

std::vector<DeviceInfo::DeviceSite *> *
ParallelCLBPacker::findNeiborSitesFromBinGrid(DesignInfo::DesignCellType curCellType, float targetX, float targetY,
                                              float displacementLowerbound, float displacementUpperbound,
                                              float y2xRatio, bool clockRegionAware)
{
    assert(displacementLowerbound < displacementUpperbound);
    // please note that the input DesignCell is only used to find the corresponding binGrid for site search.
    std::vector<DeviceInfo::DeviceSite *> *res = new std::vector<DeviceInfo::DeviceSite *>();
    res->clear();

    int binIdX, binIdY;
    placementInfo->getGridXY(targetX, targetY, binIdX, binIdY);

    auto sharedTypeIds = placementInfo->getPotentialBELTypeIDs(curCellType);

    int clockRegionX, clockRegionY;
    placementInfo->getDeviceInfo()->getClockRegionByLocation(targetX, targetY, clockRegionX, clockRegionY);

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
            for (auto tmpSite : curBin->getCorrespondingSites())
            {
                float tmpPUDis = fabs(targetX - tmpSite->X()) + y2xRatio * fabs(targetY - tmpSite->Y());
                if (tmpPUDis > displacementLowerbound && tmpPUDis <= displacementUpperbound)
                {
                    int siteClockRegionX, siteClockRegionY;
                    placementInfo->getDeviceInfo()->getClockRegionByLocation(tmpSite->X(), targetY, siteClockRegionX,
                                                                             siteClockRegionY);
                    if (siteClockRegionX != clockRegionX && clockRegionAware)
                        continue;

                    res->push_back(tmpSite);
                }
            }

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

    return res;
}

void ParallelCLBPacker::setPULocationToPackedSite()
{
    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                if (packingSite->getDeterminedClusterInSite()->getPUs().size())
                {
                    for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                    {
                        tmpPU->setAnchorLocationAndForgetTheOriginalOne(packingSite->getCLBSite()->X(),
                                                                        packingSite->getCLBSite()->Y());
                        tmpPU->setFixed();
                    }
                }
            }
        }
    }
    placementInfo->updateElementBinGrid();
    placementInfo->updateB2BAndGetTotalHPWL();
}

void ParallelCLBPacker::setPUsToBePacked()
{
    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                if (packingSite->getDeterminedClusterInSite()->getPUs().size())
                {
                    for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                    {
                        tmpPU->setPacked();
                    }
                }
            }
        }
    }
}

void ParallelCLBPacker::updatePackedMacro(bool setPUPseudoNetToCLBSite, bool setCLBFixed)
{
    std::vector<PlacementInfo::PlacementUnit *> packedPUs;
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();
    std::map<PlacementInfo::PlacementUnit *, float> PU2X, PU2Y;
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> PU2Sites;
    PU2Sites.clear();
    PU2X.clear();
    PU2Y.clear();
    for (auto packingSite : packingSites)
    {
        if (packingSite)
        {
            if (packingSite->getDeterminedClusterInSite())
            {
                if (packingSite->getDeterminedClusterInSite()->getPUs().size() >= 1)
                {
                    std::vector<DesignInfo::DesignCellType> cellsToAdd_cellType(0);
                    std::vector<DesignInfo::DesignCell *> cellsToAdd(0);
                    for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                    {
                        if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                        {
                            cellsToAdd.push_back(unpackCell->getCell());
                            cellsToAdd_cellType.push_back(unpackCell->getCell()->getCellType());
                        }
                        else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                        {
                            assert(curMacro->getMacroType() !=
                                       PlacementInfo::PlacementMacro::PlacementMacroType_CARRY &&
                                   curMacro->getMacroType() != PlacementInfo::PlacementMacro::PlacementMacroType_MCLB);
                            assert(!curMacro->isPacked());
                            int cellIdInMacro = 0;
                            for (auto tmpCell : curMacro->getCells())
                            {
                                cellsToAdd.push_back(tmpCell);
                                cellsToAdd_cellType.push_back(curMacro->getVirtualCellType(cellIdInMacro));
                                cellIdInMacro++;
                            }
                        }
                        placementUnits[tmpPU->getId()] = nullptr;
                        placementInfo->deleteLegalizationInfoFor(tmpPU);
                        delete tmpPU;
                    }
                    assert(cellsToAdd.size() >= 1);
                    PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
                        cellsToAdd[0]->getName(), -1, PlacementInfo::PlacementMacro::PlacementMacroType_LCLB);
                    curMacro->setAnchorLocationAndForgetTheOriginalOne(packingSite->getCLBSite()->X(),
                                                                       packingSite->getCLBSite()->Y());
                    int totalWeight = 0;
                    for (unsigned int tmpId = 0; tmpId < cellsToAdd.size(); tmpId++)
                    {
                        curMacro->addCell(cellsToAdd[tmpId], cellsToAdd_cellType[tmpId], 0, 0.0);
                        cellLoc[cellsToAdd[tmpId]->getCellId()].X = packingSite->getCLBSite()->X();
                        cellLoc[cellsToAdd[tmpId]->getCellId()].Y = packingSite->getCLBSite()->Y();

                        cellId2PlacementUnit[cellsToAdd[tmpId]->getCellId()] = curMacro;
                        cellInMacros.insert(cellsToAdd[tmpId]);
                        totalWeight += placementInfo->getCompatiblePlacementTable()
                                           ->cellType2sharedBELTypeOccupation[cellsToAdd_cellType[tmpId]];
                    }
                    curMacro->setWeight(totalWeight);
                    curMacro->setPacked();
                    if (setCLBFixed)
                        curMacro->setFixed();
                    assert((unsigned int)curMacro->getWeight() >= curMacro->getCells().size());
                    curMacro->addOccupiedSite(0.0, 1);
                    packedPUs.push_back(curMacro);
                    placementUnits.push_back(curMacro);
                    if (setPUPseudoNetToCLBSite)
                    {
                        PU2X[curMacro] = packingSite->getCLBSite()->X();
                        PU2Y[curMacro] = packingSite->getCLBSite()->Y();
                        PU2Sites[curMacro] = std::vector<DeviceInfo::DeviceSite *>();
                        PU2Sites[curMacro].push_back(packingSite->getCLBSite());
                    }

                    for (auto LUTPair : packingSite->getDeterminedClusterInSite()->getPairedLUTs())
                    {
                        placementInfo->setDeterminedOccupation(LUTPair.first->getCellId(), 1);
                        placementInfo->setDeterminedOccupation(LUTPair.second->getCellId(), 1);
                    }

                    if (cellsToAdd.size() > 8)
                    {
                        for (auto &tmpControlSet : packingSite->getDeterminedClusterInSite()->getFFControlSets())
                        {
                            float estimatedFFUtil = 4.0 / (float)tmpControlSet.getSize();

                            for (auto tmpFF : tmpControlSet.getFFs())
                            {
                                placementInfo->setDeterminedOccupation(tmpFF->getCellId(), estimatedFFUtil);
                            }
                        }
                    }
                }
            }
        }
    }
    unsigned int validCnt = 0;
    placementMacros.clear();
    placementUnpackedCells.clear();
    for (unsigned int tmpPUId = 0; tmpPUId < placementUnits.size(); tmpPUId++)
    {
        if (placementUnits[tmpPUId])
        {
            placementUnits[validCnt] = placementUnits[tmpPUId];
            placementUnits[validCnt]->renewId(validCnt);
            if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(placementUnits[validCnt]))
            {
                placementUnpackedCells.push_back(unpackCell);
            }
            else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(placementUnits[validCnt]))
            {
                placementMacros.push_back(curMacro);
                assert((unsigned int)curMacro->getWeight() >= curMacro->getCells().size());
            }
            validCnt++;
        }
    }
    placementUnits.resize(validCnt);

    print_status("ParallelCLBPacker Updating Cell-PlacementUnit Mapping");
    placementInfo->updateCells2PlacementUnits();

    // since new placement units might be generated, we need to update the net information
    print_status("ParallelCLBPacker Loading Nets");
    placementInfo->reloadNets();

    // ensure that each cell is only assigned into one placementUnit.
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

    placementInfo->setPULegalSite(PU2Sites);
    placementInfo->setPULegalXY(PU2X, PU2Y);
    placementInfo->updateB2BAndGetTotalHPWL();
}

bool hasLUT62(ParallelCLBPacker::PackingCLBSite::SiteBELMapping &slots)
{
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (slots.LUTs[i][j][k])
                {
                    if (slots.LUTs[i][j][k]->isVirtualCell())
                        continue;
                    if (slots.LUTs[i][j][k]->getOriCellType() == DesignInfo::CellType_LUT6_2)
                        return true;
                }
            }
        }
    }
    return false;
}

void ParallelCLBPacker::addDSPBRAMPackingSites()
{
    for (auto &DSPBRAM_LegalSitePair : placementInfo->getPULegalSite())
    {
        if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(DSPBRAM_LegalSitePair.first))
        {
            if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_BRAM)
            {
                assert(tmpMacro->getCells().size() == DSPBRAM_LegalSitePair.second.size());
                for (unsigned int i = 0; i < tmpMacro->getCells().size(); i++)
                {
                    auto curCell = tmpMacro->getCells()[i];
                    auto targetSite = DSPBRAM_LegalSitePair.second[i];
                    if (!curCell->isVirtualCell())
                    {
                        if (curCell->getOriCellType() == DesignInfo::CellType_RAMB36E2 ||
                            curCell->getOriCellType() == DesignInfo::CellType_FIFO36E2)
                        {
                            assert(targetSite->getSiteY() % 2 == 0);
                            PackingCLBSite *tmpPackingSite = new PackingCLBSite(
                                placementInfo, targetSite, unchangedIterationThr, numNeighbor, deltaD, curD, maxD,
                                PQSize, y2xRatio, HPWLWeight, PUId2PackingCLBSite);
                            tmpPackingSite->setDSPBRAM(curCell);
                            deviceSite2PackingSite[targetSite] = tmpPackingSite;
                            packingSites.push_back(tmpPackingSite);
                        }
                        else if (curCell->getOriCellType() == DesignInfo::CellType_RAMB18E2 ||
                                 curCell->getOriCellType() == DesignInfo::CellType_FIFO18E2)
                        {
                            PackingCLBSite *tmpPackingSite = new PackingCLBSite(
                                placementInfo, targetSite, unchangedIterationThr, numNeighbor, deltaD, curD, maxD,
                                PQSize, y2xRatio, HPWLWeight, PUId2PackingCLBSite);
                            tmpPackingSite->setDSPBRAM(curCell);
                            deviceSite2PackingSite[targetSite] = tmpPackingSite;
                            packingSites.push_back(tmpPackingSite);
                        }
                        else
                        {
                            assert(false && "undefined situtation");
                        }
                    }
                    else
                    {
                        assert(targetSite->getSiteY() % 2 == 1);
                    }
                }
            }
            else if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_DSP)
            {
                assert(tmpMacro->getCells().size() == DSPBRAM_LegalSitePair.second.size());
                for (unsigned int i = 0; i < tmpMacro->getCells().size(); i++)
                {
                    auto curCell = tmpMacro->getCells()[i];
                    auto targetSite = DSPBRAM_LegalSitePair.second[i];
                    if (!curCell->isVirtualCell())
                    {
                        PackingCLBSite *tmpPackingSite =
                            new PackingCLBSite(placementInfo, targetSite, unchangedIterationThr, numNeighbor, deltaD,
                                               curD, maxD, PQSize, y2xRatio, HPWLWeight, PUId2PackingCLBSite);
                        tmpPackingSite->setDSPBRAM(curCell);
                        deviceSite2PackingSite[targetSite] = tmpPackingSite;
                        packingSites.push_back(tmpPackingSite);
                    }
                    else
                    {
                        assert(false && "there should be no virtual DSP");
                    }
                }
            }
            else
            {
                // assert(false && "undefined situtation");
            }
        }
        else if (auto tmpUnppackedCell =
                     dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(DSPBRAM_LegalSitePair.first))
        {
            assert(1 == DSPBRAM_LegalSitePair.second.size());
            auto curCell = tmpUnppackedCell->getCell();
            auto targetSite = DSPBRAM_LegalSitePair.second[0];
            if (curCell->getOriCellType() == DesignInfo::CellType_RAMB18E2)
            {
                PackingCLBSite *tmpPackingSite =
                    new PackingCLBSite(placementInfo, targetSite, unchangedIterationThr, numNeighbor, deltaD, curD,
                                       maxD, PQSize, y2xRatio, HPWLWeight, PUId2PackingCLBSite);
                tmpPackingSite->setDSPBRAM(curCell);
                deviceSite2PackingSite[targetSite] = tmpPackingSite;
                packingSites.push_back(tmpPackingSite);
            }
            else if (curCell->getOriCellType() == DesignInfo::CellType_DSP48E2)
            {
                PackingCLBSite *tmpPackingSite =
                    new PackingCLBSite(placementInfo, targetSite, unchangedIterationThr, numNeighbor, deltaD, curD,
                                       maxD, PQSize, y2xRatio, HPWLWeight, PUId2PackingCLBSite);
                tmpPackingSite->setDSPBRAM(curCell);
                deviceSite2PackingSite[targetSite] = tmpPackingSite;
                packingSites.push_back(tmpPackingSite);
            }
            else
            {
                // assert(false && "undefined situtation");
            }
        }
    }
}

void ParallelCLBPacker::dumpDSPBRAMPlacementTcl(std::ofstream &outfileTcl)
{
    for (auto &DSPBRAM_LegalSitePair : placementInfo->getPULegalSite())
    {
        std::string placementStr = "";
        if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(DSPBRAM_LegalSitePair.first))
        {
            if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_BRAM)
            {
                assert(tmpMacro->getCells().size() == DSPBRAM_LegalSitePair.second.size());
                for (unsigned int i = 0; i < tmpMacro->getCells().size(); i++)
                {
                    auto curCell = tmpMacro->getCells()[i];
                    auto targetSite = DSPBRAM_LegalSitePair.second[i];
                    if (!curCell->isVirtualCell())
                    {
                        if (curCell->getOriCellType() == DesignInfo::CellType_RAMB36E2 ||
                            curCell->getOriCellType() == DesignInfo::CellType_FIFO36E2)
                        {
                            assert(targetSite->getSiteY() % 2 == 0);
                            placementStr += "  " + curCell->getName() + " RAMB36_X" +
                                            std::to_string(targetSite->getSiteX()) + "Y" +
                                            std::to_string(targetSite->getSiteY() / 2) + "\n";
                        }
                        else if (curCell->getOriCellType() == DesignInfo::CellType_RAMB18E2 ||
                                 curCell->getOriCellType() == DesignInfo::CellType_FIFO18E2)
                        {
                            if (targetSite->getSiteY() % 2)
                            {
                                placementStr +=
                                    "  " + curCell->getName() + " " + targetSite->getName() + "/RAMB18E2_U" + "\n";
                            }
                            else
                            {
                                placementStr +=
                                    "  " + curCell->getName() + " " + targetSite->getName() + "/RAMB18E2_L" + "\n";
                            }
                        }
                        else
                        {
                            assert(false && "undefined situtation");
                        }
                    }
                    else
                    {
                        assert(targetSite->getSiteY() % 2 == 1);
                    }
                }
            }
            else if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_DSP)
            {
                assert(tmpMacro->getCells().size() == DSPBRAM_LegalSitePair.second.size());
                for (unsigned int i = 0; i < tmpMacro->getCells().size(); i++)
                {
                    auto curCell = tmpMacro->getCells()[i];
                    auto targetSite = DSPBRAM_LegalSitePair.second[i];
                    if (!curCell->isVirtualCell())
                    {
                        placementStr += "  " + curCell->getName() + " " + targetSite->getName() + "/DSP_ALU" + "\n";
                    }
                    else
                    {
                        assert(false && "there should be no virtual DSP");
                    }
                }
            }
            else
            {
                // assert(false && "undefined situtation");
            }
        }
        else if (auto tmpUnppackedCell =
                     dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(DSPBRAM_LegalSitePair.first))
        {
            assert(1 == DSPBRAM_LegalSitePair.second.size());
            auto curCell = tmpUnppackedCell->getCell();
            auto targetSite = DSPBRAM_LegalSitePair.second[0];
            if (curCell->getOriCellType() == DesignInfo::CellType_RAMB18E2)
            {
                if (targetSite->getSiteY() % 2)
                {
                    placementStr += "  " + curCell->getName() + " " + targetSite->getName() + "/RAMB18E2_U" + "\n";
                }
                else
                {
                    placementStr += "  " + curCell->getName() + " " + targetSite->getName() + "/RAMB18E2_L" + "\n";
                }
            }
            else if (curCell->getOriCellType() == DesignInfo::CellType_DSP48E2)
            {
                placementStr += "  " + curCell->getName() + " " + targetSite->getName() + "/DSP_ALU" + "\n";
            }
            else
            {
                // assert(false && "undefined situtation");
            }
        }
        if (placementStr != "")
        {
            std::string oriBrace = "[";
            std::string newBrace = "\\[";
            outfileTcl << "set result "
                       << "[catch {place_cell {" << placementStr << "}}]\n"
                       << "if {$result} {\n incr errorNum \n";
            std::vector<std::string> splitItems;
            splitItems.clear();
            strSplit(placementStr, splitItems, "\n");
            for (auto item : splitItems)
            {
                replaceAll(item, oriBrace, newBrace);
                outfileTcl << "puts $fo \"" << item << " \"\n";
            }
            outfileTcl << "\n}\n";
            placementStr = "";
        }
    }
}

bool containLUTRAMCells(PlacementInfo::PlacementUnit *curPU)
{
    if (auto unpackedCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
    {
        return unpackedCell->getCell()->originallyIsLUTRAM();
    }
    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
    {
        for (auto tmpCell : curMacro->getCells())
        {
            if (tmpCell->originallyIsLUTRAM())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void ParallelCLBPacker::dumpCLBPlacementTcl(std::ofstream &outfileTcl, bool packingRelatedToLUT6_2)
{
    std::string placementStr = "";
    int cnt = 0;
    for (auto tmpPackingSite : packingSites)
    {
        auto slotMapping = tmpPackingSite->getSlotMapping();
        auto CLBSite = tmpPackingSite->getCLBSite();
        if ((packingRelatedToLUT6_2 && !hasLUT62(slotMapping)) || (!packingRelatedToLUT6_2 && hasLUT62(slotMapping)))
            continue;

        if (tmpPackingSite->getDeterminedClusterInSite())
        {

            if (tmpPackingSite->checkIsCarrySite())
            {
                cnt++;
                placementStr += slotMapping.Carry->getName() + " " + CLBSite->getName() + "/CARRY8  \n";
            }
            else if (tmpPackingSite->checkIsMuxSite())
            {
                for (int i = 0; i < 2; i++)
                {
                    if (slotMapping.MuxF8[i])
                    {
                        if (slotMapping.MuxF8[i]->isVirtualCell())
                            continue;
                        assert(slotMapping.MuxF8[i]->getOriCellType() == DesignInfo::CellType_MUXF8);
                        cnt++;
                        placementStr += "  " + slotMapping.MuxF8[i]->getName() + "  " + CLBSite->getName() + "/" +
                                        slotMapping.MuxF8SlotNames[i] + "  \n";
                    }
                    for (int j = 0; j < 2; j++)
                    {
                        if (slotMapping.MuxF7[i][j])
                        {
                            if (slotMapping.MuxF7[i][j]->isVirtualCell())
                                continue;
                            cnt++;
                            placementStr += "  " + slotMapping.MuxF7[i][j]->getName() + "  " + CLBSite->getName() +
                                            "/" + slotMapping.MuxF7SlotNames[i][j] + "  \n";
                        }
                    }
                }
            }
            else if (tmpPackingSite->checkIsLUTRAMSite())
            {
                auto tmpMacro = tmpPackingSite->getLUTRAMMacro();
                if (containLUTRAMCells(tmpMacro))
                {
                    if (tmpMacro->getFixedCellInfoVec().size() % 2 ==
                        0) // if the number of fixed cells is odd and >1, there might be weird errors from Vivado.
                    {
                        if (tmpMacro->getFixedCellInfoVec().size() > 0)
                        {
                            for (unsigned int i = 0; i < tmpMacro->getFixedCellInfoVec().size(); i++)
                            {
                                DesignInfo::DesignCell *curCell = tmpMacro->getFixedCellInfoVec()[i].cell;
                                placementStr += "  " + curCell->getName() + "  " + CLBSite->getName() + "/" +
                                                tmpMacro->getFixedCellInfoVec()[i].BELName + "  \n";
                            }
                        }
                        else
                        {
                            placementStr += "  " + tmpMacro->getName() + "  " + CLBSite->getName() + "/H6LUT  \n";
                        }
                    }
                    else
                    {
                        if (tmpMacro->getFixedCellInfoVec().size() == 1)
                        {
                            placementStr += "  " + tmpMacro->getName() + "  " + CLBSite->getName() + "/H6LUT  \n";
                        }
                    }
                }
            }

            for (int i = 0; i < 2; i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        if (slotMapping.LUTs[i][j][k])
                        {
                            if (slotMapping.LUTs[i][j][k]->isVirtualCell())
                                continue;
                            assert(slotMapping.LUTs[i][j][k]->isLUT());
                            char LUTCode = (i * 4 + k) + 'A';
                            if (j == 0)
                            {
                                std::string LUTSiteName = std::string(1, LUTCode) + "6LUT";
                                cnt++;
                                placementStr += "  " + slotMapping.LUTs[i][j][k]->getName() + " " + CLBSite->getName() +
                                                "/" + LUTSiteName + "\n";
                            }
                            else
                            {
                                if (slotMapping.LUTs[i][j][k]->getOriCellType() == DesignInfo::CellType_LUT6_2)
                                    continue;

                                std::string LUTSiteName = std::string(1, LUTCode) + "5LUT";
                                cnt++;
                                placementStr += "  " + slotMapping.LUTs[i][j][k]->getName() + " " + CLBSite->getName() +
                                                "/" + LUTSiteName + "\n";
                            }
                        }
                    }
                }
            }
            for (int i = 0; i < 2; i++)
            {
                for (int k = 0; k < 4; k++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        if (slotMapping.FFs[i][j][k])
                        {
                            if (slotMapping.FFs[i][j][k]->isVirtualCell())
                                continue;
                            assert(slotMapping.FFs[i][j][k]->isFF());
                            auto PU = placementInfo->getPlacementUnitByCell(slotMapping.FFs[i][j][k]);
                            if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(PU))
                            {
                                if (tmpMacro->getMacroType() ==
                                    PlacementInfo::PlacementMacro::
                                        PlacementMacroType_LCLB) // currently we failed to predict the LCLB macros, so
                                                                 // leave it to Vivado placement.
                                    continue;
                            }
                            char FFCode = (i * 4 + k) + 'A';
                            if (j == 0)
                            {
                                std::string FFSiteName = std::string(1, FFCode) + "FF";
                                cnt++;
                                placementStr += "  " + slotMapping.FFs[i][j][k]->getName() + " " + CLBSite->getName() +
                                                "/" + FFSiteName + "\n";
                            }
                            else
                            {
                                std::string FFSiteName = std::string(1, FFCode) + "FF2";
                                cnt++;
                                placementStr += "  " + slotMapping.FFs[i][j][k]->getName() + " " + CLBSite->getName() +
                                                "/" + FFSiteName + "\n";
                            }
                        }
                    }
                }
            }
        }
        if (cnt > 100)
        {
            if (placementStr != "")
            {
                std::string oriBrace = "[";
                std::string newBrace = "\\[";
                outfileTcl << "set result "
                           << "[catch {place_cell {" << placementStr << "}}]\n"
                           << "if {$result} {\n incr errorNum \n";
                std::vector<std::string> splitItems;
                splitItems.clear();
                strSplit(placementStr, splitItems, "\n");
                for (auto item : splitItems)
                {
                    replaceAll(item, oriBrace, newBrace);
                    outfileTcl << "puts $fo \"" << item << " \"\n";
                }
                outfileTcl << "\n}\n";
                placementStr = "";
            }
            cnt = 0;
        }
    }
    if (placementStr != "")
    {
        outfileTcl << "set result "
                   << "[catch {place_cell {" << placementStr << "}}]\n"
                   << "if {$result} {\n incr errorNum \nputs $fo \"" << placementStr << "\"\n}\n";
    }
}

void ParallelCLBPacker::dumpPlacementTcl(std::string dumpTclFile)
{
    std::ofstream outfileTcl(dumpTclFile);
    assert(outfileTcl.is_open() && outfileTcl.good() &&
           "The path for placement Tcl dumping does not exist and please check your path settings");
    outfileTcl << "set script_path [ file dirname [ file normalize [ info script ] ] ]\n";
    outfileTcl
        << "set fo [open \"${script_path}/initialPlacementError\" \"w\"]\nplace_design -unplace\nset errorNum 0\n";
    dumpDSPBRAMPlacementTcl(outfileTcl);
    dumpCLBPlacementTcl(outfileTcl, true);
    // Except LUT6_2 cells, remove the packing information in Vivado and use our packing
    outfileTcl << "set_property HLUTNM {} [get_cells -hierarchical *]\n";
    outfileTcl << "set_property SOFT_HLUTNM {} [get_cells -hierarchical *]\n";
    dumpCLBPlacementTcl(outfileTcl, false);
    // outfileTcl << "place_design\n";
    outfileTcl << "$errorNum\n";
    outfileTcl << "close $fo \n"
                  "set a [open \"${script_path}/initialPlacementError\"]\n"
                  "set lines [split [read $a] \"\\n\"]\n"
                  "close $a;\n"
                  "exec rm \"${script_path}/initialPlacementError\"\n"
                  "set b [open \"${script_path}/placementError\" \"w\"]\n"
                  "set lineCnt 0\n"
                  "set placeBatch {}\n"
                  "foreach line $lines {\n"
                  "   incr lineCnt\n"
                  "   set placeBatch [concat $placeBatch $line]\n"
                  "   if {$lineCnt == 2 } {\n"
                  "       set result [catch {place_cell  $placeBatch }]\n"
                  "       if {$result} {\n"
                  "           puts $b $placeBatch\n"
                  "       }\n"
                  "       set lineCnt 0\n"
                  "       set placeBatch \"\"\n"
                  "    }\n"
                  "}\n"
                  "close $b\n";
    outfileTcl << "place_design\nroute_design\n";
    outfileTcl.close();
}

void ParallelCLBPacker::dumpFinalPacking()
{
    if (JSONCfg.find("DumpCLBPacking") != JSONCfg.end())
    {
        std::string dumpTclFile =
            JSONCfg["DumpCLBPacking"] + "-" + packerName + "-" + std::to_string(DumpCLBPackingCnt) + ".tcl";
        std::string dumpFile =
            JSONCfg["DumpCLBPacking"] + "-" + packerName + "-" + std::to_string(DumpCLBPackingCnt) + ".gz";

        print_status("ParallelCLBPacker: dumping CLBPacking archieve to: " + dumpFile);

        std::stringstream outfile0;

        for (auto tmpPackingSite : packingSites)
        {
            if (tmpPackingSite->getDeterminedClusterInSite())
            {
                outfile0 << tmpPackingSite->getDeterminedClusterInSite() << "\n";
                if (tmpPackingSite->checkIsCarrySite())
                {
                    outfile0 << "LUTSlots:\n";
                    for (int i = 0; i < 2; i++)
                    {
                        for (int k = 0; k < 4; k++)
                        {
                            for (int j = 0; j < 2; j++)
                            {
                                if (tmpPackingSite->getSlotMapping().LUTs[i][j][k])
                                {
                                    outfile0 << "i,j,k:" << i << "," << j << "," << k << "   "
                                             << tmpPackingSite->getSlotMapping().LUTs[i][j][k] << "\n";
                                }
                            }
                        }
                    }
                }
                if (tmpPackingSite->checkIsCarrySite())
                {
                    outfile0 << "FFSlots:\n";
                    for (int i = 0; i < 2; i++)
                    {
                        for (int k = 0; k < 4; k++)
                        {
                            for (int j = 0; j < 2; j++)
                            {
                                if (tmpPackingSite->getSlotMapping().FFs[i][j][k])
                                {
                                    outfile0 << "i,j,k:" << i << "," << j << "," << k << "   "
                                             << tmpPackingSite->getSlotMapping().FFs[i][j][k] << "\n";
                                }
                            }
                        }
                    }
                }
            }
        }

        writeStrToGZip(dumpFile, outfile0);
        print_status("ParallelCLBPacker: dumped CLBPacking archieve to: " + dumpFile);

        std::vector<PackingCLBSite *> PUId2PackingCLBSite;
        PUId2PackingCLBSite.clear();
        PUId2PackingCLBSite.resize(placementInfo->getPlacementUnits().size(), nullptr);

        std::vector<DeviceInfo::DeviceSite *> PUId2NeighborSite(placementInfo->getPlacementUnits().size(), nullptr);
        int mappedPUCnt = 0;
        for (auto packingSite : packingSites)
        {
            if (packingSite)
            {
                if (packingSite->getDeterminedClusterInSite())
                {
                    for (auto tmpPU : packingSite->getDeterminedClusterInSite()->getPUs())
                    {
                        assert(!PUId2PackingCLBSite[tmpPU->getId()]);
                        PUId2PackingCLBSite[tmpPU->getId()] = packingSite;
                        mappedPUCnt++;
                    }
                }
            }
        }
        for (auto packingSite : packingSites)
        {
            if (packingSite)
            {
                for (auto tmpPU : packingSite->getNeighborPUs())
                {
                    if (PUId2PackingCLBSite[tmpPU->getId()])
                        continue;
                    if (!PUId2NeighborSite[tmpPU->getId()])
                    {
                        PUId2NeighborSite[tmpPU->getId()] = packingSite->getCLBSite();
                    }
                    else
                    {
                        float oriDis = std::abs(PUId2NeighborSite[tmpPU->getId()]->X() - tmpPU->X()) +
                                       y2xRatio * std::abs(PUId2NeighborSite[tmpPU->getId()]->Y() - tmpPU->Y());
                        float newDis = std::abs(packingSite->getCLBSite()->X() - tmpPU->X()) +
                                       y2xRatio * std::abs(packingSite->getCLBSite()->Y() - tmpPU->Y());
                        if (oriDis > newDis)
                        {
                            PUId2NeighborSite[tmpPU->getId()] = packingSite->getCLBSite();
                        }
                    }
                }
            }
        }

        print_info("#mappedPUCnt = " + std::to_string(mappedPUCnt));

        dumpFile = JSONCfg["DumpCLBPacking"] + "-" + packerName + "-unpackablePUs-" +
                   std::to_string(DumpCLBPackingCnt) + ".gz";
        print_status("ParallelCLBPacker: dumping unpackable PUs archieve to: " + dumpFile);
        std::stringstream outfile1;

        int PUNeedMappingCnt = 0;
        std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> dumpedPUs;
        dumpedPUs.clear();
        for (auto tmpCell : placementInfo->getCells())
        {
            if (tmpCell->isLUT() || tmpCell->isFF())
            {
                auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
                if (tmpPU->isPacked())
                    continue;
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                {
                    if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_CARRY ||
                        tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MCLB)
                    {
                        continue;
                    }
                }
                if (dumpedPUs.find(tmpPU) != dumpedPUs.end())
                    continue;
                dumpedPUs.insert(tmpPU);
                PUNeedMappingCnt++;
                if (PUId2PackingCLBSite[tmpPU->getId()])
                    continue;
                outfile1 << tmpPU;
                if (PUId2NeighborSite[tmpPU->getId()])
                {
                    outfile1 << "candidate neighbor site: " << PUId2NeighborSite[tmpPU->getId()]->getName()
                             << " X:" << PUId2NeighborSite[tmpPU->getId()]->X()
                             << " Y:" << PUId2NeighborSite[tmpPU->getId()]->Y() << "\n";
                    outfile1 << tmpCell
                             << " estimated util:" << placementInfo->getActualOccupationByCellId(tmpCell->getCellId())
                             << "\n";
                }
                else
                {
                    std::vector<DeviceInfo::DeviceSite *> *candidateSites =
                        placementInfo->findNeiborSiteFromBinGrid(tmpCell, tmpPU->X(), tmpPU->Y(), 5.0, 10);
                    if (candidateSites->size())
                    {
                        for (auto tmpSite : *candidateSites)
                        {
                            if (!PUId2NeighborSite[tmpPU->getId()])
                            {
                                PUId2NeighborSite[tmpPU->getId()] = tmpSite;
                            }
                            else
                            {
                                float oriDis = std::abs(PUId2NeighborSite[tmpPU->getId()]->X() - tmpPU->X()) +
                                               y2xRatio * std::abs(PUId2NeighborSite[tmpPU->getId()]->Y() - tmpPU->Y());
                                float newDis = std::abs(tmpSite->X() - tmpPU->X()) +
                                               y2xRatio * std::abs(tmpSite->Y() - tmpPU->Y());
                                if (oriDis > newDis)
                                {
                                    PUId2NeighborSite[tmpPU->getId()] = tmpSite;
                                }
                            }
                        }
                    }
                    delete candidateSites;
                    outfile1 << "closed neighbor site: " << PUId2NeighborSite[tmpPU->getId()]->getName()
                             << " X:" << PUId2NeighborSite[tmpPU->getId()]->X()
                             << " Y:" << PUId2NeighborSite[tmpPU->getId()]->Y() << "\n";
                    outfile1 << tmpCell
                             << " estimated util:" << placementInfo->getActualOccupationByCellId(tmpCell->getCellId())
                             << "\n";
                }
            }
        }
        writeStrToGZip(dumpFile, outfile1);
        print_status("ParallelCLBPacker: dumped unpackable PUs archieve to: " + dumpFile);
        print_info("#PUNeedMappingCnt = " + std::to_string(PUNeedMappingCnt));

        print_status("ParallelCLBPacker: dumping placementTcl archieve to: " + dumpTclFile);
        dumpPlacementTcl(dumpTclFile);
        print_status("ParallelCLBPacker: dumped placementTcl archieve to: " + dumpTclFile);
        DumpCLBPackingCnt++;
    }
}

void ParallelCLBPacker::dumpAllCellsCoordinate()
{
    if (JSONCfg.find("DumpAllCoordTrace") != JSONCfg.end())
    {
        std::string dumpFile =
            JSONCfg["DumpAllCoordTrace"] + "-Packing-" + std::to_string(allCoordinateDumpCnt) + ".gz";
        print_status("GlobalPlacer: dumping coordinate archieve to: " + dumpFile);
        allCoordinateDumpCnt++;
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
                    outfile0 << cellX << " " << cellY << " " << curCell->getName() << "\n";
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

                        if (curMacro->getCell(vId))
                            outfile0 << cellX << " " << cellY << " " << curMacro->getCell(vId)->getName() << "\n";
                        else
                            outfile0 << cellX << " " << cellY << "\n";
                    }
                }
            }
            writeStrToGZip(dumpFile, outfile0);
            print_status("GlobalPlacer: dumped coordinate archieve to: " + dumpFile);
        }
    }
}