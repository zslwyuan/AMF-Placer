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
    if (JSONCfg.find("pin2pinEnhance") != JSONCfg.end())
    {
        pin2pinEnhance = std::stof(JSONCfg["pin2pinEnhance"]);
    }
    if (JSONCfg.find("DSPCritical") != JSONCfg.end())
        DSPCritical = JSONCfg["DSPCritical"] == "true";
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

    netPinEnhanceRate.clear();
    for (auto pNet : placementInfo->getPlacementNets())
    {
        auto designNet = pNet->getDesignNet();
        if (!designNet->checkIsPowerNet() && !designNet->checkIsGlobalClock())
        {
            netPinEnhanceRate[designNet] = std::vector<float>(designNet->getPins().size(), -1);
        }
    }

    if (placementInfo->getDesignInfo()->getCell(targetCellName))
        targetCellId = placementInfo->getDesignInfo()->getCell(targetCellName)->getCellId();
    else
        targetCellId = -1;
}

void WirelengthOptimizer::GlobalPlacementQPSolve(float pesudoNetWeight, bool firstIteration,
                                                 bool forwardSolutionToNextIteration, bool enableMacroPseudoNet2Site,
                                                 bool considerNetNum, bool enableUserDefinedClusterOpt,
                                                 float displacementLimit, PlacementTimingOptimizer *timingOptimizer)
{
    if (verbose)
        print_status("A QP Iteration Started.");

    if (firstIteration)
    {
        if (timingOptimizer)
        {
            slackThr = timingOptimizer->getSlackThr();
            addPseudoNet_SlackBased((0.2 * timingOptimizer->getEffectFactor()) * generalTimingNetWeight,
                                    slackPowerFactor, timingOptimizer, true);
        }
    }

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
    solverWriteBackData(displacementLimit);

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

void WirelengthOptimizer::solverWriteBackData(float displacementLimit)
{
    bool displacementLimitEnable = displacementLimit > 0;
    assert(xSolver->solverSettings.solutionForward == ySolver->solverSettings.solutionForward);
    unsigned int numPUs = placementInfo->getPlacementUnits().size();
    if (displacementLimitEnable)
    {
        if (xSolver->solverSettings.solutionForward)
        {
#pragma omp parallel for
            for (unsigned int tmpPUId = 0; tmpPUId < numPUs; tmpPUId++)
            {
                auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
                if (tmpPU->isFixed())
                    continue;
                float fX = xSolver->solverData.oriSolution[tmpPUId];
                float fY = ySolver->solverData.oriSolution[tmpPUId];
                float disX = std::fabs(fX - tmpPU->X());
                float disY = std::fabs(fY - tmpPU->Y());
                float dis = std::sqrt(disX * disX + disY * disY);
                float disRatio = displacementLimit / dis;
                if (disRatio < 1)
                {
                    fX = tmpPU->X() + (fX - tmpPU->X()) * disRatio;
                    fY = tmpPU->Y() + (fY - tmpPU->Y()) * disRatio;
                }
                placementInfo->legalizeXYInArea(tmpPU, fX, fY);
                tmpPU->setAnchorLocation(fX, fY);
            }
        }
        else
        {
#pragma omp parallel for
            for (unsigned int tmpPUId = 0; tmpPUId < numPUs; tmpPUId++)
            {
                auto tmpPU = placementInfo->getPlacementUnits()[tmpPUId];
                if (tmpPU->isFixed())
                    continue;
                float fX = xSolver->solverData.solution[tmpPUId];
                float fY = ySolver->solverData.solution[tmpPUId];
                float disX = std::fabs(fX - tmpPU->X());
                float disY = std::fabs(fY - tmpPU->Y());
                float dis = std::sqrt(disX * disX + disY * disY);
                float disRatio = displacementLimit / dis;
                if (disRatio < 1)
                {
                    fX = tmpPU->X() + (fX - tmpPU->X()) * disRatio;
                    fY = tmpPU->Y() + (fY - tmpPU->Y()) * disRatio;
                }
                placementInfo->legalizeXYInArea(tmpPU, fX, fY);
                tmpPU->setAnchorLocation(fX, fY);
            }
        }
    }
    else
    {
        if (xSolver->solverSettings.solutionForward)
        {
#pragma omp parallel for
            for (unsigned int tmpPUId = 0; tmpPUId < numPUs; tmpPUId++)
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
#pragma omp parallel for
            for (unsigned int tmpPUId = 0; tmpPUId < numPUs; tmpPUId++)
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
        addPseudoNet_SlackBased((0.2 * timingOptimizer->getEffectFactor()) * generalTimingNetWeight, slackPowerFactor,
                                timingOptimizer);
        if (timingOptimizer->getEffectFactor() > 0.5)
            LUTLUTPairing_TimingDriven((0.2 * timingOptimizer->getEffectFactor()) * generalTimingNetWeight,
                                       pin2pinEnhance, timingOptimizer);
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

void WirelengthOptimizer::addPseudoNet_SlackBased(float timingWeight, double slackPowFactor,
                                                  PlacementTimingOptimizer *timingOptimizer, bool calculate)
{
    if (calculate)
    {
        for (auto list : PNetId2SlackEnhanceTuples)
        {
            if (list)
                delete list;
        }
        PNetId2SlackEnhanceTuples.clear();
        PNetId2SlackEnhanceTuples.resize(placementInfo->getPlacementNets().size(), nullptr);

        assert(placementInfo->getTimingInfo());
        if (slackPowFactor < 0 || timingWeight < 0)
            return;

        // float maxEnhanceRatio = 0;
        auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
        // float clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
        auto &cellLoc = placementInfo->getCellId2location();
        assert(cellLoc.size() == timingNodes.size());
        auto &netActualSlackPinNum = timingOptimizer->getNetActualSlackPinNum();

        int enhanceNetCnt = 0;
        unsigned int highFanoutThr = 10000;

        if (placementInfo->getNetDistributionByDensity(512) < 200)
        {
            highFanoutThr = 1000;
            print_warning("highFanoutThr is set to 1000");
        }

        // int targetCellId = placementInfo->getDesignInfo()->getCell(targetCellName)->getCellId();

        int netNum = placementInfo->getPlacementNets().size();

#pragma omp parallel for
        for (int PNetId = 0; PNetId < netNum; PNetId++)
        {
            PNetId2SlackEnhanceTuples[PNetId] = new std::vector<slackEnhanceTuple>();
            auto curNet = placementInfo->getPlacementNets()[PNetId];
            assert(curNet);
            auto designNet = curNet->getDesignNet();
            assert(designNet);
            assert(curNet->getId() < PNetId2SlackEnhanceTuples.size());
            assert(curNet->getId() == PNetId);
            if (designNet->checkIsPowerNet() || designNet->checkIsGlobalClock())
                continue;

            if (curNet->getDriverUnits().size() != 1 || curNet->getUnits().size() <= 1 ||
                curNet->getUnits().size() >= highFanoutThr)
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

            if (netActualSlackPinNum[curNet] == 0)
                continue;

            float w = 2 * timingWeight / std::pow((float)(netActualSlackPinNum[curNet]), 0.5);

            if (srcCell->getCellId() == targetCellId)
            {
                std::cout << "driver: " << targetCellName << " x: " << srcLoc.X << " y: " << srcLoc.Y << "\n";
            }
            auto &pinEnhanceRate = netPinEnhanceRate[designNet];
            // iterate the sinkPin for evaluation and enhancement
            float timingEffect = timingOptimizer->getEffectFactor();
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
                float netDelay =
                    timingOptimizer->getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
                float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;
                float clockPeriod = sinkNode->getClockPeriod();
                if (clockPeriod < 0)
                {
                    clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
                    std::cout << sinkCell << " has no clock setting!!\n";
                }
                if (slack > 0)
                    continue;
                enhanceNetCnt++;
                // enhance the net based on the slack
                float enhanceRatio = std::pow(1 - slack / clockPeriod, slackPowFactor);
                if (slack < slackThr)
                {
                    enhanceRatio = std::pow(enhanceRatio, slack / slackThr * 3);
                }
                // * std::pow(netDelay / expectedAvgDelay_driver, 0.6);

                if (timingOptimizer->getEffectFactor() < 0.5)
                {
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
                }

                if (pinEnhanceRate[pinBeDriven] < 0)
                {
                    pinEnhanceRate[pinBeDriven] = enhanceRatio;
                }
                else
                {
                    if (enhanceRatio > 0)
                    {
                        float shrinkRatio = pinEnhanceRate[pinBeDriven] / enhanceRatio;
                        if (timingOptimizer->getEffectFactor() >= 1)
                        {
                            if (shrinkRatio > 300)
                            {
                                enhanceRatio = std::pow(enhanceRatio, 0.1) * std::pow(pinEnhanceRate[pinBeDriven], 0.9);
                            }
                            else if (shrinkRatio > 200)
                            {
                                enhanceRatio = std::pow(enhanceRatio, 0.2) * std::pow(pinEnhanceRate[pinBeDriven], 0.8);
                            }
                            else if (shrinkRatio > 100)
                            {
                                enhanceRatio =
                                    std::pow(enhanceRatio, 0.33) * std::pow(pinEnhanceRate[pinBeDriven], 0.67);
                            }
                            else if (shrinkRatio > 64)
                            {
                                enhanceRatio = std::pow(enhanceRatio, 0.4) * std::pow(pinEnhanceRate[pinBeDriven], 0.6);
                            }
                        }
                        pinEnhanceRate[pinBeDriven] = enhanceRatio;
                    }
                }
                if (srcCell->getCellId() == targetCellId)
                {
                    std::cout << "sink: " << sinkCell->getName() << " x: " << sinkLoc.X << " y: " << sinkLoc.Y
                              << " netDelay: " << netDelay << " slack: " << slack << " w: " << w
                              << " enhanceRatio: " << enhanceRatio << "\n";
                }
                if (sinkCell->getCellId() == targetCellId)
                {
                    std::cout << "src: " << srcCell->getName() << " x: " << srcLoc.X << " y: " << srcLoc.Y
                              << " netDelay: " << netDelay << " slack: " << slack << " w: " << w
                              << " enhanceRatio: " << enhanceRatio << "\n";
                }

                PNetId2SlackEnhanceTuples[curNet->getId()]->emplace_back(w * enhanceRatio, PUs[driverPinInNet]->getId(),
                                                                         PUs[pinBeDriven]->getId(), driverPinInNet,
                                                                         pinBeDriven);
                // curNet->addPseudoNet_enhancePin2Pin(
                //     xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                //     xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                //     PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);

                // curNet->addPseudoNet_enhancePin2Pin(
                //     ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                //     ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                //     PUs[driverPinInNet]->getId(), PUs[pinBeDriven]->getId(), driverPinInNet, pinBeDriven);
            }
        }
    }
    else
    {

        int netNum = placementInfo->getPlacementNets().size();
        for (int PNetId = 0; PNetId < netNum; PNetId++)
        {
            auto curNet = placementInfo->getPlacementNets()[PNetId];
            for (auto enhanceTuple : *PNetId2SlackEnhanceTuples[PNetId])
            {
                curNet->addPseudoNet_enhancePin2Pin(
                    xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                    xSolver->solverData.objectiveVector, enhanceTuple.weight, y2xRatio, true, false, enhanceTuple.PUAId,
                    enhanceTuple.PUBId, enhanceTuple.PinAId, enhanceTuple.PinBId);

                curNet->addPseudoNet_enhancePin2Pin(
                    ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                    ySolver->solverData.objectiveVector, enhanceTuple.weight, y2xRatio, false, true, enhanceTuple.PUAId,
                    enhanceTuple.PUBId, enhanceTuple.PinAId, enhanceTuple.PinBId);
            }
        }
    }

    print_status("WirelengthOptimizer: addPseudoNet_SlackBased done");
    // outfile0.close();
}

void WirelengthOptimizer::LUTLUTPairing_TimingDriven(float timingWeight, float disThreshold,
                                                     PlacementTimingOptimizer *timingOptimizer)
{
    float w = 2 * timingWeight / std::pow(2, 0.5);
    if (disThreshold < 0)
        return;
    print_status("WirelengthOptimizer: Timing Driven Pairing LUTs.");
    std::vector<PlacementInfo::Location> &cellLoc = placementInfo->getCellId2location();

    assert(placementInfo->getTimingInfo());

    // float maxEnhanceRatio = 0;
    auto timingNodes = placementInfo->getTimingInfo()->getSimplePlacementTimingInfo();
    // float clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
    assert(cellLoc.size() == timingNodes.size());

    int LUTLUTPairCnt = 0;
    int longLenThr = placementInfo->getLongPathThresholdLevel();

    // std::sort(LUTsToEnhanceNet.begin(), LUTsToEnhanceNet.end(),
    //           [](const CellWithScore &a, const CellWithScore &b) -> bool { return a.score < b.score; });
    // int targetCellId = placementInfo->getDesignInfo()->getCell(targetCellName)->getCellId();
    for (auto curCell : placementInfo->getDesignInfo()->getCells())
    {
        auto predLUTPU =
            dynamic_cast<PlacementInfo::PlacementUnit *>(placementInfo->getPlacementUnitByCellId(curCell->getCellId()));
        if ((curCell->isLUT() || curCell->isMux() || curCell->isCarry()) && !curCell->isVirtualCell())
        {
            assert(curCell->getOutputPins().size() > 0);
            if (curCell->getOutputPins().size() == 1)
            {
                if (curCell->getOutputPins()[0]
                        ->isUnconnected()) // interestingly, some LUTs generated by Vivado might have no output
                    continue;
                assert(curCell->getOutputPins()[0]->getNet());

                auto curNet = curCell->getOutputPins()[0]->getNet();
                auto curPNet = placementInfo->getPlacementNetByDesignNetId(curNet->getElementIdInType());

                if (!curPNet)
                    continue;
                auto srcCell = curCell;
                unsigned int srcCellId = srcCell->getCellId();
                auto srcNode = timingNodes[srcCellId];
                auto srcLoc = cellLoc[srcCellId];
                int driverPathLen = timingNodes[srcCellId]->getLongestPathLength();

                if (driverPathLen < longLenThr)
                    continue;

                float worstSlack = 0.0;
                DesignInfo::DesignCell *targetSinkCell = nullptr;
                int pinBeDriven = -1;
                int pinOffsetId = -1;
                if (curNet->getPins().size() > 16)
                    continue;

                for (auto curPin : curNet->getPins())
                {
                    pinOffsetId++;
                    auto sinkCell = curPin->getCell();
                    if (sinkCell == curCell)
                        continue;

                    if (sinkCell->isLUT())
                    {
                        unsigned int sinkCellId = sinkCell->getCellId();
                        auto sinkNode = timingNodes[sinkCellId];
                        auto sinkLoc = cellLoc[sinkCellId];
                        int succPathLen = sinkNode->getLongestPathLength();
                        if (succPathLen < longLenThr)
                            continue;
                        float netDelay = timingOptimizer->getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y,
                                                                          srcLoc.X, srcLoc.Y);
                        float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;

                        float curDis = getCellDistance(srcLoc, sinkLoc);

                        if (curDis < disThreshold && slack < worstSlack)
                        {
                            PlacementInfo::PlacementUnit *tmpSuccPU =
                                placementInfo->getPlacementUnitByCellId(sinkCell->getCellId());

                            worstSlack = slack;
                            targetSinkCell = sinkCell;
                            pinBeDriven = pinOffsetId;
                        }
                    }
                }

                if (targetSinkCell)
                {
                    unsigned int sinkCellId = targetSinkCell->getCellId();
                    auto sinkNode = timingNodes[sinkCellId];
                    auto sinkLoc = cellLoc[sinkCellId];
                    float clockPeriod = sinkNode->getClockPeriod();
                    if (clockPeriod < 0)
                    {
                        clockPeriod = placementInfo->getTimingInfo()->getSimplePlacementTimingGraph()->getClockPeriod();
                    }
                    PlacementInfo::PlacementUnit *succLUTPU =
                        placementInfo->getPlacementUnitByCellId(targetSinkCell->getCellId());
                    float enhanceRatio = std::pow(1 - worstSlack / clockPeriod, slackPowerFactor);

                    int driverPinInNet = -1;
                    auto &pins = curNet->getPins();
                    for (int i = 0; i < pins.size(); i++)
                    {
                        if (pins[i]->isOutputPort())
                        {
                            driverPinInNet = i;
                            break;
                        }
                    }

                    float netDelay =
                        timingOptimizer->getDelayByModel(sinkNode, srcNode, sinkLoc.X, sinkLoc.Y, srcLoc.X, srcLoc.Y);
                    float slack = sinkNode->getRequiredArrivalTime() - srcNode->getLatestOutputArrival() - netDelay;

                    if (srcCell->getCellId() == targetCellId)
                    {
                        std::cout << "LUTParing sink: " << targetSinkCell->getName() << " x: " << sinkLoc.X
                                  << " y: " << sinkLoc.Y << " netDelay: " << netDelay << " slack: " << slack
                                  << " w: " << w << " enhanceRatio: " << enhanceRatio << "\n";
                    }
                    if (targetSinkCell->getCellId() == targetCellId)
                    {
                        std::cout << "LUTParing src: " << srcCell->getName() << " x: " << srcLoc.X << " y: " << srcLoc.Y
                                  << " netDelay: " << netDelay << " slack: " << slack << " w: " << w
                                  << " enhanceRatio: " << enhanceRatio << "\n";
                    }
                    curPNet->addPseudoNet_enhancePin2Pin(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, true, false,
                        predLUTPU->getId(), succLUTPU->getId(), driverPinInNet, pinBeDriven);

                    curPNet->addPseudoNet_enhancePin2Pin(
                        ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                        ySolver->solverData.objectiveVector, w * enhanceRatio, y2xRatio, false, true,
                        predLUTPU->getId(), succLUTPU->getId(), driverPinInNet, pinBeDriven);
                }
            }
            else
            {
                // this is a LUT6_2, has two output pins and we don't pack them temporarily.
            }
        }
    }
}

void WirelengthOptimizer::addPseudoNet2LoctionForAllPUs(float pesudoNetWeight, bool considerNetNum)
{
    int numPUs = placementInfo->getPlacementUnits().size();
    float minDist = 0.5;
    float powFactor = placementInfo->getProgress() * 0.5 + 0.5;

    std::vector<int> netCnt(101, 0);

    int netSizeThr = 1;
    for (int PUId = 0; PUId < numPUs; PUId++)
    {
        auto curPU = placementInfo->getPlacementUnits()[PUId];

        float curX = curPU->X();
        int size = curPU->getNetsSetPtr()->size() / 5;
        if (size > 100)
            size = 100;
        netCnt[size]++;
    }

    int highInterconnectPUCnt = 0;
    for (int i = 100; i >= 2; i--)
    {
        highInterconnectPUCnt += netCnt[i];
        if (highInterconnectPUCnt > 300)
            break;
        netSizeThr = i * 5;
    }

    print_info("WirelengthOptimizer: PU net pin cnt distribution (netSizeThr=" + std::to_string(netSizeThr) + "):");

    for (int i = 0; i <= 100; i++)
        std::cout << netCnt[i] << " ";
    std::cout << "\n";

    if (netSizeThr > 100)
        netSizeThr = 128;
    else
        netSizeThr = 64; //+ placementInfo->getProgress() * 128;

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
                        float size = curPU->getNetsSetPtr()->size();
                        if (size > netSizeThr)
                            size = netSizeThr;
                        placementInfo->addPseudoNetsInPlacementInfo(
                            xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                            xSolver->solverData.objectiveVector, curPU, curX,
                            std::pow(size, powFactor) * pesudoNetWeight / std::max(minDist, std::fabs(lastX - curX)),
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
                        float size = curPU->getNetsSetPtr()->size();
                        if (size > netSizeThr)
                            size = netSizeThr;
                        placementInfo->addPseudoNetsInPlacementInfo(
                            ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
                            ySolver->solverData.objectiveVector, curPU, curY,
                            std::pow(size, powFactor) * pesudoNetWeight / std::max(minDist, std::fabs(lastY - curY)),
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
    if (userDefinedClusterFadeOutFactor < 0.1)
        return;
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
    auto &PU2ClockRegionColumn = placementInfo->getPU2ClockRegionColumn();
    auto &clockRegions = placementInfo->getDeviceInfo()->getClockRegions();

    if (PU2ClockRegionCenter.size() <= 0)
        return;

    for (auto PUXY : PU2ClockRegionCenter)
    {
        auto curPU = PUXY.first;
        float cX = PUXY.second.first;
        if (PU2ClockRegionColumn.find(curPU) != PU2ClockRegionColumn.end())
        {
            int clockRegionX = PU2ClockRegionColumn[curPU];

            if (clockRegionX == 2)
            {
                if (std::fabs(curPU->X() - cX) > 6)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * std::pow(curPU->getNetsSetPtr()->size(), 1.1), y2xRatio, true, false);
                else if (std::fabs(curPU->X() - cX) > 3 && !DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true, false);
                else if (!DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        std::fabs(curPU->X() - cX) / 3 * pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio,
                        true, false);
            }
            else if (clockRegionX < 2)
            {

                float rLeft = clockRegions[0][1]->getLeft();
                float rRight = clockRegions[0][1]->getRight();
                cX = (rLeft + rRight) / 2;
                if ((curPU->X() - cX) > 6)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * std::pow(curPU->getNetsSetPtr()->size(), 1.1), y2xRatio, true, false);
                else if ((curPU->X() - cX) > 3 && !DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true, false);
                else if ((curPU->X() - cX) > 0 && !DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        (curPU->X() - cX) / 3 * pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true,
                        false);
            }
            else if (clockRegionX > 2)
            {
                float rLeft = clockRegions[0][3]->getLeft();
                float rRight = clockRegions[0][3]->getRight();
                cX = (rLeft + rRight) / 2;
                if ((curPU->X() - cX) < -6)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * std::pow(curPU->getNetsSetPtr()->size(), 1.1), y2xRatio, true, false);
                else if ((curPU->X() - cX) < -3 && !DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true, false);
                else if ((curPU->X() - cX) < 0 && !DSPCritical)
                    placementInfo->addPseudoNetsInPlacementInfo(
                        xSolver->solverData.objectiveMatrixTripletList, xSolver->solverData.objectiveMatrixDiag,
                        xSolver->solverData.objectiveVector, curPU, cX,
                        (cX - curPU->X()) / 3 * pesudoNetWeight * curPU->getNetsSetPtr()->size(), y2xRatio, true,
                        false);
            }
        }

        // placementInfo->addPseudoNetsInPlacementInfo(
        //     ySolver->solverData.objectiveMatrixTripletList, ySolver->solverData.objectiveMatrixDiag,
        //     ySolver->solverData.objectiveVector, curPU, cY, pesudoNetWeight * curPU->getNetsSetPtr()->size(),
        //     y2xRatio, false, true);
    }

    print_warning("update pseudo net of clockt region for " + std::to_string(PU2ClockRegionCenter.size()) + " PUs");
}
