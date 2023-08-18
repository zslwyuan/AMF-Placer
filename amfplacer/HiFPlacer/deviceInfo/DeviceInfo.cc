/**
 * @file DeviceInfo.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This implementation file contains APIs' implementation for a standalone device.
 * @version 0.1
 * @date 2021-06-03
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "DeviceInfo.h"
#include "readZip.h"
#include "strPrint.h"
#include "stringCheck.h"
#include <algorithm>
#include <assert.h>

bool siteSortCmp(DeviceInfo::DeviceSite *a, DeviceInfo::DeviceSite *b)
{
    const float eps = 1e-3;
    if (a->Y() <= b->Y() + eps && a->Y() >= b->Y() - eps)
        return a->X() < b->X();
    else if (a->Y() < b->Y() - eps)
        return true;
    else if (a->Y() > b->Y() + eps)
        return false;
    return true;
}

DeviceInfo::DeviceInfo(std::map<std::string, std::string> JSONCfg, std::string _deviceName) : JSONCfg(JSONCfg)
{
    deviceArchievedTextFileName = std::string(JSONCfg["vivado extracted device information file"]);
    specialPinOffsetFileName = std::string(JSONCfg["special pin offset info file"]);

    // site=> SLICE_X59Y220 tile=> CLEL_R_X36Y220 sitetype=> SLICEL tiletype=> CLE_R centerx=> 37.25 centery=> 220.15
    // BELs=>
    // [SLICE_X59Y220/A5LUT,SLICE_X59Y220/A6LUT,SLICE_X59Y220/AFF,SLICE_X59Y220/AFF2,SLICE_X59Y220/B5LUT,SLICE_X59Y220/B6LUT,SLICE_X59Y220/BFF,SLICE_X59Y220/BFF2,SLICE_X59Y220/C5LUT,SLICE_X59Y220/C6LUT,SLICE_X59Y220/CARRY8,SLICE_X59Y220/CFF,SLICE_X59Y220/CFF2,SLICE_X59Y220/D5LUT,SLICE_X59Y220/D6LUT,SLICE_X59Y220/DFF,SLICE_X59Y220/DFF2,SLICE_X59Y220/E5LUT,SLICE_X59Y220/E6LUT,SLICE_X59Y220/EFF,SLICE_X59Y220/EFF2,SLICE_X59Y220/F5LUT,SLICE_X59Y220/F6LUT,SLICE_X59Y220/F7MUX_AB,SLICE_X59Y220/F7MUX_CD,SLICE_X59Y220/F7MUX_EF,SLICE_X59Y220/F7MUX_GH,SLICE_X59Y220/F8MUX_BOT,SLICE_X59Y220/F8MUX_TOP,SLICE_X59Y220/F9MUX,SLICE_X59Y220/FFF,SLICE_X59Y220/FFF2,SLICE_X59Y220/G5LUT,SLICE_X59Y220/G6LUT,SLICE_X59Y220/GFF,SLICE_X59Y220/GFF2,SLICE_X59Y220/H5LUT,SLICE_X59Y220/H6LUT,SLICE_X59Y220/HFF,SLICE_X59Y220/HFF2]
    // site=> RAMB18_X7Y88 tile=> BRAM_X36Y220 sitetype=> RAMBFIFO18 tiletype=> CLE_R centerx=> 36.75 centery=>
    // 221.96249999999998 BELs=> [RAMB18_X7Y88/RAMBFIFO18] site=> RAMB18_X7Y89 tile=> BRAM_X36Y220 sitetype=> RAMB181
    // tiletype=> CLE_R centerx=> 36.75 centery=> 224.11249999999998 BELs=> [RAMB18_X7Y89/RAMB18E2_U]

    BELTypes.clear();
    BELType2BELs.clear();
    BELs.clear();

    siteTypes.clear();
    siteType2Sites.clear();
    sites.clear();

    tileTypes.clear();
    tileType2Tiles.clear();
    tiles.clear();

    BELType2FalseBELType.clear();
    coord2ClockRegion.clear();

    if (JSONCfg.find("mergedSharedCellType2sharedCellType") != JSONCfg.end())
    {
        loadBELType2FalseBELType(JSONCfg["mergedSharedCellType2sharedCellType"]);
    }
    deviceName = _deviceName;

    std::string unzipCmnd = "unzip -p " + deviceArchievedTextFileName;
    FILEbuf sbuf(popen(unzipCmnd.c_str(), "r"));
    std::istream infile(&sbuf);
    // std::ifstream infile(designTextFileName.c_str());

    std::string line;
    std::string siteName, tileName, siteType, tileType, strBELs, clockRegionName, fill0, fill1, fill2, fill3, fill4,
        fill5, fill6, fill7;
    float centerX, centerY;

    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> fill0 >> siteName >> fill1 >> tileName >> fill7 >> clockRegionName >> fill2 >> siteType >> fill3 >>
            tileType >> fill4 >> centerX >> fill5 >> centerY >> fill6 >> strBELs;
        assert(fill0 == "site=>");
        assert(fill1 == "tile=>");
        assert(fill7 == "clockRegionName=>");
        assert(fill2 == "sitetype=>");
        assert(fill3 == "tiletype=>");
        std::string tmpFrom = "X";
        std::string tmpTo = "";
        replaceAll(clockRegionName, tmpFrom, tmpTo);
        tmpFrom = "Y";
        tmpTo = " ";
        replaceAll(clockRegionName, tmpFrom, tmpTo);
        std::vector<std::string> coordNumbers(0);
        strSplit(clockRegionName, coordNumbers, " ");
        assert(coordNumbers.size() == 2);
        int clockRegionX = std::stoi(coordNumbers[0]);
        int clockRegionY = std::stoi(coordNumbers[1]);
        if (clockRegionX + 1 > clockRegionNumX)
            clockRegionNumX = clockRegionX + 1;
        if (clockRegionY + 1 > clockRegionNumY)
            clockRegionNumY = clockRegionY + 1;

        std::pair<int, int> clockRegionCoord(clockRegionX, clockRegionY);

        // print("site=> " + curSite.siteName
        //       + " tile=> " + curSite.tileName
        //       + " clockRegionName=> " + curSite.clockRegionName
        //       + " sitetype=> " + curSite.siteType
        //       + " tiletype=> " + curSite.tileType
        //       + " centerx=> " + str(cx)
        //       + " centery=> " + str(cy)
        //       + " BELs=> " + str(curSite.BELs).replace(" ", "").replace("\'", ""), file=exportfile)

        if (strContains(fill0, "site=>"))
        {
            addTile(tileName, tileType);
            DeviceTile *curTile = name2Tile[tileName];

            addSite(siteName, siteType, centerX, centerY, clockRegionX, clockRegionY, curTile);
            DeviceSite *curSite = name2Site[siteName];
            if (coord2ClockRegion.find(clockRegionCoord) == coord2ClockRegion.end())
            {
                ClockRegion *newCR = new ClockRegion(curSite);
                coord2ClockRegion[clockRegionCoord] = newCR;
            }
            else
            {
                coord2ClockRegion[clockRegionCoord]->addSite(curSite);
            }
            strBELs = strBELs.substr(1, strBELs.size() - 2); // remove [] in string
            std::vector<std::string> BELnames;
            strSplit(strBELs, BELnames, ",");

            for (std::string BELname : BELnames)
            {
                std::vector<std::string> splitedName;
                strSplit(BELname, splitedName, "/");
                addBEL(BELname, splitedName[1], curSite);
            }
        }
        else
            assert(false && "Parser Error");
    }

    std::sort(sites.begin(), sites.end(), siteSortCmp);

    std::map<std::string, std::vector<DeviceSite *>>::iterator tmpIt;
    for (tmpIt = siteType2Sites.begin(); tmpIt != siteType2Sites.end(); tmpIt++)
    {
        std::sort(tmpIt->second.begin(), tmpIt->second.end(), siteSortCmp);
    }

    loadPCIEPinOffset(specialPinOffsetFileName);

    print_info("There are " + std::to_string(clockRegionNumY) + "x" + std::to_string(clockRegionNumX) +
               "(YxX) clock regions on the device");

    mapClockRegionToArray();
    print_status("New Device Info Created.");
}

void DeviceInfo::mapClockRegionToArray()
{
    clockRegions.clear();
    clockRegions.resize(clockRegionNumY, std::vector<ClockRegion *>(clockRegionNumX, nullptr));
    for (int i = 0; i < clockRegionNumY; i++)
    {
        for (int j = 0; j < clockRegionNumX; j++)
        {
            std::pair<int, int> clockRegionCoord(j, i);
            assert(coord2ClockRegion.find(clockRegionCoord) != coord2ClockRegion.end());
            clockRegions[i][j] = coord2ClockRegion[clockRegionCoord];
        }
    }
    for (int i = 0; i < clockRegionNumY; i++)
    {
        int j = 0;
        clockRegions[i][j]->setLeft(clockRegions[i][j]->getLeft() - boundaryTolerance);
    }
    for (int i = 0; i < clockRegionNumY; i++)
    {
        int j = clockRegionNumX - 1;
        clockRegions[i][j]->setRight(clockRegions[i][j]->getRight() + boundaryTolerance);
    }
    for (int j = 0; j < clockRegionNumX; j++)
    {
        int i = 0;
        clockRegions[i][j]->setBottom(clockRegions[i][j]->getBottom() - boundaryTolerance);
    }
    for (int j = 0; j < clockRegionNumX; j++)
    {
        int i = clockRegionNumY - 1;
        clockRegions[i][j]->setTop(clockRegions[i][j]->getTop() + boundaryTolerance);
    }

    for (int i = 0; i < clockRegionNumY; i++)
    {
        for (int j = 0; j < clockRegionNumX; j++)
        {
            if (j > 0)
            {
                float midX = (clockRegions[i][j - 1]->getRight() + clockRegions[i][j]->getLeft()) / 2;
                clockRegions[i][j]->setLeft(midX);
                clockRegions[i][j - 1]->setRight(midX);
            }
            if (i > 0)
            {
                float midY = (clockRegions[i - 1][j]->getTop() + clockRegions[i][j]->getBottom()) / 2;
                clockRegions[i - 1][j]->setTop(midY);
                clockRegions[i][j]->setBottom(midY);
            }
        }
    }

    clockRegionXBounds.clear();
    clockRegionYBounds.clear();
    for (int j = 0; j < clockRegionNumX; j++)
    {
        clockRegionXBounds.push_back(clockRegions[0][j]->getLeft());
    }
    for (int i = 0; i < clockRegionNumY; i++)
    {
        clockRegionYBounds.push_back(clockRegions[i][0]->getBottom());
    }

    for (int i = 0; i < clockRegionNumY; i++)
    {
        for (int j = 0; j < clockRegionNumX; j++)
        {
            assert(clockRegions[i][j]->getLeft() == clockRegionXBounds[j]);
            assert(clockRegions[i][j]->getBottom() == clockRegionYBounds[i]);
            // std::cout << "dealing with clock region : X " << j << " Y " << i << "\n";
            clockRegions[i][j]->mapSiteToClockColumns();
        }
    }

    clockColumns.clear();
    for (int i = 0; i < clockRegionNumY; i++)
    {
        for (int j = 0; j < clockRegionNumX; j++)
        {
            for (auto colRow : clockRegions[i][j]->getClockColumns())
            {
                for (auto curClockCol : colRow)
                {
                    curClockCol->setId(clockColumns.size());
                    clockColumns.push_back(curClockCol);
                }
            }
        }
    }
}

void DeviceInfo::ClockRegion::mapSiteToClockColumns()
{
    assert(sites.size() > 0);
    std::sort(sites.begin(), sites.end(), [](DeviceSite *a, DeviceSite *b) -> bool {
        return a->getSiteY() == b->getSiteY() ? (a->getSiteX() < b->getSiteX()) : a->getSiteY() < b->getSiteY();
    });

    assert(sites.size() > 0);
    for (unsigned int i = 0; i < sites.size(); i++)
    {
        if (sites[i]->getName().find("SLICE") == 0)
        {
            leftSiteX = sites[i]->getSiteX();
            bottomSiteY = sites[i]->getSiteY();
            break;
        }
    }
    for (int i = sites.size() - 1; i >= 0; i--)
    {
        if (sites[i]->getName().find("SLICE") == 0)
        {
            rightSiteX = sites[i]->getSiteX();
            topSiteY = sites[i]->getSiteY();
            break;
        }
    }

    for (unsigned int i = 0; i < sites.size(); i++)
    {
        if (sites[i]->getTile()->getTileIdX() < leftTileIdX)
        {
            leftTileIdX = sites[i]->getTile()->getTileIdX();
        }
        if (sites[i]->getTile()->getTileIdX() > rightTileIdX)
        {
            rightTileIdX = sites[i]->getTile()->getTileIdX();
        }
        if (sites[i]->getName().find("SLICE") == 0)
        {
            if (sites[i]->getTile()->getTileIdY() < bottomTileIdY)
            {
                bottomTileIdY = sites[i]->getTile()->getTileIdY();
            }
            if (sites[i]->getTile()->getTileIdY() > topTileIdY)
            {
                topTileIdY = sites[i]->getTile()->getTileIdY();
            }
        }
    }

    // std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
    clockColumns = std::vector<std::vector<ClockColumn *>>(
        columnNumY, std::vector<ClockColumn *>((rightTileIdX - leftTileIdX + 1), nullptr));
    for (int levelY = 0; levelY < columnNumY; levelY++)
    {
        for (unsigned int colOffset = 0; colOffset < clockColumns[levelY].size(); colOffset++)
        {
            clockColumns[levelY][colOffset] = new ClockColumn();
        }
    }
    int eachLevelY = (topTileIdY - bottomTileIdY + 1) / columnNumY;
    for (auto curSite : sites)
    {
        if (curSite->getName().find("SLICE") != 0)
        {
            // currently, BRAM/DSP slices will not lead to clock utilization overflow
            continue;
        }
        int levelY = (curSite->getSiteY() - bottomSiteY) / eachLevelY;
        int offsetX = curSite->getTile()->getTileIdX() - leftTileIdX;
        if (levelY < 0)
        {
            std::cout << curSite->getName() << "\n";
        }
        assert(levelY >= 0);
        if (levelY >= (int)clockColumns.size())
        {
            std::cout << curSite->getName() << "\n";
        }
        assert(levelY < (int)clockColumns.size());
        assert(offsetX >= 0);
        assert(offsetX < (int)clockColumns[levelY].size());
        clockColumns[levelY][offsetX]->addSite(curSite);
        curSite->setClockHalfColumn(clockColumns[levelY][offsetX]);
    }

    assert(clockColumns.size() > 0);
    colHeight = (topY - bottomY) / columnNumY;
    colWidth = (rightX - leftX) / clockColumns[0].size();

    float offsetY = bottomY;
    // std::cout << "ClockRegionBoundary:  l:" << leftX << " r:" << rightX << " b:" << bottomY << " t:" << topY << "\n";
    for (int levelY = 0; levelY < columnNumY; levelY++)
    {
        for (unsigned int colOffset = 0; colOffset < clockColumns[levelY].size(); colOffset++)
        {
            if (clockColumns[levelY][colOffset]->getSites().size() == 0)
                continue;
            clockColumns[levelY][colOffset]->setBottom(bottomY + levelY * colHeight);
            clockColumns[levelY][colOffset]->setTop(bottomY + (levelY + 1) * colHeight);
            if (colOffset == 0)
            {
                clockColumns[levelY][colOffset]->setLeft(leftX);
            }
            else
            {
                clockColumns[levelY][colOffset]->setLeft(clockColumns[levelY][colOffset]->getLeft() - 0.25);
            }
            if (colOffset == clockColumns[levelY].size() - 1)
            {
                clockColumns[levelY][colOffset]->setRight(rightX);
            }
            else
            {
                clockColumns[levelY][colOffset]->setRight(clockColumns[levelY][colOffset]->getRight() + 0.25);
            }
            // std::cout << "   l:" << clockColumns[levelY][colOffset]->getLeft()
            //           << " r:" << clockColumns[levelY][colOffset]->getRight()
            //           << " b:" << clockColumns[levelY][colOffset]->getBottom()
            //           << " t:" << clockColumns[levelY][colOffset]->getTop() << "\n";
        }
    }
}

void DeviceInfo::recordClockRelatedCell(float locX, float locY, int regionX, int regionY, int cellId, int netId)
{
    auto clockRegion = clockRegions[regionY][regionX];
    clockRegion->addClockAndCell(netId, cellId, locX, locY);
}

void DeviceInfo::loadPCIEPinOffset(std::string specialPinOffsetFileName)
{

    std::ifstream infile(specialPinOffsetFileName.c_str());
    assert(infile.good() &&
           "The file for special pin offset information does not exist and please check your path settings");

    std::string line;
    std::string refpinname, fill0, fill1, fill2;
    float offsetX, offsetY;

    std::vector<std::string> refPinsName;
    std::map<std::string, float> name2offsetX;
    std::map<std::string, float> name2offsetY;
    refPinsName.clear();
    name2offsetY.clear();
    name2offsetX.clear();

    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> fill0 >> refpinname >> fill1 >> offsetX >> fill2 >> offsetY;
        if (strContains(fill0, "pin=>"))
        {
            refPinsName.push_back(refpinname);
            name2offsetX[refpinname] = offsetX;
            name2offsetY[refpinname] = offsetY;
        }
        else
            assert(false && "Parser Error");
    }

    std::string STR_PCIE_3_1 = "PCIE_3_1";
    for (DeviceSite *PCIESite : getSitesInType(STR_PCIE_3_1))
    {
        DeviceSite::DeviceSitePinInfos *sitePins = new DeviceSite::DeviceSitePinInfos();
        sitePins->refPinsName = refPinsName;
        sitePins->name2offsetX = name2offsetX;
        sitePins->name2offsetY = name2offsetY;
        PCIESite->setSitePinInfos(sitePins);
    }
}

void DeviceInfo::DeviceBEL::setSite(DeviceElement *parentPtr)
{
    DeviceSite *sitePtr = dynamic_cast<DeviceSite *>(parentPtr);
    assert(sitePtr);
    site = sitePtr;
}

void DeviceInfo::DeviceSite::setTile(DeviceElement *parentTilePtr)
{
    DeviceTile *tilePtr = dynamic_cast<DeviceTile *>(parentTilePtr);
    assert(tilePtr);
    parentTile = tilePtr;
}

void DeviceInfo::DeviceSite::setParentSite(DeviceElement *parentSitePtr)
{
}

void DeviceInfo::DeviceSite::addChildSite(DeviceElement *sitePtr)
{
}

void DeviceInfo::DeviceTile::addChildSite(DeviceInfo::DeviceElement *sitePtr)
{
    DeviceSite *childSite = dynamic_cast<DeviceSite *>(sitePtr);
    assert(childSite);
    childrenSites.push_back(childSite);
}

void DeviceInfo::printStat(bool verbose)
{
    print_info("#ExtractedTile= " + std::to_string(tiles.size()));
    print_info("#ExtractedSite= " + std::to_string(sites.size()));
    print_info("#ExtractedBEL= " + std::to_string(BELs.size()));

    std::string existTypes = "";
    for (std::string tmptype : tileTypes)
    {
        existTypes += tmptype + " ";
    }
    print_info("Tile(" + std::to_string(tileTypes.size()) + " types): " + existTypes);
    if (verbose)
    {
        for (std::map<std::string, std::vector<DeviceTile *>>::iterator it = tileType2Tiles.begin();
             it != tileType2Tiles.end(); ++it)
        {
            print_info("#" + it->first + ": " + std::to_string(it->second.size()));
        }
    }

    existTypes = "";
    for (std::string tmptype : siteTypes)
    {
        existTypes += tmptype + " ";
    }
    print_info("Site(" + std::to_string(siteTypes.size()) + " types): " + existTypes);
    if (verbose)
    {
        for (std::map<std::string, std::vector<DeviceSite *>>::iterator it = siteType2Sites.begin();
             it != siteType2Sites.end(); ++it)
        {
            print_info("#" + it->first + ": " + std::to_string(it->second.size()));
        }
    }

    existTypes = "";
    for (std::string tmptype : BELTypes)
    {
        existTypes += tmptype + " ";
    }
    print_info("BEL(" + std::to_string(BELTypes.size()) + " types): " + existTypes);
    if (verbose)
    {
        for (std::map<std::string, std::vector<DeviceBEL *>>::iterator it = BELType2BELs.begin();
             it != BELType2BELs.end(); ++it)
        {
            print_info("#" + it->first + ": " + std::to_string(it->second.size()));
        }
    }
}

void DeviceInfo::addBEL(std::string &BELName, std::string &BELType, DeviceSite *parent)
{
    assert(name2BEL.find(BELName) == name2BEL.end());

    DeviceBEL *newBEL = new DeviceBEL(BELName, BELType, parent, parent->getChildrenSites().size());
    BELs.push_back(newBEL);
    name2BEL[BELName] = newBEL;

    addBELTypes(newBEL->getBELType());

    if (BELType2BELs.find(newBEL->getBELType()) == BELType2BELs.end())
        BELType2BELs[newBEL->getBELType()] = std::vector<DeviceBEL *>();
    BELType2BELs[newBEL->getBELType()].push_back(newBEL);

    parent->addChildBEL(newBEL);
}

void DeviceInfo::addSite(std::string &siteName, std::string &siteType, float locx, float locy, int clockRegionX,
                         int clockRegionY, DeviceTile *parentTile)
{
    assert(name2Site.find(siteName) == name2Site.end());

    DeviceSite *newSite = new DeviceSite(siteName, siteType, parentTile, locx, locy, clockRegionX, clockRegionY,
                                         parentTile->getChildrenSites().size());
    sites.push_back(newSite);
    name2Site[siteName] = newSite;

    addSiteTypes(newSite->getSiteType());

    if (siteType2Sites.find(newSite->getSiteType()) == siteType2Sites.end())
        siteType2Sites[newSite->getSiteType()] = std::vector<DeviceSite *>();
    siteType2Sites[newSite->getSiteType()].push_back(newSite);

    parentTile->addChildSite(newSite);
}

void DeviceInfo::addTile(std::string &tileName, std::string &tileType)
{
    if (name2Tile.find(tileName) == name2Tile.end())
    {
        DeviceTile *tile = new DeviceTile(tileName, tileType, this, tiles.size());
        tiles.push_back(tile);
        name2Tile[tileName] = tile;

        addTileTypes(tile->getTileType());

        if (tileType2Tiles.find(tile->getTileType()) == tileType2Tiles.end())
            tileType2Tiles[tile->getTileType()] = std::vector<DeviceTile *>();
        tileType2Tiles[tile->getTileType()].push_back(tile);
    }
}

void DeviceInfo::loadBELType2FalseBELType(std::string curFileName)
{
    std::string line;
    std::ifstream infile(curFileName.c_str());
    assert(infile.good() &&
           "The mergedSharedCellType2sharedCellType file does not exist and please check your path settings");
    BELType2FalseBELType.clear();
    while (std::getline(infile, line))
    {
        std::vector<std::string> BELTypePair;
        strSplit(line, BELTypePair, " ");
        BELType2FalseBELType[BELTypePair[0]] = BELTypePair[1];
        print_info("mapping BELType [" + BELTypePair[0] + "] to [" + BELType2FalseBELType[BELTypePair[0]] +
                   "] when creating PlacementBins");
    }
}
//#if CMAKE_PYBIND11_TYPE
//PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
//{
//   pybind11::class_<DeviceInfo>(m, "DeviceInfo")
//       .def(py::init<std::map<std::string, std::string>&, std::string>())
//        .def("printStat", &DeviceInfo::printStat, pybind11::arg("verbose") = false);
//  // FFSRCompatible
//}
//#endif