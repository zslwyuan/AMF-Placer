/**
 * @file DesignInfo.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation for a standalone design netlist.
 * @version 0.1
 * @date 2021-06-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "DesignInfo.h"
#include "readZip.h"
#include "strPrint.h"
#include "stringCheck.h"
#include <regex>
#include <assert.h>

void DesignInfo::DesignPin::updateParentCellNetInfo()
{
    DesignInfo::DesignCell *parentCell = getCell();
    parentCell->addNetForPin(this, netPtr);
}

DesignInfo::DesignPinType DesignInfo::DesignPin::checkPinType(DesignCell *cell, std::string &_refpinName, bool isInput)
{
    if (cell->isLUT())
    {
        if (isInput)
            return PinType_LUTInput;
        else
            return PinType_LUTOutput;
    }
    else if (cell->isFF())
    {
        if (isInput)
        {
            if (_refpinName == "R" || _refpinName == "S" || _refpinName == "SRI" || _refpinName == "CLR" ||
                _refpinName == "PRE")
                return PinType_SR;
            if (_refpinName == "C" || _refpinName == "G")
                return PinType_CLK;
            if (_refpinName == "D" || _refpinName == "DI")
                return PinType_D;
            if (_refpinName == "CE" || _refpinName == "GE")
                return PinType_E;
            print_error("no such pin type:" + _refpinName);
            assert(false && "no such pin type");
            return PinType_SR; // no meaning
        }
        else
        {
            if (_refpinName == "Q" || _refpinName == "O")
                return PinType_Q;
            std::cout << "cell: " << cell << "\n";
            print_error("no such pin type:" + _refpinName);
            assert(false && "no such pin type");
            return PinType_SR; // no meaning
        }
    }
    else
    {
        return PinType_Others;
    }
}

void DesignInfo::DesignNet::connectToPinName(const std::string &_pinName)
{
    pinNames.push_back(_pinName);
}

void DesignInfo::DesignNet::connectToPinVariable(DesignPin *_pinPtr)
{
    pinPtrs.push_back(_pinPtr);
    if (_pinPtr->isOutputPort())
        driverPinPtrs.push_back(_pinPtr);
    else
        BeDrivenPinPtrs.push_back(_pinPtr);
}

void DesignInfo::DesignCell::addNetForPin(DesignPin *_pinPtr, DesignNet *_netPtr)
{
    netPtrs.push_back(_netPtr);
    if (_netPtr)
    {
        netNames.push_back(_netPtr->getName());
        if (_pinPtr->isOutputPort())
        {
            outputNetPtrs.push_back(_netPtr);
        }
        else
        {
            inputNetPtrs.push_back(_netPtr);
        }
    }
    else
    {
        netNames.push_back(std::string(""));
    }
}

void DesignInfo::DesignCell::addPin(DesignPin *_pinPtr)
{
    pinPtrs.push_back(_pinPtr);
    pinNames.push_back(_pinPtr->getName());
    if (_pinPtr->isOutputPort())
    {
        outputPinPtrs.push_back(_pinPtr);
    }
    else
    {
        inputPinPtrs.push_back(_pinPtr);
    }
}

void DesignInfo::addPinToNet(DesignPin *curPin)
{
    if (name2Net.find(curPin->getNetName()) == name2Net.end())
    {
        DesignNet *curNet = new DesignNet(curPin->getNetName(), getNumNets());
        netlist.push_back(curNet);
        name2Net[curPin->getNetName()] = curNet;
    }

    DesignNet *curNet = name2Net[curPin->getNetName()];
    curNet->connectToPinName(curPin->getName());
    curNet->connectToPinVariable(curPin);
}

DesignInfo::DesignInfo(std::map<std::string, std::string> &JSONCfg, DeviceInfo *deviceInfo) : JSONCfg(JSONCfg)
{

    // curCell=>
    // design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1
    // type=> LUT4
    //    pin=>
    //    design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/O
    //    dir=> OUT net=>
    //    design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/next_fwft_state__0[0]
    //    drivepin=>
    //    design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/O
    //    pin=>
    //    design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/FSM_sequential_gen_fwft.curr_fwft_state[0]_i_1/I0
    //    dir=> IN net=>
    //    design_1_i/axis_clock_converter_0/inst/gen_async_conv.axisc_async_clock_converter_0/xpm_fifo_async_inst/gnuram_async_fifo.xpm_fifo_base_inst/rd_en
    //    drivepin=> design_1_i/face_detect_0/inst/regslice_both_input_r_U/ibuf_inst/input_r_TREADY_INST_0/O

    designArchievedTextFileName = std::string(JSONCfg["vivado extracted design information file"]);
    netlist.clear();
    cells.clear();
    pins.clear();
    name2Net.clear();
    name2Cell.clear();
    type2Cells.clear();
    connectedPinsWithSmallNet.clear();
    CLKSRCEFFType2ControlSetInfoId.clear();
    controlSets.clear();
    clock2Cells.clear();
    clocks.clear();
    clockSet.clear();

    print_status("Design Information Loading.");

    std::string unzipCmnd = "unzip -p " + designArchievedTextFileName;
    FILEbuf sbuf(popen(unzipCmnd.c_str(), "r"));
    std::istream infile(&sbuf);
    // std::ifstream infile(designTextFileName.c_str());

    std::string line;
    std::getline(infile, line);
    std::string cellType, cellName, targetName, dir, refpinname, netName, drivepinName, fill0, fill1, fill2, fill3;
    std::istringstream iss(line);
    iss >> fill0 >> cellName >> fill1 >> cellType;

    DesignCell *curCell = new DesignCell(cellName, fromStringToCellType(cellName, cellType), getNumCells());
    curCell = addCell(curCell);

    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> fill0 >> targetName;
        if (strContains(fill0, "pin=>"))
        {
            iss >> fill3 >> refpinname >> fill0 >> dir >> fill1 >> netName >> fill2 >> drivepinName;
            DesignPin *curPin = new DesignPin(targetName, refpinname,
                                              DesignPin::checkPinType(curCell, refpinname, dir == std::string("IN")),
                                              dir == std::string("IN"), curCell, pins.size());
            pins.push_back(curPin);
            curCell->addPin(curPin);

            if (netName == "drivepin=>")
            {
                curPin->updateParentCellNetInfo();
                curPin->setUnconnected();
                continue; // not connected
            }
            assert(drivepinName != "");

            assert(fill1 == "net=>");
            std::regex GNDpattern(".*/<const0>");
            if (std::regex_match(netName, GNDpattern))
            {
                netName = drivepinName = "<const0>";
            }
            std::regex VCCpattern(".*/<const1>");
            if (std::regex_match(netName, VCCpattern))
            {
                netName = drivepinName = "<const1>";
            }
            netName = drivepinName; // don't use the net name, which has aliases in Vivado, otherwise will fail to map
            curPin->setDriverPinName(drivepinName); // bind to a net name first
            curPin->connectToNetName(netName);
            addPinToNet(curPin);                             // update net in netlist
            curPin->connectToNetVariable(name2Net[netName]); // bind to a net pointer
            curPin->updateParentCellNetInfo();
        }
        else if (strContains(fill0, "curCell=>"))
        {
            iss >> fill1 >> cellType;
            curCell = new DesignCell(targetName, fromStringToCellType(targetName, cellType), getNumCells());
            curCell = addCell(curCell);
        }
        else
            assert(false && "Parser Error");
    }

    for (DesignNet *curNet : netlist)
    {
        for (DesignPin *driverPin : curNet->getDriverPins())
            for (DesignPin *pinBeDriven : curNet->getPinsBeDriven())
            {
                pinBeDriven->setDriverPin(driverPin);
            }
    }

    for (DesignNet *curNet : netlist)
    {
        if (curNet->getPins().size() < 32)
        {
            for (DesignPin *tmpDrivePin : curNet->getPins())
                for (DesignPin *tmpPinBeDriven : curNet->getPins())
                {
                    if (tmpDrivePin != tmpPinBeDriven)
                    {
                        connectedPinsWithSmallNet.insert(
                            std::pair<DesignPin *, DesignPin *>(tmpDrivePin, tmpPinBeDriven));
                    }
                }
        }
    }

    print_info("#Connected Cell Pairs in Small Nets = " + std::to_string(connectedPinsWithSmallNet.size()));

    std::string STR_PCIE_3_1 = "PCIE_3_1";

    assert(deviceInfo->getSitesInType(STR_PCIE_3_1).size() > 0 && "info for PCIE should be included in deviceInfo.");
    DeviceInfo::DeviceSite::DeviceSitePinInfos *PCIESitePinInfo =
        deviceInfo->getSitesInType(STR_PCIE_3_1)[0]->getSitePinInfos();
    assert(PCIESitePinInfo);
    for (DesignCell *PCIECell : type2Cells[CellType_PCIE_3_1])
    {
        for (DesignPin *curPin : PCIECell->getPins())
        {
            assert(PCIESitePinInfo->name2offsetX.find(curPin->getRefPinName()) != PCIESitePinInfo->name2offsetX.end());
            curPin->setOffsetInCell(PCIESitePinInfo->name2offsetX[curPin->getRefPinName()],
                                    PCIESitePinInfo->name2offsetY[curPin->getRefPinName()]);
        }
    }

    updateFFControlSets();

    if (JSONCfg.find("clock file") != JSONCfg.end())
    {
        std::string clockFileName = std::string(JSONCfg["clock file"]);
        loadClocks(clockFileName);
    }
    else
    {
        print_warning("No clock file to specify clock signals in design! It might lead to clock partitioning error in "
                      "final placement in vendor tools!");
    }

    loadUserDefinedClusterNets();

    print_status("New Design Info Created.");
}

void DesignInfo::loadClocks(std::string clockFileName)
{
    std::ifstream clockFile(clockFileName);
    while (clockFile.peek() != EOF)
    {
        std::string clockDriverPinName;
        clockFile >> clockDriverPinName;
        if (clockDriverPinName == "")
            continue;
        if (name2Net.find(clockDriverPinName) == name2Net.end())
        {
            print_warning("global clock: [" + clockDriverPinName +
                          "] is not found in design info. It might not be a problem as long as it is an external pin "
                          "or only connected to one instance.");
            continue;
        }
        assert(name2Net.find(clockDriverPinName) != name2Net.end());
        assert(clockSet.find(name2Net[clockDriverPinName]) == clockSet.end());
        clockSet.insert(name2Net[clockDriverPinName]);
        clocks.push_back(name2Net[clockDriverPinName]);
        clock2Cells[name2Net[clockDriverPinName]] = std::set<DesignCell *>();
        name2Net[clockDriverPinName]->setGlobalClock();
        // if (name2Net[clockDriverPinName]->getPins().size() < 4000)
        //     name2Net[clockDriverPinName]->setOverallNetEnhancement(1.1);
    }

    for (auto tmpCell : cells)
    {
        for (auto tmpPin : tmpCell->getPins())
        {
            if (!tmpPin->isUnconnected())
            {
                if (tmpPin->getNet())
                {
                    if (isDesignClock(tmpPin->getNet()))
                    {
                        clock2Cells[tmpPin->getNet()].insert(tmpCell);
                    }
                }
            }
        }
    }

    print_info("#global clock=" + std::to_string(clockSet.size()));
    // for (auto pair : clock2Cells)
    // {
    //     std::cout << "clock: [" << pair.first->getName() << "] drives " << pair.second.size() << " loads\n";
    // }
}

void DesignInfo::updateFFControlSets()
{
    if (controlSets.size())
    {
        for (auto CS : controlSets)
            delete CS;
    }

    controlSets.clear();
    FFId2ControlSetId.clear();
    CLKSRCEFFType2ControlSetInfoId.clear();
    FFId2ControlSetId.resize(cells.size(), -1);
    for (auto curCell : cells)
    {
        if (curCell->isFF())
        {
            if (curCell->isVirtualCell())
                continue;
            int CLKId, SRId, CEId;
            getCLKSRCENetId(curCell, CLKId, SRId, CEId);
            std::tuple<int, int, int, int> CS(CLKId, SRId, CEId, getFFSRType(curCell->getOriCellType()));
            ControlSetInfo *curCS = nullptr;
            if (CLKSRCEFFType2ControlSetInfoId.find(CS) != CLKSRCEFFType2ControlSetInfoId.end())
            {
                curCS = controlSets[CLKSRCEFFType2ControlSetInfoId[CS]];
                curCell->setControlSetInfo(curCS);
            }
            else
            {
                curCS = new ControlSetInfo(curCell, controlSets.size());
                CLKSRCEFFType2ControlSetInfoId[CS] = controlSets.size();
                controlSets.push_back(curCS);
                curCell->setControlSetInfo(curCS);
            }
            curCS->addFF(curCell);
        }
    }
}

void DesignInfo::enhanceFFControlSetNets()
{
    if (controlSets.size())
    {
        for (auto CS : controlSets)
        {
            if (CS->getCE())
            {
                if (CS->getCE()->getPins().size() < 5000 && CS->getCE()->getPins().size() > 25)
                    CS->getCE()->enhanceOverallNetEnhancement(1.2);
            }
            if (CS->getSR())
            {
                if (CS->getSR()->getPins().size() < 5000 && CS->getSR()->getPins().size() > 25)
                    CS->getSR()->enhanceOverallNetEnhancement(1.2);
            }
        }
    }
}

void DesignInfo::loadUserDefinedClusterNets()
{
    if (JSONCfg.find("designCluster") != JSONCfg.end())
    {
        std::string patterFile = std::string(JSONCfg["designCluster"]);
        print_status("Design User-Defined Cluster Information Loading.");
        std::string unzipCmnd = "unzip -p " + patterFile;
        FILEbuf sbuf(popen(unzipCmnd.c_str(), "r"));
        std::istream infile(&sbuf);
        std::string line;
        std::vector<std::string> strV;
        std::set<DesignCell *> userDefinedClusterCells;
        std::vector<DesignCell *> userDefinedClusterCellsVec;
        std::set<DesignCell *> allCellsInClusters;
        std::set<DesignNet *> enhancedNets;
        enhancedNets.clear();
        allCellsInClusters.clear();

        while (std::getline(infile, line))
        {
            strV.clear();
            userDefinedClusterCells.clear();
            userDefinedClusterCellsVec.clear();
            strSplit(line, strV, " ");
            for (auto tmpName : strV)
            {
                assert(name2Cell.find(tmpName) != name2Cell.end());
                if (userDefinedClusterCells.find(name2Cell[tmpName]) == userDefinedClusterCells.end())
                {
                    userDefinedClusterCellsVec.push_back(name2Cell[tmpName]);
                    userDefinedClusterCells.insert(name2Cell[tmpName]);
                    allCellsInClusters.insert(name2Cell[tmpName]);
                }
            }
            predefinedClusters.push_back(userDefinedClusterCellsVec);

            bool hasDSP = false;
            bool hasBRAM = false;
            for (auto cellA : userDefinedClusterCells)
            {
                hasBRAM |= cellA->isBRAM();
                hasDSP |= cellA->isDSP();
            }
            if (hasBRAM || hasDSP)
                continue;

            if (userDefinedClusterCells.size() > 200 || userDefinedClusterCells.size() < 16)
                continue;

            // float enhanceRatio = userDefinedClusterCells.size() / 100.0 * 0.5;
            // if (enhanceRatio > 0.5)
            //     enhanceRatio = 0.5;
            float enhanceRatio = 1.5 - (0.5 * userDefinedClusterCells.size() / 200.0);
            if (enhanceRatio < 1)
                continue;
            for (auto cellA : userDefinedClusterCells)
            {
                int pinAIdInNet = 0;
                for (DesignPin *curPinA : cellA->getPins())
                {
                    if (curPinA->getNet())
                    {
                        if (curPinA->getNet()->getPins().size() > 64)
                            continue;
                        int pinBIdInNet = 0;
                        for (DesignPin *pinInNet : curPinA->getNet()->getPins())
                        {
                            if (userDefinedClusterCells.find(pinInNet->getCell()) != userDefinedClusterCells.end())
                            {
                                if (pinInNet->getCell() != cellA)
                                {
                                    if (connectedPinsWithSmallNet.find(std::pair<DesignPin *, DesignPin *>(
                                            curPinA, pinInNet)) != connectedPinsWithSmallNet.end())
                                    {
                                        // if (cellA->isDSP() || pinInNet->getCell()->isDSP() || cellA->isBRAM() ||
                                        //     pinInNet->getCell()->isBRAM())
                                        // {
                                        //     curPinA->getNet()->enhance(pinAIdInNet, pinBIdInNet, 1.2);
                                        //     curPinA->getNet()->enhance(pinBIdInNet, pinAIdInNet, 1.2);
                                        // }
                                        // else
                                        // {
                                        //     curPinA->getNet()->enhance(pinAIdInNet, pinBIdInNet, 1.1);
                                        //     curPinA->getNet()->enhance(pinBIdInNet, pinAIdInNet, 1.1);
                                        // }
                                        curPinA->getNet()->enhance(pinAIdInNet, pinBIdInNet, enhanceRatio);
                                        curPinA->getNet()->enhance(pinBIdInNet, pinAIdInNet, enhanceRatio);
                                        enhancedNets.insert(curPinA->getNet());
                                    }
                                }
                            }
                            pinBIdInNet++;
                        }
                    }
                    pinAIdInNet++;
                }
            }
        }

        int numClusters = predefinedClusters.size();
        for (int i = 0; i < numClusters; i++)
            for (int j = i + 1; j < numClusters; j++)
                if (predefinedClusters[i].size() < predefinedClusters[j].size())
                {
                    std::vector<DesignCell *> tmpSet = predefinedClusters[i];
                    predefinedClusters[i] = predefinedClusters[j];
                    predefinedClusters[j] = tmpSet;
                }

        print_info("#Nets Enhanced for userDefinedClusters = " + std::to_string(enhancedNets.size()));
        print_info("#Cells in userDefinedClusters = " + std::to_string(allCellsInClusters.size()));
    }
}

DesignInfo::DesignCellType DesignInfo::fromStringToCellType(std::string &cellName, std::string &typeName)
{
    for (std::size_t i = 0; i < DesignCellTypeStr.size(); ++i)
    {
        if (DesignCellTypeStr[i] == typeName)
            return static_cast<DesignCellType>(i);
    }
    print_error("no such type: " + typeName + " for cell: " + cellName);
    assert(false && "no such type. Please check your DesignCellType enum definition and CELLTYPESTRS macro.");
}

DesignInfo::DesignCell *DesignInfo::addCell(DesignCell *curCell)
{
    if (name2Cell.find(curCell->getName()) != name2Cell.end())
    {
        auto existingCell = name2Cell[curCell->getName()];
        print_warning("get duplicated cells from the design archieve. Maybe bug in Vivado Tcl Libs.");
        std::cout << "duplicated cell: " << existingCell << "\n";
        delete curCell;
        return existingCell;
    }
    cells.push_back(curCell);
    name2Cell[curCell->getName()] = curCell;
    if (type2Cells.find(curCell->getCellType()) == type2Cells.end())
        type2Cells[curCell->getCellType()] = std::vector<DesignCell *>();
    type2Cells[curCell->getCellType()].push_back(curCell);
    return curCell;
}

void DesignInfo::printStat(bool verbose)
{
    print_info("#Cell= " + std::to_string(cells.size()));
    print_info("#Net= " + std::to_string(netlist.size()));

    std::string existTypes = "";

    for (std::map<DesignCellType, std::vector<DesignCell *>>::iterator it = type2Cells.begin(); it != type2Cells.end();
         ++it)
    {
        existTypes += DesignCellTypeStr[it->first] + " ";
    }

    print_info("CellTypes(" + std::to_string(type2Cells.size()) + " types): " + existTypes);

    int LUTCnt = 0;
    int FFCnt = 0;
    int CarryCnt = 0;
    int MuxCnt = 0;
    int LUTRAMCnt = 0;
    int DSPCnt = 0;
    int BRAMCnt = 0;
    for (std::map<DesignCellType, std::vector<DesignCell *>>::iterator it = type2Cells.begin(); it != type2Cells.end();
         ++it)
    {
        if (verbose)
            print_info("#" + DesignCellTypeStr[it->first] + ": " + std::to_string(it->second.size()));
        if (isLUT(it->first))
            LUTCnt += it->second.size();
        if (isFF(it->first))
            FFCnt += it->second.size();
        if (isDSP(it->first))
            DSPCnt += it->second.size();
        if (isBRAM(it->first))
            BRAMCnt += it->second.size();
        if (isMux(it->first))
            MuxCnt += it->second.size();
        if (isLUTRAM(it->first))
            LUTRAMCnt += it->second.size();
        if (isCarry(it->first))
            CarryCnt += it->second.size();
    }
    print_info("#LUTCnt: " + std::to_string(LUTCnt));
    print_info("#FFCnt: " + std::to_string(FFCnt));
    print_info("#CarryCnt: " + std::to_string(CarryCnt));
    print_info("#MuxCnt: " + std::to_string(MuxCnt));
    print_info("#LUTRAMCnt: " + std::to_string(LUTRAMCnt));
    print_info("#DSPCnt: " + std::to_string(DSPCnt));
    print_info("#BRAMCnt: " + std::to_string(BRAMCnt));
}

std::ostream &operator<<(std::ostream &os, DesignInfo::DesignCell *cell)
{
    static const char *DesignCellTypeStr_const[] = {CELLTYPESTRS};
    os << "DesignCell: (" << DesignCellTypeStr_const[cell->getCellType()] << ") " << cell->getName();
    return os;
}

std::ostream &operator<<(std::ostream &os, DesignInfo::DesignPin *pin)
{
    auto cell = pin->getCell();
    static const char *DesignCellTypeStr_const[] = {CELLTYPESTRS};
    os << "DesignPin: " << pin->getName() << " = refPin: (" << pin->getRefPinName() << ") of "
       << " (" << DesignCellTypeStr_const[cell->getCellType()] << ") " << cell->getName();
    return os;
}