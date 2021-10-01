/**
 * @file InitialPacker.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "InitialPacker.h"
#include "readZip.h"
#include "strPrint.h"
#include "stringCheck.h"
#include <assert.h>
#include <cmath>
#include <queue>

void InitialPacker::pack()
{
    cellId2PlacementUnit.clear();
    placementUnits.clear();
    placementMacros.clear();
    cellInMacros.clear();
    placementUnpackedCells.clear();

    print_status("InitialPacker Finding Macros");
    findCARRYMacros();
    findMuxMacros();
    findBRAMMacros();
    findDSPMacros();

    if (JSONCfg.find("unpredictable macro file") != JSONCfg.end())
    {
        loadOtherCLBMacros(JSONCfg["unpredictable macro file"]);
    }

    LUTFFPairing();

    print_status("InitialPacker Finding unpacked units");
    findUnpackedUnits();

    if (JSONCfg.find("fixed units file") != JSONCfg.end())
    {
        loadFixedPlacementUnits(JSONCfg["fixed units file"]);
    }
    else
    {
        print_error(
            "No fixed cell in the design. Floating placement is not allowed temporarily."); // this can be disabled but
                                                                                            // might lead to
                                                                                            // unpredictable placement.
        assert(false && "No fixed cell in the design. Floating placement is not allowed temporarily.");
    }

    print_info("#Cells In Macro = " + std::to_string(cellInMacros.size()));

    placementInfo->updateCells2PlacementUnits();

    // since new placement units might be generated, we need to update the net information
    print_status("InitialPacker Loading Nets");
    placementInfo->reloadNets();

    // update the BEL information for all the cells (including those virtual cells)
    compatiblePlacementTable->setBELTypeForCells(designInfo);

    // for each FF, check/record its control set
    designInfo->updateFFControlSets();
    placementInfo->calculateNetNumDistributionOfPUs();

    // enhanceIONets();
    // std::string dumpFile = "netNumDisribution.gz";
    // print_status("ParallelCLBPacker: dumping unpackable PUs archieve to: " + dumpFile);
    // std::stringstream outfile1;

    // for (auto tmpPU : placementInfo->getPlacementUnits())
    // {
    //     outfile1 << tmpPU->getNetsSetPtr()->size() << " " << tmpPU->getName() << "\n";
    // }
    // writeStrToGZip(dumpFile, outfile1);

    dumpMacroHighLight();
}

void InitialPacker::enhanceIONets()
{
    for (auto net : placementInfo->getPlacementNets())
    {
        if (net->getDriverUnits().size() == 1)
        {
            if (net->getDriverUnits()[0]->isFixed())
            {
                net->getDesignNet()->enhanceOverallNetEnhancement(1.5);
            }
        }
        else if (net->getUnitsBeDriven().size() == 1)
        {
            if (net->getUnitsBeDriven()[0]->isFixed())
            {
                net->getDesignNet()->enhanceOverallNetEnhancement(1.5);
            }
        }
    }
}

std::vector<DesignInfo::DesignCell *>
InitialPacker::BFSExpandViaSpecifiedPorts(std::string portPattern, DesignInfo::DesignCell *startCell, bool exactMatch)
{
    std::set<DesignInfo::DesignCell *> tmpCellInMacros;
    std::vector<DesignInfo::DesignCell *> res;
    res.clear();
    tmpCellInMacros.clear();
    tmpCellInMacros.insert(startCell);
    res.push_back(startCell);

    std::queue<DesignInfo::DesignCell *> cellToBFS;
    while (!cellToBFS.empty())
        cellToBFS.pop();
    cellToBFS.push(startCell);

    while (!cellToBFS.empty())
    {
        DesignInfo::DesignCell *curCell = cellToBFS.front();
        cellToBFS.pop();

        for (DesignInfo::DesignNet *curOutputNet : curCell->getOutputNets())
        {
            for (DesignInfo::DesignPin *pinBeDriven : curOutputNet->getPinsBeDriven())
            {
                if ((exactMatch && pinBeDriven->getRefPinName() == portPattern) ||
                    (!exactMatch && pinBeDriven->getRefPinName().find(portPattern) == 0))
                {
                    DesignInfo::DesignCell *tmpCell = pinBeDriven->getCell();
                    if (startCell->getCellType() != tmpCell->getCellType())
                        continue;
                    if (tmpCellInMacros.find(tmpCell) == tmpCellInMacros.end())
                    {
                        tmpCellInMacros.insert(tmpCell);
                        cellToBFS.push(tmpCell);
                        res.push_back(tmpCell);
                    }
                }
            }
        }
    }
    return res;
}

std::vector<DesignInfo::DesignCell *> InitialPacker::BFSExpandViaSpecifiedPorts(std::vector<std::string> portPatterns,
                                                                                DesignInfo::DesignCell *startCell,
                                                                                bool exactMatch)
{
    std::set<DesignInfo::DesignCell *> tmpCellInMacros;
    std::vector<DesignInfo::DesignCell *> res;
    res.clear();
    tmpCellInMacros.clear();
    tmpCellInMacros.insert(startCell);
    res.push_back(startCell);

    std::queue<DesignInfo::DesignCell *> cellToBFS;
    while (!cellToBFS.empty())
        cellToBFS.pop();
    cellToBFS.push(startCell);

    while (!cellToBFS.empty())
    {
        DesignInfo::DesignCell *curCell = cellToBFS.front();
        cellToBFS.pop();

        for (DesignInfo::DesignNet *curOutputNet : curCell->getOutputNets())
        {
            for (DesignInfo::DesignPin *pinBeDriven : curOutputNet->getPinsBeDriven())
            {
                bool isCAS = false;
                for (auto portPattern : portPatterns)
                {
                    if ((exactMatch && pinBeDriven->getRefPinName() == portPattern) ||
                        (!exactMatch && pinBeDriven->getRefPinName().find(portPattern) == 0))
                    {
                        isCAS = true;
                        break;
                    }
                }

                if (isCAS)
                {
                    DesignInfo::DesignCell *tmpCell = pinBeDriven->getCell();
                    if (startCell->getCellType() != tmpCell->getCellType())
                        continue;
                    if (tmpCellInMacros.find(tmpCell) == tmpCellInMacros.end())
                    {
                        tmpCellInMacros.insert(tmpCell);
                        cellToBFS.push(tmpCell);
                        res.push_back(tmpCell);
                    }
                }
            }
        }
    }
    return res;
}

// DSP with ACIN*/BCIN*/PCIN* connected to other DSP should be a Macro
void InitialPacker::findDSPMacros()
{
    float DSPHeight = 2.5;
    std::vector<PlacementInfo::PlacementMacro *> res;
    res.clear();

    std::vector<DesignInfo::DesignCell *> &curCellsInDesign = designInfo->getCells();
    int curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];
        if (!curCell->isDSP())
            continue;

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

        bool noCASInput = true;

        for (DesignInfo::DesignPin *pinBeDriven : curCell->getInputPins())
        {
            if (pinBeDriven->getRefPinName().find("ACIN[") == 0 || pinBeDriven->getRefPinName().find("BCIN[") == 0 ||
                pinBeDriven->getRefPinName().find("PCIN[") == 0)
            {
                if (pinBeDriven->getDriverPin())
                {
                    noCASInput = false;
                    break;
                }
            }
        }

        if (!noCASInput)
            continue;

        std::vector<std::string> portPatterns{"ACIN[", "BCIN[", "PCIN["};
        std::vector<DesignInfo::DesignCell *> curMacroCores = BFSExpandViaSpecifiedPorts(portPatterns, curCell, false);

        if (curMacroCores.size() <= 1)
            continue;

        PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
            curMacroCores[0]->getName(), placementUnits.size(), PlacementInfo::PlacementMacro::PlacementMacroType_DSP);

        float coreOffset = 0;
        for (unsigned int i = 0; i < curMacroCores.size(); i++)
        {
            curMacro->addOccupiedSite(coreOffset, 1);
            curMacro->addCell(curMacroCores[i], curMacroCores[i]->getCellType(), 0, coreOffset);
            coreOffset += DSPHeight / 2;
        }
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }
        curMacro->setWeight(totalWeight * 16);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    int cellInBRAMMacrosCnt = 0;
    for (auto macro : res)
        cellInBRAMMacrosCnt += macro->getCells().size();

    // std::cout << "highlight_objects -color yellow [get_cells { ";
    // for (auto macro : res)
    //     for (auto cell : macro->getCells())
    //         std::cout << " " << cell->getName();
    // std::cout << "}]\n";

    print_info("#DSP Macro: " + std::to_string(res.size()));
    print_info("#DSP Macro Cells: " + std::to_string(cellInBRAMMacrosCnt));
    print_status("DSP Macro Extracted.");
}

// BRAM with CAS* connected to other BRAM should be a Macro
void InitialPacker::findBRAMMacros()
{
    float BRAM36Height = 5;
    float BRAM18Height = 2.5;
    std::vector<PlacementInfo::PlacementMacro *> res;
    res.clear();

    std::vector<DesignInfo::DesignCell *> &curCellsInDesign = designInfo->getCells();
    int curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];
        if (!curCell->isBRAM())
            continue;

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

        bool noCASInput = true;

        for (DesignInfo::DesignPin *pinBeDriven : curCell->getInputPins())
        {
            if (pinBeDriven->getRefPinName().find("CAS") == 0)
            {
                if (pinBeDriven->getDriverPin())
                {
                    noCASInput = false;
                    break;
                }
            }
        }

        if (!noCASInput)
            continue;

        std::vector<DesignInfo::DesignCell *> curMacroCores = BFSExpandViaSpecifiedPorts("CAS", curCell, false);

        if (curMacroCores.size() <= 1 && curCell->getCellType() != DesignInfo::CellType_RAMB36E2)
            continue;

        PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
            curMacroCores[0]->getName(), placementUnits.size(), PlacementInfo::PlacementMacro::PlacementMacroType_BRAM);

        float coreOffset = 0;
        for (unsigned int i = 0; i < curMacroCores.size(); i++)
        {
            if (curMacroCores[i]->getCellType() == DesignInfo::CellType_RAMB36E2)
            {
                curMacro->addOccupiedSite(coreOffset, 1.0);
                curMacro->addCell(curMacroCores[i], curMacroCores[i]->getCellType(), 0, coreOffset);
                curMacro->addOccupiedSite(coreOffset + BRAM18Height, 1.0);
                curMacro->addVirtualCell(curMacroCores[i]->getName(), designInfo, DesignInfo::CellType_RAMB18E2, 0,
                                         coreOffset + BRAM18Height);
            }
            else if (curMacroCores[i]->getCellType() == DesignInfo::CellType_RAMB18E2)
            {
                curMacro->addOccupiedSite(coreOffset, 1.0);
                curMacro->addCell(curMacroCores[i], curMacroCores[i]->getCellType(), 0, coreOffset);
            }
            else
            {
                assert(false);
            }

            if (i < curMacroCores.size() - 1)
            {
                if (curMacroCores[i]->getCellType() == DesignInfo::CellType_RAMB36E2)
                    coreOffset += BRAM36Height;
                else
                    coreOffset += BRAM18Height;
            }
        }
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }
        curMacro->setWeight(totalWeight * 16);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    int cellInBRAMMacrosCnt = 0;
    for (auto macro : res)
        cellInBRAMMacrosCnt += macro->getCells().size();

    print_info("#BRAM Macro: " + std::to_string(res.size()));
    print_info("#BRAM Macro Cells: " + std::to_string(cellInBRAMMacrosCnt));
    print_status("BRAM Macro Extracted.");
}

bool canLUTPacked(DesignInfo::DesignCell *ALUT, DesignInfo::DesignCell *BLUT)
{
    std::set<DesignInfo::DesignPin *> driverPins;
    driverPins.clear();
    assert(ALUT->isLUT() && BLUT->isLUT());
    for (DesignInfo::DesignPin *pinBeDriven : ALUT->getInputPins())
    {
        if (pinBeDriven->isUnconnected())
            continue;
        if (!pinBeDriven->getDriverPin()) // pin connect to GND/VCC which has no specifc driver pin
        {
            continue;
        }
        driverPins.insert(pinBeDriven->getDriverPin());
    }
    for (DesignInfo::DesignPin *pinBeDriven : BLUT->getInputPins())
    {
        if (pinBeDriven->isUnconnected())
            continue;
        if (!pinBeDriven->getDriverPin()) // pin connect to GND/VCC which has no specifc driver pin
        {
            continue;
        }
        driverPins.insert(pinBeDriven->getDriverPin());
    }
    return driverPins.size() <= 5;
}

std::vector<DesignInfo::DesignCell *> InitialPacker::checkCompatibleFFs(std::vector<DesignInfo::DesignCell *> FFs)
{
    std::vector<PackedControlSet> FFControlSets;
    const int MaxNum_ControlSet = 4;

    for (auto curFF : FFs)
    {
        assert(curFF->getControlSetInfo());
        bool added = false;
        for (unsigned int i = 0; i < FFControlSets.size(); i++)
        {
            if (FFControlSets[i].getCSId() == curFF->getControlSetInfo()->getId() &&
                FFControlSets[i].getFFs().size() < MaxNum_ControlSet)
            {
                FFControlSets[i].addFF(curFF);
                added = true;
                break;
            }
        }
        if (!added)
        {
            FFControlSets.emplace_back(curFF);
        }
    }

    std::sort(FFControlSets.begin(), FFControlSets.end(),
              [](const PackedControlSet &a, const PackedControlSet &b) -> bool {
                  return a.getFFs().size() > b.getFFs().size();
              });

    typedef struct _CLKSRCombination
    {
        DesignInfo::DesignNet *CLK = nullptr;
        DesignInfo::DesignNet *SR = nullptr;
        unsigned int cnt = 0;
    } CLKSRCombination;
    CLKSRCombination halfCLBSettings[1];
    std::vector<DesignInfo::DesignCell *> resFF;

    for (auto &FFCS : FFControlSets)
    {
        auto curFF = FFCS.getFFs()[0];
        assert(curFF->getControlSetInfo());
        if (halfCLBSettings[0].cnt == 0 || (halfCLBSettings[0].cnt < MaxNum_ControlSet / 2 &&
                                            halfCLBSettings[0].CLK == curFF->getControlSetInfo()->getCLK() &&
                                            halfCLBSettings[0].SR == curFF->getControlSetInfo()->getSR()))
        {
            halfCLBSettings[0].CLK = curFF->getControlSetInfo()->getCLK();
            halfCLBSettings[0].SR = curFF->getControlSetInfo()->getSR();
            halfCLBSettings[0].cnt++;
            for (auto tmpFF : FFCS.getFFs())
                resFF.push_back(tmpFF);
        }
    }

    return resFF;
}

void InitialPacker::mapCarryRelatedRouteThru(PlacementInfo::PlacementMacro *CARRYChain,
                                             DesignInfo::DesignCell *coreCell, float CARRYChainSiteOffset)
{
    SiteBELMapping slotMapping;
    std::set<DesignInfo::DesignCell *> mappedCells;
    std::set<DesignInfo::DesignCell *> mappedLUTs;
    std::set<DesignInfo::DesignCell *> mappedFFs;

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
                        if (CARRYChain->hasCell(pinBeDriven->getDriverPin()->getCell()) &&
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
                        if (CARRYChain->hasCell(pinBeDriven->getDriverPin()->getCell()) &&
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
                                    if (CARRYChain->hasCell(pinBeDriven->getCell()))
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
                                slotMapping.FFs[FFPinCellId / 4][1][FFPinCellId % 4] = theFF;
                                mappedCells.insert(theFF);
                                mappedFFs.insert(theFF);
                                // std::string FFSiteName = std::string(1, FFCode) + "FF2";
                                // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                                //          << "\n";
                            }
                            else if (driverPin->getRefPinName().find("O[") != std::string::npos)
                            {
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

    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            if (slotMapping.FFs[i][0][k] && slotMapping.FFs[i][1][k])
            {
                if (slotMapping.FFs[i][0][k]->isVirtualCell() && slotMapping.FFs[i][1][k]->isVirtualCell())
                {
                    for (int jj = 0; jj < 2; jj++)
                    {
                        for (int kk = 0; kk < 4; kk++)
                        {
                            if (!slotMapping.FFs[i][jj][kk])
                            {
                                if (jj == 0)
                                    slotMapping.FFs[i][jj][kk] = CARRYChain->addVirtualCell(
                                        coreCell->getName() + "__FF" + std::to_string(i * 4 + kk), designInfo,
                                        DesignInfo::CellType_FDCE, 0, CARRYChainSiteOffset);
                                else
                                    slotMapping.FFs[i][jj][kk] = CARRYChain->addVirtualCell(
                                        coreCell->getName() + "__FF2" + std::to_string(i * 4 + kk), designInfo,
                                        DesignInfo::CellType_FDCE, 0, CARRYChainSiteOffset);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0; i < 2; i++)
        for (int k = 0; k < 4; k++)
        {
            if (slotMapping.LUTs[i][0][k] && !slotMapping.FFs[i][0][k])
            {
                if (slotMapping.LUTs[i][0][k]->isLUT6())
                {
                    slotMapping.FFs[i][0][k] =
                        CARRYChain->addVirtualCell(coreCell->getName() + "__FF" + std::to_string(i * 4 + k), designInfo,
                                                   DesignInfo::CellType_FDCE, 0, CARRYChainSiteOffset);
                    if (!slotMapping.FFs[i][1][k])
                    {
                        slotMapping.FFs[i][1][k] =
                            CARRYChain->addVirtualCell(coreCell->getName() + "__FF2" + std::to_string(i * 4 + k),
                                                       designInfo, DesignInfo::CellType_FDCE, 0, CARRYChainSiteOffset);
                    }
                }
            }
            if (slotMapping.LUTs[i][1][k] && !slotMapping.FFs[i][1][k])
            {
                if (slotMapping.LUTs[i][1][k]->isLUT6())
                {
                    if (slotMapping.LUTs[i][1][k]->getOriCellType() != DesignInfo::CellType_LUT6_2)
                    {
                        std::cout << slotMapping.LUTs[i][1][k] << "\n";
                        assert(!slotMapping.LUTs[i][1][k]->isLUT6()); // this slot cannot handle LUT6
                    }
                }
            }
        }
}

// CARRY with CIN connected to other DSP should be a Macro. CARRY with external input from outside of CLB will occupy
// specific LUT too.
void InitialPacker::findCARRYMacros()
{
    std::vector<PlacementInfo::PlacementMacro *> res;
    res.clear();

    std::vector<DesignInfo::DesignCell *> &curCellsInDesign = designInfo->getCells();
    int curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];
        if (curCell->getCellType() != DesignInfo::CellType_CARRY8)
            continue;

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

        bool noCASInput = true;
        bool AXUsed = false;

        for (DesignInfo::DesignPin *pinBeDriven : curCell->getInputPins())
        {
            if (pinBeDriven->getRefPinName() == "CI")
            {
                if (pinBeDriven->getDriverPin())
                {
                    if (pinBeDriven->getDriverPin()->getCell()->getCellType() == DesignInfo::CellType_CARRY8)
                        noCASInput = false;
                    else
                        AXUsed = true; // non-CARRY CI will use AX
                }
                break;
            }
        }

        if (!noCASInput)
            continue;

        std::vector<DesignInfo::DesignCell *> curMacroCores = BFSExpandViaSpecifiedPorts("CI", curCell, true);

        if (curMacroCores.size() < 1)
            continue;

        // expand CARRY MACRO with input LUT / output FF / external input occupying
        PlacementInfo::PlacementMacro *curMacro =
            new PlacementInfo::PlacementMacro(curMacroCores[0]->getName(), placementUnits.size(),
                                              PlacementInfo::PlacementMacro::PlacementMacroType_CARRY);

        std::vector<std::string> checkLUTRefPins{"S["}; //"DI[",
        std::vector<std::string> checkFFRefPins{"O[", "CO["};

        int coreOffset = 0;
        for (auto coreCell : curMacroCores)
        {
            curMacro->addOccupiedSite(coreOffset, 1.0);
            curMacro->addCell(coreCell, coreCell->getCellType(), 0, coreOffset);
            coreOffset++;
        }

        coreOffset = 0;
        for (auto coreCell : curMacroCores)
        {

            std::vector<std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>> SPinCell(0);
            SPinCell.resize(8, std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(nullptr, nullptr));
            std::vector<std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>> DIPinCell(0);
            DIPinCell.resize(8, std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(nullptr, nullptr));
            std::vector<bool> externalSignal(0);
            externalSignal.resize(8, false);

            for (DesignInfo::DesignPin *pinBeDriven : coreCell->getInputPins())
            {
                if (pinBeDriven->isUnconnected())
                    continue;
                if (!pinBeDriven->getDriverPin()) // pin connect to GND/VCC which has no specifc driver pin
                {
                    if (pinBeDriven->getRefPinName().find("DI[") == 0)
                    {
                        char DIPinCellId =
                            pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                        DIPinCell[DIPinCellId] =
                            std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(pinBeDriven, nullptr);
                    }
                    continue;
                }

                if (pinBeDriven->getRefPinName().find("DI[") == 0)
                {
                    char DIPinCellId = pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                    DIPinCell[DIPinCellId] = std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(
                        pinBeDriven, pinBeDriven->getDriverPin()->getCell());
                }
            }

            for (DesignInfo::DesignPin *pinBeDriven : coreCell->getInputPins())
            {
                if (pinBeDriven->isUnconnected())
                    continue;
                if (!pinBeDriven->getDriverPin()) // pin connect to GND/VCC which has no specifc driver pin
                {
                    if (pinBeDriven->getRefPinName().find("S[") == 0)
                    {
                        char SPinCellId =
                            pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                        SPinCell[SPinCellId] =
                            std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(pinBeDriven, nullptr);
                    }
                    continue;
                }

                if (pinBeDriven->getRefPinName().find("S[") == 0)
                {
                    char SPinCellId = pinBeDriven->getRefPinName()[pinBeDriven->getRefPinName().find("[") + 1] - '0';
                    SPinCell[SPinCellId] = std::pair<DesignInfo::DesignPin *, DesignInfo::DesignCell *>(
                        pinBeDriven, pinBeDriven->getDriverPin()->getCell());
                }
            }

            for (unsigned int i = 0; i < 8; i++)
            {
                if (AXUsed && i == 0 && coreOffset < 0.005)
                {
                    if (DIPinCell[i].second && SPinCell[i].second)
                    {
                        bool DICanBeInSlice = DIPinCell[i].second->isLUT() && !DIPinCell[i].second->isLUT6() &&
                                              DIPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                        bool SCanBeInSlice =
                            SPinCell[i].second->isLUT() && SPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                        if (DICanBeInSlice && SCanBeInSlice)
                        {
                            if (canLUTPacked(DIPinCell[i].second, SPinCell[i].second))
                            {
                                curMacro->addCell(DIPinCell[i].second, DIPinCell[i].second->getCellType(), 0,
                                                  coreOffset);
                                curMacro->addCell(SPinCell[i].second, SPinCell[i].second->getCellType(), 0, coreOffset);
                            }
                            else
                            {
                                curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                         DesignInfo::CellType_LUT6, 0, coreOffset);
                            }
                        }
                        else if (SCanBeInSlice)
                        {
                            curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                     DesignInfo::CellType_LUT6, 0, coreOffset);
                        }
                        else
                        {
                            curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                     DesignInfo::CellType_LUT6, 0, coreOffset);
                        }
                    }
                    else if (SPinCell[i].second && !DIPinCell[i].first)
                    {
                        bool SCanBeInSlice =
                            SPinCell[i].second->isLUT() && SPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                        if (SCanBeInSlice && !SPinCell[i].second->isLUT6())
                        {
                            curMacro->addCell(SPinCell[i].second, DesignInfo::CellType_LUT6, 0, coreOffset);
                        }
                        else
                        {
                            curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                     DesignInfo::CellType_LUT6, 0, coreOffset);
                        }
                    }
                    else
                    {
                        curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                 DesignInfo::CellType_LUT6, 0, coreOffset);
                    }
                }
                else if (DIPinCell[i].second && SPinCell[i].second)
                {
                    bool DICanBeInSlice = DIPinCell[i].second->isLUT() && !DIPinCell[i].second->isLUT6() &&
                                          DIPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                    bool SCanBeInSlice =
                        SPinCell[i].second->isLUT() && SPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                    if (DICanBeInSlice && SCanBeInSlice)
                    {
                        if (canLUTPacked(DIPinCell[i].second, SPinCell[i].second))
                        {
                            curMacro->addCell(DIPinCell[i].second, DIPinCell[i].second->getCellType(), 0, coreOffset);
                            curMacro->addCell(SPinCell[i].second, SPinCell[i].second->getCellType(), 0, coreOffset);
                        }
                        else
                        {
                            curMacro->addCell(SPinCell[i].second, DesignInfo::CellType_LUT6, 0, coreOffset);
                        }
                    }
                    else if (SCanBeInSlice)
                    {
                        curMacro->addCell(SPinCell[i].second, DesignInfo::CellType_LUT6, 0, coreOffset);
                    }
                    else
                    {
                        curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                 DesignInfo::CellType_LUT6, 0, coreOffset);
                    }
                }
                else if (DIPinCell[i].second && !SPinCell[i].first)
                {
                    bool DICanBeInSlice = DIPinCell[i].second->isLUT() && !DIPinCell[i].second->isLUT6() &&
                                          DIPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                    if (DICanBeInSlice)
                    {
                        curMacro->addCell(DIPinCell[i].second, DIPinCell[i].second->getCellType(), 0, coreOffset);
                    }
                }
                else if (SPinCell[i].second)
                {
                    bool SCanBeInSlice =
                        SPinCell[i].second->isLUT() && SPinCell[i].first->getNet()->getPinsBeDriven().size() == 1;
                    if (SCanBeInSlice)
                    {
                        curMacro->addCell(SPinCell[i].second, DesignInfo::CellType_LUT6, 0, coreOffset);
                    }
                    else
                    {
                        curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                                 DesignInfo::CellType_LUT6, 0, coreOffset);
                    }
                }
                else if (DIPinCell[i].first || SPinCell[i].first)
                {
                    curMacro->addVirtualCell(coreCell->getName() + "__" + std::to_string(i), designInfo,
                                             DesignInfo::CellType_LUT6, 0, coreOffset);
                }
            }

            std::vector<DesignInfo::DesignCell *> drivenTopFFs;
            std::vector<DesignInfo::DesignCell *> drivenBottomFFs;
            for (DesignInfo::DesignPin *driverPin : coreCell->getOutputPins())
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
                                theFF = pinBeDriven->getCell();
                        }
                    }
                    if (FFcnt == 1 && theFF)
                    {
                        char FFPinCellId = driverPin->getRefPinName()[driverPin->getRefPinName().find("[") + 1] - '0';
                        if (FFPinCellId < 4)
                            drivenBottomFFs.push_back(theFF);
                        else
                            drivenTopFFs.push_back(theFF);
                        // curMacro->addCell(theFF, theFF->getCellType(), 0, coreOffset);
                    }
                }
            }

            std::set<DesignInfo::DesignCell *> addedFFs;
            addedFFs.clear();
            drivenBottomFFs = checkCompatibleFFs(drivenBottomFFs);
            for (auto theFF : drivenBottomFFs)
            {
                curMacro->addCell(theFF, theFF->getCellType(), 0, coreOffset);
                addedFFs.insert(theFF);
            }
            drivenTopFFs = checkCompatibleFFs(drivenTopFFs);
            for (auto theFF : drivenTopFFs)
            {
                curMacro->addCell(theFF, theFF->getCellType(), 0, coreOffset);
                addedFFs.insert(theFF);
            }

            for (DesignInfo::DesignPin *driverPin : coreCell->getOutputPins())
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
                                if (addedFFs.find(pinBeDriven->getCell()) != addedFFs.end())
                                    theFF = pinBeDriven->getCell();
                            }
                        }
                    }
                    if (FFcnt == 1 && theFF)
                    {
                        // char FFPinCellId = driverPin->getRefPinName()[driverPin->getRefPinName().find("[") + 1] -
                        // '0'; if (driverPin->getRefPinName().find("CO[") != std::string::npos)
                        // {
                        //     slotMapping.FFs[FFPinCellId / 4][1][FFPinCellId % 4] = theFF;
                        //     // std::string FFSiteName = std::string(1, FFCode) + "FF2";
                        //     // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                        //     //          << "\n";
                        // }
                        // else if (driverPin->getRefPinName().find("O[") != std::string::npos)
                        // {
                        //     slotMapping.FFs[FFPinCellId / 4][0][FFPinCellId % 4] = theFF;
                        //     // std::string FFSiteName = std::string(1, FFCode) + "FF";
                        //     // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                        //     //          << "\n";
                        // }
                    }
                    else
                    {
                        char FFPinCellId = driverPin->getRefPinName()[driverPin->getRefPinName().find("[") + 1] - '0';
                        if (driverPin->getRefPinName().find("CO[") != std::string::npos)
                        {
                            curMacro->addVirtualCell(coreCell->getName() + "__FF2" + std::to_string(FFPinCellId),
                                                     designInfo, DesignInfo::CellType_FDCE, 0, coreOffset);
                            // std::string FFSiteName = std::string(1, FFCode) + "FF2";
                            // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                            //          << "\n";
                        }
                        else if (driverPin->getRefPinName().find("O[") != std::string::npos)
                        {
                            curMacro->addVirtualCell(coreCell->getName() + "__FF" + std::to_string(FFPinCellId),
                                                     designInfo, DesignInfo::CellType_FDCE, 0, coreOffset);
                            // std::string FFSiteName = std::string(1, FFCode) + "FF";
                            // outfile0 << "  " << theFF->getName() << " " << CLBSite->getName() << "/" + FFSiteName
                            //          << "\n";
                        }
                    }
                }
            }
            mapCarryRelatedRouteThru(curMacro, coreCell, coreOffset);
            coreOffset++;
        }
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            if (tmpCell->isCarry())
                totalWeight += 8;
            else
                totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }
        curMacro->setWeight(totalWeight);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    int cellInCarryMacrosCnt = 0;
    for (auto macro : res)
        cellInCarryMacrosCnt += macro->getCells().size();
    print_info("#CARRY Macro: " + std::to_string(res.size()));
    print_info("#CARRY Macro Cells: " + std::to_string(cellInCarryMacrosCnt));
    print_status("CARRY Macro Extracted.");
}

// LUTs and Muxs connected are Macros
void InitialPacker::findMuxMacros()
{
    // TODO: consider F9MUX

    std::vector<PlacementInfo::PlacementMacro *> res;
    res.clear();
    std::vector<DesignInfo::DesignCell *> &curCellsInDesign = designInfo->getCells();
    int curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];
        if (curCell->getCellType() != DesignInfo::CellType_MUXF8)
            continue;

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

        std::vector<DesignInfo::DesignCell *> curMacroCores;
        curMacroCores.clear();
        curMacroCores.push_back(curCell);

        PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
            curMacroCores[0]->getName(), placementUnits.size(), PlacementInfo::PlacementMacro::PlacementMacroType_MUX8);

        curMacro->addCell(curCell, curCell->getCellType(), 0, 0);
        bool muxF8HasDirectFF =
            false; // chipset/chipset_impl/mc_top/i_ddr4_0/inst/u_ddr4_mem_intfc/u_ddr_cal_riu/mcs0/inst/microblaze_I/U0/MicroBlaze_Core_I/Performance.Core/Data_Flow_I/Operand_Select_I/Gen_Bit[3].MUXF7_I1
        for (DesignInfo::DesignPin *driverPin : curCell->getOutputPins())
        {
            if (driverPin->getRefPinName() == "O")
            {
                auto curOutputNet = driverPin->getNet();
                if (curOutputNet->getPinsBeDriven().size() != 1)
                    continue;
                auto pinBeDriven = curOutputNet->getPinsBeDriven()[0];
                if (pinBeDriven->getCell()->isFF())
                {
                    if (pinBeDriven->getRefPinName().find("D") != std::string::npos)
                    {
                        muxF8HasDirectFF = true;
                        curMacro->addCell(pinBeDriven->getCell(), pinBeDriven->getCell()->getCellType(), 0, 0);
                    }
                }
            }
        }
        if (!muxF8HasDirectFF)
            curMacro->addVirtualCell(curCell->getName() + "__FFF8", designInfo, DesignInfo::CellType_FDCE, 0, 0);
        curMacro->addOccupiedSite(0.0, 0.5);

        for (DesignInfo::DesignPin *pinBeDriven : curCell->getInputPins())
        {
            if (pinBeDriven->getRefPinName() == "I0" || pinBeDriven->getRefPinName() == "I1")
            {
                if (!pinBeDriven->getDriverPin())
                {
                    auto tmpMuxf7 =
                        curMacro->addVirtualCell(curCell->getName() + "__MUXF7" + pinBeDriven->getRefPinName(),
                                                 designInfo, DesignInfo::CellType_MUXF7, 0, 0);

                    curMacro->addVirtualCell(curCell->getName() + "__FFF7", designInfo, DesignInfo::CellType_FDCE, 0,
                                             0);
                    curMacro->addVirtualCell(tmpMuxf7->getName() + "__I0" + pinBeDriven->getRefPinName(), designInfo,
                                             DesignInfo::CellType_LUT6, 0, 0);
                    curMacro->addVirtualCell(tmpMuxf7->getName() + "__I1" + pinBeDriven->getRefPinName(), designInfo,
                                             DesignInfo::CellType_LUT6, 0, 0);
                }
                else
                {
                    assert(pinBeDriven->getDriverPin());
                    assert(pinBeDriven->getDriverPin()->getCell()->getCellType() == DesignInfo::CellType_MUXF7);
                    curMacro->addVirtualCell(pinBeDriven->getDriverPin()->getCell()->getName() + "__FFF7", designInfo,
                                             DesignInfo::CellType_FDCE, 0, 0);
                    curMacro->addCell(pinBeDriven->getDriverPin()->getCell(),
                                      pinBeDriven->getDriverPin()->getCell()->getCellType(), 0, 0);
                }
            }
        }

        unsigned int lastMux = 0;
        assert(curMacro->getCells().size() >= 4);
        DesignInfo::DesignCell *tmpCell = nullptr;
        for (unsigned int i = lastMux + 1; i < curMacro->getCells().size(); i++)
        {
            if (curMacro->getCells()[i]->isMux())
            {
                lastMux = i;
                tmpCell = curMacro->getCells()[i];
                break;
            }
        }
        assert(tmpCell);
        assert(tmpCell->isMux());
        for (DesignInfo::DesignPin *pinBeDriven : tmpCell->getInputPins())
        {
            if (pinBeDriven->getRefPinName() == "I0" || pinBeDriven->getRefPinName() == "I1")
            {
                if (pinBeDriven->isUnconnected())
                {
                    curMacro->addVirtualCell(tmpCell->getName() + "__" + pinBeDriven->getRefPinName(), designInfo,
                                             DesignInfo::CellType_LUT6, 0, 0);
                }
                else
                {
                    assert(pinBeDriven->getDriverPin()->getCell()->isLUT() &&
                           "just a guess assert. maybe just to use a if-else branch");
                    curMacro->addCell(pinBeDriven->getDriverPin()->getCell(), DesignInfo::CellType_LUT6, 0, 0);
                }
            }
        }

        assert(curMacro->getCells().size() >= 7);
        tmpCell = nullptr;
        for (unsigned int i = lastMux + 1; i < curMacro->getCells().size(); i++)
        {
            if (curMacro->getCells()[i]->isMux())
            {
                lastMux = i;
                tmpCell = curMacro->getCells()[i];
                break;
            }
        }
        assert(tmpCell);
        assert(tmpCell->isMux());
        if (tmpCell->isMux())
        {
            for (DesignInfo::DesignPin *pinBeDriven : tmpCell->getInputPins())
            {
                if (pinBeDriven->getRefPinName() == "I0" || pinBeDriven->getRefPinName() == "I1")
                {
                    if (pinBeDriven->isUnconnected())
                    {
                        curMacro->addVirtualCell(tmpCell->getName() + "__" + pinBeDriven->getRefPinName(), designInfo,
                                                 DesignInfo::CellType_LUT6, 0, 0);
                    }
                    else
                    {
                        assert(pinBeDriven->getDriverPin()->getCell()->isLUT() &&
                               "just a guess assert. maybe just to use a if-else branch");
                        curMacro->addCell(pinBeDriven->getDriverPin()->getCell(), DesignInfo::CellType_LUT6, 0, 0);
                    }
                }
            }
        }

        assert(curMacro->getCells().size() == 10);
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }
        curMacro->setWeight(totalWeight);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];
        if (curCell->getCellType() != DesignInfo::CellType_MUXF7)
            continue;

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

        std::vector<DesignInfo::DesignCell *> curMacroCores;
        curMacroCores.clear();
        curMacroCores.push_back(curCell);
        PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
            curMacroCores[0]->getName(), placementUnits.size(), PlacementInfo::PlacementMacro::PlacementMacroType_MUX7);
        curMacro->addOccupiedSite(0.0, 0.25);
        curMacro->addCell(curCell, curCell->getCellType(), 0, 0);

        for (DesignInfo::DesignPin *driverPin : curCell->getOutputPins())
        {
            if (driverPin->getRefPinName() == "O")
            {
                auto curOutputNet = driverPin->getNet();
                if (curOutputNet->getPinsBeDriven().size() != 1)
                    continue;
                auto pinBeDriven = curOutputNet->getPinsBeDriven()[0];
                if (pinBeDriven->getCell()->isFF())
                {
                    if (pinBeDriven->getRefPinName().find("D") != std::string::npos)
                    {
                        curMacro->addCell(pinBeDriven->getCell(), pinBeDriven->getCell()->getCellType(), 0, 0);
                    }
                }
            }
        }
        curMacro->addVirtualCell(curCell->getName() + "__FFF7", designInfo, DesignInfo::CellType_FDCE, 0, 0);

        for (DesignInfo::DesignPin *pinBeDriven : curMacroCores[0]->getInputPins())
        {
            if (pinBeDriven->getRefPinName() == "I0" || pinBeDriven->getRefPinName() == "I1")
            {
                if (pinBeDriven->isUnconnected())
                {
                    curMacro->addVirtualCell(curCell->getName() + "__" + pinBeDriven->getRefPinName(), designInfo,
                                             DesignInfo::CellType_LUT6, 0, 0);
                }
                else
                {
                    if (pinBeDriven->getDriverPin())
                    {
                        assert(pinBeDriven->getDriverPin()->getCell()->isLUT() &&
                               "just a guess assert. maybe just to use a if-else branch");
                        curMacro->addCell(pinBeDriven->getDriverPin()->getCell(), DesignInfo::CellType_LUT6, 0, 0);
                    }
                    else
                    {
                        curMacro->addVirtualCell(curCell->getName() + "__" + pinBeDriven->getRefPinName(), designInfo,
                                                 DesignInfo::CellType_LUT6, 0, 0);
                    }
                }
            }
        }
        assert(curMacro->getCells().size() >= 4);
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }
        curMacro->setWeight(totalWeight);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    int cellInMuxMacrosCnt = 0;
    for (auto macro : res)
        cellInMuxMacrosCnt += macro->getCells().size();

    print_info("#Mux Macro: " + std::to_string(res.size()));
    print_info("#Mux Macro Cells: " + std::to_string(cellInMuxMacrosCnt));
    print_status("Mux Macro Extracted.");
}

// LUTs and Muxs connected are Macros
void InitialPacker::loadOtherCLBMacros(std::string RAMMacroListFromVivadoFileName)
{
    // currently, some macros of distributed RAMs is difficult. However, the good news is that the number of the
    // unpredictable macros is very small. Therefore, temporarily, we load this information extracted from Vivado.

    std::vector<PlacementInfo::PlacementMacro *> res;
    res.clear();

    std::ifstream infile(RAMMacroListFromVivadoFileName.c_str());

    std::string line;
    std::getline(infile, line);
    std::string cellName, siteName, BELName, fill0, fill1, fill2;
    std::istringstream iss(line);

    std::map<std::string, std::vector<std::string>> siteName2Cells;
    std::map<std::string, std::string> cellName2BELLoc;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> fill0 >> cellName >> fill1 >> siteName >> fill2 >> BELName;
        if (strContains(fill0, "name=>"))
        {
            if (siteName2Cells.find(siteName) == siteName2Cells.end())
            {
                siteName2Cells[siteName] = std::vector<std::string>();
            }
            siteName2Cells[siteName].push_back(cellName);
            cellName2BELLoc[cellName] = BELName;
        }
    }

    for (auto it = siteName2Cells.begin(); it != siteName2Cells.end(); it++)
    {
        auto curSiteName = it->first;
        auto curSite = deviceInfo->getSiteWithName(curSiteName);
        std::set<DesignInfo::DesignCell *> macroCells;
        std::set<char> BELset;
        macroCells.clear();
        BELset.clear();

        for (auto cellName : it->second)
        {
            macroCells.insert(designInfo->getCell(cellName));
            BELset.insert(cellName2BELLoc[cellName][6]);
        }

        PlacementInfo::PlacementMacro *curMacro = nullptr;

        if (curSite->getSiteType() == "SLICEM") // is LUTRAM macro
        {
            curMacro = new PlacementInfo::PlacementMacro((*macroCells.begin())->getName(), placementUnits.size(),
                                                         PlacementInfo::PlacementMacro::PlacementMacroType_MCLB);
            curMacro->addOccupiedSite(0.0, 1.0);
            for (DesignInfo::DesignCell *cell : macroCells)
            {
                std::vector<std::string> splited;
                strSplit(cellName2BELLoc[cell->getName()], splited, ".");
                curMacro->addFixedCellInfo(cell, splited[0], splited[1]);
                if (cell->isLUT() || cell->isLUTRAM())
                {
                    if (compatiblePlacementTable->getOccupation(cell->getCellType()) == 2)
                        curMacro->addCell(cell, DesignInfo::CellType_LUT6, 0, 0);
                    else
                        curMacro->addCell(cell, DesignInfo::CellType_LUT5, 0, 0);
                }
                else
                {
                    curMacro->addCell(cell, cell->getCellType(), 0, 0);
                    curMacro->addLUTRAM(); // just let it be a fake LUTRAM so it can be mapped to SLICEM
                }

                // curMacro->setLocInMacro(cell, 0, 0);
            }
        }
        else
        {
            curMacro = new PlacementInfo::PlacementMacro((*macroCells.begin())->getName(), placementUnits.size(),
                                                         PlacementInfo::PlacementMacro::PlacementMacroType_LCLB);
            curMacro->addOccupiedSite(0.0, 1.0);
            for (DesignInfo::DesignCell *cell : macroCells)
            {
                curMacro->addCell(cell, cell->getCellType(), 0, 0);
            }
        }

        // int factor = macroCells[0]->getCellType() == DesignInfo::CellType_RAM32X1D ? 2 : 1;
        // for (unsigned int i = 0; i < BELset.size() * factor; i++)
        // {
        //     curMacro->addVirtualCell(designInfo, DesignInfo::CellType_LUT6, 0, 0);
        // }
        int totalWeight = 0;
        for (auto tmpCell : curMacro->getCells())
        {
            assert(cellInMacros.find(tmpCell) == cellInMacros.end());
            cellId2PlacementUnit[tmpCell->getElementIdInType()] = curMacro;
            cellInMacros.insert(tmpCell);
            totalWeight += compatiblePlacementTable->cellType2sharedBELTypeOccupation[tmpCell->getCellType()];
        }

        curMacro->setWeight(totalWeight);
        res.push_back(curMacro);
        placementMacros.push_back(curMacro);
        placementUnits.push_back(curMacro);
    }

    int cellInMuxMacrosCnt = 0;
    for (auto macro : res)
        cellInMuxMacrosCnt += macro->getCells().size();

    print_info("#CLB Macro: " + std::to_string(res.size()));
    print_info("#CLB Macro Cells: " + std::to_string(cellInMuxMacrosCnt));
    print_status("CLB Macro Extracted.");
}

void InitialPacker::LUTFFPairing()
{
    print_status("InitialPacker Pairing LUTs and FFs.");

    std::vector<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> LUTFFPairs;
    LUTFFPairs.clear();
    std::vector<bool> packed(placementUnpackedCells.size(), false);
    std::vector<PlacementInfo::PlacementUnpackedCell *> oriPlacementUnpackedCells = placementUnpackedCells;

    placementUnits.resize(placementMacros.size());

    int LUTTO1FFPackedCnt = 0;

    std::vector<DesignInfo::DesignCell *> &curCellsInDesign = designInfo->getCells();
    int curNumCells = designInfo->getNumCells();
    for (int curCellId = 0; curCellId < curNumCells; curCellId++)
    {
        auto curCell = curCellsInDesign[curCellId];

        if (cellInMacros.find(curCell) != cellInMacros.end())
            continue;

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
                        LUTFFPairs.emplace_back(curCell, FFBeDriven);

                        PlacementInfo::PlacementMacro *curMacro = new PlacementInfo::PlacementMacro(
                            curCell->getName(), placementUnits.size(),
                            PlacementInfo::PlacementMacro::PlacementMacroType_LUTFFPair);

                        curMacro->addOccupiedSite(0.0, 0.0625);
                        curMacro->addCell(curCell, curCell->getCellType(), 0, 0.0);
                        curMacro->addCell(FFBeDriven, FFBeDriven->getCellType(), 0, 0.0);

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
            else
            {
                // this is a LUT6_2, has two output pins and we don't pack them temporarily.
            }
        }
    }

    print_info("InitialPacker: LUTTO1FFPackedCnt=" + std::to_string(LUTTO1FFPackedCnt));

    print_status("InitialPacker Paired LUTs and FFs (#Pairs = " + std::to_string(LUTFFPairs.size()) + ")");
}

void InitialPacker::findUnpackedUnits()
{
    // const char *celltypestr[] = {CELLTYPESTRS};
    for (DesignInfo::DesignCell *cell : designInfo->getCells())
    {
        if (cellInMacros.find(cell) == cellInMacros.end())
        {
            assert(!cell->isVirtualCell());
            PlacementInfo::PlacementUnpackedCell *curUnpackedCell =
                new PlacementInfo::PlacementUnpackedCell(cell->getName(), placementUnits.size(), cell);
            curUnpackedCell->setWeight(compatiblePlacementTable->cellType2sharedBELTypeOccupation[cell->getCellType()]);

            cellId2PlacementUnit[cell->getElementIdInType()] = curUnpackedCell;
            placementUnits.push_back(curUnpackedCell);
            placementUnpackedCells.push_back(curUnpackedCell);
        }
    }
}

void packerUtil_StrReplaceAll(std::string &str, const std::string from, const std::string to)
{
    if (from.empty())
        return;
    std::string::size_type start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void InitialPacker::loadFixedPlacementUnits(std::string fixedPlacementUnitsFromVivadoFileName)
{
    fixedPlacementUnits.clear();
    std::ifstream infile(fixedPlacementUnitsFromVivadoFileName.c_str());

    std::string line;
    std::getline(infile, line);
    std::string cellName, siteName, BELName, fill0, fill1, fill2;
    std::istringstream iss(line);

    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> fill0 >> cellName >> fill1 >> siteName >> fill2 >> BELName;
        if (strContains(fill0, "name=>"))
        {
            auto curCell = designInfo->getCell(cellName);
            if (PlacementInfo::PlacementUnpackedCell *unpackedPU = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(
                    cellId2PlacementUnit[curCell->getElementIdInType()]))
            {
                unpackedPU->setLockedAt(siteName, BELName, deviceInfo);
                fixedPlacementUnits.push_back(unpackedPU);
            }
            else if (PlacementInfo::PlacementMacro *curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(
                         cellId2PlacementUnit[curCell->getElementIdInType()]))
            {
                // We can only handle BRAM36 currently
                assert(curCell->getCellType() == DesignInfo::CellType_RAMB36E2);
                assert(curMacro->getCells().size() == 2);
                assert(curMacro->getCells()[0]->getCellType() == DesignInfo::CellType_RAMB36E2);
                assert(curMacro->getCells()[1]->getCellType() == DesignInfo::CellType_RAMB18E2 &&
                       curMacro->getCells()[1]->isVirtualCell());

                // 12
                // 012345678901
                // RAMB18_X16Y2 bel=>  RAMB180.RAMB18E2_L
                int locY = std::stoi(siteName.substr(siteName.find('Y') + 1, siteName.size() - siteName.find('Y') - 1));
                std::string newSiteName = siteName.substr(0, siteName.find('Y') + 1);
                std::string oriS = "RAMB36_";
                std::string newS = "RAMB18_";
                packerUtil_StrReplaceAll(newSiteName, oriS, newS);
                std::string newSiteNameA = newSiteName + std::to_string(locY * 2);
                std::string newSiteNameB = newSiteName + std::to_string(locY * 2 + 1);
                std::string newBELNameA = "RAMB180.RAMB18E2_L";
                std::string newBELNameB = "RAMB181.RAMB18E2_U";

                curMacro->addFixedCellInfo(curMacro->getCells()[0], newSiteNameA, newBELNameA);
                curMacro->addFixedCellInfo(curMacro->getCells()[1], newSiteNameB, newBELNameB);
                deviceInfo->getSite(newSiteNameA)->setOccupied();
                deviceInfo->getSite(newSiteNameB)->setOccupied();
                curMacro->setAnchorLocation(deviceInfo->getSite(newSiteNameA)->X(),
                                            deviceInfo->getSite(newSiteNameA)->Y());

                curMacro->setFixed();
                curMacro->setPlaced();
                curMacro->setLocked();

                fixedPlacementUnits.push_back(curMacro);
                // assert(false && "unimplemented.");
            }
        }
    }
}

void InitialPacker::dumpMacroHighLight()
{
    std::ofstream tmpColorFile("color.tcl");
    // 2 -> carry
    // 1 -> DSP
    // 3 -> BRAM
    // 4 -> LUTRAM
    // 7 -> MUX

    tmpColorFile << "highlight -color_index 2 [ get_cells { \n";
    for (auto tmpMacro : placementInfo->getPlacementMacros())
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_CARRY)
        {
            if (tmpMacro->getCells().size() == 1)
                continue;
            for (auto tmpCell : tmpMacro->getCells())
            {
                if (!tmpCell->isVirtualCell())
                {
                    tmpColorFile << tmpCell->getName() << "\n";
                }
            }
        }
    }
    tmpColorFile << "}]\n";

    tmpColorFile << "highlight -color_index 1 [ get_cells { \n";
    for (auto tmpMacro : placementInfo->getPlacementMacros())
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_DSP)
        {
            if (tmpMacro->getCells().size() == 1)
                continue;
            for (auto tmpCell : tmpMacro->getCells())
            {
                if (!tmpCell->isVirtualCell())
                {
                    tmpColorFile << tmpCell->getName() << "\n";
                }
            }
        }
    }
    tmpColorFile << "}]\n";

    tmpColorFile << "highlight -color_index 3 [ get_cells { \n";
    for (auto tmpMacro : placementInfo->getPlacementMacros())
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_BRAM)
        {
            if (tmpMacro->getCells().size() == 1)
                continue;
            for (auto tmpCell : tmpMacro->getCells())
            {
                if (!tmpCell->isVirtualCell())
                {
                    tmpColorFile << tmpCell->getName() << "\n";
                }
            }
        }
    }
    tmpColorFile << "}]\n";

    tmpColorFile << "highlight -color_index 4 [ get_cells { \n";
    for (auto tmpMacro : placementInfo->getPlacementMacros())
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MCLB)
        {
            if (tmpMacro->getCells().size() == 1)
                continue;
            for (auto tmpCell : tmpMacro->getCells())
            {
                if (!tmpCell->isVirtualCell())
                {
                    tmpColorFile << tmpCell->getName() << "\n";
                }
            }
        }
    }
    tmpColorFile << "}]\n";

    tmpColorFile << "highlight -color_index 7 [ get_cells { \n";
    for (auto tmpMacro : placementInfo->getPlacementMacros())
    {
        if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX7 ||
            tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX8 ||
            tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX9)
        {
            if (tmpMacro->getCells().size() == 1)
                continue;
            for (auto tmpCell : tmpMacro->getCells())
            {
                if (!tmpCell->isVirtualCell())
                {
                    tmpColorFile << tmpCell->getName() << "\n";
                }
            }
        }
    }
    tmpColorFile << "}]\n";
    tmpColorFile.close();
}