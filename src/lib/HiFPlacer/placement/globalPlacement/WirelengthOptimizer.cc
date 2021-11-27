/**
 * @file WirelengthOptimizer.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation of the WirelengthOptimizer which builds numerical
 * models based on the element locations and calls solvers to find an optimal solution of the placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "WirelengthOptimizer.h"

#include <cmath>
#include <omp.h>

WirelengthOptimizer::WirelengthOptimizer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                                         bool verbose)
    : placementInfo(placementInfo), JSONCfg(JSONCfg), verbose(verbose)
{
    if (JSONCfg.find("MKL") != JSONCfg.end())
    {
        MKLorNot = JSONCfg["MKL"] == "true";
    }
    if (JSONCfg.find("useUnconstrainedCG") != JSONCfg.end())
    {
        useUnconstrainedCG = JSONCfg["useUnconstrainedCG"] == "true";
    }
    if (JSONCfg.find("y2xRatio") != JSONCfg.end())
        y2xRatio = std::stof(JSONCfg["y2xRatio"]);
    float leftBound = placementInfo->getGlobalMinX() - 0.5;
    float rightBound = placementInfo->getGlobalMaxX() + 0.5;
    float bottomBound = placementInfo->getGlobalMinY() - 0.5;
    float topBound = placementInfo->getGlobalMaxY() + 0.5;
    xSolver = new QPSolverWrapper(useUnconstrainedCG, MKLorNot, leftBound, rightBound,
                                  placementInfo->getPlacementUnits().size(), verbose);
    ySolver = new QPSolverWrapper(useUnconstrainedCG, MKLorNot, bottomBound, topBound,
                                  placementInfo->getPlacementUnits().size(), verbose);
    if (JSONCfg.find("DirectMacroLegalize") != JSONCfg.end())
    {
        directMacroLegalize = JSONCfg["DirectMacroLegalize"] == "true";
        if (directMacroLegalize)
        {
            print_warning("Direct Macro Legalization is enbaled. It might undermine the HPWL.");
        }
    }
}

void WirelengthOptimizer::reloadPlacementInfo()
{
    if (xSolver)
        delete xSolver;
    if (ySolver)
        delete ySolver;
    float leftBound = placementInfo->getGlobalMinX() - 0.5;
    float rightBound = placementInfo->getGlobalMaxX() + 0.5;
    float bottomBound = placementInfo->getGlobalMinY() - 0.5;
    float topBound = placementInfo->getGlobalMaxY() + 0.5;
    xSolver = new QPSolverWrapper(useUnconstrainedCG, MKLorNot, leftBound, rightBound,
                                  placementInfo->getPlacementUnits().size(), verbose);
    ySolver = new QPSolverWrapper(useUnconstrainedCG, MKLorNot, bottomBound, topBound,
                                  placementInfo->getPlacementUnits().size(), verbose);
}

void WirelengthOptimizer::GlobalPlacementQPSolve(float pesudoNetWeight, bool firstIteration,
                                                 bool forwardSolutionToNextIteration, bool enableMacroPseudoNet2Site,
                                                 bool considerNetNum, bool enableUserDefinedClusterOpt,
                                                 PlacementTimingOptimizer *timingOptimizer)
{
    if (verbose)
        print_status("A QP Iteration Started.");

    updateB2BNetWeight(pesudoNetWeight, enableMacroPseudoNet2Site, considerNetNum, enableUserDefinedClusterOpt,
                       timingOptimizer);
    if (firstIteration)
        solverLoadData();

    xSolver->solverSettings.solutionForward = forwardSolutionToNextIteration;
    ySolver->solverSettings.solutionForward = forwardSolutionToNextIteration;

    if (verbose)
        print_status("Solver Running.");
    std::thread t1(QPSolverWrapper::QPSolve, std::ref(xSolver));
    std::thread t2(QPSolverWrapper::QPSolve, std::ref(ySolver));
    t1.join();
    t2.join();
    if (verbose)
        print_status("Solver Done.");

    // solverLoadFixedData();

    solverWriteBackData();

    if (verbose)
        print_status("A QP Iteration Started.");
}

void WirelengthOptimizer::solverLoadData()
{
    for (unsigned int tmpPUId = 0; tmpPUId < placementInfo->getPlacementUnits().size(); tmpPUId++)
    {
        auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
        xSolver->solverData.oriSolution[tmpPUId] = tmpPU->X();
        ySolver->solverData.oriSolution[tmpPUId] = tmpPU->Y();
    }
}

void WirelengthOptimizer::solverLoadFixedData()
{
    assert(xSolver->solverSettings.solutionForward == ySolver->solverSettings.solutionForward);
    if (xSolver->solverSettings.solutionForward)
    {
        for (unsigned int tmpPUId = 0; tmpPUId < placementInfo->getPlacementUnits().size(); tmpPUId++)
        {
            auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
            if (tmpPU->isFixed())
            {
                xSolver->solverData.oriSolution[tmpPUId] = tmpPU->X();
                ySolver->solverData.oriSolution[tmpPUId] = tmpPU->Y();
            }
        }
    }
    else
    {
        for (unsigned int tmpPUId = 0; tmpPUId < placementInfo->getPlacementUnits().size(); tmpPUId++)
        {
            auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
            if (tmpPU->isFixed())
            {
                xSolver->solverData.solution[tmpPUId] = tmpPU->X();
                ySolver->solverData.solution[tmpPUId] = tmpPU->Y();
            }
        }
    }
}

void WirelengthOptimizer::solverWriteBackData()
{
    assert(xSolver->solverSettings.solutionForward == ySolver->solverSettings.solutionForward);
    if (xSolver->solverSettings.solutionForward)
    {
        for (unsigned int tmpPUId = 0; tmpPUId < placementInfo->getPlacementUnits().size(); tmpPUId++)
        {
            auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
            if (tmpPU->isFixed())
                continue;
            float fX = xSolver->solverData.oriSolution[tmpPUId];
            float fY = ySolver->solverData.oriSolution[tmpPUId];
            placementInfo->legalizeXYInArea(tmpPU, fX, fY);
            tmpPU->setAnchorLocation(fX, fY);
        }
    }
    else
    {
        for (unsigned int tmpPUId = 0; tmpPUId < placementInfo->getPlacementUnits().size(); tmpPUId++)
        {
            auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
            if (tmpPU->isFixed())
                continue;
            float fX = xSolver->solverData.solution[tmpPUId];
            float fY = ySolver->solverData.solution[tmpPUId];
            placementInfo->legalizeXYInArea(tmpPU, fX, fY);
            tmpPU->setAnchorLocation(fX, fY);
        }
    }
}

void WirelengthOptimizer::updateB2BNetWeight(float pesudoNetWeight, bool enableMacroPseudoNet2Site, bool considerNetNum,
                                             bool enableUserDefinedClusterOpt,
                                             PlacementTimingOptimizer *timingOptimizer)
{
    if (verbose)
        print_status("update B2B Net Weight Start.");

    bool updateTrue = true, updateFalse = false;
    std::thread t1(updateB2BNetWeightWorker, std::ref(placementInfo),
                   std::ref(xSolver->solverData.objectiveMatrixTripletList),
                   std::ref(xSolver->solverData.objectiveMatrixDiag), std::ref(xSolver->solverData.objectiveVector),
                   std::ref(generalNetWeight), std::ref(y2xRatio), std::ref(updateTrue), std::ref(updateFalse));
    std::thread t2(updateB2BNetWeightWorker, std::ref(placementInfo),
                   std::ref(ySolver->solverData.objectiveMatrixTripletList),
                   std::ref(ySolver->solverData.objectiveMatrixDiag), std::ref(ySolver->solverData.objectiveVector),
                   std::ref(generalNetWeight), std::ref(y2xRatio), std::ref(updateFalse), std::ref(updateTrue));
    t1.join();
    t2.join();

    if (enableMacroPseudoNet2Site && !directMacroLegalize)
    {
        addPseudoNetForMacros(pesudoNetWeight, considerNetNum);
    }
    else
    {
        print_warning("addPseudoNetForMacros is disabled according to placer configuration.");
    }

    if (timingOptimizer)
    {
        // addPseudoNet_LevelBased(
        //     placementInfo->getLongPathThresholdLevel(), (0.05 * timingOptimizer->getEffectFactor()) *
        //     generalNetWeight, placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod()
        //     * 2.5);
        addPseudoNet_SlackBased((0.2 * timingOptimizer->getEffectFactor()) * generalNetWeight, 1.1, timingOptimizer);
    }

    if (enableUserDefinedClusterOpt)
    {
        updatePseudoNetForUserDefinedClusters(pesudoNetWeight);
    }

    addPseudoNet2LoctionForAllPUs(pesudoNetWeight, considerNetNum);
    updatePseudoNetForClockRegion(0.2 * pesudoNetWeight);

    if (verbose)
        print_status("update B2B Net Weight Done.");

    if (JSONCfg.find("DrawNetAfterEachIteration") != JSONCfg.end())
    {
        if (JSONCfg["DrawNetAfterEachIteration"] == "true")
        {
            int netId;
            std::cout << "please input the ID of net you want to show (-1 for stopping):\n";
            std::cin >> netId;
            while (netId >= 0)
            {
                auto net = placementInfo->getPlacementNets()[netId];
                net->drawNet();
                std::cout << "please input the ID of net you want to show (-1 for stopping):\n";
                std::cin >> netId;
            }
        }
    }
}

void WirelengthOptimizer::updateB2BNetWeightWorker(PlacementInfo *placementInfo,
                                                   std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                                   std::vector<float> &objectiveMatrixDiag,
                                                   Eigen::VectorXd &objectiveVector, float generalNetWeight,
                                                   float y2xRatio, bool updateX, bool updateY)
{
    objectiveMatrixTripletList.clear();
    objectiveMatrixDiag.clear();
    objectiveMatrixDiag.resize(placementInfo->getPlacementUnits().size(), 0);
    objectiveVector = Eigen::VectorXd::Zero(placementInfo->getPlacementUnits().size());
    for (auto net : placementInfo->getPlacementNets())
    {
        if (net->getDesignNet()->checkIsPowerNet()) // Power nets are on the entrie device. Ignore them.
            continue;

        float enhanceFixedPinRatio = 1.0;
        if (net->getDesignNet()->checkContainFixedPins())
            enhanceFixedPinRatio = 5.0;
        if (net->updateNetBounds(updateX, updateY))
        {
            net->updateBound2BoundNetWeight(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector,
                                            generalNetWeight, y2xRatio, updateX, updateY);
        }
    }
}
void WirelengthOptimizer::addPseudoNetForMacros(float pesudoNetWeight, bool considerNetNum)
{
    std::map<PlacementInfo::PlacementUnit *, float> &PUX = placementInfo->getPULegalXY().first;
    std::map<PlacementInfo::PlacementUnit *, float> &PUY = placementInfo->getPULegalXY().second;
    macroPseudoNetCnt++;

    if (placementInfo->getProgress() > 0.8)
        oriMacroLegalizationWeight *= 1.025;
    if (placementInfo->getProgress() > 0.90)
        oriMacroLegalizationWeight *= 1.05;
    if (macroPseudoNetCnt > 40)
        oriMacroLegalizationWeight *= 1.05;
    float macroPseudoNetFactor = (float)macroPseudoNetCnt / 20 * oriMacroLegalizationWeight;
    print_info("WLOptimizer macroLegalizePseudoNet=" + std::to_string(macroPseudoNetFactor * pesudoNetWeight));
    if (considerNetNum)
    {
        for (auto pairPUX : PUX)
        {
            if (!pairPUX.first->checkHasBRAM() && !pairPUX.first->checkHasDSP() && !pairPUX.first->checkHasCARRY())
                placementInfo->addPseudoNetsInPlacementInfo(
                    xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                    xSolver->solverData.objectiveVector, pairPUX.first, pairPUX.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUX.first->getNetsSetPtr()->size() / 3, y2xRatio, true,
                    false); // CLB-like element
            else if (pairPUX.first->checkHasCARRY())
                placementInfo->addPseudoNetsInPlacementInfo(
                    xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                    xSolver->solverData.objectiveVector, pairPUX.first, pairPUX.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUX.first->getNetsSetPtr()->size() / 5, y2xRatio, true,
                    false); // CARRY-CHAIN-like element
            else
                placementInfo->addPseudoNetsInPlacementInfo(
                    xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                    xSolver->solverData.objectiveVector, pairPUX.first, pairPUX.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUX.first->getNetsSetPtr()->size(), y2xRatio, true,
                    false); // DSP-BRAM-like element
        }
        for (auto pairPUY : PUY)
        {
            if (!pairPUY.first->checkHasBRAM() && !pairPUY.first->checkHasDSP() && !pairPUY.first->checkHasCARRY())
                placementInfo->addPseudoNetsInPlacementInfo(
                    ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                    ySolver->solverData.objectiveVector, pairPUY.first, pairPUY.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUY.first->getNetsSetPtr()->size() / 3, y2xRatio,
                    false, true); // CLB-like element
            else if (pairPUY.first->checkHasCARRY())
                placementInfo->addPseudoNetsInPlacementInfo(
                    ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                    ySolver->solverData.objectiveVector, pairPUY.first, pairPUY.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUY.first->getNetsSetPtr()->size() / 5, y2xRatio,
                    false, true); // CARRY-CHAIN-like element
            else
                placementInfo->addPseudoNetsInPlacementInfo(
                    ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                    ySolver->solverData.objectiveVector, pairPUY.first, pairPUY.second,
                    macroPseudoNetFactor * pesudoNetWeight * pairPUY.first->getNetsSetPtr()->size(), y2xRatio, false,
                    true); // DSP-BRAM-like element
        }
    }
    else
    {
        for (auto pairPUX : PUX)
        {
            placementInfo->addPseudoNetsInPlacementInfo(
                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                xSolver->solverData.objectiveVector, pairPUX.first, pairPUX.second,
                macroPseudoNetFactor * pesudoNetWeight, y2xRatio, true, false);
        }
        for (auto pairPUY : PUY)
        {
            placementInfo->addPseudoNetsInPlacementInfo(
                ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                ySolver->solverData.objectiveVector, pairPUY.first, pairPUY.second,
                macroPseudoNetFactor * pesudoNetWeight, y2xRatio, false, true);
        }
    }
}

void WirelengthOptimizer::addPseudoNet_LevelBased(int levelThr, float timingWeight, double disExpected)
{
    assert(placementInfo->getTimingInfo());
    if (levelThr < 4)
        return;

    float maxEnhanceRatio = 0;
    auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
    auto &cellLoc = placementInfo->getCellId2location();
    assert(cellLoc.size() == timingNodes.size());

    float powFactor = 0.5 + 0.2 * placementInfo->getProgress();
    // std::ofstream outfile0("timingOptProc.log");

    auto deviceInfo = placementInfo->getDeviceInfo();

    for (auto curNet : placementInfo->getPlacementNets())
    {
        auto designNet = curNet->getDesignNet();
        if (designNet->checkIsPowerNet())
            continue;

        if (curNet->getDriverUnits().size() != 1 || curNet->getUnits().size() <= 1 || curNet->getUnits().size() >= 500)
            continue;
        auto &PUs = curNet->getUnits();
        auto &pins = designNet->getPins();
        int pinNum = pins.size();

        assert(curNet->getUnits().size() == (unsigned int)pinNum);

        int driverPinInNet = -1;

        for (int i = 0; i < pinNum; i++)
        {
            if (pins[i]->isOutputPort())
            {
                driverPinInNet = i;
                break;
            }
        }

        assert(driverPinInNet >= 0);

        // get the srcPin information
        auto srcCell = pins[driverPinInNet]->getCell();
        unsigned int srcCellId = srcCell->getCellId();
        int driverPathLen = timingNodes[srcCellId]->getLongestPathLength();
        int driverBackwardLevel = timingNodes[srcCellId]->getBackwardLevel();
        int driverForwardLevel = timingNodes[srcCellId]->getForwardLevel();
        auto srcLoc = cellLoc[srcCellId];

        int srcClockRegionXId, srcClockRegionYId;
        deviceInfo->getClockRegionByLocation(srcLoc.X, srcLoc.Y, srcClockRegionXId, srcClockRegionYId);

        // outfile0 << "handling net:" << curNet->getDesignNet()->getName() << "\n";
        // outfile0 << "handling driverpin:" << pins[driverPinInNet]->getName()
        //          << " driverBackwardLevel:" << driverBackwardLevel << " locX:" << srcLoc.X << " locY:" << srcLoc.Y
        //          << "\n";
        if (srcCell->isTimingEndPoint())
        {
            // outfile0 << "    is TimingEndPoint\n";
            // calculate the base weight
            float w = 2 * timingWeight / std::pow((float)(pinNum - 1), 0.5);

            // iterate the sinkPin for evaluation and enhancement
            for (int pinBeDriven = 0; pinBeDriven < pinNum; pinBeDriven++)
            {
                if (pinBeDriven == driverPinInNet)
                    continue;

                // get the sinkPin information
                auto sinkCell = pins[pinBeDriven]->getCell();
                unsigned int sinkCellId = sinkCell->getCellId();
                auto sinkLoc = cellLoc[sinkCellId];
                int succBackwardLevel = timingNodes[sinkCellId]->getBackwardLevel();
                // outfile0 << "        analyzing pin:" << pins[pinBeDriven]->getName()
                //          << " succBackwardLevel:" << succBackwardLevel << " locX:" << sinkLoc.X << " locY:" <<
                //          sinkLoc.Y
                //          << "\n";
                // calculate the distance
                double dis = manhattanDis(srcLoc.X, srcLoc.Y, sinkLoc.X, sinkLoc.Y);

                int sinkClockRegionXId, sinkClockRegionYId;
                deviceInfo->getClockRegionByLocation(sinkLoc.X, sinkLoc.Y, sinkClockRegionXId, sinkClockRegionYId);

                // dis += std::abs(sinkClockRegionXId - srcClockRegionXId) * 10;

                // handle the potential sinkPins on the longest path by checking the backward level
                if (succBackwardLevel + 1 >= levelThr)
                {
                    // the expected distance
                    double disThr = disExpected / (succBackwardLevel + 1);
                    if (dis > disThr)
                    {
                        // enhance the net based on the distance overflow ratio
                        float enhanceRatio = std::pow(dis / disThr, (double)powFactor);
                        // outfile0 << "        enhanced by " << enhanceRatio << " dis:" << dis << " disThr:" << disThr
                        //          << "\n";
                        curNet->addPseudoNet_enhancePin2Pin(
                            xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                            xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                            PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);

                        curNet->addPseudoNet_enhancePin2Pin(
                            ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                            ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                            PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);
                    }
                }
            }
        }
        else
        {
            // outfile0 << "    is NOT TimingEndPoint\n";
            if (driverPathLen < levelThr)
                continue;

            // calculate the base weight
            float w = 2 * timingWeight / std::pow((float)(pinNum - 1), 0.5);

            // iterate the sinkPin for evaluation and enhancement
            for (int pinBeDriven = 0; pinBeDriven < pinNum; pinBeDriven++)
            {
                if (pinBeDriven == driverPinInNet)
                    continue;

                // get the sinkPin information
                auto sinkCell = pins[pinBeDriven]->getCell();
                unsigned int sinkCellId = sinkCell->getCellId();
                auto sinkLoc = cellLoc[sinkCellId];
                int succPathLen = timingNodes[sinkCellId]->getLongestPathLength();
                int succBackwardLevel = timingNodes[sinkCellId]->getBackwardLevel();
                // outfile0 << "        analyzing pin:" << pins[pinBeDriven]->getName()
                //          << " succBackwardLevel:" << succBackwardLevel << " locX:" << sinkLoc.X << " locY:" <<
                //          sinkLoc.Y
                //          << "\n";
                // calculate the distance
                double dis = manhattanDis(srcLoc.X, srcLoc.Y, sinkLoc.X, sinkLoc.Y);

                int sinkClockRegionXId, sinkClockRegionYId;
                deviceInfo->getClockRegionByLocation(sinkLoc.X, sinkLoc.Y, sinkClockRegionXId, sinkClockRegionYId);

                // dis += std::abs(sinkClockRegionXId - srcClockRegionXId) * 10;

                // handle the potential sinkPins on the longest path by checking the backward level
                if (succBackwardLevel >= driverBackwardLevel - 3)
                {
                    if (timingNodes[sinkCellId]->checkIsRegister() && driverForwardLevel >= driverPathLen * 0.9)
                    {
                        // the expected distance
                        double disThr = disExpected / (driverForwardLevel + 1);
                        if (dis > disThr)
                        {
                            // enhance the net based on the distance overflow ratio
                            float enhanceRatio = std::pow(dis / disThr, powFactor);
                            // outfile0 << "        enhanced by " << enhanceRatio << " dis:" << dis << " disThr:" <<
                            // disThr
                            //          << "\n";
                            curNet->addPseudoNet_enhancePin2Pin(
                                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                                xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);

                            curNet->addPseudoNet_enhancePin2Pin(
                                ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                                ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);
                        }
                    }
                    else if (succPathLen >= 1 && succPathLen >= driverPathLen * 0.9)
                    {
                        // the expected distance
                        double disThr = disExpected / succPathLen;
                        if (dis > disThr)
                        {
                            // enhance the net based on the distance overflow ratio
                            float enhanceRatio = std::pow(dis / disThr, powFactor);
                            // outfile0 << "        enhanced by " << enhanceRatio << " dis:" << dis << " disThr:" <<
                            // disThr
                            //          << "\n";
                            curNet->addPseudoNet_enhancePin2Pin(
                                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                                xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);

                            curNet->addPseudoNet_enhancePin2Pin(
                                ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                                ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);
                        }
                    }
                }
            }
        }
    }

    print_status("WirelengthOptimizer: enhanceNetWeight_LevelBased done (maxEnhancedRatio=" +
                 std::to_string(maxEnhanceRatio) + ")");
    // outfile0.close();
}

void WirelengthOptimizer::addPseudoNet_SlackBased(float timingWeight, double slackPowFactor,
                                                  PlacementTimingOptimizer *timingOptimizer)
{
    assert(placementInfo->getTimingInfo());
    if (slackPowFactor < 0 || timingWeight < 0)
        return;

    // float maxEnhanceRatio = 0;
    auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
    float clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
    auto &cellLoc = placementInfo->getCellId2location();
    assert(cellLoc.size() == timingNodes.size());

    // std::ofstream outfile0("timingOptProc.log");

    // auto deviceInfo = placementInfo->getDeviceInfo();

    int enhanceNetCnt = 0;
    for (auto curNet : placementInfo->getPlacementNets())
    {
        auto designNet = curNet->getDesignNet();
        if (designNet->checkIsPowerNet() || designNet->checkIsGlobalClock())
            continue;

        if (curNet->getDriverUnits().size() != 1 || curNet->getUnits().size() <= 1 || curNet->getUnits().size() >= 1000)
            continue;
        auto &PUs = curNet->getUnits();
        auto &pins = designNet->getPins();
        int pinNum = pins.size();

        assert(curNet->getUnits().size() == (unsigned int)pinNum);

        int driverPinInNet = -1;

        for (int i = 0; i < pinNum; i++)
        {
            if (pins[i]->isOutputPort())
            {
                driverPinInNet = i;
                break;
            }
        }

        assert(driverPinInNet >= 0);

        // get the srcPin information
        auto srcCell = pins[driverPinInNet]->getCell();
        unsigned int srcCellId = srcCell->getCellId();
        auto srcNode = timingNodes[srcCellId];
        auto srcLoc = cellLoc[srcCellId];
        int driverPathLen = timingNodes[srcCellId]->getLongestPathLength();

        float w = 2 * timingWeight / std::pow((float)(pinNum - 1), 0.5);

        if (netPinEnhanceRate.find(designNet) == netPinEnhanceRate.end())
        {
            netPinEnhanceRate[designNet] = std::vector<float>(pinNum, 1.0);
        }

        auto &pinEnhanceRate = netPinEnhanceRate[designNet];
        // iterate the sinkPin for evaluation and enhancement
        for (int pinBeDriven = 0; pinBeDriven < pinNum; pinBeDriven++)
        {
            if (pinBeDriven == driverPinInNet)
                continue;

            // get the sinkPin information
            auto sinkCell = pins[pinBeDriven]->getCell();
            unsigned int sinkCellId = sinkCell->getCellId();
            auto sinkNode = timingNodes[sinkCellId];
            auto sinkLoc = cellLoc[sinkCellId];
            int succPathLen = sinkNode->getLongestPathLength();
            float netDelay = timingOptimizer->getDelayByModel(sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
            float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestArrival() - netDelay;

            if (slack > 0)
                continue;
            enhanceNetCnt++;
            // enhance the net based on the slack
            float enhanceRatio = std::pow(1 - slack / clockPeriod, slackPowFactor);
            // * std::pow(netDelay / expectedAvgDelay_driver, 0.6);

            if (srcNode->checkIsRegister())
            {
                if (succPathLen > 0)
                {
                    float expectedAvgDelay_succ = clockPeriod / succPathLen;
                    if (netDelay > expectedAvgDelay_succ)
                        enhanceRatio =
                            std::max(enhanceRatio, (float)std::pow((netDelay / expectedAvgDelay_succ), 0.66));
                }
            }
            else
            {
                if (driverPathLen > 0)
                {
                    float expectedAvgDelay_driver = clockPeriod / driverPathLen;
                    if (netDelay > expectedAvgDelay_driver)
                        enhanceRatio =
                            std::max(enhanceRatio, (float)std::pow((netDelay / expectedAvgDelay_driver), 0.66));
                }
            }

            // enhanceRatio = std::pow(enhanceRatio, 0.5) * std::pow(pinEnhanceRate[pinBeDriven], 0.5);
            // pinEnhanceRate[pinBeDriven] = enhanceRatio;

            curNet->addPseudoNet_enhancePin2Pin(
                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);

            curNet->addPseudoNet_enhancePin2Pin(
                ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);
        }
    }

    print_status("WirelengthOptimizer: addPseudoNet_SlackBased done (" + std::to_string(enhanceNetCnt) +
                 " pin2pin nets have been enhanced.");
    // outfile0.close();
}

void WirelengthOptimizer::addPseudoNet2LoctionForAllPUs(float pesudoNetWeight, bool considerNetNum)
{
    int numPUs = placementInfo->getPlacementUnits().size();
    float minDist = 0.5;
    float powFactor = placementInfo->getProgress() * 0.45 + 0.5;

    if (considerNetNum)
    {
#pragma omp parallel sections
        {
#pragma omp section
            {
#pragma omp parallel for // schedule(dynamic, 16)
                for (int PUId = 0; PUId < numPUs; PUId++)
                {
                    auto curPU = placementInfo->getPlacementUnits()[PUId];
                    bool movable = !curPU->isFixed();
                    if (movable)
                    {
                        float curX = curPU->X();
                        float lastX = curPU->lastX();
                        placementInfo->addPseudoNetsInPlacementInfo(
                            xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                            xSolver->solverData.objectiveVector, curPU, curX,
                            std::pow(curPU->getNetsSetPtr()->size(), powFactor) * pesudoNetWeight /
                                std::max(minDist, std::fabs(lastX - curX)),
                            y2xRatio, true, false);
                    }
                }
            }
#pragma omp section
            {
#pragma omp parallel for // schedule(dynamic, 16)
                for (int PUId = 0; PUId < numPUs; PUId++)
                {
                    auto curPU = placementInfo->getPlacementUnits()[PUId];
                    bool movable = !curPU->isFixed();
                    if (movable)
                    {
                        float curY = curPU->Y();
                        float lastY = curPU->lastY();
                        placementInfo->addPseudoNetsInPlacementInfo(
                            ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                            ySolver->solverData.objectiveVector, curPU, curY,
                            std::pow(curPU->getNetsSetPtr()->size(), powFactor) * pesudoNetWeight /
                                std::max(minDist, std::fabs(lastY - curY)),
                            y2xRatio, false, true);
                    }
                }
            }
        }
    }
    else
    {
#pragma omp parallel sections
        {
#pragma omp section
            {
#pragma omp parallel for // schedule(dynamic, 16)
                for (int PUId = 0; PUId < numPUs; PUId++)
                {
                    auto curPU = placementInfo->getPlacementUnits()[PUId];
                    bool movable = !curPU->isFixed();
                    if (movable)
                    {
                        float curX = curPU->X();
                        float lastX = curPU->lastX();
                        placementInfo->addPseudoNetsInPlacementInfo(
                            xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                            xSolver->solverData.objectiveVector, curPU, curX,
                            pesudoNetWeight / std::max(minDist, std::fabs(lastX - curX)), y2xRatio, true, false);
                    }
                }
            }
#pragma omp section
            {
#pragma omp parallel for // schedule(dynamic, 16)
                for (int PUId = 0; PUId < numPUs; PUId++)
                {
                    auto curPU = placementInfo->getPlacementUnits()[PUId];
                    bool movable = !curPU->isFixed();
                    if (movable)
                    {
                        float curY = curPU->Y();
                        float lastY = curPU->lastY();
                        placementInfo->addPseudoNetsInPlacementInfo(
                            ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                            ySolver->solverData.objectiveVector, curPU, curY,
                            pesudoNetWeight / std::max(minDist, std::fabs(lastY - curY)), y2xRatio, false, true);
                    }
                }
            }
        }
    }
}

void WirelengthOptimizer::updatePseudoNetForUserDefinedClusters(float pesudoNetWeight)
{

    userDefinedClusterFadeOutFactor *= 0.9;
    std::vector<std::vector<DesignInfo::DesignCell *>> &predefinedCellClusters =
        placementInfo->getDesignInfo()->getPredefinedClusters();

    std::set<PlacementInfo::PlacementUnit *> reallocatedPUs;

    int reallocateCells = 0;
    for (auto &clusterCellSet : predefinedCellClusters)
    {
        std::set<PlacementInfo::PlacementUnit *> PUsInCurCluster;
        PUsInCurCluster.clear();

        double avgX = 0;
        double avgY = 0;
        int totalCellNum = clusterCellSet.size();
        if (totalCellNum < 24)
            continue;
        float clusterFactor = (200.0 / totalCellNum);
        if (clusterFactor > 1)
            clusterFactor = 1;
        // if (clusterFactor < 0.5)
        //     clusterFactor = 0.333;
        int totalNetNum = 0;

        float x0 = 10000, y0 = 10000, x1 = -10000, y1 = -10000;
        for (auto tmpCell : clusterCellSet)
        {
            auto tmpPU = placementInfo->getPlacementUnitByCell(tmpCell);
            if (PUsInCurCluster.find(tmpPU) == PUsInCurCluster.end())
            {
                PUsInCurCluster.insert(tmpPU);
                avgX += tmpPU->X() * tmpPU->getNetsSetPtr()->size();
                avgY += tmpPU->Y() * tmpPU->getNetsSetPtr()->size();
                totalNetNum += tmpPU->getNetsSetPtr()->size();

                if (tmpPU->X() < x0)
                    x0 = tmpPU->X();
                if (tmpPU->X() > x1)
                    x1 = tmpPU->X();
                if (tmpPU->Y() < y0)
                    y0 = tmpPU->Y();
                if (tmpPU->Y() > y1)
                    y1 = tmpPU->Y();
            }
        }

        if (PUsInCurCluster.size() > 1)
        {
            if ((x1 - x0) * (y1 - y0) / PUsInCurCluster.size() < 1 && (x1 - x0) * (y1 - y0) > 1)
            {
                avgX /= totalNetNum;
                avgY /= totalNetNum;

                for (auto tmpPU : PUsInCurCluster)
                {
                    float fX = avgX;
                    float fY = avgY;
                    placementInfo->legalizeXYInArea(tmpPU, fX, fY);

                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, tmpPU, fX,
                        userDefinedClusterFadeOutFactor * clusterFactor * pesudoNetWeight, y2xRatio, true, false);

                    placementInfo->addPseudoNetsInPlacementInfo(
                        ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                        ySolver->solverData.objectiveVector, tmpPU, fY,
                        1.0 / y2xRatio * userDefinedClusterFadeOutFactor * clusterFactor * pesudoNetWeight, y2xRatio,
                        false, true);

                    if (reallocatedPUs.find(tmpPU) == reallocatedPUs.end())
                    {
                        reallocatedPUs.insert(tmpPU);
                        reallocateCells += tmpPU->getWeight();
                    }
                }
            }
        }
    }

    print_warning("set user-defined cluster pseudo net for " + std::to_string(reallocatedPUs.size()) + " PU(s) and " +
                  std::to_string(reallocateCells) + " cell(s).");
}

void WirelengthOptimizer::updatePseudoNetForClockRegion(float pesudoNetWeight)
{
    if (pesudoNetWeight <= 0)
        return;
    auto &PU2ClockRegionCenter = placementInfo->getPU2ClockRegionCenters();

    if (PU2ClockRegionCenter.size() <= 0)
        return;

    for (auto PUXY : PU2ClockRegionCenter)
    {
        auto curPU = PUXY.first;
        float cX = PUXY.second.first;
        if (std::fabs(curPU->X() - cX) > 6)
            placementInfo->addPseudoNetsInPlacementInfo(
                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                xSolver->solverData.objectiveVector, curPU, cX,
                pesudoNetWeight * std::pow(curPU->getNetsSetPtr()->size(), 1.1), y2xRatio, true, false);
        else if (std::fabs(curPU->X() - cX) > 3)
            placementInfo->addPseudoNetsInPlacementInfo(
                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                xSolver->solverData.objectiveVector, curPU, cX, pesudoNetWeight * curPU->getNetsSetPtr()->size(),
                y2xRatio, true, false);
        else
            placementInfo->addPseudoNetsInPlacementInfo(
                xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                xSolver->solverData.objectiveVector, curPU, cX,
                std::fabs(curPU->X() - cX) / 3 * pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true,
                false);
        // placementInfo->addPseudoNetsInPlacementInfo(
        //     ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
        //     ySolver->solverData.objectiveVector, curPU, cY, pesudoNetWeight * curPU->getNetsSetPtr()->size(),
        //     y2xRatio, false, true);
    }

    print_warning("update pseudo net of clockt region for " + std::to_string(PU2ClockRegionCenter.size()) + " PUs");
}
