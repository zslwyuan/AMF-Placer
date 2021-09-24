#include "ParallelCLBPacker.h"

void ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::maxCardinalityMatching(bool verbose)
{
    std::vector<DesignInfo::DesignCell *> LUTs;
    LUTs.clear();
    for (auto tmpLUT : singleLUTs)
        LUTs.push_back(tmpLUT);
    for (auto tmpLUTPair : pairedLUTs)
    {
        LUTs.push_back(tmpLUTPair.first);
        LUTs.push_back(tmpLUTPair.second);
        assert(singleLUTs.find(tmpLUTPair.first) == singleLUTs.end());
        assert(singleLUTs.find(tmpLUTPair.second) == singleLUTs.end());
        singleLUTs.insert(tmpLUTPair.first);
        singleLUTs.insert(tmpLUTPair.second);
    }
    std::sort(LUTs.begin(), LUTs.end(), [](DesignInfo::DesignCell *a, DesignInfo::DesignCell *b) -> bool {
        return a->getCellId() > b->getCellId();
    });

    pairedLUTs.clear();

    // std::vector<std::pair<int, int>> LUTPairCandidates;
    std::vector<float> LUTPairCost;
    // LUTPairCandidates.clear();

    MaximalCardinalityMatching::Graph G(LUTs.size());

    for (unsigned int i = 0; i < LUTs.size(); i++)
    {
        if (LUTs[i]->getInputPins().size() <= 5 &&
            !LUTs[i]->isLUT6()) // it might be a virtual cell or it has to occupy one LUT slot
        {
            for (unsigned int j = i + 1; j < LUTs.size(); j++)
            {
                if (LUTs[j]->getInputPins().size() <= 5 && !LUTs[j]->isLUT6())
                {
                    int pairPinNum = getPairPinNum(LUTs[i], LUTs[j]);
                    if (pairPinNum <= 5)
                    {
                        if (!(parentPackingCLB->conflictLUTsContain(LUTs[i]) &&
                              parentPackingCLB->conflictLUTsContain(LUTs[j])))
                        {
                            G.AddEdge(i, j);
                        }

                        // LUTPairCandidates.emplace_back(i, j);
                        // LUTPairCost.push_back(10 - LUTs[i]->getInputPins().size() - LUTs[j]->getInputPins().size());
                    }
                }
            }
        }
    }

    MaximalCardinalityMatching M(G);

    std::list<int> matching;
    matching = M.SolveMaximumMatching();

    std::set<int> addedNodes;
    addedNodes.clear();
    for (std::list<int>::iterator it = matching.begin(); it != matching.end(); it++)
    {
        std::pair<int, int> e = G.GetEdge(*it);
        assert(addedNodes.find(e.first) == addedNodes.end());
        assert(addedNodes.find(e.second) == addedNodes.end());
        assert(e.first != e.second);
        auto LUTA = LUTs[e.first];
        auto LUTB = LUTs[e.second];

        assert(singleLUTs.find(LUTA) != singleLUTs.end() && singleLUTs.find(LUTB) != singleLUTs.end());
        singleLUTs.erase(LUTA);
        singleLUTs.erase(LUTB);
        addedNodes.insert(e.first);
        addedNodes.insert(e.second);
        if (LUTA->getCellId() < LUTB->getCellId())
        {
            pairedLUTs.emplace(LUTA, LUTB);
        }
        else
        {
            pairedLUTs.emplace(LUTB, LUTA);
        }
    }
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addLUT(DesignInfo::DesignCell *curLUT)
{

    assert(singleLUTs.find(curLUT) == singleLUTs.end());

    if (singleLUTs.size() + 1 + pairedLUTs.size() + parentPackingCLB->getFixedPairedLUTs().size() <= MaxNum_LUTSite)
    {
        singleLUTs.insert(curLUT);

        return true;
    }
    if (curLUT->getInputPins().size() <= 5 &&
        curLUT->isLUT6()) // it might be a virtual cell or it has to occupy one LUT slot
    {
        DesignInfo::DesignCell *LUTToPair = nullptr;
        for (auto singleLUT : singleLUTs)
        {
            if (getPairPinNum(curLUT, singleLUT) <= 5)
            {
                if (!(parentPackingCLB->conflictLUTsContain(curLUT) &&
                      parentPackingCLB->conflictLUTsContain(singleLUT)))
                    LUTToPair = singleLUT;
                break;
            }
        }
        if (LUTToPair)
        {
            singleLUTs.erase(LUTToPair);
            if (LUTToPair->getCellId() < curLUT->getCellId())
            {
                pairedLUTs.emplace(LUTToPair, curLUT);
            }
            else
            {
                pairedLUTs.emplace(curLUT, LUTToPair);
            }
            return true;
        }
    }

    singleLUTs.insert(curLUT);

    if (singleLUTs.size() + pairedLUTs.size() + parentPackingCLB->getFixedPairedLUTs().size() <= MaxNum_LUTSite)
    {
        return true;
    }

    maxCardinalityMatching();

    return singleLUTs.size() + pairedLUTs.size() + parentPackingCLB->getFixedPairedLUTs().size() <= MaxNum_LUTSite;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addToFFSet(DesignInfo::DesignCell *curFF, int halfCLB)
{
    int i = halfCLB;
    if (FFControlSets[i].getSize() < MaxNum_FFinControlSet)
    {
        if (curFF->isVirtualCell())
        {
            FFControlSets[i].addFF(curFF);
            return true;
        }
        else if (FFControlSets[i].getCSId() < 0)
        {
            assert(curFF->getControlSetInfo());
            int anotherSetId = i - 1 + ((i % 2 == 0) ? 2 : 0);
            if (FFControlSets[anotherSetId].getCSId() < 0)
            {
                FFControlSets[i].addFF(curFF);
                return true;
            }
            else if (FFControlSets[anotherSetId].getCLK() == curFF->getControlSetInfo()->getCLK() &&
                     FFControlSets[anotherSetId].getSR() == curFF->getControlSetInfo()->getSR() &&
                     DesignInfo::FFSRCompatible(FFControlSets[anotherSetId].getFFType(), curFF->getOriCellType()))
            {
                FFControlSets[i].addFF(curFF);
                return true;
            }
        }
        else if (FFControlSets[i].getCSId() == curFF->getControlSetInfo()->getId())
        {
            FFControlSets[i].addFF(curFF);
            return true;
        }
    }
    return false;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::compatibleInOneHalfCLB(DesignInfo::ControlSetInfo *CSPtr,
                                                                                  int anotherHalfCLB)
{
    if (!CSPtr)
    {
        return true;
    }

    assert(CSPtr);
    if (FFControlSets[anotherHalfCLB].getCSId() < 0)
    {
        return true;
    }
    else if (FFControlSets[anotherHalfCLB].getCLK() == CSPtr->getCLK() &&
             FFControlSets[anotherHalfCLB].getSR() == CSPtr->getSR() &&
             DesignInfo::FFSRCompatible(FFControlSets[anotherHalfCLB].getFFType(), CSPtr->getFFType()))
    {
        return true;
    }

    return false;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addToFFSet(std::vector<DesignInfo::DesignCell *> curFFs,
                                                                      int halfCLB)
{
    bool succ = true;
    for (auto tmpFF : curFFs)
        succ &= addToFFSet(tmpFF, halfCLB);

    assert(succ && "adding FF group should be checked in advance and should be successful.");

    return succ;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addFF(DesignInfo::DesignCell *curFF, int enforceHalfCLB,
                                                                 bool enforceMainFFSlot)
{
    if (enforceHalfCLB < 0)
    {
        if (enforceMainFFSlot)
        {
            for (unsigned int i = 0; i < FFControlSets.size(); i += 2)
            {
                if (addToFFSet(curFF, i))
                    return true;
            }
        }
        else
        {
            for (unsigned int i = 0; i < FFControlSets.size(); i++)
            {
                if (addToFFSet(curFF, i))
                    return true;
            }
        }
    }
    else
    {
        if (addToFFSet(curFF, enforceHalfCLB))
            return true;
    }
    return false;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::checkNumMuxCompatibleInFFSet(int i, int addNum)
{
    int res = 0;
    for (auto tmpFF : FFControlSets[i].getFFs())
    {
        if (!tmpFF->isVirtualCell())
            continue;
        if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(
                parentPackingCLB->getPlacementInfo()->getPlacementUnitByCell(tmpFF)))
        {
            if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX7 ||
                tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX8)
            {
                res++;
            }
        }
    }
    if (res == 0)
        return true;
    if (res <= 1 && addNum <= 2) // muxF8 will add 3
        return true;
    return false;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addFFGroup(std::vector<DesignInfo::DesignCell *> curFFs,
                                                                      int enforceHalfCLB, bool enforceMainFFSlot,
                                                                      bool isMuxMacro)
{
    int CSId = -1;
    int addNum = curFFs.size();
    DesignInfo::ControlSetInfo *CSPtr = nullptr;
    for (auto FF : curFFs)
    {
        if (FF->getControlSetInfo())
        {
            if (CSId == -1)
            {
                CSId = FF->getControlSetInfo()->getId();
                CSPtr = FF->getControlSetInfo();
            }
            else
            {
                assert(CSId == FF->getControlSetInfo()->getId());
            }
        }
    }
    if (enforceHalfCLB < 0)
    {
        if (enforceMainFFSlot)
        {
            for (unsigned int i = 0; i < FFControlSets.size(); i += 2)
            {
                int anotherSetId = i - 1 + ((i % 2 == 0) ? 2 : 0);
                if (FFControlSets[i].getFFs().size() + curFFs.size() <= 4 && FFControlSets[i].compatibleWith(CSId))
                {
                    if (compatibleInOneHalfCLB(CSPtr, anotherSetId))
                    {
                        if (!isMuxMacro || (isMuxMacro && checkNumMuxCompatibleInFFSet(i, addNum)))
                        {
                            assert(addToFFSet(curFFs, i));
                            return true;
                        }
                    }
                }
            }
        }
        else
        {
            for (unsigned int i = 0; i < FFControlSets.size(); i++)
            {
                int anotherSetId = i - 1 + ((i % 2 == 0) ? 2 : 0);
                if (FFControlSets[i].getFFs().size() + curFFs.size() <= 4 && FFControlSets[i].compatibleWith(CSId))
                {
                    if (compatibleInOneHalfCLB(CSPtr, anotherSetId))
                    {
                        if (!isMuxMacro || (isMuxMacro && checkNumMuxCompatibleInFFSet(i, addNum)))
                        {
                            assert(addToFFSet(curFFs, i));
                            return true;
                        }
                    }
                }
            }
        }
    }
    else
    {
        int anotherSetId = enforceHalfCLB - enforceHalfCLB + ((enforceHalfCLB % 2 == 0) ? 2 : 0);
        if (FFControlSets[enforceHalfCLB].getFFs().size() + curFFs.size() <= 4 &&
            FFControlSets[enforceHalfCLB].compatibleWith(CSId))
        {
            if (compatibleInOneHalfCLB(CSPtr, anotherSetId))
            {
                if (!isMuxMacro || (isMuxMacro && checkNumMuxCompatibleInFFSet(enforceHalfCLB, addNum)))
                {
                    assert(addToFFSet(curFFs, enforceHalfCLB));
                    return true;
                }
            }
        }
    }
    return false;
}

void ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addPUFailReason(PlacementInfo::PlacementUnit *tmpPU)
{
    std::vector<DesignInfo::DesignCell *> cellsToAdd(0);
    if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
    {
        cellsToAdd.push_back(unpackCell->getCell());
    }
    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
    {
        for (auto tmpCell : curMacro->getCells())
            cellsToAdd.push_back(tmpCell);
    }

    for (auto curCell : cellsToAdd)
    {
        std::cout << "checking:\n" << curCell << "\n";
        bool succ = false;
        if (curCell->isLUT())
        {
            succ = addLUT(curCell);
            if (!succ)
            {
                std::cout << "failed to add LUT cell:\n" << curCell << "\nWWWWWWWWWWWWWWWWWWWWWWWWWWW\n";
                std::cout << this << "\n";
                break;
            }
            else
            {
                std::cout << "succedd to add LUT cell:\n" << curCell << "\n";
            }
        }
        else if (curCell->isFF())
        {
            succ = addFF(curCell);
            if (!succ)
            {
                std::cout << "failed to add FF cell:\n" << curCell << "\nWWWWWWWWWWWWWWWWWWWWWWWWWWW\n";
                for (auto inputPin : curCell->getInputPins())
                {
                    if (inputPin->getPinType() == DesignInfo::PinType_CLK)
                    {
                        if (!inputPin->isUnconnected())
                        {
                            std::cout << "CLK: " << inputPin->getNet()->getName() << "\n";
                        }
                    }
                    else if (inputPin->getPinType() == DesignInfo::PinType_E)
                    {
                        if (!inputPin->isUnconnected())
                        {
                            std::cout << "CE: " << inputPin->getNet()->getName() << "\n";
                        }
                    }
                    else if (inputPin->getPinType() == DesignInfo::PinType_SR)
                    {
                        if (!inputPin->isUnconnected())
                        {
                            std::cout << "SR: " << inputPin->getNet()->getName() << "\n";
                        }
                    }
                    else if (inputPin->getPinType() == DesignInfo::PinType_D)
                    {
                        // bypass
                    }
                    else
                    {
                        assert(false && "undefined FF input pin type.");
                    }
                }
                std::cout << this << "\n";
                break;
            }
            else
            {
                std::cout << "succedd to add FF cell:\n" << curCell << "\n";
            }
        }
        else
        {
            succ = true;
            assert(curCell->isMux());
        }
    }
}

int ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::getInternalPinsNum(PlacementInfo::PlacementNet *curNet)
{
    int res = 0;
    if (curNet->getUnits().size() > 512) // ignore large net
    {
        return 1;
    }
    for (auto tmpPU : curNet->getUnits())
    {
        if (PUs.find(tmpPU) != PUs.end())
        {
            res++;
        }
    }
    return res;
}

void ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::updateScoreInSite()
{
    std::set<PlacementInfo::PlacementNet *> nets;
    nets.clear();
    // net2ConnectivityScore.clear();

    HPWLChange = 0;
    totalCellNum = 0;
    totalLen = 0;
    for (auto tmpPU : PUs)
    {
        HPWLChange += getHPWLChangeForPU(tmpPU);
        for (auto tmpNet : *tmpPU->getNetsSetPtr())
        {
            if (tmpNet->getUnits().size() > 64) // ignore large net
                continue;

            if (nets.find(tmpNet) == nets.end())
            {
                nets.insert(tmpNet);
            }
        }
        totalCellNum += tmpPU->getNetsSetPtr()->size() * 0.5; //(tmpPU->getWeight() > 1 ? (tmpPU->getWeight() / 2) : 1);
        totalLen += getPlacementUnitMaxPathLen(tmpPU);
    }

    totalConnectivityScore = 0;
    for (auto tmpNet : nets)
    {
        if (tmpNet->getUnits().size() > 1)
        {
            float tmpConnectivityVal = (float)(getInternalPinsNum(tmpNet) - 1) / (float)(tmpNet->getUnits().size() - 1);
            //   net2ConnectivityScore[tmpNet] = tmpConnectivityVal;
            totalConnectivityScore += tmpConnectivityVal;
        }
        else
        {
            //  net2ConnectivityScore[tmpNet] = 1;
            totalConnectivityScore += 1;
        }
    }

    scoreInSite = totalCellNum * 0.45 + 0.01 * totalLen + totalConnectivityScore - HPWLWeight * HPWLChange;
}

void ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::incrementalUpdateScoreInSite(
    PlacementInfo::PlacementUnit *curPU)
{
    HPWLChange += getHPWLChangeForPU(curPU);
    totalCellNum += curPU->getNetsSetPtr()->size() * 0.5; // (curPU->getWeight() > 1 ? (curPU->getWeight() / 2) : 1);
    totalLen += getPlacementUnitMaxPathLen(curPU);

    std::set<PlacementInfo::PlacementNet *> nets;
    nets.clear();
    // net2ConnectivityScore.clear();

    for (auto tmpPU : PUs)
    {
        for (auto tmpNet : *tmpPU->getNetsSetPtr())
        {
            if (tmpNet->getUnits().size() > 64) // ignore large net
                continue;

            if (nets.find(tmpNet) == nets.end())
            {
                nets.insert(tmpNet);
            }
        }
    }

    totalConnectivityScore = 0;
    for (auto tmpNet : nets)
    {
        if (tmpNet->getUnits().size() > 1)
        {
            float tmpConnectivityVal = (float)(getInternalPinsNum(tmpNet) - 1) / (float)(tmpNet->getUnits().size() - 1);
            //   net2ConnectivityScore[tmpNet] = tmpConnectivityVal;
            totalConnectivityScore += tmpConnectivityVal;
        }
        else
        {
            //  net2ConnectivityScore[tmpNet] = 1;
            totalConnectivityScore += 1;
        }
    }

    scoreInSite = totalCellNum * 0.45 + 0.01 * totalLen + totalConnectivityScore - HPWLWeight * HPWLChange;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::addPU(PlacementInfo::PlacementUnit *tmpPU, bool allowOverlap)
{
    if (parentPackingCLB->checkIsLUTRAMSite())
        return false;

    if (!allowOverlap)
    {
        if (PUs.find(tmpPU) != PUs.end())
            return false;
    }
    else
    {
        if (PUs.find(tmpPU) != PUs.end())
            return true;
    }

    bool enforceMainFF = false;
    bool isMUXMacro = false;
    auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU);
    if (tmpMacro)
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX7 ||
            tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX8)
        {
            enforceMainFF = true;
            isMUXMacro = true;
            if (parentPackingCLB->checkIsPrePackedSite())
            {
                return false;
            }
        }
        else if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LCLB)
        {
            if (parentPackingCLB->getCLBSite()->getSiteType() == "SLICEM")
                return false;
        }
    }

    std::vector<DesignInfo::DesignCell *> cellsToAdd(0);
    std::vector<DesignInfo::DesignCell *> FFsToAdd(0);
    if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
    {
        cellsToAdd.push_back(unpackCell->getCell());
        if (isMUXMacro && unpackCell->getCell()->isFF())
            FFsToAdd.push_back(unpackCell->getCell());
    }
    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
    {
        for (auto tmpCell : curMacro->getCells())
        {
            cellsToAdd.push_back(tmpCell);
            if (isMUXMacro && tmpCell->isFF())
            {
                FFsToAdd.push_back(tmpCell);
            }
        }
    }

    if (isMUXMacro && FFsToAdd.size())
    {
        if (!addFFGroup(FFsToAdd, -1, true, true))
        {
            return false;
        }
    }
    //  assert(checkCellCorrectness(tmpPU, true));

    for (auto curCell : cellsToAdd)
    {
        bool succ = false;
        if (curCell->isLUT())
        {
            succ = addLUT(curCell);
        }
        else if (curCell->isFF())
        {
            if (isMUXMacro)
                continue;
            // assert(!containFF(curCell));
            succ = addFF(curCell, -1, enforceMainFF);
        }
        else
        {
            if (parentPackingCLB->checkIsCarrySite())
                succ = false;
            else
                succ = true;
            assert(curCell->isMux());
            if (succ)
            {
                // don't need to check mux, because the muxes have virtual LUT6s. As long as the virtual LUT6s can be
                // placed, the MUXes can be placed.
            }
        }
        if (!succ)
        {
            if (!(PUs.size() > 0 || parentPackingCLB->checkIsPrePackedSite()))
            {
                printMyself();
                std::cout << " curPU=\n" << tmpPU << "\n";
                std::cout << " curCell=\n" << curCell << "\n";
                if (curCell->isFF())
                {
                    if (!curCell->getControlSetInfo())
                    {
                        std::cout << " CLK: virtual\n";
                        std::cout << " SR: virtual\n";
                        std::cout << " CE: virtual\n";
                        continue;
                    }
                    else
                    {
                        if (curCell->getControlSetInfo()->getCLK())
                            std::cout << " CLK: " << curCell->getControlSetInfo()->getCLK()->getName() << "\n";
                        if (curCell->getControlSetInfo()->getSR())
                            std::cout << " SR: " << curCell->getControlSetInfo()->getSR()->getName() << "\n";
                        if (curCell->getControlSetInfo()->getCE())
                            std::cout << " CE: " << curCell->getControlSetInfo()->getCE()->getName() << "\n";
                    }
                }
                std::cout.flush();
            }
            assert(PUs.size() > 0 || parentPackingCLB->checkIsPrePackedSite());

            return false;
        }
    }
    PUs.insert(tmpPU);
    //  assert(checkCellCorrectness(tmpPU, true));
    hashed = false;
    if (isMUXMacro)
    {
        numMuxes++;
    }
    return true;
}

bool ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::checkCellCorrectness(PlacementInfo::PlacementUnit *tmpPU,
                                                                                bool isAddPU)
{
    for (auto tmpLUTPair0 : pairedLUTs)
    {
        auto tmpPUA = parentPackingCLB->getPlacementInfo()->getPlacementUnitByCell(tmpLUTPair0.second);
        auto tmpPUB = parentPackingCLB->getPlacementInfo()->getPlacementUnitByCell(tmpLUTPair0.first);
        if (PUs.find(tmpPUA) == PUs.end() || PUs.find(tmpPUB) == PUs.end())
        {
            bool Aok = false;
            bool Bok = false;
            if (PUs.find(tmpPUA) == PUs.end())
            {
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPUA))
                {
                    if (tmpMacro->checkHasCARRY())
                    {
                        Aok = true;
                    }
                }
                if (tmpPUA == parentPackingCLB->getCarryMacro())
                {
                    Aok = true;
                }
            }
            else
            {
                Aok = true;
            }
            if (PUs.find(tmpPUB) == PUs.end())
            {
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPUB))
                {
                    if (tmpMacro->checkHasCARRY())
                    {
                        Bok = true;
                    }
                }
                if (tmpPUB == parentPackingCLB->getCarryMacro())
                {
                    Bok = true;
                }
            }
            else
            {
                Bok = true;
            }
            if (Aok && Bok)
            {
                continue;
            }
            if (tmpPU)
            {
                if (isAddPU)
                    std::cout << "adding PU=\n" << tmpPU << "\n";
                else
                    std::cout << "removing PU\n" << tmpPU << "\n";
            }
            printMyself();
            std::cout << "problem is this pair:\n" << tmpPUA << "\n" << tmpPUB << "\n";
            return (false);
        }
    }

    for (auto tmpLUT : singleLUTs)
    {
        auto tmpPUA = parentPackingCLB->getPlacementInfo()->getPlacementUnitByCell(tmpLUT);
        if (PUs.find(tmpPUA) == PUs.end())
        {
            bool Aok = false;
            if (PUs.find(tmpPUA) == PUs.end())
            {
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPUA))
                {
                    if (tmpMacro->checkHasCARRY())
                    {
                        Aok = true;
                    }
                }
                if (tmpPUA == parentPackingCLB->getCarryMacro())
                {
                    Aok = true;
                }
            }
            else
            {
                Aok = true;
            }

            if (Aok)
            {
                continue;
            }
            if (tmpPU)
            {
                if (isAddPU)
                    std::cout << "adding PU=\n" << tmpPU << "\n";
                else
                    std::cout << "removing PU\n" << tmpPU << "\n";
            }
            printMyself();
            std::cout << "problem is this PU LUT:\n" << tmpPUA << "\n";
            return (false);
        }
    }

    for (auto &CS : FFControlSets)
    {
        for (auto tmpFF : CS.getFFs())
        {
            auto tmpPUA = parentPackingCLB->getPlacementInfo()->getPlacementUnitByCell(tmpFF);
            if (PUs.find(tmpPUA) == PUs.end())
            {
                bool Aok = false;
                if (PUs.find(tmpPUA) == PUs.end())
                {
                    if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPUA))
                    {
                        if (tmpMacro->checkHasCARRY())
                        {
                            Aok = true;
                        }
                    }
                    if (tmpPUA == parentPackingCLB->getCarryMacro())
                    {
                        Aok = true;
                    }
                }
                else
                {
                    Aok = true;
                }

                if (Aok)
                {
                    continue;
                }
                if (tmpPU)
                {
                    if (isAddPU)
                        std::cout << "adding PU=\n" << tmpPU << "\n";
                    else
                        std::cout << "removing PU\n" << tmpPU << "\n";
                }
                printMyself();
                std::cout << "problem is this PU FF:\n" << tmpPUA << "\n";
                return (false);
            }
        }
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const ParallelCLBPacker::PackingCLBSite::PackingCLBCluster *tmpCluster)
{
    os << "===============================================\n";
    int tmpSize = 0;
    for (auto tmpPU : tmpCluster->getPUs())
    {
        if (tmpPU->getType() == PlacementInfo::PlacementUnitType_Macro)
            tmpSize += tmpPU->getWeight();
        else
            tmpSize += 1;
    }
    os << "PackingCLBCluster: id=" << tmpCluster->getId() << " #cell=" << tmpSize
       << " hash=" << tmpCluster->getHashConst() << " score=" << tmpCluster->getScoreInSite() << " \n";
    os << "  targetSite: " << tmpCluster->getParentPackingCLB()->getCLBSite()->getName()
       << " X:" << tmpCluster->getParentPackingCLB()->getCLBSite()->X()
       << " Y:" << tmpCluster->getParentPackingCLB()->getCLBSite()->Y() << "\n";

    if (tmpCluster->getParentPackingCLB()->getCarryMacro())
    {
        os << "CARRY part (offset=" << tmpCluster->getParentPackingCLB()->getCarrySiteOffset()
           << "):" << tmpCluster->getParentPackingCLB()->getCarryMacro() << "\n";
    }

    for (auto tmpPU : tmpCluster->getPUs())
    {
        os << tmpPU;
        float HPWLChange = 0;
        for (auto tmpNet : *tmpPU->getNetsSetPtr())
        {
            if (tmpNet->getUnits().size() > 64) // ignore large net
                continue;
            HPWLChange += tmpNet->getNewHPWLByTrying(tmpPU, tmpCluster->getParentPackingCLB()->getCLBSite()->X(),
                                                     tmpCluster->getParentPackingCLB()->getCLBSite()->Y(),
                                                     tmpCluster->getParentPackingCLB()->getY2xRatio());
            HPWLChange -= tmpNet->getHPWL(tmpCluster->getParentPackingCLB()->getY2xRatio());
        }
        os << " HPWLChange=" << HPWLChange << "\n";
    }
    int cnt = 0;

    for (auto singleLUT : tmpCluster->getSingleLUTs())
    {
        os << "SingleLUT#" << cnt << ": " << singleLUT << "\n";
        cnt++;
    }
    cnt = 0;
    for (auto tmpPair : tmpCluster->getPairedLUTs())
    {
        os << "LUTPair#" << cnt << "\n";
        cnt++;
        os << "  LUT-A:  " << tmpPair.first << "\n";
        os << "  LUT-B:  " << tmpPair.second << "\n";
    }
    for (auto tmpPair : tmpCluster->getParentPackingCLB()->getFixedPairedLUTs())
    {
        os << "FixedLUTPair#" << cnt << "\n";
        cnt++;
        os << "  LUT-A:  " << tmpPair.first << "\n";
        os << "  LUT-B:  " << tmpPair.second << "\n";
    }
    cnt = 0;
    for (auto &FG : tmpCluster->getFFControlSets())
    {
        os << "FG#" << cnt << "\n";
        if (FG.getFFs().size() == 0)
            continue;
        if (FG.getFFs().size() == 0)
            continue;
        if (FG.getCSId() >= 0)
        {
            if (FG.getCLK())
                os << " CLK:" << FG.getCLK()->getName() << "\n";
            if (FG.getSR())
                os << " SR:" << FG.getSR()->getName() << "\n";
            if (FG.getCE())
                os << " CE:" << FG.getCE()->getName() << "\n";
        }
        else

        {
            os << " CLK: virtual.\n";
            os << " SR: virtual.\n";
            os << " CE: virtual.\n";
        }

        cnt++;
        for (auto curFF : FG.getFFs())
        {
            os << "  FF:  " << curFF << "\n";
        }
    }
    os << "===============================================\n";
    return os;
}

void ParallelCLBPacker::PackingCLBSite::PackingCLBCluster::printMyself()
{
    std::cout << "===============================================\n";
    int tmpSize = 0;
    for (auto tmpPU : getPUs())
    {
        if (tmpPU->getType() == PlacementInfo::PlacementUnitType_Macro)
            tmpSize += tmpPU->getWeight();
        else
            tmpSize += 1;
    }
    std::cout << "PackingCLBCluster: id=" << getId() << " #cell=" << tmpSize << " hash=" << getHashConst()
              << " score=" << getScoreInSite() << " \n";
    std::cout << "  targetSite: " << getParentPackingCLB()->getCLBSite()->getName()
              << " X:" << getParentPackingCLB()->getCLBSite()->X() << " Y:" << getParentPackingCLB()->getCLBSite()->Y()
              << "\n";

    if (getParentPackingCLB()->getCarryMacro())
    {
        std::cout << "CARRY part (offset=" << getParentPackingCLB()->getCarrySiteOffset()
                  << "):" << getParentPackingCLB()->getCarryMacro() << "\n";
    }

    for (auto tmpPU : getPUs())
    {
        std::cout << tmpPU;
        float HPWLChange = 0;
        for (auto tmpNet : *tmpPU->getNetsSetPtr())
        {
            if (tmpNet->getUnits().size() > 64) // ignore large net
                continue;
            HPWLChange += tmpNet->getNewHPWLByTrying(tmpPU, getParentPackingCLB()->getCLBSite()->X(),
                                                     getParentPackingCLB()->getCLBSite()->Y(),
                                                     getParentPackingCLB()->getY2xRatio());
            HPWLChange -= tmpNet->getHPWL(getParentPackingCLB()->getY2xRatio());
        }
        std::cout << " HPWLChange=" << HPWLChange << "\n";
    }
    int cnt = 0;

    for (auto singleLUT : getSingleLUTs())
    {
        std::cout << "SingleLUT#" << cnt << ": " << singleLUT << "\n";
        cnt++;
    }
    cnt = 0;
    for (auto tmpPair : getPairedLUTs())
    {
        std::cout << "LUTPair#" << cnt << "\n";
        cnt++;
        std::cout << "  LUT-A:  " << tmpPair.first << "\n";
        std::cout << "  LUT-B:  " << tmpPair.second << "\n";
    }
    for (auto tmpPair : parentPackingCLB->getFixedPairedLUTs())
    {
        std::cout << "FixedLUTPair#" << cnt << "\n";
        cnt++;
        std::cout << "  LUT-A:  " << tmpPair.first << "\n";
        std::cout << "  LUT-B:  " << tmpPair.second << "\n";
    }
    cnt = 0;
    for (auto &FG : getFFControlSets())
    {
        std::cout << "FG#" << cnt << "\n";
        if (FG.getFFs().size() == 0)
            continue;
        if (FG.getCSId() >= 0)
        {
            if (FG.getCLK())
                std::cout << " CLK:" << FG.getCLK()->getName() << "\n";
            if (FG.getSR())
                std::cout << " SR:" << FG.getSR()->getName() << "\n";
            if (FG.getCE())
                std::cout << " CE:" << FG.getCE()->getName() << "\n";
        }
        else

        {
            std::cout << " CLK: virtual.\n";
            std::cout << " SR: virtual.\n";
            std::cout << " CE: virtual.\n";
        }
        cnt++;
        for (auto curFF : FG.getFFs())
        {
            std::cout << "  FF:  " << curFF << "\n";
        }
    }
    std::cout << "===============================================\n";
}
