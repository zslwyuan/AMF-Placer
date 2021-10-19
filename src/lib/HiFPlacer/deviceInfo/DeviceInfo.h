/**
 * @file DeviceInfo.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the classes of data for a standalone device.
 * @version 0.1
 * @date 2021-06-03
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _DeviceINFO
#define _DeviceINFO

#include "strPrint.h"
#include <assert.h>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Information class related to FPGA device, including the details of BEL/Site/Tile/ClockRegion.
 *
 */
class DeviceInfo
{
  public:
    /**
     * @brief types of the elements in a device
     *
     */
    enum DeviceElementType
    {
        DeviceElementType_BEL = 0,
        DeviceElementType_Site,
        DeviceElementType_Tile,
        DeviceElementType_CLOCKREGION,
        DeviceElementType_Device
    };

    /**
     * @brief basic class of device element
     *
     */
    class DeviceElement
    {
      public:
        DeviceElement(std::string &name, DeviceElement *parentPtr, DeviceElementType type, int id)
            : name(name), parentPtr(parentPtr), type(type), id(id)
        {
        }

        DeviceElement(std::string &name, DeviceElementType type, int id)
            : name(name), parentPtr(nullptr), type(type), id(id)
        {
        }

        virtual ~DeviceElement()
        {
        }

        inline std::string &getName()
        {
            return name;
        }
        inline DeviceElement *getParentPtr()
        {
            return parentPtr;
        }
        inline DeviceElementType getElementType()
        {
            return type;
        }
        inline int getElementIdInParent()
        {
            return id;
        }

      private:
        std::string name;
        DeviceElement *parentPtr;
        DeviceElementType type;
        int id;
    };

    class DeviceBEL;
    class DeviceSite;
    class DeviceTile;
    class ClockRegion;
    class ClockColumn;

    /**
     * @brief BEL(Basic Element of Logic), the smallest undividable element
     *
     */
    class DeviceBEL : public DeviceElement
    {
      public:
        /**
         * @brief Construct a new Device BEL object
         *
         * @param name the name of the BEL in the device
         * @param BELType a string to indicate the type of element, determined by device input file
         * @param parentPtr parent site of the BEL. A site is the combination of a group of BELs
         * @param id
         */
        DeviceBEL(std::string &name, std::string &BELType, DeviceElement *parentPtr, int id)
            : DeviceElement(name, parentPtr, DeviceElementType_BEL, id), BELType(BELType)
        {
            setSite(parentPtr);
        }

        ~DeviceBEL()
        {
        }

        void setSite(DeviceElement *parentPtr);
        inline DeviceSite *getSite()
        {
            return site;
        };
        inline std::string &getBELType()
        {
            return BELType;
        }

      private:
        /**
         * @brief BEL Type recorded by a string (for flexibility)
         *
         * TODO: replace this way by integer type id
         *
         */
        std::string BELType;
        DeviceSite *site = nullptr;
        // TODO: also record the pins on BEL for later routing.
    };

    /**
     * @brief Site class for site on device
     *
     *  A site contains a combination of BEL. This class record the name/type/location/clockRegion of the site
     *
     */
    class DeviceSite : public DeviceElement
    {
      public:
        /**
         * @brief Construct a new Device Site object
         *
         * @param name the name of the site
         * @param siteType a string for the type of the site
         * @param parentPtr the tile that contains this site
         * @param locX
         * @param locY
         * @param clockRegionX
         * @param clockRegionY
         * @param id
         */
        DeviceSite(std::string &name, std::string &siteType, DeviceElement *parentPtr, float locX, float locY,
                   int clockRegionX, int clockRegionY, int id)
            : DeviceElement(name, parentPtr, DeviceElementType_Site, id), siteType(siteType), locX(locX), locY(locY),
              clockRegionX(clockRegionX), clockRegionY(clockRegionY)
        {
            setTile(parentPtr);

            // extract the integer XY from the siteName from later processing
            // WARNING: In the site name, these XY values are not actual LOCATION!!!
            if (name.find('_') != std::string::npos)
            {
                std::string tmpName = name.substr(name.rfind('_') + 1, name.size() - name.rfind('_') - 1);
                if (tmpName.find('Y') != std::string::npos)
                {
                    siteY = std::stoi(tmpName.substr(tmpName.find('Y') + 1, tmpName.size() - tmpName.find('Y') - 1));
                    siteX = std::stoi(tmpName.substr(tmpName.find('X') + 1, tmpName.find('Y') - tmpName.find('X') - 1));
                }
                else
                {
                    siteX = -1;
                    siteY = -1;
                }
            }
            else
            {
                siteX = -1;
                siteY = -1;
            }
        }

        ~DeviceSite()
        {
            if (sitePins)
                delete sitePins;
        }

        /**
         * @brief a class record the pin on the site boundary
         *
         */
        class DeviceSitePinInfos
        {
          public:
            DeviceSitePinInfos()
            {
                refPinsName.clear();
                name2offsetY.clear();
                name2offsetX.clear();
            }
            ~DeviceSitePinInfos()
            {
            }
            std::vector<std::string> refPinsName;
            std::map<std::string, float> name2offsetX;
            std::map<std::string, float> name2offsetY;
        };

        void setTile(DeviceElement *parentTilePtr);
        inline DeviceTile *getTile()
        {
            return tile;
        };

        /**
         * @brief Set the Parent Site object
         *
         * it seems that some sites on FPGA might contain children site?
         *
         * @param parentSitePtr
         */
        void setParentSite(DeviceElement *parentSitePtr);
        inline DeviceSite *getParentSite()
        {
            return parentSite;
        };

        /**
         * @brief Set the Parent Site object
         *
         * it seems that some sites on FPGA might contain children site?
         *
         * @param parentSitePtr
         */
        void addChildSite(DeviceElement *parentSitePtr);
        inline std::vector<DeviceSite *> &getChildrenSites()
        {
            return childrenSites;
        };

        inline void setSiteLocation(float x, float y)
        {
            locX = x;
            locY = y;
        };

        inline std::string &getSiteType()
        {
            return siteType;
        };

        inline void addChildBEL(DeviceBEL *child)
        {
            childrenBELs.push_back(child);
        };

        inline std::vector<DeviceBEL *> &getChildrenBELs()
        {
            return childrenBELs;
        };

        inline float X()
        {
            return locX;
        }
        inline float Y()
        {
            return locY;
        }

        inline DeviceSitePinInfos *getSitePinInfos()
        {
            return sitePins;
        }
        inline void setSitePinInfos(DeviceSitePinInfos *_sitePins)
        {
            sitePins = _sitePins;
        }

        /**
         * @brief check whether this site is occupied by fixed element
         *
         * @return true
         * @return false
         */
        inline bool isOccupied()
        {
            return occupied;
        }
        inline void setOccupied()
        {
            occupied = true;
        }

        /**
         * @brief check whether this site is mapped to an design element (might be movable elements)
         *
         * @return true
         * @return false
         */
        inline bool isMapped()
        {
            return mapped;
        }
        inline void setMapped()
        {
            mapped = true;
        }
        inline void resetMapped()
        {
            mapped = false;
        }

        inline int getSiteY()
        {
            assert(siteY >= 0);
            return siteY;
        }

        inline int getSiteX()
        {
            assert(siteX >= 0);
            return siteX;
        }

        inline int getClockRegionX()
        {
            return clockRegionX;
        }

        inline int getClockRegionY()
        {
            return clockRegionY;
        }

        inline void setClockRegion(ClockRegion *_clockRegion)
        {
            clockRegion = _clockRegion;
        }

        inline void setClockHalfColumn(ClockColumn *_clockHalfColumn)
        {
            clockHalfColumn = _clockHalfColumn;
        }

      private:
        std::string siteType;
        DeviceTile *tile;
        ClockRegion *clockRegion = nullptr;
        ClockColumn *clockHalfColumn = nullptr;
        std::vector<DeviceBEL *> childrenBELs;
        DeviceSite *parentSite; // mainly to handle overlapped sites
        std::vector<DeviceSite *> childrenSites;
        DeviceTile *parentTile = nullptr;
        DeviceSitePinInfos *sitePins = nullptr;
        float locX;
        float locY;
        int clockRegionX = -1;
        int clockRegionY = -1;
        int siteY;
        int siteX;

        /**
         * @brief a flag that this site is occupied by fixed element
         *
         */
        bool occupied = false;

        /**
         * @brief a flag that this site is mapped to an design element (might be movable elements)
         *
         */
        bool mapped = false;
    };

    /**
     * @brief A tile is a combination of sites
     *
     */
    class DeviceTile : public DeviceElement
    {
      public:
        /**
         * @brief Construct a new Device Tile object
         *
         * @param name the tile name in the input file
         * @param tileType a string indicating the type of the tile
         * @param device
         * @param id
         */
        DeviceTile(std::string &name, std::string &tileType, DeviceInfo *device, int id)
            : DeviceElement(name, DeviceElementType_Tile, id), tileType(tileType), device(device)
        {
        }

        ~DeviceTile()
        {
        }

        void addChildSite(DeviceElement *sitePtr);
        inline std::vector<DeviceSite *> &getChildrenSites()
        {
            return childrenSites;
        };
        inline std::string &getTileType()
        {
            return tileType;
        }

      private:
        std::string tileType;
        std::vector<DeviceSite *> childrenSites;
        DeviceInfo *device;
    };

    /**
     * @brief a column of site in clock region
     *
     * clock region contains an array of clock regions
     *
     */
    class ClockColumn
    {
      public:
        ClockColumn()
        {
            sites.clear();
            clockNetId2Cnt.clear();
            clockNetId2CellIds.clear();
            clockNetId2Sites.clear();
        };
        ~ClockColumn()
        {
            sites.clear();
            clockNetId2Cnt.clear();
            clockNetId2CellIds.clear();
            clockNetId2Sites.clear();
        }

        void addSite(DeviceSite *curSite)
        {
            sites.push_back(curSite);
        }

        /**
         * @brief reset the clock net information, including cells in it
         *
         */
        void resetClockInfo()
        {
            clockNetId2Cnt.clear();
            clockNetId2CellIds.clear();
            clockNetId2Sites.clear();
        }

        /**
         * @brief add a cell in a specific clock domain
         *
         * @param clockNetId
         * @param cellId
         */
        inline void addClockNetId(int clockNetId, int cellId)
        {
            if (clockNetId2Cnt.find(clockNetId) == clockNetId2Cnt.end())
            {
                clockNetId2Cnt[clockNetId] = 0;
            }
            clockNetId2Cnt[clockNetId] += 1;
            if (clockNetId2CellIds.find(clockNetId) == clockNetId2CellIds.end())
            {
                clockNetId2CellIds[clockNetId] = std::vector<int>();
            }
            clockNetId2CellIds[clockNetId].push_back(cellId);
        }

        inline int getClockNum()
        {
            return clockNetId2Cnt.size();
        }

        /**
         * @brief Set the Boundary of the clock column
         *
         * @param _left
         * @param _right
         * @param _top
         * @param _bottom
         */
        inline void setBoundary(float _left, float _right, float _top, float _bottom)
        {
            left = _left;
            right = _right;
            top = _top;
            bottom = _bottom;
        }

        inline std::map<int, std::vector<int>> &getClockNetId2CellIds()
        {
            return clockNetId2CellIds;
        }

      private:
        std::vector<DeviceSite *> sites;
        float left, right, top, bottom;

        /**
         * @brief counter for the elements for each clock in the clock column
         *
         */
        std::map<int, int> clockNetId2Cnt;

        /**
         * @brief the elements for each clock in the clock column
         *
         */
        std::map<int, std::vector<int>> clockNetId2CellIds;
        std::map<int, DeviceSite *> clockNetId2Sites;
    };

    /**
     * @brief class for clock regions on FPGA
     *
     * Clock region is a concept in the FPGA clocking to constrain the number of clocks in a specific region
     *
     */
    class ClockRegion
    {
      public:
        /**
         * @brief Construct a new Clock Region object
         *
         * @param curSite a site in the clock region for some variables' initialization
         */
        ClockRegion(DeviceSite *curSite)
        {
            assert(curSite);
            sites.clear();
            clockColumns.clear();
            sites.push_back(curSite);
            gridX = curSite->getClockRegionX();
            gridY = curSite->getClockRegionY();
            curSite->setClockRegion(this);
            leftX = curSite->X();
            rightX = curSite->X();
            topY = curSite->Y();
            bottomY = curSite->Y();
        }

        ~ClockRegion()
        {
            sites.clear();
            for (auto &row : clockColumns)
            {
                for (auto clockColumn : row)
                {
                    if (clockColumn)
                        delete clockColumn;
                }
            }
            clockColumns.clear();
        }

        /**
         * @brief add a site in the clock region and update the boundary of the clock region
         *
         * @param tmpSite
         */
        inline void addSite(DeviceSite *tmpSite)
        {
            assert(tmpSite);
            tmpSite->setClockRegion(this);
            if (tmpSite->X() < leftX)
                leftX = tmpSite->X();
            if (tmpSite->X() > rightX)
                rightX = tmpSite->X();
            if (tmpSite->Y() > topY)
                topY = tmpSite->Y();
            if (tmpSite->Y() < bottomY)
                bottomY = tmpSite->Y();
            sites.push_back(tmpSite);
        }

        inline void setLeft(float x)
        {
            leftX = x;
        }

        inline void setRight(float x)
        {
            rightX = x;
        }

        inline void setTop(float y)
        {
            topY = y;
        }

        inline void setBottom(float y)
        {
            bottomY = y;
        }

        /**
         * @brief Get the Left boundary of the clock region
         *
         * @return float
         */
        inline float getLeft()
        {
            return leftX;
        }

        /**
         * @brief Get the Right boundary of the clock region
         *
         * @return float
         */
        inline float getRight()
        {
            return rightX;
        }

        /**
         * @brief Get the Top boundary of the clock region
         *
         * @return float
         */
        inline float getTop()
        {
            return topY;
        }

        /**
         * @brief Get the Bottom boundary of the clock region
         *
         * @return float
         */
        inline float getBottom()
        {
            return bottomY;
        }

        /**
         * @brief for the clock column constraints, we need to map site to columns.
         *
         */
        void mapSiteToClockColumns();

        /**
         * @brief reset the clock utilization for each column in the clock region
         *
         */
        void resetClockUtilizationInfo()
        {
            for (auto &row : clockColumns)
            {
                for (auto clockColumn : row)
                {
                    clockColumn->resetClockInfo();
                }
            }
        }

        /**
         * @brief add a cell into the clock region
         *
         * @param clockNetId the driver clock net (id) of the cell
         * @param cellId
         * @param x cell location X
         * @param y cell location Y
         */
        inline void addClockAndCell(int clockNetId, int cellId, float x, float y)
        {
            float offsetXInRegion = x - leftX;
            float offsetYInRegion = y - bottomY;

            // this approach is not accurate enought since the slice columns are not evenly
            // distributed in a clock region. However, it is fast for frequently checking of
            // clock utilization
            int colX = offsetXInRegion / colWidth;
            int colY = offsetYInRegion / colHeight;
            assert(colX >= -1);
            assert(colY >= -1);
            assert(clockColumns.size());
            assert(colY <= (int)clockColumns.size());
            assert(colX <= (int)clockColumns[0].size());
            if (colX < 0)
                colX = 0;
            if (colY < 0)
                colY = 0;
            if (colX >= (int)clockColumns[0].size())
            {
                colX = clockColumns[0].size() - 1;
            }
            if (colY >= (int)clockColumns.size())
            {
                colY = clockColumns.size() - 1;
            }
            clockColumns[colY][colX]->addClockNetId(clockNetId, cellId);
        }

        /**
         * @brief Get the Max Utilization Of Clock Columns
         *
         * @return int
         */
        inline int getMaxUtilizationOfClockColumns()
        {
            int res = 0;
            for (auto &row : clockColumns)
            {
                for (auto clockColumn : row)
                {
                    if (clockColumn)
                    {
                        if (clockColumn->getClockNum() > res)
                        {
                            res = clockColumn->getClockNum();
                        }
                    }
                }
            }
            return res;
        }

        inline std::vector<std::vector<ClockColumn *>> &getClockColumns()
        {
            return clockColumns;
        }

      private:
        std::vector<DeviceSite *> sites;
        std::vector<std::vector<ClockColumn *>> clockColumns;
        int columnNumY = 2;
        int gridX, gridY;
        float leftX, rightX, topY, bottomY;
        int leftSiteX, bottomSiteY, rightSiteX, topSiteY;
        float colHeight;
        float colWidth;
    };

    // clang-format off
    /**
     * @brief Construct a new Device Info object
     *
     * @param JSONCfg user configuration JSON mapping
     * @param _deviceName device file name
     * 
     * The device information file will contain information like the text shown below
     ~~~~{.tcl}
     site=> SLICE_X59Y220 tile=> CLEL_R_X36Y220 sitetype=> SLICEL tiletype=> CLE_R centerx=> 37.25 centery=> 220.15
     BELs=> [SLICE_X59Y220/A5LUT,SLICE_X59Y220/A6LUT,SLICE_X59Y220/AFF,SLICE_X59Y220/AFF2,SLICE_X59Y220/B5LUT,SLICE_X59Y220/B6LUT,SLICE_X59Y220/BFF,SLICE_X59Y220/BFF2,SLICE_X59Y220/C5LUT,SLICE_X59Y220/C6LUT,SLICE_X59Y220/CARRY8,SLICE_X59Y220/CFF,SLICE_X59Y220/CFF2,SLICE_X59Y220/D5LUT,SLICE_X59Y220/D6LUT,SLICE_X59Y220/DFF,SLICE_X59Y220/DFF2,SLICE_X59Y220/E5LUT,SLICE_X59Y220/E6LUT,SLICE_X59Y220/EFF,SLICE_X59Y220/EFF2,SLICE_X59Y220/F5LUT,SLICE_X59Y220/F6LUT,SLICE_X59Y220/F7MUX_AB,SLICE_X59Y220/F7MUX_CD,SLICE_X59Y220/F7MUX_EF,SLICE_X59Y220/F7MUX_GH,SLICE_X59Y220/F8MUX_BOT,SLICE_X59Y220/F8MUX_TOP,SLICE_X59Y220/F9MUX,SLICE_X59Y220/FFF,SLICE_X59Y220/FFF2,SLICE_X59Y220/G5LUT,SLICE_X59Y220/G6LUT,SLICE_X59Y220/GFF,SLICE_X59Y220/GFF2,SLICE_X59Y220/H5LUT,SLICE_X59Y220/H6LUT,SLICE_X59Y220/HFF,SLICE_X59Y220/HFF2]
     
     site=> RAMB18_X7Y88 tile=> BRAM_X36Y220 sitetype=> RAMBFIFO18 tiletype=> CLE_R centerx=> 36.75 centery=> 221.96249999999998 
     BELs=> [RAMB18_X7Y88/RAMBFIFO18] site=> RAMB18_X7Y89 tile=> BRAM_X36Y220 sitetype=> RAMB181

     tiletype=> CLE_R centerx=> 36.75 centery=> 224.11249999999998 BELs=> [RAMB18_X7Y89/RAMB18E2_U]
    ~~~~
     */
    // clang-format on
    DeviceInfo(std::map<std::string, std::string> &JSONCfg, std::string _deviceName);
    ~DeviceInfo()
    {
        for (auto bel : BELs)
            delete bel;
        for (auto site : sites)
            delete site;
        for (auto tile : tiles)
            delete tile;
    }

    void printStat(bool verbose = false);

    /**
     * @brief add a BEL type into the set of the existing BEL types
     *
     * @param strBELType
     */
    inline void addBELTypes(std::string &strBELType)
    {
        BELTypes.insert(strBELType);
    }

    /**
     * @brief add a BEL element into the device information
     *
     * @param BELName
     * @param BELType
     * @param parentSite
     */
    void addBEL(std::string &BELName, std::string &BELType, DeviceSite *parentSite);

    /**
     * @brief add a Site type into the set of the existing site types
     *
     * @param strSiteType
     */
    inline void addSiteTypes(std::string &strSiteType)
    {
        siteTypes.insert(strSiteType);
    }

    /**
     * @brief add a site into the device information class
     *
     * @param siteName site name string
     * @param siteType site type string
     * @param locx the location (X) of the site on the device
     * @param locy the location (Y) of the site on the device
     * @param clockRegionX the clock region ID X of the site
     * @param clockRegionY the clock region ID Y of the site
     * @param parentTile parent tile of this site
     */
    void addSite(std::string &siteName, std::string &siteType, float locx, float locy, int clockRegionX,
                 int clockRegionY, DeviceTile *parentTile);

    inline void addTileTypes(std::string &strTileType)
    {
        tileTypes.insert(strTileType);
    }
    void addTile(std::string &tileName, std::string &tileType);

    inline std::string &getDeviceName()
    {
        return deviceName;
    }

    /**
     * @brief get BEL types in the device
     *
     * @return std::set<std::string>&
     */
    inline std::set<std::string> &getBELTypes()
    {
        return BELTypes;
    }

    /**
     * @brief get a BEL based on a given name
     *
     * @param BELName
     * @return DeviceBEL*
     */
    inline DeviceBEL *getBELWithName(std::string &BELName)
    {
        assert(name2BEL.find(BELName) != name2BEL.end());
        return name2BEL[BELName];
    }

    /**
     * @brief get BELs of a specific type
     *
     * @param BELType the target BEL type
     * @return std::vector<DeviceBEL *>&
     */
    inline std::vector<DeviceBEL *> &getBELsInType(std::string &BELType)
    {
        assert(BELType2BELs.find(BELType) != BELType2BELs.end());
        return BELType2BELs[BELType];
    }

    /**
     * @brief Get the Site Types in the device
     *
     * @return std::set<std::string>&
     */
    inline std::set<std::string> &getSiteTypes()
    {
        return siteTypes;
    }

    /**
     * @brief Get a site based on a given name
     *
     * @param siteName
     * @return DeviceSite*
     */
    inline DeviceSite *getSiteWithName(std::string &siteName)
    {
        assert(name2Site.find(siteName) != name2Site.end());
        return name2Site[siteName];
    }

    /**
     * @brief Get sites of a specfic type
     *
     * @param siteType
     * @return std::vector<DeviceSite *>&
     */
    inline std::vector<DeviceSite *> &getSitesInType(std::string &siteType)
    {
        assert(siteType2Sites.find(siteType) != siteType2Sites.end());
        return siteType2Sites[siteType];
    }

    /**
     * @brief Get the tile types in the device
     *
     * @return std::set<std::string>&
     */
    inline std::set<std::string> &getTileTypes()
    {
        return tileTypes;
    }

    /**
     * @brief Get a tile With name
     *
     * @param tileName
     * @return DeviceTile*
     */
    inline DeviceTile *getTileWithName(std::string &tileName)
    {
        assert(name2Tile.find(tileName) != name2Tile.end());
        return name2Tile[tileName];
    }

    /**
     * @brief Get the tiles of a specific type
     *
     * @param tileType
     * @return std::vector<DeviceTile *>&
     */
    inline std::vector<DeviceTile *> &getTilesInType(std::string &tileType)
    {
        assert(tileType2Tiles.find(tileType) != tileType2Tiles.end());
        return tileType2Tiles[tileType];
    }

    /**
     * @brief load the detailed location information of pins on PCIE IO slot
     *
     *  PCIE I/O is a long slot which contains pins at various location so we
     * cannot ignore the pin location on the PCIE BEL!
     *
     * @param specialPinOffsetFileName a file name which records these information
     */
    void loadPCIEPinOffset(std::string specialPinOffsetFileName);

    inline std::vector<DeviceBEL *> &getBELs()
    {
        return BELs;
    }
    inline std::vector<DeviceSite *> &getSites()
    {
        return sites;
    }
    inline std::vector<DeviceTile *> &getTiles()
    {
        return tiles;
    }

    inline DeviceBEL *getBEL(std::string &Name)
    {
        assert(name2BEL.find(Name) != name2BEL.end());
        return name2BEL[Name];
    }
    inline DeviceSite *getSite(std::string &Name)
    {
        assert(name2Site.find(Name) != name2Site.end());
        return name2Site[Name];
    }
    inline DeviceTile *getTile(std::string &Name)
    {
        assert(name2Tile.find(Name) != name2Tile.end());
        return name2Tile[Name];
    }

    inline std::string getBELType2FalseBELType(std::string curBELType)
    {
        if (BELType2FalseBELType.find(curBELType) == BELType2FalseBELType.end())
        {
            return curBELType;
        }
        else
        {
            return BELType2FalseBELType[curBELType];
        }
    }

    /**
     * @brief load BEL type remapping information
     * since some cell can be placed in sites of different sites. For cell spreading, we need to remap some BEL types to
     * a unified BEL types. Belows are some examples:
     *
     * SLICEM_CARRY8 => SLICEL_CARRY8
     *
     * SLICEM_LUT => SLICEL_LUT
     *
     * SLICEM_FF => SLICEL_FF
     *
     * @param curFileName
     */
    void loadBELType2FalseBELType(std::string curFileName);

    /**
     * @brief remove the mapped flags for all sites without fixed elements
     *
     */
    void resetAllSiteMapping()
    {
        for (auto curSite : sites)
        {
            if (!curSite->isOccupied())
            {
                curSite->resetMapped();
            }
        }
    }

    /**
     * @brief map recognized clock regions into an array for later clock utilization evaluation
     *
     */
    void mapClockRegionToArray();

    inline float getBoundaryTolerance()
    {
        return boundaryTolerance;
    }

    /**
     * @brief Get the clock region ID (X/Y) by a given location (X/Y)
     *
     * @param locX
     * @param locY
     * @param clockRegionX
     * @param clockRegionY
     */
    inline void getClockRegionByLocation(float locX, float locY, int &clockRegionX, int &clockRegionY)
    {
        int resX = 0, resY = 0;
        for (int j = 0; j < clockRegionNumX; j++)
        {
            if (clockRegionXBounds[j] < locX)
            {
                resX = j;
            }
            else
            {
                break;
            }
        }
        for (int i = 0; i < clockRegionNumY; i++)
        {
            if (clockRegionYBounds[i] < locY)
            {
                resY = i;
            }
            else
            {
                break;
            }
        }
        clockRegionX = resX;
        clockRegionY = resY;
    }

    /**
     * @brief Get the number of columns of the clock region array
     *
     * @return int
     */
    inline int getClockRegionNumX()
    {
        return clockRegionNumX;
    }

    /**
     * @brief Get the number of rows of the clock region array
     *
     * @return int
     */
    inline int getClockRegionNumY()
    {
        return clockRegionNumY;
    }

    /**
     * @brief record the information of cell in a specific clock region
     *
     * @param locX cell location X
     * @param locY cell location Y
     * @param regionX clock region ID X
     * @param regionY clock region ID Y
     * @param cellId target cell ID
     * @param netId clock net ID
     */
    void recordClockRelatedCell(float locX, float locY, int regionX, int regionY, int cellId, int netId);

    /**
     * @brief Get the maximum utilization among the clock columns in clock region
     *
     * @param regionX clock region ID X
     * @param regionY clock region ID Y
     * @return int
     */
    inline int getMaxUtilizationOfClockColumns_InClockRegion(int regionX, int regionY)
    {
        return clockRegions[regionY][regionX]->getMaxUtilizationOfClockColumns();
    }

    inline std::vector<std::vector<ClockRegion *>> &getClockRegions()
    {
        return clockRegions;
    }

  private:
    std::string deviceName;
    std::set<std::string> BELTypes;
    std::map<std::string, std::vector<DeviceBEL *>> BELType2BELs;
    std::vector<DeviceBEL *> BELs;

    std::set<std::string> siteTypes;
    std::map<std::string, std::vector<DeviceSite *>> siteType2Sites;
    std::vector<DeviceSite *> sites;

    std::set<std::string> tileTypes;
    std::map<std::string, std::vector<DeviceTile *>> tileType2Tiles;
    std::vector<DeviceTile *> tiles;

    std::map<std::string, std::string> BELType2FalseBELType;

    std::map<std::string, DeviceBEL *> name2BEL;
    std::map<std::string, DeviceSite *> name2Site;
    std::map<std::string, DeviceTile *> name2Tile;

    std::map<std::pair<int, int>, ClockRegion *> coord2ClockRegion;
    std::vector<std::vector<ClockRegion *>> clockRegions;
    std::vector<float> clockRegionXBounds;
    std::vector<float> clockRegionYBounds;

    std::map<std::string, std::string> &JSONCfg;
    std::string deviceArchievedTextFileName;
    std::string specialPinOffsetFileName;

    float boundaryTolerance = 0.5;
    int clockRegionNumX = 1;
    int clockRegionNumY = 1;
};

#endif