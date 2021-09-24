/**
 * @file PlacementInfo.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file mainly contains the definition of class PlacementInfo, including information related to FPGA
 * placement (wirelength optimization, cell spreading, legalization, packing)
 * @version 0.1
 * @date 2021-06-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _PlacementINFO
#define _PlacementINFO

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "PlacementTimingInfo.h"
#include "Eigen/Core"
#include "Eigen/SparseCore"
#include "dumpZip.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Information related to FPGA placement (wirelength optimization, cell spreading, legalization, packing)
 *
 * includes design information, device information
 * includes numerical information for:
 * wirelength optimization: Bound2Bound model, HPWL for nets, macros
 * cell spreading: bin grid, instance density, routing congestion, packability
 * legalization: mapping design instance to device site at coarse-grained level
 * packing: mapping fine-grained instances to sites (e.g., CLB site)
 *
 */
class PlacementInfo
{
  public:
    /**
     * @brief Placement Instance Types
     *
     * UnpackedCell: the smallest, indivisible, representable component
     * Macro:  a fixed group of multiple standard cells with constraints of their relative locations
     */
    enum PlacementUnitType
    {
        PlacementUnitType_UnpackedCell = 0,
        PlacementUnitType_Macro
    };

    class PlacementSiteTypeInfo;
    class PlacementBinInfo;
    class CompatiblePlacementTable;
    class PlacementNet;

    /**
     * @brief describes the type mapping from design to device, where a cell can be placed (which BEL in which site)
     *
     * Since different cell types can be mapped to a group of resource BEL types, we handle the mapping in the following
     * way, with a intermediate Shared BEL Type:
     *
     * cell type A  =>     A     =>   BEL type 1
     *
     * cell type B  =>  Shared   =>   BEL type 2
     *
     * cell type C  =>    BEL    =>   BEL type 3
     *
     * cell type D  =>   Type    =>   BEL type 4
     *
     */
    class CompatiblePlacementTable
    {
      public:
        /**
         * @brief Construct a new Compatible Placement Table object
         *
         * @param cellType2fixedAmoFileName a file indicating how many resource BEL slots will be cost for a design cell
         * of specific type
         * @param cellType2sharedCellTypeFileName a file indicating the mapping from design cell types to device
         * resource type groups
         * @param sharedCellType2BELtypeFileName a file indicating the mapping from specific resource types to device
         * BEL resource slots
         * @param designInfo design inforamtion
         * @param deviceInfo device information
         */
        CompatiblePlacementTable(std::string cellType2fixedAmoFileName, std::string cellType2sharedCellTypeFileName,
                                 std::string sharedCellType2BELtypeFileName, DesignInfo *designInfo,
                                 DeviceInfo *deviceInfo);
        ~CompatiblePlacementTable()
        {
        }

        /**
         * @brief the actual BEL types according to the definition file
         *
         */
        std::vector<std::string> realBELTypes;

        /**
         * @brief each actual BEL type gets an integer ID for checking
         *
         */
        std::map<std::string, int> realBELTypeName2ID;

        /**
         * @brief some BEL types (design cells should be mapped to the BEL slot on FPGA device)
         *
         * e.g., some BEL types are mapped to the same resource slots on FPGA device, such as SLICEL_LUT,
         * SLICEM_LUT,SLICEL_FF,SLICEM_FF,RAMB18E2_L,RAMB18E2_U,SLICEL_MUXF8,SLICEM_MUXF8...
         * We call them SharedBELType
         *
         */
        std::vector<std::string> sharedCellBELTypes;

        /**
         * @brief the resource demand of specific types. (cost how many slot/BELs)
         *
         * the resource demand of specific types. (cost how many slot/BELs), e.g., LUT6 will take 2
         * SLICEM_LUT slots in a CLB site.
         *
         */
        std::map<DesignInfo::DesignCellType, int> cellType2sharedBELTypeOccupation;

        /**
         * @brief the mapping from design cell type to device resource type
         *
         * the mapping from design cell type to device resource type (we call SharedBELType). Such as LUT1-6 should be
         * mapped to SLICEM_LUT/SLICEL_LUT.
         *
         */
        std::map<DesignInfo::DesignCellType, std::vector<std::string>> cellType2sharedBELTypes;

        /**
         * @brief the mapping from design cell type to device resource type ID
         *
         * string overhead is high. We use integer id to represent the BEL types.
         *
         */
        std::map<DesignInfo::DesignCellType, std::vector<int>> cellType2sharedBELTypeIDs;

        /**
         * @brief The mapping from shared cell types to device site types
         *
         *  For example, SLICEM_LUT should be mapped to SLICEM.
         *
         */
        std::map<std::string, std::string> sharedCellType2SiteType;

        /**
         * @brief The mapping from shared cell types to device BEL types
         *
         *  For example, SLICEM_LUT should be mapped to
         * A5LUT,A6LUT,B5LUT,B6LUT,C5LUT,C6LUT,D5LUT,D6LUT,E5LUT,E6LUT,F5LUT,F6LUT,G5LUT,G6LUT,H5LUT,H6LUT. On Xilinx
         * Ultrascale.
         *
         */
        std::map<std::string, std::vector<std::string>> sharedCellType2BELNames;

        /**
         * @brief Get the Potential BEL Type IDs for a specific cell object
         *
         * @param cell
         * @return std::vector<int>&
         */
        inline std::vector<int> &getPotentialBELTypeIDs(DesignInfo::DesignCell *cell)
        {
            // be careful, this function will remap the BELType according to the user configuration
            // for example, SLICEM_CARRY8 will be mapped to SLICEL_CARRY8, so they will be treated equally during cell
            // spreading.
            assert(cell);
            assert((unsigned int)cell->getCellId() < cellId2SharedCellBELTypeID.size());
            return cellId2SharedCellBELTypeID[cell->getCellId()];
        }

        /**
         * @brief Get the potential BEL Type IDs for a specific cell type
         *
         * @param cellType
         * @return std::vector<int>&
         */
        inline std::vector<int> &getPotentialBELTypeIDs(DesignInfo::DesignCellType cellType)
        {
            // be careful, this function will not remap the BELType.
            return cellType2sharedBELTypeIDs[cellType];
        }

        /**
         * @brief get the ID of SharedBELType from a string
         *
         * @param tmpStr
         * @return int
         */
        inline int getSharedBELTypeId(std::string tmpStr)
        {
            assert(sharedCellBELTypeName2ID.find(tmpStr) != sharedCellBELTypeName2ID.end());
            return sharedCellBELTypeName2ID[tmpStr];
        }

        /**
         * @brief Get the theoratical occupation of a specific cell type
         *
         * @param cellType
         * @return float
         */
        inline float getOccupation(DesignInfo::DesignCellType cellType)
        {
            return cellType2sharedBELTypeOccupation[cellType];
        }

        /**
         * @brief Get the actual occupation of a specific cell
         *
         * @param cell
         * @return float
         *
         * it will be the multiplication of cellId2Occupation and cellId2InfationRatio.
         * cellId2Occupation might be adjusted by the packing feasibility.
         * cellId2InfationRatio might be adjusted by the routing congestion level.
         */
        inline float getActualOccupation(DesignInfo::DesignCell *cell)
        {
            return cellId2Occupation[cell->getCellId()] * cellId2InfationRatio[cell->getCellId()];
        }

        /**
         * @brief Get the inflate ratio of a cell
         *
         * @param cell
         * @return float
         */
        inline float getInflateRatio(DesignInfo::DesignCell *cell)
        {
            return cellId2InfationRatio[cell->getCellId()];
        }

        /**
         * @brief Get the Actual Occupation By Cell Id
         *
         * @param id
         * @return float
         */
        inline float getActualOccupationByCellId(int id)
        {
            return cellId2Occupation[id] * cellId2InfationRatio[id];
        }

        /**
         * @brief get the reference of cellId2Occupation for convenience
         *
         * @return std::vector<float>&
         */
        inline std::vector<float> &getcellId2Occupation()
        {
            return cellId2Occupation;
        }

        /**
         * @brief get the reference of cellId2InfationRatio for convenience
         *
         * @return std::vector<float>&
         */
        inline std::vector<float> &getcellId2InfationRatio()
        {
            return cellId2InfationRatio;
        }

        /**
         * @brief set the mapping from cells in design netlist to BEL type ID for later processing.
         *
         * set the mapping variable cellId2SharedCellBELTypeID for later processing
         *
         * @param designInfo
         *
         */
        void setBELTypeForCells(DesignInfo *designInfo);

        /**
         * @brief forget the occupation adjustment by packing feasibility and routing congestion
         *
         */
        void resetCellOccupationToDefault();

      private:
        std::map<std::string, int> sharedCellBELTypeName2ID;
        std::vector<std::vector<int>> cellId2SharedCellBELTypeID;
        std::vector<float> cellId2Occupation;
        std::vector<float> cellId2InfationRatio;
        std::vector<float> defaultCellId2Occupation;
        DesignInfo *designInfo;
        DeviceInfo *deviceInfo;
    };

    /**
     * @brief information for a site, e.g. what BEL in site and where are these kind of sites
     *
     * This class is temporarily not used.
     *
     */
    class PlacementSiteTypeInfo
    {
      public:
        typedef struct location
        {
            float locX;
            float locY;
        } location;

        PlacementSiteTypeInfo(std::string siteType, std::vector<DeviceInfo::DeviceSite *> &correspondingSites)
            : siteType(siteType), correspondingSites(correspondingSites)
        {
            assert(correspondingSites.size() > 0);

            BELNames.clear();
            for (DeviceInfo::DeviceBEL *curBEL : correspondingSites[0]->getChildrenBELs())
            {
                BELNames.insert(curBEL->getBELType());
            }

            potentialLocations.clear();
            for (DeviceInfo::DeviceSite *curSite : correspondingSites)
            {
                location tmploc;
                tmploc.locX = curSite->X();
                tmploc.locY = curSite->Y();
                potentialLocations.push_back(tmploc);
            }
        }

        ~PlacementSiteTypeInfo()
        {
            potentialLocations.clear();
            BELNames.clear();
        }

      private:
        std::string siteType;
        std::set<std::string> BELNames;
        std::vector<location> potentialLocations;
        const std::vector<DeviceInfo::DeviceSite *> &correspondingSites;
    };

    /**
     * @brief BEL bin for global placement for a specific shared BEL type
     *
     * To easier to find the neighbors of a design instance, we divide the design instances in the placement
     * into a grid of bins. A placement bin will record the instances and resource sites inside it.
     *
     */
    class PlacementBinInfo
    {
      public:
        /**
         * @brief Construct a new Placement Bin Info object
         *
         * To construct a placement bin, we need to indicate its boundaries and target cell type for later information
         * processing.
         *
         * @param sharedCellType
         * @param leftX bin left boundary
         * @param rightX bin right boundary
         * @param bottomY bin bottom boundary
         * @param topY bin top boundary
         * @param row which row in the grid
         * @param column which column in the grid
         * @param compatiblePlacementTable
         *
         */
        PlacementBinInfo(std::string sharedCellType, float leftX, float rightX, float bottomY, float topY, int row,
                         int column, CompatiblePlacementTable *compatiblePlacementTable)
            : sharedCellType(sharedCellType), compatiblePlacementTable(compatiblePlacementTable), leftX(leftX),
              rightX(rightX), topY(topY), bottomY(bottomY), row(row), column(column)

        {
            correspondingSites.clear();
            cells.clear();
        }

        ~PlacementBinInfo()
        {
            correspondingSites.clear();
            cells.clear();
        }

        /**
         * @brief Get the shortest Manhattan distance from the bin to a specific location
         *
         * @param inX
         * @param inY
         * @return float
         */
        inline float getManhattanDistanceTo(float inX, float inY)
        {
            if (leftX < inX && inX <= rightX && bottomY < inY && inY <= topY)
            {
                return 0;
            }
            if (leftX < inX && inX <= rightX)
            {
                return std::min(std::fabs(inY - topY), std::fabs(inY - bottomY));
            }
            if (bottomY < inY && inY <= topY)
            {
                return std::min(std::fabs(inX - leftX), std::fabs(inX - rightX));
            }
            return std::min(std::fabs(inX - leftX), std::fabs(inX - rightX)) +
                   std::min(std::fabs(inY - topY), std::fabs(inY - bottomY));
        }

        /**
         * @brief add a resource site into the bin
         *
         * check the resources in the site and increase the resource capacity of the bin
         *
         * @param curSite
         *
         */
        void addSiteIntoBin(DeviceInfo::DeviceSite *curSite);

        /**
         * @brief check if the bin covers a given location on the device
         *
         * @param x
         * @param y
         * @return true if the bin covers the given location on the device
         * @return false if the bin does not cover the given location on the device
         */
        inline bool inRange(float x, float y)
        {
            return (x < rightX && x >= leftX && y < topY && y >= bottomY);
        }

        inline bool inRangeY(float y)
        {
            return (y < topY && y >= bottomY);
        }

        /**
         * @brief add a design cell into the bin
         *
         * we have to set the mutex locked during the process since we enable multi-threading
         * in the placer.
         *
         * @param cell
         * @param occupationAdded how many slots will the cell occupy
         *
         */
        inline void addCell(DesignInfo::DesignCell *cell, int occupationAdded)
        {
            mtx.lock();
            assert(cells.find(cell) == cells.end());
            cells.insert(cell);
            assert(occupationAdded >= 0);
            utilization += occupationAdded;
            mtx.unlock();
        }

        /**
         * @brief remove a design cell from the bin
         *
         * @param cell
         * @param occupationAdded how many slots were occupied by the cell
         *
         * we have to set the mutex locked during the process since we enable multi-threading
         * in the placer.
         */
        inline void removeCell(DesignInfo::DesignCell *cell, int occupationAdded)
        {
            // if (cell)
            mtx.lock();
            assert(cells.find(cell) != cells.end());
            cells.erase(cell);
            utilization -= occupationAdded;
            assert(utilization >= 0);
            mtx.unlock();
        }

        inline bool contains(DesignInfo::DesignCell *cell)
        {
            // if (cell)
            return cells.find(cell) != cells.end();
        }

        inline void reset()
        {
            cells.clear();
            utilization = 0;
            binShrinkRatio = requiredBinShrinkRatio;
            overflowCnt = 0;
            noOverflowCnt = 0;
            switchDemandForNets = 0;
            switchSupplyForNets = 0;
        }

        /**
         * @brief reduce the resource capacity by a given ratio
         *
         * @param r
         */
        inline void shrinkBinBy(float r)
        {
            binShrinkRatio *= (1 - r);
        }

        /**
         * @brief increase the resource capacity by a given ratio
         *
         * @param r
         */
        inline void inflateBinBy(float r)
        {
            binShrinkRatio *= (1 + r);
        }

        inline void resetBinShrinkRatio()
        {
            binShrinkRatio = requiredBinShrinkRatio;
        }

        inline float getRequiredBinShrinkRatio()
        {
            return requiredBinShrinkRatio;
        }

        /**
         * @brief Set the Required Bin Shrink Ratio for a bin
         *
         * For example, to resolve routing congestion, we will modify the default resource capacity of a bin
         *
         * @param r
         */
        inline void setRequiredBinShrinkRatio(float r)
        {
            requiredBinShrinkRatio = r;
        }

        /**
         * @brief Get the reference of the set of cells in the bin
         *
         * @return std::set<DesignInfo::DesignCell *>&
         */
        inline std::set<DesignInfo::DesignCell *> &getCells()
        {
            return cells;
        }

        inline float getBinShrinkRatio()
        {
            return binShrinkRatio;
        }

        /**
         * @brief Get the Utilization Rate: utilization / (capacity * binShrinkRatio)
         *
         * @return float
         */
        inline float getUtilizationRate()
        {
            if (capacity == 0)
                return utilization / 0.01;
            assert(capacity != 0);
            return (float)utilization / (capacity * binShrinkRatio);
        }

        /**
         * @brief Get the theoratical utilization rate (use LUT theoratical resource utilization without any adjustment
         * to dump bin information)
         *
         * @return float
         */
        inline float getRealUtilizationRate()
        {
            if (cells.size())
            {
                std::vector<DesignInfo::DesignCell *> cellVec;
                cellVec.clear();
                for (auto cell : cells)
                {
                    cellVec.push_back(cell);
                }
                if (cellVec[0]->isLUT())
                {
                    float tmputilization = 0;
                    for (auto cell : cells)
                    {
                        if (cell->isLUT6())
                            tmputilization += 2.0;
                        else
                            tmputilization += 1;
                    }
                    return (float)tmputilization / capacity;
                }
                else
                {
                    return getUtilizationRate();
                }
            }
            return 0;
        }

        inline float getUtilization()
        {
            return (float)utilization;
        }

        inline float getCapacity()
        {
            return (float)capacity * binShrinkRatio;
        }

        /**
         * @brief  check whether the resource demand in the bin is higher than the supply.
         *
         * @param overflowThreshold
         * @return true if the resource supply is enough.
         * @return false if the resource supply is not enough.
         */
        inline bool isOverflow(float overflowThreshold)
        {
            if (capacity == 0)
            {
                if (utilization == 0)
                    return false;
                else
                    return true;
            }
            assert(utilization >= 0);
            assert(capacity != 0);
            return ((float)utilization / (capacity)) > overflowThreshold + eps;
            // return ((float)utilization / (binShrinkRatio * capacity)) > overflowThreshold + eps;
        }

        /**
         * @brief check whether we can add some BEL demand to the bin
         *
         * @param BELAmo
         * @return true when there are available resources meeting the BEL demand.
         * @return false when there is no available resource meeting the BEL demand.
         */
        inline bool canAddMore(int BELAmo)
        {
            if (capacity == 0)
                return false;
            return ((float)(utilization + BELAmo) / (binShrinkRatio * capacity)) <= 1.00 + eps;
        }

        /**
         * @brief return the row of the bin in the grid
         *
         * @return int
         */
        inline int Y()
        {
            return row;
        }

        /**
         * @brief return the column of the bin in the grid
         *
         * @return int
         */
        inline int X()
        {
            return column;
        }

        /**
         * @brief return the left boundary of the bin
         *
         * @return float
         */
        inline float left()
        {
            return leftX;
        }

        /**
         * @brief return the right boundary of the bin
         *
         * @return float
         */
        inline float right()
        {
            return rightX;
        }

        /**
         * @brief return the top boundary of the bin
         *
         * @return float
         */
        inline float top()
        {
            return topY;
        }

        /**
         * @brief return the bottom boundary of the bin
         *
         * @return float
         */
        inline float bottom()
        {
            return bottomY;
        }

        inline std::string getType()
        {
            return sharedCellType;
        }

        /**
         * @brief increase one time of overflow situation
         *
         */
        inline void countOverflow()
        {
            overflowCnt++;
        }

        /**
         * @brief increase one time of non-overflow situation
         *
         */
        inline void countNoOverflow()
        {
            if (noOverflowCnt < 100)
            {
                noOverflowCnt++;
            }
        }
        inline void resetNoOverflowCounter()
        {
            noOverflowCnt = 0;
        }
        inline void resetOverflowCounter()
        {
            overflowCnt = 0;
        }
        inline int getNoOverflowCounter()
        {
            return noOverflowCnt;
        }
        inline int getOverflowCounter()
        {
            return overflowCnt;
        }

        /**
         * @brief Get the reference of the set of sites in the bin
         *
         * @return std::vector<DeviceInfo::DeviceSite *>&
         */
        inline std::vector<DeviceInfo::DeviceSite *> &getCorrespondingSites()
        {
            return correspondingSites;
        }

        inline std::string &getSharedCellType()
        {
            return sharedCellType;
        }

        /**
         * @brief increase the net routing demand of the bin
         *
         * @param additionalDemand
         */
        inline void increaseSWDemandBy(float additionalDemand)
        {
            switchDemandForNets += additionalDemand;
        }

        /**
         * @brief get the net routing demand of the bin
         *
         */
        inline float getSwitchDemandForNets() const
        {
            return switchDemandForNets;
        }

      private:
        std::string sharedCellType;
        std::vector<DeviceInfo::DeviceSite *> correspondingSites;
        CompatiblePlacementTable *compatiblePlacementTable;
        std::set<DesignInfo::DesignCell *> cells;

        int capacity = 0;
        int utilization = 0;
        float binShrinkRatio = 1.0;
        float requiredBinShrinkRatio = 1.0;

        const float leftX;
        const float rightX;
        const float topY;
        const float bottomY;

        float eps = 1e-5;
        const int row;
        const int column;

        int overflowCnt = 0;
        int noOverflowCnt = 0;

        float switchDemandForNets = 0.0;
        float switchSupplyForNets = 0.0; // only consider the general sites (DSP/BRAM/SLICE)
        std::mutex mtx;
    };

    /**
     * @brief BEL bin for global placement for multiple specific shared BEL types
     *
     * This bin class is not for a specific cell type.
     * It is not used in the current implementation.
     */
    class PlacementHybridBinInfo
    {
      public:
        PlacementHybridBinInfo(PlacementBinInfo *curBin)
            : leftX(curBin->left()), rightX(curBin->right()), topY(curBin->top()), bottomY(curBin->bottom()),
              row(curBin->Y()), column(curBin->X())
        {
            mergedBins.clear();
            correspondingSites.clear();
            cells.clear();
            mergedBins.push_back(curBin);
        }

        ~PlacementHybridBinInfo()
        {
            correspondingSites.clear();
            cells.clear();
        }

        inline bool inRange(float x, float y)
        {
            return (x <= rightX && x > leftX && y <= topY && y > bottomY);
        }

        inline bool inRangeY(float y)
        {
            return (y <= topY && y > bottomY);
        }

        inline void addCell(DesignInfo::DesignCell *cell, int occupationAdded)
        {
            // if (cell)
            cells.insert(cell);
            utilization += occupationAdded;
        }

        inline void removeCell(DesignInfo::DesignCell *cell, int occupationAdded)
        {
            // if (cell)
            assert(cells.find(cell) != cells.end());
            cells.erase(cell);
            utilization -= occupationAdded;
        }

        inline bool contains(DesignInfo::DesignCell *cell)
        {
            // if (cell)
            return cells.find(cell) != cells.end();
        }

        inline void reset()
        {
            cells.clear();
            utilization = 0;
        }

        inline std::set<DesignInfo::DesignCell *> &getCells()
        {
            return cells;
        }

        inline float getUtilizationRate()
        {
            if (capacity == 0)
                return utilization / 0.01;
            assert(capacity != 0);
            return (float)binShrinkRatio * utilization / capacity;
        }

        inline float getUtilization()
        {
            return (float)utilization;
        }

        inline float getCapacity()
        {
            return (float)capacity / binShrinkRatio;
        }

        inline bool isOverflow()
        {
            if (capacity == 0)
            {
                if (utilization == 0)
                    return false;
                else
                    return true;
            }
            assert(capacity != 0);
            return ((float)binShrinkRatio * utilization / capacity) > 1.00 + eps;
        }

        inline bool canAddMore(int BELAmo)
        {
            if (capacity == 0)
                return false;
            return ((float)binShrinkRatio * (utilization + BELAmo) / capacity) <= 1.00 + eps;
        }
        inline void setYX(int i, int j)
        {
            row = i;
            column = j;
        }

        inline int Y()
        {
            return row;
        }
        inline int X()
        {
            return column;
        }

        inline float left()
        {
            return leftX;
        }
        inline float right()
        {
            return rightX;
        }
        inline float top()
        {
            return topY;
        }
        inline float bottom()
        {
            return bottomY;
        }

      private:
        std::vector<DeviceInfo::DeviceSite *> correspondingSites;
        std::vector<PlacementBinInfo *> mergedBins;
        std::set<DesignInfo::DesignCell *> cells;
        int capacity = 0;
        int utilization = 0;
        float binShrinkRatio = 1.0;
        float leftX;
        float rightX;
        float topY;
        float bottomY;
        float eps = 1e-5;
        int row;
        int column;
    };

    /**
     * @brief a movement unit in placement with information of location and resource demand
     *
     */
    class PlacementUnit
    {
      public:
        PlacementUnit(std::string name, int id, PlacementUnitType unitType) : name(name), id(id), unitType(unitType)
        {
        }
        virtual ~PlacementUnit()
        {
        }
        void getAnchorLocation(float &x, float &y)
        {
            x = anchorX;
            y = anchorY;
        }

        inline float X()
        {
            return anchorX;
        }

        inline float Y()
        {
            return anchorY;
        }

        inline float lastX()
        {
            return lastAnchorX;
        }

        inline float lastY()
        {
            return lastAnchorY;
        }

        /**
         * @brief Set the Anchor Location for the PlacementUnit
         *
         * meanwhile, if it is not the first iteration in placement, record last anchor location
         *
         * @param x
         * @param y
         */
        inline void setAnchorLocation(float x, float y)
        {
            assert(!locked);
            if (anchorX < -100 && anchorY < -100)
            {
                lastAnchorX = x;
                lastAnchorY = y;
            }
            else
            {
                lastAnchorX = anchorX;
                lastAnchorY = anchorY;
            }
            anchorX = x;
            anchorY = y;
        }

        /**
         * @brief Set the Spread Location based on forgetting ratio
         *
         * @param x
         * @param y
         * @param forgetRatio how much should the PlacementUnit forget the original location from later iteration
         */
        inline void setSpreadLocation(float x, float y, float forgetRatio)
        {
            assert(!locked);

            if (lastSpreadX > -100 && lastSpreadY > -100)
            {
                assert(forgetRatio <= 1);
                x = x * forgetRatio + lastSpreadX * (1 - forgetRatio);
                y = y * forgetRatio + lastSpreadY * (1 - forgetRatio);
                lastSpreadX = x;
                lastSpreadY = y;
            }
            lastAnchorX = x;
            lastAnchorY = y;
            anchorX = x;
            anchorY = y;
        }

        inline void setAnchorLocationAndForgetTheOriginalOne(float x, float y)
        {
            assert(!locked);

            lastAnchorX = x;
            lastAnchorY = y;
            anchorX = x;
            anchorY = y;
        }

        inline void recordSpreadLocatin()
        {
            lastSpreadX = anchorX;
            lastSpreadY = anchorY;
        }

        inline void setFixed()
        {
            assert(!locked);
            fixed = true;
        }

        inline void setUnfixed()
        {
            assert(!locked);
            fixed = false;
        }

        inline void setLocked()
        {
            locked = true;
        }

        inline void setUnlocked()
        {
            locked = false;
        }

        inline bool isLocked()
        {
            return locked;
        }

        inline void setPlaced()
        {
            assert(!locked);
            placed = true;
        }

        inline bool isFixed()
        {
            return fixed;
        }
        inline bool isPlaced()
        {
            return placed;
        }

        inline std::string &getName()
        {
            return name;
        }

        inline PlacementUnitType getType()
        {
            return unitType;
        }

        inline void setWeight(int numCell)
        {
            weight = numCell;
        }

        inline int getWeight()
        {
            return weight;
        }

        inline unsigned int getId()
        {
            return id;
        }

        inline void renewId(int newId)
        {
            id = newId;
        }

        /**
         * @brief Set the Nets Set Ptr object which records the nets connecting to the PlacementUnit
         *
         * @param _nets
         */
        inline void setNetsSetPtr(std::vector<PlacementNet *> *_nets)
        {
            nets = _nets;
        }

        /**
         * @brief Get the Nets Set Ptr object which records the nets connecting to the PlacementUnit
         *
         * @return std::vector<PlacementNet *>*
         */
        inline std::vector<PlacementNet *> *getNetsSetPtr()
        {
            return nets;
        }

        inline void addDSP()
        {
            DSPcnt++;
        }
        inline void addBRAM()
        {
            BRAMcnt++;
        }
        inline void addLUTRAM()
        {
            LUTRAMcnt++;
        }
        inline void addLUT()
        {
            LUTcnt++;
        }
        inline void addFF()
        {
            FFcnt++;
        }
        inline void addCARRY()
        {
            CARRYcnt++;
        }
        inline void addMUX()
        {
            MUXcnt++;
        }
        inline bool checkHasDSP()
        {
            return DSPcnt;
        }
        inline bool checkHasBRAM()
        {
            return BRAMcnt;
        }
        inline bool checkHasLUTRAM()
        {
            return LUTRAMcnt;
        }
        inline bool checkHasLUT()
        {
            return LUTcnt;
        }
        inline bool checkHasFF()
        {
            return FFcnt;
        }
        inline bool checkHasCARRY()
        {
            return CARRYcnt;
        }
        inline bool checkHasMUX()
        {
            return MUXcnt;
        }
        inline bool hasRegister()
        {
            return (checkHasFF() || checkHasDSP() || checkHasBRAM() || checkHasLUTRAM());
        }
        inline bool hasLogic()
        {
            return (checkHasFF() || checkHasDSP() || checkHasBRAM() || checkHasLUTRAM() || checkHasCARRY() ||
                    checkHasLUT());
        }
        inline bool isMCLB()
        {
            return LUTRAMcnt;
        }
        inline bool isLCLB()
        {
            return LUTcnt > 0 && CARRYcnt == 0; // LogicCLB with Carray will be handled by macroLegalizer
        }
        inline int getDSPNum()
        {
            return DSPcnt;
        }
        inline int getBRAMNum()
        {
            return BRAMcnt;
        }
        inline int getLUTRAMNum()
        {
            return LUTRAMcnt;
        }
        inline int getLUTNum()
        {
            return LUTcnt;
        }
        inline int getCARRYNum()
        {
            return CARRYcnt;
        }
        inline int getMUXNum()
        {
            return MUXcnt;
        }
        inline void setPacked()
        {
            packed = true;
        }
        inline void resetPacked()
        {
            packed = false;
        }
        inline bool isPacked()
        {
            return packed;
        }

      private:
        std::string name;
        int id;
        float anchorX = -2000, anchorY = -2000;
        float lastAnchorX = -2000, lastAnchorY = -2000;
        float lastSpreadX = -2000, lastSpreadY = -2000;
        PlacementUnitType unitType;

        /**
         * @brief record the nets connected to this PlacementUnit
         *
         */
        std::vector<PlacementNet *> *nets;

        /**
         * @brief fixed simply means the PlacementUnit cannot be moved.
         *
         */
        bool fixed = false;

        /**
         * @brief the PlacementUnit is placed to BEL slot.
         *
         * Currently, this attribute is not used.
         *
         */
        bool placed = false;

        /**
         * @brief if locked, the attributes of the PlacementUnit cannot be changed (more than fixed.)
         *
         */
        bool locked = false;
        int weight = 1;

        int DSPcnt = 0;
        int BRAMcnt = 0;
        int LUTRAMcnt = 0;
        int LUTcnt = 0;
        int FFcnt = 0;
        int CARRYcnt = 0;
        int MUXcnt = 0;

        bool packed = false;
    };

    /**
     * @brief the smallest, indivisible, representable component. It will include only one standard cell
     *
     */
    class PlacementUnpackedCell : public PlacementUnit
    {
      public:
        /**
         * @brief Construct a new Placement Unpacked Cell object
         *
         * @param name
         * @param id a unique ID for this placement unit
         * @param cell
         */
        PlacementUnpackedCell(std::string name, int id, DesignInfo::DesignCell *cell)
            : PlacementUnit(name, id, PlacementUnitType_UnpackedCell), cell(cell)
        {
            if (cell->isBRAM())
                addBRAM();
            if (cell->isDSP())
                addDSP();
            if (cell->isFF())
                addFF();
            if (cell->isLUTRAM())
                addLUTRAM();
            if (cell->isLUT())
                addLUT();
            if (cell->isCarry())
                addCARRY();
            if (cell->isMux())
                addMUX();
        }
        ~PlacementUnpackedCell()
        {
        }

        void setLockedAt(std::string _siteName, std::string _BELName, DeviceInfo *deviceInfo, bool lock = true)
        {
            setFixed();
            setPlaced();
            siteName = _siteName;
            BELName = _BELName;
            setAnchorLocation(deviceInfo->getSite(siteName)->X(), deviceInfo->getSite(siteName)->Y());
            deviceInfo->getSite(siteName)->setOccupied();
            if (lock)
                setLocked();
        }

        inline DesignInfo::DesignCell *getCell()
        {
            return cell;
        }

        inline std::string getFixedBELName()
        {
            return BELName;
        }

        inline std::string getFixedSiteName()
        {
            return siteName;
        }

      private:
        DesignInfo::DesignCell *cell;
        // DesignInfo::DesignCellType virtualCellType;
        std::string siteName;
        std::string BELName;
    };

    /**
     * @brief  a fixed group of multiple standard cells with constraints of their relative locations
     *
     */
    class PlacementMacro : public PlacementUnit
    {
      public:
        enum PlacementMacroType
        {
            PlacementMacroType_LUTFFPair = 0, // LUT-FF pair during  incremental packing
            PlacementMacroType_FFFFPair,      // FF-FF pair during  incremental packing
            PlacementMacroType_HALFCLB,
            PlacementMacroType_LCLB,  // vendor defined primitives, like FFs for system reset, have to be placed in
                                      // SLICEL
            PlacementMacroType_MCLB,  // LUTRAM cells sharing address/data bits or vendor defined primitives have to be
                                      // placed in SLICEM
            PlacementMacroType_CARRY, // chained CARRYs, its directly connected LUTs/FFs and routing BELs.
            PlacementMacroType_DSP,   // Cascaded DSPs
            PlacementMacroType_BRAM,  // Cascaded BRAMs
            PlacementMacroType_MUX7,  // MUXF7, its directly connected LUTs/FFs and routing BELs.
            PlacementMacroType_MUX8,  // MUXF7, its directly connected MUXF7/LUTs/FFs and routing BELs.
            PlacementMacroType_MUX9   // MUXF9, its directly connected MUXF7/MUXF8/LUTs/FFs and routing BELs.
        };

        PlacementMacro(std::string name, int id, PlacementMacroType macroType)
            : PlacementUnit(name, id, PlacementUnitType_Macro), macroType(macroType)
        {
            fixedCells.clear();
            cell2IdInMacro.clear();
            cellsInMacro.clear();
            cellSet.clear();
            // siteOccupations.clear();
            top = -10000;
            bottom = 10000;
            left = 10000;
            right = -10000;
        };
        ~PlacementMacro()
        {
        }

        inline bool hasCell(DesignInfo::DesignCell *curCell)
        {
            return cellSet.find(curCell) != cellSet.end();
        }

        /**
         * @brief add a real cell into the macro with its offsets in the macro
         *
         * @param curCell
         * @param cellType update the cell's "virtual" cell type so it can occupy specific resource (e.g., make an LUT1
         * an LUT6)
         * @param x
         * @param y
         */
        inline void addCell(DesignInfo::DesignCell *curCell, DesignInfo::DesignCellType cellType, float x, float y)
        {
            if (curCell)
            {
                if (curCell->isBRAM())
                    addBRAM();
                if (curCell->isDSP())
                    addDSP();
                if (!curCell->isVirtualCell())
                {
                    if (curCell->isFF())
                        addFF();
                }
                if (curCell->isLUTRAM())
                    addLUTRAM();
                if (curCell->isLUT())
                    addLUT();
                if (curCell->isCarry())
                    addCARRY();
                if (curCell->isMux())
                    addMUX();
                curCell->setVirtualType(cellType);
                cell2IdInMacro[curCell] = offsetX.size();
                cellSet.insert(curCell);
            }
            else
            {
                assert(false && "unexpected");
            }
            cellsInMacro.push_back(curCell);
            cells_Type.push_back(cellType);
            offsetX.push_back(x);
            offsetY.push_back(y);
            if (x < left)
                left = x;
            if (x > right)
                right = x;
            if (y < bottom)
                bottom = y;
            if (y > top)
                top = y;
        }

        /**
         * @brief  add a virtual cell with a given name into the macro with its offsets in the macro. Usually it is to
         * occupy routing resource in a site.
         *
         * @param virtualCellName the name of the virtual cell to be added into the design
         * @param designInfo since we are creating virtual cell, it should be added into design information
         * @param cellType the type of the virtual cell type
         * @param x
         * @param y
         * @return DesignInfo::DesignCell*
         */
        inline DesignInfo::DesignCell *addVirtualCell(std::string virtualCellName, DesignInfo *designInfo,
                                                      DesignInfo::DesignCellType cellType, float x, float y)
        {
            DesignInfo::DesignCell *vCell =
                new DesignInfo::DesignCell(true, virtualCellName, cellType, designInfo->getNumCells());
            designInfo->addCell(vCell); // add the virtual cell to design info for later processing
            cells_Type.push_back(cellType);
            cellsInMacro.push_back(vCell);
            cell2IdInMacro[vCell] = offsetX.size();
            cellSet.insert(vCell);
            offsetX.push_back(x);
            offsetY.push_back(y);
            if (x < left)
                left = x;
            if (x > right)
                right = x;
            if (y < bottom)
                bottom = y;
            if (y > top)
                top = y;

            if (vCell->isBRAM())
                addBRAM();
            if (vCell->isDSP())
                addDSP();
            // if (vCell->isFF())
            //     addFF();
            if (vCell->isLUTRAM())
                addLUTRAM();
            if (vCell->isLUT())
                addLUT();
            if (vCell->isCarry())
                addCARRY();
            if (vCell->isMux())
                addMUX();
            return vCell;
        }

        /**
         * @brief  add a virtual cell without given name into the macro with its offsets in the macro. Usually it is to
         * occupy routing resource in a site.
         *
         * @param designInfo since we are creating virtual cell, it should be added into design information
         * @param cellType the type of the virtual cell type
         * @param x
         * @param y
         */
        inline void addVirtualCell(DesignInfo *designInfo, DesignInfo::DesignCellType cellType, float x, float y)
        {
            DesignInfo::DesignCell *vCell = new DesignInfo::DesignCell(true, cellType, designInfo->getNumCells());
            designInfo->addCell(vCell); // add the virtual cell to design info for later processing
            cells_Type.push_back(cellType);
            cellsInMacro.push_back(vCell);
            cell2IdInMacro[vCell] = offsetX.size();
            cellSet.insert(vCell);
            offsetX.push_back(x);
            offsetY.push_back(y);
            if (x < left)
                left = x;
            if (x > right)
                right = x;
            if (y < bottom)
                bottom = y;
            if (y > top)
                top = y;

            if (vCell->isBRAM())
                addBRAM();
            if (vCell->isDSP())
                addDSP();
            // if (vCell->isFF())
            //     addFF();
            if (vCell->isLUTRAM())
                addLUTRAM();
            if (vCell->isLUT())
                addLUT();
            if (vCell->isCarry())
                addCARRY();
            if (vCell->isMux())
                addMUX();
        }

        inline std::vector<DesignInfo::DesignCell *> &getCells()
        {
            return cellsInMacro;
        }

        /**
         * @brief some constaints of elements' relative locations are defined by the design. We need to record this.
         *
         */
        typedef struct _fixedPlacementInfo_inMacro
        {
            _fixedPlacementInfo_inMacro(DesignInfo::DesignCell *cell, std::string siteName, std::string BELName)
                : cell(cell), siteName(siteName), BELName(BELName)
            {
            }
            DesignInfo::DesignCell *cell;
            std::string siteName;
            std::string BELName;
        } fixedPlacementInfo_inMacro;

        /**
         * @brief add information of a fixed cell
         *
         * @param cell
         * @param siteName
         * @param BELName
         */
        inline void addFixedCellInfo(DesignInfo::DesignCell *cell, std::string siteName, std::string BELName)
        {
            fixedCells.emplace_back(cell, siteName, BELName);
        }

        inline float getCellOffsetXInMacro(DesignInfo::DesignCell *cell)
        {
            assert(cell2IdInMacro.find(cell) != cell2IdInMacro.end());
            return offsetX[cell2IdInMacro[cell]];
        }

        inline float getCellOffsetYInMacro(DesignInfo::DesignCell *cell)
        {
            assert(cell2IdInMacro.find(cell) != cell2IdInMacro.end());
            return offsetY[cell2IdInMacro[cell]];
        }

        // bool setFixedCombination(std::vector<DesignInfo::DesignCell *> _cells, std::vector<std::string> _siteNames,
        //                          std::vector<std::string> _BELNames, DeviceInfo *designInfo);

        /**
         * @brief Get the virtual cell information, including offsets and cell type
         *
         * @param vId
         * @param x
         * @param y
         * @param cellType
         */
        inline void getVirtualCellInfo(int vId, float &x, float &y, DesignInfo::DesignCellType &cellType)
        {
            x = offsetX[vId];
            y = offsetY[vId];
            cellType = cells_Type[vId];
        }

        inline DesignInfo::DesignCellType getVirtualCellType(int vId)
        {
            return cells_Type[vId];
        }

        inline int getNumOfCells()
        {
            return cells_Type.size();
        }

        DesignInfo::DesignCell *getCell(unsigned int id)
        {
            assert(id < cellsInMacro.size());
            return cellsInMacro[id];
        }

        inline float getTopOffset()
        {
            return top;
        }
        inline float getBottomOffset()
        {
            return bottom;
        }
        inline float getLeftOffset()
        {
            return left;
        }
        inline float getRightOffset()
        {
            return right;
        }

        /**
         * @brief for site-level cell spreading
         *
         * not used in current implmementation.
         *
         * @param siteOffset
         * @param occ
         */
        inline void addOccupiedSite(float siteOffset, float occ)
        {
            //     siteOccupations.push_back(std::pair<float, float>(siteOffset, occ));
        }
        // inline std::vector<std::pair<float, float>> &getOccupiedSiteInfo()
        // {
        //     return siteOccupations;
        // }

        inline bool isCellInMacro(DesignInfo::DesignCell *curCell)
        {
            return cellSet.find(curCell) != cellSet.end();
        }

        inline PlacementMacroType getMacroType()
        {
            return macroType;
        }

        inline std::vector<fixedPlacementInfo_inMacro> &getFixedCellInfoVec()
        {
            return fixedCells;
        }

      private:
        // std::vector<std::string> siteNames;
        // std::vector<std::string> BELNames;
        std::set<DesignInfo::DesignCell *> cellSet;
        std::map<DesignInfo::DesignCell *, int> cell2IdInMacro;
        std::vector<DesignInfo::DesignCell *> cellsInMacro;
        std::vector<DesignInfo::DesignCellType>
            cells_Type; // BELTypeOccupation.size()>=realCells.size() because somtime a cell or a connection net
                        // can occupy multiple BEL
        std::vector<float> offsetX, offsetY; // offsetX.size() == BELTypeOccupation.size()
        std::vector<fixedPlacementInfo_inMacro> fixedCells;
        // std::vector<std::pair<float, float>> siteOccupations;
        float left, right, top, bottom;
        PlacementMacroType macroType;
    };

    /**
     * @brief Placement net, compared to design net, includes information related to placement.
     *
     * Placement net, compared to design net, includes information related to placement: HPWL bounding box,
     * interconnection between placement units (unpacked/macro), APIs to check wirelength.
     *
     * Please note that PlacementNet is HyperEdge, connecting to multiple pins.
     *
     */
    class PlacementNet
    {
      public:
        /**
         * @brief Construct a new Placement Net object
         *
         * @param designNet each PlacementNet is binded to design net.
         * @param id
         * @param cellId2PlacementUnitVec
         */
        PlacementNet(DesignInfo::DesignNet *designNet, int id, std::vector<PlacementUnit *> &cellId2PlacementUnitVec)
            : designNet(designNet), id(id)
        {
            unitsOfNetPins.clear();
            unitsOfDriverPins.clear();
            unitsOfPinsBeDriven.clear();
            PUSet.clear();
            for (DesignInfo::DesignPin *curPin : designNet->getPins())
            {
                PlacementUnit *tmpPU = cellId2PlacementUnitVec[curPin->getCell()->getElementIdInType()];
                unitsOfNetPins.push_back(tmpPU);
                PUSet.insert(tmpPU);

                if (curPin->isOutputPort())
                {
                    unitsOfDriverPins.push_back(tmpPU);
                }
                else
                {
                    unitsOfPinsBeDriven.push_back(tmpPU);
                }

                if (tmpPU->getType() == PlacementUnitType_UnpackedCell)
                {
                    pinOffset tmpPinOffset = pinOffset(curPin->getOffsetXInCell(), curPin->getOffsetYInCell());
                    pinOffsetsInUnit.push_back(tmpPinOffset);
                }
                else if (tmpPU->getType() == PlacementUnitType_Macro)
                {
                    PlacementMacro *tmpM = dynamic_cast<PlacementMacro *>(tmpPU);
                    assert(tmpM);
                    pinOffset tmpPinOffset =
                        pinOffset(curPin->getOffsetXInCell() + tmpM->getCellOffsetXInMacro(curPin->getCell()),
                                  curPin->getOffsetYInCell() + tmpM->getCellOffsetYInMacro(curPin->getCell()));
                    pinOffsetsInUnit.push_back(tmpPinOffset);
                }
            }
            leftPuId = rightPuId = topPuId = bottomPuId = -1;
        }
        ~PlacementNet()
        {
        }

        typedef struct _pinOffset
        {
            _pinOffset(float x, float y) : x(x), y(y)
            {
            }
            float x = 0.0, y = 0.0;
        } pinOffset;

        /**
         * @brief Get the reference of the vector of PlacementUnits connected to the net
         * The placement units in the vector might be duplicated because a net might connect to multiple pins of a unit
         * @return std::vector<PlacementUnit *>&
         */
        inline std::vector<PlacementUnit *> &getUnits()
        {
            return unitsOfNetPins;
        }

        /**
         * @brief Get the reference of the vector of the driver units that drive the net
         *
         * @return std::vector<PlacementUnit *>&
         */
        inline std::vector<PlacementUnit *> &getDriverUnits()
        {
            return unitsOfDriverPins;
        }

        /**
         * @brief Get the reference of the vector of the PlacementUnits driven by the net
         *
         * @return std::vector<PlacementUnit *>&
         */
        inline std::vector<PlacementUnit *> &getUnitsBeDriven()
        {
            return unitsOfPinsBeDriven;
        }

        /**
         * @brief Get the Design Net object
         *
         * @return DesignInfo::DesignNet*
         */
        inline DesignInfo::DesignNet *getDesignNet()
        {
            return designNet;
        }

        /**
         * @brief Get the Id of the net in current placement procedure
         *
         * @return int
         */
        inline int getId()
        {
            return id;
        }

        /**
         * @brief Get the Pin Offsets (x,y) of the Units object
         *
         * @return std::vector<pinOffset>&
         */
        inline std::vector<pinOffset> &getPinOffsetsInUnit()
        {
            return pinOffsetsInUnit;
        }

        /**
         * @brief update the bounding box of the net
         *
         * @param updateX if true, update the bounding box of the net in X coordinate
         * @param updateY if true, update the bounding box of the net in Y coordinate
         * @return true if the pins of the net is not at the same location
         * @return false if all pins of the net is at the same location
         */
        inline bool updateNetBounds(bool updateX, bool updateY)
        {
            if (updateX)
            {
                leftPUX = 1e5;
                rightPUX = -1e5;
                leftPinX = 1e5;
                rightPinX = -1e5;
                for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
                {
                    auto tmpPU = unitsOfNetPins[pinId_net];
                    auto tmpPUId = tmpPU->getId();
                    auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                    float cellX = tmpPU->X();
                    float pinX = tmpPU->X() + tmpPinOffset.x;
                    if (pinX < leftPinX)
                    {
                        leftPinX = pinX;
                        leftPUX = cellX;
                        leftPuId = tmpPUId;
                        leftPinId_net = pinId_net;
                    }
                    if (pinX > rightPinX)
                    {
                        rightPinX = pinX;
                        rightPUX = cellX;
                        rightPuId = tmpPUId;
                        rightPinId_net = pinId_net;
                    }
                }
            }
            if (updateY)
            {
                topPUY = -1e5;
                bottomPUY = 1e5;
                topPinY = -1e5;
                bottomPinY = 1e5;
                for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
                {
                    auto tmpPU = unitsOfNetPins[pinId_net];
                    auto tmpPUId = tmpPU->getId();
                    auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                    float cellY = tmpPU->Y();
                    float pinY = tmpPU->Y() + tmpPinOffset.y;
                    if (pinY < bottomPinY)
                    {
                        bottomPinY = pinY;
                        bottomPUY = cellY;
                        bottomPuId = tmpPUId;
                        bottomPinId_net = pinId_net;
                    }
                    if (pinY > topPinY)
                    {
                        topPinY = pinY;
                        topPUY = cellY;
                        topPuId = tmpPUId;
                        topPinId_net = pinId_net;
                    }
                }
            }
            return (updateX && (leftPuId != rightPuId)) || (updateY && (topPuId != bottomPuId));
        }

        /**
         * @brief get current HPWL of the net
         *
         * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
         * in X-coordinate
         * @return float
         */
        inline float getHPWL(float y2xRatio)
        {
            return std::fabs(rightPinX - leftPinX) + y2xRatio * std::fabs(topPinY - bottomPinY);
        }

        /**
         * @brief Get the New HPWL By Trying to move a PlacementUnit object
         *
         * For some procedures like legalization and packing, evaluate the wirelength change if the location of a
         * PlacementUnit is changed.
         *
         * @param curPU the PlacementUnit to be moved
         * @param targetPUX
         * @param targetPUY
         * @param y2xRatio
         * @return float
         */
        inline float getNewHPWLByTrying(PlacementUnit *curPU, double targetPUX, double targetPUY, float y2xRatio) const
        {
            float tmp_leftX = 1e5;
            float tmp_rightX = -1e5;
            float tmp_topY = -1e5;
            float tmp_bottomY = 1e5;

            for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
            {
                auto tmpPU = unitsOfNetPins[pinId_net];
                auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                float pinX;
                if (tmpPU == curPU)
                {
                    pinX = targetPUX + tmpPinOffset.x;
                }
                else
                {
                    pinX = tmpPU->X() + tmpPinOffset.x;
                }
                if (pinX < tmp_leftX)
                {
                    tmp_leftX = pinX;
                }
                if (pinX > tmp_rightX)
                {
                    tmp_rightX = pinX;
                }
            }
            for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
            {
                auto tmpPU = unitsOfNetPins[pinId_net];
                auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                float pinY;
                if (tmpPU == curPU)
                {
                    pinY = targetPUY + tmpPinOffset.y;
                }
                else
                {
                    pinY = tmpPU->Y() + tmpPinOffset.y;
                }
                if (pinY < tmp_bottomY)
                {
                    tmp_bottomY = pinY;
                }
                if (pinY > tmp_topY)
                {
                    tmp_topY = pinY;
                }
            }
            return std::fabs(tmp_rightX - tmp_leftX) + y2xRatio * std::fabs(tmp_topY - tmp_bottomY);
        }

        /**
         * @brief update the weights of 2-pin nets between PlacementUnits in this hyperedge(PlacementNet) according to
         * Bound2Bound net model
         *
         * In the quadratic placement, the wirelength(HPWL) can be modeled into a quadratic equation based on
         * Bound2Bound net model. The equation can be represented by matrix operation (XQX^T+PX)
         *
         * @param objectiveMatrixTripletList The non-Diag elements in matrix Q, stored in the vector of Eigen Triplet
         * (i,j,val)
         * @param objectiveMatrixDiag The Diag elements in matrix Q, stored in a 1-D vector
         * @param objectiveVector The elements in the vector P
         * @param generalWeight a weight given from external setting
         * @param pseudoWeight pseudo net weight to constrain the movement of PlacementUnits from their locations in
         * last optimization iteration
         * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
         * in X-coordinate
         * @param updateX update the X-coordinate term in the quadratic problem
         * @param updateY update the X-coordinate term in the quadratic problem
         */
        inline void updateBound2BoundNetWeight(std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                               std::vector<float> &objectiveMatrixDiag,
                                               Eigen::VectorXd &objectiveVector, float generalWeight, float y2xRatio,
                                               bool updateX, bool updateY)
        {
            assert(updateX ^ updateY);
            if (pinOffsetsInUnit.size() <= 1)
                return;
            float w = 2.0 * generalWeight / (float)(pinOffsetsInUnit.size() - 1);
            int nPins = pinOffsetsInUnit.size();

            // adopt netDegree Weight from RippeFPGA
            if (nPins < 10)
                w *= 1.00;
            else if (nPins < 20)
                w *= 1.2;
            else if (nPins < 50)
                w *= 1.6;
            else if (nPins < 100)
                w *= 1.8;
            else if (nPins < 200)
                w *= 2.1;
            else
                w *= 2.5;

            w *= designNet->getOverallEnhanceRatio();
            if (updateX)
            {
                // add net between left node and right node
                addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, leftPuId, rightPuId,
                          leftPUX, rightPUX, pinOffsetsInUnit[leftPinId_net].x, pinOffsetsInUnit[rightPinId_net].x,
                          !unitsOfNetPins[leftPinId_net]->isFixed(), !unitsOfNetPins[rightPinId_net]->isFixed(),
                          designNet->getPinPairEnhanceRatio(leftPinId_net, rightPinId_net) * w /
                              std::max(minDist, rightPinX - leftPinX));

                // add net between internal node and left/right node
                for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
                {
                    auto tmpPU = unitsOfNetPins[pinId_net];
                    auto tmpPUId = tmpPU->getId();
                    auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                    float curX = tmpPU->X();
                    bool movable = !tmpPU->isFixed();
                    if (pinId_net != leftPinId_net && pinId_net != rightPinId_net)
                    {
                        addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, tmpPUId, leftPuId,
                                  curX, leftPUX, tmpPinOffset.x, pinOffsetsInUnit[leftPinId_net].x, movable,
                                  !unitsOfNetPins[leftPinId_net]->isFixed(),
                                  designNet->getPinPairEnhanceRatio(pinId_net, leftPinId_net) * w /
                                      std::max(minDist, (curX + tmpPinOffset.x) - leftPinX));
                        addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, tmpPUId, rightPuId,
                                  curX, rightPUX, tmpPinOffset.x, pinOffsetsInUnit[rightPinId_net].x, movable,
                                  !unitsOfNetPins[rightPinId_net]->isFixed(),
                                  designNet->getPinPairEnhanceRatio(pinId_net, rightPinId_net) * w /
                                      std::max(minDist, rightPinX - (curX + tmpPinOffset.x)));
                    }
                }
            }
            if (updateY)
            {
                w *= y2xRatio;

                // add net between top node and bottom node
                addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, bottomPuId, topPuId,
                          bottomPUY, topPUY, pinOffsetsInUnit[bottomPinId_net].y, pinOffsetsInUnit[topPinId_net].y,
                          !unitsOfNetPins[bottomPinId_net]->isFixed(), !unitsOfNetPins[topPinId_net]->isFixed(),
                          designNet->getPinPairEnhanceRatio(topPinId_net, bottomPinId_net) * w /
                              std::max(minDist, topPinY - bottomPinY));

                // add net between internal node and top/bottom node
                for (unsigned int pinId_net = 0; pinId_net < unitsOfNetPins.size(); pinId_net++)
                {
                    auto tmpPU = unitsOfNetPins[pinId_net];
                    auto tmpPUId = tmpPU->getId();
                    auto tmpPinOffset = pinOffsetsInUnit[pinId_net];
                    float curY = tmpPU->Y();
                    bool movable = !tmpPU->isFixed();
                    if (pinId_net != topPinId_net && pinId_net != bottomPinId_net)
                    {
                        addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, tmpPUId, bottomPuId,
                                  curY, bottomPUY, tmpPinOffset.y, pinOffsetsInUnit[bottomPinId_net].y, movable,
                                  !unitsOfNetPins[bottomPinId_net]->isFixed(),
                                  designNet->getPinPairEnhanceRatio(pinId_net, bottomPinId_net) * w /
                                      std::max(minDist, (curY + tmpPinOffset.y) - bottomPinY));
                        addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, tmpPUId, topPuId,
                                  curY, topPUY, tmpPinOffset.y, pinOffsetsInUnit[topPinId_net].y, movable,
                                  !unitsOfNetPins[topPinId_net]->isFixed(),
                                  designNet->getPinPairEnhanceRatio(pinId_net, topPinId_net) * w /
                                      std::max(minDist, topPinY - (curY + tmpPinOffset.y)));
                    }
                }
            }
        }

        inline void addPseudoNet_enhancePin2Pin(std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                                std::vector<float> &objectiveMatrixDiag,
                                                Eigen::VectorXd &objectiveVector, float generalWeight, float y2xRatio,
                                                bool updateX, bool updateY, int PUIdA, int PUIdB, int pinIdA_net,
                                                int pinIdB_net)
        {
            float w = generalWeight;

            if (updateX)
            {
                // add net between left node and right node
                addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, PUIdA, PUIdB,
                          unitsOfNetPins[pinIdA_net]->X(), unitsOfNetPins[pinIdB_net]->X(),
                          pinOffsetsInUnit[pinIdA_net].x, pinOffsetsInUnit[pinIdB_net].x,
                          !unitsOfNetPins[pinIdA_net]->isFixed(), !unitsOfNetPins[pinIdB_net]->isFixed(), w);
            }
            if (updateY)
            {
                w *= y2xRatio;

                // add net between top node and bottom node
                addB2BNet(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, PUIdA, PUIdB,
                          unitsOfNetPins[pinIdA_net]->Y(), unitsOfNetPins[pinIdB_net]->Y(),
                          pinOffsetsInUnit[pinIdA_net].y, pinOffsetsInUnit[pinIdB_net].y,
                          !unitsOfNetPins[pinIdA_net]->isFixed(), !unitsOfNetPins[pinIdB_net]->isFixed(), w);
            }
        }

        void drawNet(float generalWeight = 1.0);

        /**
         * @brief set weights of the terms in the quadratic problem
         *
         * @param objectiveMatrixTripletList The non-Diag elements in matrix Q, stored in the vector of Eigen Triplet
         * (i,j,val)
         * @param objectiveMatrixDiag The Diag elements in matrix Q, stored in a 1-D vector
         * @param objectiveVector The elements in the vector P
         * @param puId0 PlacementUnit 0's Id (might be invaid -1)
         * @param puId1 PlacementUnit 1's Id (might be invaid -1)
         * @param pos0 PlacementUnit 0's position on one of the dimensions
         * @param pos1 PlacementUnit 1's position on one of the dimensions
         * @param pinOffset0 The pin's offset in PlacementUnit 0
         * @param pinOffset1 The pin's offset in PlacementUnit 1
         * @param movable0 whether the object 0 is movable
         * @param movable1 whether the object 0 is movable
         * @param w the weight of the net
         */
        inline void addB2BNet(std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                              std::vector<float> &objectiveMatrixDiag, Eigen::VectorXd &objectiveVector, int puId0,
                              int puId1, float pos0, float pos1, float pinOffset0, float pinOffset1, bool movable0,
                              bool movable1, float w)
        {
            // min_x 0.5 * x'Px + q'x
            // s.t.  l <= Ax <= u
            if (puId0 == puId1)
                return;
            if (movable0 && movable1)
            {
                // x0^2 + x1^2 - 2x0x1 + 2(x0c-x1c)x0 + 2(x1c-x0c)x1
                // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId0, w));
                // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId1, w));
                objectiveMatrixDiag[puId0] += w;
                objectiveMatrixDiag[puId1] += w;
                objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId1, -w));
                objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId0, -w));
                if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
                {
                    // 2(x0c-x1c)x0 + 2(x1c-x0c)x1
                    objectiveVector[puId0] += w * (pinOffset0 - pinOffset1);
                    objectiveVector[puId1] += w * (pinOffset1 - pinOffset0);
                }
            }
            else if (movable0)
            {
                // x0^2 - 2x0x1 + 2(x0c-x1c)x0
                // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId0, w));
                objectiveMatrixDiag[puId0] += w;
                objectiveVector[puId0] += -w * pos1;
                if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
                {
                    // 2(x0c-x1c)x0
                    objectiveVector[puId0] += w * (pinOffset0 - pinOffset1);
                }
            }
            else if (movable1)
            {
                // x1^2 - 2x0x1 + 2(x1c-x0c)x1
                // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId1, w));
                objectiveMatrixDiag[puId1] += w;
                objectiveVector[puId1] += -w * pos0;
                if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
                {
                    // 2(x1c-x0c)x1
                    objectiveVector[puId1] += w * (pinOffset1 - pinOffset0);
                }
            }
        }

        inline std::set<PlacementUnit *> &getPUSet()
        {
            return PUSet;
        }

        inline float getLeftPinX()
        {
            assert(leftPuId >= 0);
            return leftPinX;
        }

        inline float getRightPinX()
        {
            assert(rightPuId >= 0);
            return rightPinX;
        }

        inline float getTopPinY()
        {
            assert(topPuId >= 0);
            return topPinY;
        }

        inline float getBottomPinY()
        {
            assert(bottomPuId >= 0);
            return bottomPinY;
        }

        inline int getLeftPUId()
        {
            assert(leftPuId >= 0);
            return leftPuId;
        }

        inline int getRightPUId()
        {
            assert(rightPuId >= 0);
            return rightPuId;
        }

        inline int getTopPUId()
        {
            assert(topPuId >= 0);
            return topPuId;
        }

        inline int getBottomPUId()
        {
            assert(bottomPuId >= 0);
            return bottomPuId;
        }

        inline bool isGlobalClock()
        {
            assert(designNet);
            return designNet->checkIsGlobalClock();
        }

      private:
        DesignInfo::DesignNet *designNet = nullptr;
        std::vector<PlacementUnit *> unitsOfNetPins;
        std::vector<PlacementUnit *> unitsOfDriverPins;
        std::vector<PlacementUnit *> unitsOfPinsBeDriven;
        std::vector<pinOffset> pinOffsetsInUnit;
        std::set<PlacementUnit *> PUSet;
        int id;
        float leftPUX, rightPUX, topPUY, bottomPUY;
        float leftPinX, rightPinX, topPinY, bottomPinY;
        unsigned int leftPuId, rightPuId, topPuId, bottomPuId;
        unsigned int leftPinId_net, rightPinId_net, topPinId_net, bottomPinId_net;
        float eps = 1e-5;
        float minDist = 1;
    };

    /**
     * @brief a group of PlacementUnits
     *
     * record the rought resource demand and PlacementUnits inside it. Usually used for initial processing (initial
     * placement.)
     *
     */
    class ClusterUnit
    {
      public:
        ClusterUnit(int id) : id(id)
        {
            PUs.clear();
            totalWeight = 0;
            totalBRAMNum = 0;
            totalDSPNum = 0;
        }
        ~ClusterUnit(){};
        inline int getWeight()
        {
            return totalWeight;
        };
        inline int getBRAMNum()
        {
            return totalBRAMNum;
        };
        inline int getDSPNum()
        {
            return totalDSPNum;
        };

        inline void addPlacementUnit(PlacementInfo::PlacementUnit *curPU)
        {
            PUs.push_back(curPU);
            totalWeight += curPU->getWeight();
            totalBRAMNum += curPU->getBRAMNum();
            totalDSPNum += curPU->getDSPNum();
        }

        inline std::vector<PlacementInfo::PlacementUnit *> &getUnits()
        {
            return PUs;
        }

        inline int getId()
        {
            return id;
        }

      private:
        std::vector<PlacementInfo::PlacementUnit *> PUs;
        int totalWeight, totalBRAMNum, totalDSPNum;
        int id;
    };

    /**
     * @brief The net between the objects of ClusterUnit class
     *
     */
    class ClusterNet
    {
      public:
        ClusterNet(int id) : id(id)
        {
            clusterUnits.clear();
        }
        ~ClusterNet(){};
        inline std::vector<ClusterUnit *> &getUnits()
        {
            return clusterUnits;
        }

        inline int getId()
        {
            return id;
        }

        inline void addClusterUnit(ClusterUnit *tmpCU)
        {
            clusterUnits.push_back(tmpCU);
        }

      private:
        std::vector<ClusterUnit *> clusterUnits;
        int id;
    };

    /**
     * @brief Site bin for global placement for some specific Site types
     *
     * Currently it is not used in the implementation.
     *
     */
    class PlacementSiteBinInfo
    {
      public:
        PlacementSiteBinInfo(float leftX, float rightX, float bottomY, float topY, int row, int column)
            : leftX(leftX), rightX(rightX), topY(topY), bottomY(bottomY), row(row), column(column)

        {
            correspondingSites.clear();
            macros.clear();
        }

        void addSiteIntoBin(DeviceInfo::DeviceSite *curSite);

        ~PlacementSiteBinInfo()
        {
            correspondingSites.clear();
            macros.clear();
        }

        inline bool inRange(float x, float y)
        {
            return (x <= rightX && x > leftX && y <= topY && y > bottomY);
        }

        inline bool inRangeY(float y)
        {
            return (y <= topY && y > bottomY);
        }

        inline void addMacroSite(PlacementMacro *curMacro, float occupationAdded)
        {
            if (curMacro)
                macros.push_back(curMacro);
            utilization += occupationAdded;
        }

        inline void reset()
        {
            macros.clear();
            utilization = 0;
        }

        inline std::vector<PlacementMacro *> &getMacros()
        {
            return macros;
        }

        inline float getUtilizationRate()
        {
            if (capacity == 0)
                return utilization / 0.01;
            assert(capacity != 0);
            return (float)binShrinkRatio * utilization / capacity;
        }

        inline float getUtilization()
        {
            return (float)utilization;
        }

        inline float getCapacity()
        {
            return (float)capacity / binShrinkRatio;
        }

        inline bool isOverflow()
        {
            if (capacity == 0)
            {
                if (utilization == 0)
                    return false;
                else
                    return true;
            }
            assert(capacity != 0);
            return ((float)binShrinkRatio * utilization / capacity) > 1.00 + eps;
        }

        inline bool canAddMore(int BELAmo)
        {
            if (capacity == 0)
                return false;
            return ((float)binShrinkRatio * (utilization + BELAmo) / capacity) <= 1.00 + eps;
        }

        inline void setYX(int i, int j)
        {
            row = i;
            column = j;
        }

        inline int Y()
        {
            return row;
        }
        inline int X()
        {
            return column;
        }

      private:
        std::vector<DeviceInfo::DeviceSite *> correspondingSites;
        std::vector<PlacementMacro *> macros;
        int capacity = 0;
        float utilization = 0;
        float binShrinkRatio = 1.0;
        float leftX;
        float rightX;
        float topY;
        float bottomY;
        float eps = 1e-5;
        int row;
        int column;
    };

    /**
     * @brief Construct a new Placement Info object based on the information of design and device
     *
     * @param designInfo
     * @param deviceInfo
     * @param JSONCfg user-defined placement configuration
     */
    PlacementInfo(DesignInfo *designInfo, DeviceInfo *deviceInfo, std::map<std::string, std::string> &JSONCfg);
    ~PlacementInfo()
    {
        delete compatiblePlacementTable;
        for (auto typeGrid : SharedBELTypeBinGrid)
            for (auto curRow : typeGrid)
                for (auto curBin : curRow)
                    delete curBin;
        for (auto curRow : siteGridForMacros)
            for (auto curBin : curRow)
                delete curBin;
        for (auto pn : placementNets)
            delete pn;
    }

    void printStat(bool verbose = false);

    /**
     * @brief describes the type mapping from design to device, where a cell can be placed (which BEL in which site)
     *
     * Since different cell types can be mapped to a group of resource BEL types, we handle the mapping in the following
     * way, with a intermediate Shared BEL Type:
     *
     * cell type A  =>     A     =>   BEL type 1
     *
     * cell type B  =>  Shared   =>   BEL type 2
     *
     * cell type C  =>    BEL    =>   BEL type 3
     *
     * cell type D  =>   Type    =>   BEL type 4
     *
     * @param cellType2fixedAmoFileName a file indicates how many slot will a cell of specific type cost
     * @param cellType2sharedCellTypeFileName a file indicates the mapping from cell types to shared BEL types
     * @param sharedCellType2BELtypeFileName a file indicates the mapping from shared BEL types to resource BEL types
     * @return CompatiblePlacementTable*
     */
    CompatiblePlacementTable *loadCompatiblePlacementTable(std::string cellType2fixedAmoFileName,
                                                           std::string cellType2sharedCellTypeFileName,
                                                           std::string sharedCellType2BELtypeFileName)
    {
        return new CompatiblePlacementTable(cellType2fixedAmoFileName, cellType2sharedCellTypeFileName,
                                            sharedCellType2BELtypeFileName, designInfo, deviceInfo);
    }

    inline CompatiblePlacementTable *getCompatiblePlacementTable()
    {
        return compatiblePlacementTable;
    }

    void setBELTypeForCells(DesignInfo *designInfo)
    {
        compatiblePlacementTable->setBELTypeForCells(designInfo);
    }

    /**
     * @brief Get the Min X of sites to identify the boundary of the device
     *
     * @param sites
     * @return float
     */
    float getMinXFromSites(std::vector<DeviceInfo::DeviceSite *> &sites);

    /**
     * @brief Get the Min Y of sites to identify the boundary of the device
     *
     * @param sites
     * @return float
     */
    float getMinYFromSites(std::vector<DeviceInfo::DeviceSite *> &sites);

    /**
     * @brief Get the Max X of sites to identify the boundary of the device
     *
     * @param sites
     * @return float
     */
    float getMaxXFromSites(std::vector<DeviceInfo::DeviceSite *> &sites);

    /**
     * @brief Get the Max Y of sites to identify the boundary of the device
     *
     * @param sites
     * @return float
     */
    float getMaxYFromSites(std::vector<DeviceInfo::DeviceSite *> &sites);

    /**
     * @brief Create a grid of bins on the device
     *
     * @param binWidth the width of each bin
     * @param binHeight  the height of each bin
     */
    void createGridBins(float binWidth, float binHeight);
    void createSiteBinGrid();

    /**
     * @brief update PlacementNet objects when there are some updates of PlacementUnit objects (e.g., some cells are
     * packed)
     *
     */
    void reloadNets();

    /**
     * @brief update the long path in the design and enhance their net weights
     *
     */
    void updateLongPaths();

    /**
     * @brief verify that each cells in the design can be mapped on the resource elements on the device.
     *
     */
    void verifyDeviceForDesign();

    inline std::vector<PlacementUnit *> &getPlacementUnits()
    {
        return placementUnits;
    }
    inline std::vector<PlacementMacro *> &getPlacementMacros()
    {
        return placementMacros;
    }
    inline std::vector<PlacementUnit *> &getFixedPlacementUnits()
    {
        return fixedPlacementUnits;
    }
    inline std::vector<PlacementNet *> &getPlacementNets()
    {
        return placementNets;
    }
    inline std::set<DesignInfo::DesignCell *> &getCellInMacros()
    {
        return cellInMacros;
    }
    inline std::map<int, PlacementUnit *> &getCellId2PlacementUnit()
    {
        return cellId2PlacementUnit;
    }
    inline std::vector<PlacementUnit *> &getCellId2PlacementUnitVec()
    {
        return cellId2PlacementUnitVec;
    }
    inline std::vector<PlacementUnpackedCell *> &getPlacementUnpackedCells()
    {
        return placementUnpackedCells;
    }
    inline int getNumCells()
    {
        return designInfo->getNumCells();
    }

    /**
     * @brief Get the Global Max X (right boundary of the device)
     *
     * @return float
     */
    inline float getGlobalMaxX()
    {
        return globalMaxX;
    }

    /**
     * @brief Get the Global Max Y (top boundary of the device)
     *
     * @return float
     */
    inline float getGlobalMaxY()
    {
        return globalMaxY;
    }

    /**
     * @brief Get the Global Min X (left boundary of the device)
     *
     * @return float
     */
    inline float getGlobalMinX()
    {
        return globalMinX;
    }

    /**
     * @brief Get the Global Min Y (bottom boundary of the device)
     *
     * @return float
     */
    inline float getGlobalMinY()
    {
        return globalMinY;
    }

    /**
     * @brief get right boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    inline float getGlobalBinMaxLocX()
    {
        return endX;
    }

    /**
     * @brief get top boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    inline float getGlobalBinMaxLocY()
    {
        return endY;
    }

    /**
     * @brief get left boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    inline float getGlobalBinMinLocX()
    {
        return startX;
    }

    /**
     * @brief get bottom boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    inline float getGlobalBinMinLocY()
    {
        return startY;
    }

    inline float getDeviceMaxEdgeLength()
    {
        return std::max(endX - startX, endY - startY);
    }

    inline std::vector<int> &getPotentialBELTypeIDs(DesignInfo::DesignCell *cell)
    {
        return compatiblePlacementTable->getPotentialBELTypeIDs(cell);
    }

    inline std::vector<int> &getPotentialBELTypeIDs(DesignInfo::DesignCellType cellType)
    {
        return compatiblePlacementTable->getPotentialBELTypeIDs(cellType);
    }

    inline int getSharedBELTypeId(std::string tmpStr)
    {
        return compatiblePlacementTable->getSharedBELTypeId(tmpStr);
    }

    /**
     * @brief  Get the actual occupation of a specific cell
     *
     * @param cell
     * @return float
     *
     * it will be the multiplication of cellId2Occupation and cellId2InfationRatio.
     * cellId2Occupation might be adjusted by the packing feasibility.
     * cellId2InfationRatio might be adjusted by the routing congestion level.
     */
    inline float getActualOccupation(DesignInfo::DesignCell *cell)
    {
        return compatiblePlacementTable->getActualOccupation(cell);
    }

    /**
     * @brief Get the inflate ratio of a cell
     *
     * @param cell
     * @return float
     */
    inline float getInflateRatio(DesignInfo::DesignCell *cell)
    {
        return compatiblePlacementTable->getInflateRatio(cell);
    }

    inline std::vector<float> &getcellId2Occupation()
    {
        return compatiblePlacementTable->getcellId2Occupation();
    }

    /**
     * @brief Get the theoratical occupation of a specific cell type
     *
     * @param cellType
     * @return float
     */
    inline float getOccupation(DesignInfo::DesignCellType cellType)
    {
        return compatiblePlacementTable->getOccupation(cellType);
    }

    /**
     * @brief Get the Actual Occupation By Cell Id
     *
     * @param id
     * @return float
     */
    inline float getActualOccupationByCellId(int id)
    {
        return compatiblePlacementTable->getActualOccupationByCellId(id);
    }

    /**
     * @brief find neibor LUTs/FFs from bin grid
     *
     * @param curCell target Cell
     * @param displacementUpperbound displacement threshold
     * @param minNumNeighbor currently not used
     * @return std::vector<DesignInfo::DesignCell *>*
     */
    inline std::vector<DesignInfo::DesignCell *> *
    findNeiborLUTFFsFromBinGrid(DesignInfo::DesignCell *curCell, float displacementUpperbound, int minNumNeighbor = 10)
    {
        // please note that the input DesignCell is only used to find the corresponding binGrid for site search.
        bool findLUT = curCell->isLUT();
        float targetX = cellId2location[curCell->getCellId()].X;
        float targetY = cellId2location[curCell->getCellId()].Y;
        std::vector<DesignInfo::DesignCell *> *res = new std::vector<DesignInfo::DesignCell *>();
        res->clear();

        int binIdX, binIdY;
        getGridXY(targetX, targetY, binIdX, binIdY);

        assert(binIdY >= 0);
        assert((unsigned int)binIdY < LUTFFBinGrid.size());
        assert(binIdX >= 0);
        assert((unsigned int)binIdX < LUTFFBinGrid[binIdY].size());
        assert(LUTFFBinGrid[binIdY][binIdX]->inRange(targetX, targetY));

        std::queue<std::pair<int, int>> binXYqueue;
        std::set<std::pair<int, int>> reachedBinXYs;
        binXYqueue.emplace(binIdX, binIdY);
        reachedBinXYs.emplace(binIdX, binIdY);

        bool findItself = false;

        while (binXYqueue.size() > 0)
        {
            std::pair<int, int> curXY = binXYqueue.front();
            binXYqueue.pop();
            int curbinIdX = curXY.first, curbinIdY = curXY.second;

            PlacementInfo::PlacementBinInfo *curBin = LUTFFBinGrid[curbinIdY][curbinIdX];
            float bin2TargetXYDistance = curBin->getManhattanDistanceTo(targetX, targetY);
            if (bin2TargetXYDistance > displacementUpperbound)
                continue;
            for (auto tmpCell : curBin->getCells())
            {
                if ((tmpCell->isLUT() && findLUT) || (tmpCell->isFF() && !findLUT))
                {
                    float tmpX = cellId2location[tmpCell->getCellId()].X;
                    float tmpY = cellId2location[tmpCell->getCellId()].Y;
                    float tmpPUDis = fabs(targetX - tmpX) + y2xRatio * fabs(targetY - tmpY);
                    if (tmpPUDis <= displacementUpperbound)
                    {
                        if (tmpCell == curCell)
                        {
                            findItself = true;
                        }
                        res->push_back(tmpCell);
                    }
                }
            }
            assert(findItself);
            for (int nextY = curbinIdY - 1; nextY <= curbinIdY + 1; nextY++)
            {
                for (int nextX = curbinIdX - 1; nextX <= curbinIdX + 1; nextX++)
                {
                    if (!(nextY >= 0))
                        continue;
                    if (!((unsigned int)nextY < LUTFFBinGrid.size()))
                        continue;
                    if (!(nextX >= 0))
                        continue;
                    if (!((unsigned int)nextX < LUTFFBinGrid[binIdY].size()))
                        continue;
                    PlacementInfo::PlacementBinInfo *nextBin = LUTFFBinGrid[nextY][nextX];
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

        assert(findItself);
        return res;
    }

    /**
     * @brief Get the Bin Grid object
     *
     * @param BELTypeId indicate the target BELtype ID, since we map the different resource to different grid for easier
     * processing
     * @return std::vector<std::vector<PlacementBinInfo *>>&
     */
    inline std::vector<std::vector<PlacementBinInfo *>> &getBinGrid(unsigned int BELTypeId)
    {
        assert(BELTypeId < SharedBELTypeBinGrid.size());
        return SharedBELTypeBinGrid[BELTypeId];
    }

    /**
     * @brief Get the Bin Grid object for all types of BEL
     *
     * @return std::vector<std::vector<std::vector<PlacementBinInfo *>>>&
     */
    inline std::vector<std::vector<std::vector<PlacementBinInfo *>>> &getBinGrid()
    {
        return SharedBELTypeBinGrid;
    }

    inline std::vector<std::vector<PlacementSiteBinInfo *>> &getSiteBinGrid()
    {
        return siteGridForMacros;
    }

    inline PlacementUnit *getPlacementUnitByCell(DesignInfo::DesignCell *curCell)
    {
        assert(curCell);
        assert((unsigned int)curCell->getCellId() < cellId2PlacementUnitVec.size());
        return cellId2PlacementUnitVec[curCell->getCellId()];
    }

    inline PlacementUnit *getPlacementUnitByCellId(int cellId)
    {
        assert(cellId >= 0);
        assert((unsigned int)cellId < cellId2PlacementUnitVec.size());
        return cellId2PlacementUnitVec[cellId];
    }

    /**
     * @brief directly set weight in the quadratic Matrix and vector according to given request.
     *
     * Usually B2B Net will be called inside PlacementNet, but we set this API to handle pseudo net settings.
     *
     * @param objectiveMatrixTripletList The non-Diag elements in matrix Q, stored in the vector of Eigen Triplet
     * (i,j,val)
     * @param objectiveMatrixDiag The Diag elements in matrix Q, stored in a 1-D vector
     * @param objectiveVector The elements in the vector P
     * @param puId0 PlacementUnit 0's Id (might be invaid -1)
     * @param puId1 PlacementUnit 1's Id (might be invaid -1)
     * @param pos0 PlacementUnit 0's position on one of the dimensions
     * @param pos1 PlacementUnit 1's position on one of the dimensions
     * @param pinOffset0 The pin's offset in PlacementUnit 0
     * @param pinOffset1 The pin's offset in PlacementUnit 1
     * @param movable0 whether the object 0 is movable
     * @param movable1 whether the object 0 is movable
     * @param w the weight of the net
     */
    inline void addB2BNetInPlacementInfo(std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                         std::vector<float> &objectiveMatrixDiag, Eigen::VectorXd &objectiveVector,
                                         int puId0, int puId1, float pos0, float pos1, float pinOffset0,
                                         float pinOffset1, bool movable0, bool movable1, float w)
    {
        // min_x 0.5 * x'Px + q'x
        // s.t.  l <= Ax <= u
        if (puId0 == puId1)
            return;
        if (movable0 && movable1)
        {
            // x0^2 + x1^2 - 2x0x1 + 2(x0c-x1c)x0 + 2(x1c-x0c)x1
            // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId0, w));
            // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId1, w));
            objectiveMatrixDiag[puId0] += w;
            objectiveMatrixDiag[puId1] += w;
            objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId1, -w));
            objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId0, -w));
            if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
            {
                // 2(x0c-x1c)x0 + 2(x1c-x0c)x1
                objectiveVector[puId0] += w * (pinOffset0 - pinOffset1);
                objectiveVector[puId1] += w * (pinOffset1 - pinOffset0);
            }
        }
        else if (movable0)
        {
            // x0^2 - 2x0x1 + 2(x0c-x1c)x0
            // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId0, puId0, w));
            objectiveMatrixDiag[puId0] += w;
            objectiveVector[puId0] += -w * pos1;
            if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
            {
                // 2(x0c-x1c)x0
                objectiveVector[puId0] += w * (pinOffset0 - pinOffset1);
            }
        }
        else if (movable1)
        {
            // x1^2 - 2x0x1 + 2(x1c-x0c)x1
            // objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(puId1, puId1, w));
            objectiveMatrixDiag[puId1] += w;
            objectiveVector[puId1] += -w * pos0;
            if (fabs(pinOffset0) > eps || fabs(pinOffset1) > eps)
            {
                // 2(x1c-x0c)x1
                objectiveVector[puId1] += w * (pinOffset1 - pinOffset0);
            }
        }
    }

    /**
     * @brief  directly set weight in the quadratic Matrix and vector according to given request.
     *
     * Usually B2B Net will be called inside PlacementNet, but we set this API to handle pseudo net settings.
     *
     * @param objectiveMatrixTripletList The non-Diag elements in matrix Q, stored in the vector of Eigen Triplet
     * (i,j,val)
     * @param objectiveMatrixDiag The Diag elements in matrix Q, stored in a 1-D vector
     * @param objectiveVector The elements in the vector P
     * @param tmpPU PlacementUnit which need a pseudo net
     * @param targetLoc the location of the anchor which the pseudo net connected to
     * @param pseudoWeight pseudo net weight to constrain the movement of PlacementUnits from their locations in
     * last optimization iteration
     * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
     * in X-coordinate
     * @param updateX update the X-coordinate term in the quadratic problem
     * @param updateY update the X-coordinate term in the quadratic problem
     */
    inline void addPseudoNetsInPlacementInfo(std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                             std::vector<float> &objectiveMatrixDiag, Eigen::VectorXd &objectiveVector,
                                             PlacementUnit *tmpPU, float targetLoc, float pseudoWeight, float y2xRatio,
                                             bool updateX, bool updateY)
    {
        assert(updateX ^ updateY);
        if (updateY)
        {
            pseudoWeight *= y2xRatio;
        }
        bool movable = !tmpPU->isFixed();
        if (movable)
        {
            addB2BNetInPlacementInfo(objectiveMatrixTripletList, objectiveMatrixDiag, objectiveVector, tmpPU->getId(),
                                     -1, targetLoc, targetLoc, 0, 0, true, false, pseudoWeight);
        }
    }

    /**
     * @brief update the mapping from Cells to PlacementUnits, since sometime, PlacementUnits might change
     *
     */
    void updateCells2PlacementUnits();

    /**
     * @brief map design cells to the bins in the bin grid.
     *
     */
    void updateElementBinGrid();

    /**
     * @brief adjust the resource demand of LUTs/FFs according to routing congestion
     * refer to RippleFPGA's implementation
     * @param enfore enfore adjustment without considering other factors
     */
    void adjustLUTFFUtilization_Routability(bool enfore);

    /**
     * @brief reset the inflate ratio of all the cells to be 1, for re-evaluation
     *
     */
    void adjustLUTFFUtilization_Routability_Reset();

    /**
     * @brief adjust the resource demand of LUTs/FFs according to packing feasibility
     *
     * @param neighborDisplacementUpperbound set displacement threshold to identify the neighbors of a cell
     * @param enfore  enfore adjustment without considering other factors
     */
    void adjustLUTFFUtilization_Packablity(float neighborDisplacementUpperbound, bool enfore);

    /**
     * @brief adjust the resource demand of LUTs/FFs according to packing feasibility and  routing congestion
     *
     * @param neighborDisplacementUpperbound set displacement threshold to identify the neighbors of a cell
     * @param enfore  enfore adjustment without considering other factors
     */
    void adjustLUTFFUtilization(float neighborDisplacementUpperbound, bool enfore = false);

    /**
     * @brief adjust the utlization of clock-related elements to mitigate the overflow of clock utilization
     *
     */
    void adjustLUTFFUtilization_Clocking();

    /**
     * @brief clean the information in bin grid
     *
     */
    void resetElementBinGrid();

    void updateSiteBinGrid();

    void resetSiteBinGrid();

    inline DesignInfo *getDesignInfo()
    {
        return designInfo;
    }

    inline DeviceInfo *getDeviceInfo()
    {
        return deviceInfo;
    }

    inline PlacementTimingInfo *getTimingInfo()
    {
        return simplePlacementTimingInfo;
    }

    /**
     * @brief get the remapped BEL type of a specific BEL type
     * since some cell can be placed in sites of different sites. For cell spreading, we need to remap some BEL types to
     * a unified BEL types. Belows are some examples:
     *
     * SLICEM_CARRY8 => SLICEL_CARRY8
     *
     * SLICEM_LUT => SLICEL_LUT
     *
     * SLICEM_FF => SLICEL_FF
     * @param curBELType
     * @return std::string
     */
    inline std::string getBELType2FalseBELType(std::string curBELType)
    {
        return deviceInfo->getBELType2FalseBELType(curBELType);
    }

    /**
     * @brief Get the Grid row/column based on given location X,Y
     *
     * @param cellX input X
     * @param cellY input Y
     * @param binIdX output target column of grid
     * @param binIdY output target row of grid
     */
    inline void getGridXY(float cellX, float cellY, int &binIdX, int &binIdY)
    {

        float coord_offsetX = cellX - startX;
        float coord_offsetY = cellY - startY;
        binIdX = static_cast<int>((coord_offsetX) / binWidth);
        binIdY = static_cast<int>((coord_offsetY) / binHeight);
    }

    /**
     * @brief move the PlacementUnit to ensure the cells in it are within the device area.
     *
     * @param curPU target PlacementUnit
     * @param fX output X of PlacementUnit that can ensure the cells in it are within the device area.
     * @param fY output Y of PlacementUnit that can ensure the cells in it are within the device area.
     */
    inline void legalizeXYInArea(PlacementUnit *curPU, float &fX, float &fY)
    {
        if (curPU->getType() == PlacementUnitType_UnpackedCell)
        {
            fX = std::max(globalMinX + eps, (std::min(fX, globalMaxX - eps)));
            fY = std::max(globalMinY + eps, (std::min(fY, globalMaxY - eps)));
        }
        else if (auto curMacro = dynamic_cast<PlacementMacro *>(curPU))
        {
            if (fY + curMacro->getTopOffset() > globalMaxY - eps)
            {
                fY = (globalMaxY - 2 * eps) - curMacro->getTopOffset();
            }
            else if (fY + curMacro->getBottomOffset() < globalMinY + eps)
            {
                fY = (globalMinY + 2 * eps) - curMacro->getBottomOffset();
            }

            if (fX + curMacro->getRightOffset() > globalMaxX - eps)
            {
                fX = (globalMaxX - 2 * eps) - curMacro->getRightOffset();
            }
            else if (fX + curMacro->getLeftOffset() < globalMinX + eps)
            {
                fX = (globalMinX + 2 * eps) - curMacro->getLeftOffset();
            }
        }
        else
        {
            assert(false && "wrong placement unit type");
        }
    }

    /**
     * @brief move the PlacementUnit to ensure the cells in it are within the device area.
     *
     * @param curPU target PlacementUnit
     */
    inline void enforceLegalizeXYInArea(PlacementUnit *curPU)
    {
        float fX = curPU->X();
        float fY = curPU->Y();
        if (curPU->getType() == PlacementUnitType_UnpackedCell)
        {
            fX = std::max(globalMinX + eps, (std::min(fX, globalMaxX - eps)));
            fY = std::max(globalMinY + eps, (std::min(fY, globalMaxY - eps)));
        }
        else if (auto curMacro = dynamic_cast<PlacementMacro *>(curPU))
        {
            if (fY + curMacro->getTopOffset() > globalMaxY - eps)
            {
                fY = (globalMaxY - 2 * eps) - curMacro->getTopOffset();
            }
            else if (fY + curMacro->getBottomOffset() < globalMinY + eps)
            {
                fY = (globalMinY + 2 * eps) - curMacro->getBottomOffset();
            }

            if (fX + curMacro->getRightOffset() > globalMaxX - eps)
            {
                fX = (globalMaxX - 2 * eps) - curMacro->getRightOffset();
            }
            else if (fX + curMacro->getLeftOffset() < globalMinX + eps)
            {
                fX = (globalMinX + 2 * eps) - curMacro->getLeftOffset();
            }
        }
        else
        {
            assert(false && "wrong placement unit type");
        }
        if (fX != curPU->X() || fY != curPU->Y())
        {
            curPU->setAnchorLocationAndForgetTheOriginalOne(fX, fY);
        }
    }

    /**
     * @brief check whether the PlacementUnit is legalized in the device area when a cell in it is placed at target
     * location
     *
     * @param curCell
     * @param targetX
     * @param targetY
     * @return true  the PlacementUnit is legalized in the device area when a cell in it is placed at target location
     * @return false  the PlacementUnit cannot be legalized in the device area when a cell in it is placed at target
     * location
     */
    inline bool isLegalLocation(DesignInfo::DesignCell *curCell, float targetX, float targetY)
    {
        auto curPU = cellId2PlacementUnit[curCell->getCellId()];

        if (curPU->getType() == PlacementUnitType_UnpackedCell)
        {
            float fX = targetX;
            float fY = targetY;
            fX = std::max(globalMinX + eps, (std::min(fX, globalMaxX - eps)));
            fY = std::max(globalMinY + eps, (std::min(fY, globalMaxY - eps)));
            return (std::fabs(fX - targetX) + std::fabs(fY - targetY)) < eps;
        }
        else if (auto curMacro = dynamic_cast<PlacementMacro *>(curPU))
        {
            float offsetX = curMacro->getCellOffsetXInMacro(curCell);
            float offsetY = curMacro->getCellOffsetYInMacro(curCell);
            float fX = targetX - offsetX;
            float fY = targetY - offsetY;
            if (fY + curMacro->getTopOffset() > globalMaxY - eps)
            {
                fY = (globalMaxY - 2 * eps) - curMacro->getTopOffset();
            }
            else if (fY + curMacro->getBottomOffset() < globalMinY + eps)
            {
                fY = (globalMinY + 2 * eps) - curMacro->getBottomOffset();
            }

            if (fX + curMacro->getRightOffset() > globalMaxX - eps)
            {
                fX = (globalMaxX - 2 * eps) - curMacro->getRightOffset();
            }
            else if (fX + curMacro->getLeftOffset() < globalMinX + eps)
            {
                fX = (globalMinX + 2 * eps) - curMacro->getLeftOffset();
            }
            return (std::fabs(fX + offsetX - targetX) + std::fabs(fY + offsetY - targetY)) < eps;
        }
        else
        {
            assert(false && "should not reach here");
            return false;
        }
    }

    /**
     * @brief check whether the PlacementUnit is legalized in the device area when it is placed at target location
     *
     * @param curPU
     * @param targetX
     * @param targetY
     * @return true  the PlacementUnit is legalized in the device area when a cell in it is placed at target location
     * @return false  the PlacementUnit cannot be legalized in the device area when a cell in it is placed at target
     * location
     */
    inline bool isLegalLocation(PlacementUnit *curPU, float targetX, float targetY)
    {
        if (curPU->getType() == PlacementUnitType_UnpackedCell)
        {
            float fX = targetX;
            float fY = targetY;
            fX = std::max(globalMinX + eps, (std::min(fX, globalMaxX - eps)));
            fY = std::max(globalMinY + eps, (std::min(fY, globalMaxY - eps)));
            return (std::fabs(fX - targetX) + std::fabs(fY - targetY)) < eps;
        }
        else if (auto curMacro = dynamic_cast<PlacementMacro *>(curPU))
        {
            float fX = targetX;
            float fY = targetY;
            if (fY + curMacro->getTopOffset() > globalMaxY - eps)
            {
                fY = (globalMaxY - 2 * eps) - curMacro->getTopOffset();
            }
            else if (fY + curMacro->getBottomOffset() < globalMinY + eps)
            {
                fY = (globalMinY + 2 * eps) - curMacro->getBottomOffset();
            }

            if (fX + curMacro->getRightOffset() > globalMaxX - eps)
            {
                fX = (globalMaxX - 2 * eps) - curMacro->getRightOffset();
            }
            else if (fX + curMacro->getLeftOffset() < globalMinX + eps)
            {
                fX = (globalMinX + 2 * eps) - curMacro->getLeftOffset();
            }
            return (std::fabs(fX - targetX) + std::fabs(fY - targetY)) < eps;
        }
        else
        {
            assert(false && "should not reach here");
            return false;
        }
    }

    inline void getPULocationByCellLocation(DesignInfo::DesignCell *curCell, float targetX, float targetY, float &PUX,
                                            float &PUY)
    {
        auto curPU = cellId2PlacementUnit[curCell->getCellId()];

        if (curPU->getType() == PlacementUnitType_UnpackedCell)
        {
            float fX = targetX;
            float fY = targetY;
            fX = std::max(globalMinX + eps, (std::min(fX, globalMaxX - eps)));
            fY = std::max(globalMinY + eps, (std::min(fY, globalMaxY - eps)));
            PUX = fX;
            PUY = fY;
        }
        else if (auto curMacro = dynamic_cast<PlacementMacro *>(curPU))
        {
            float offsetX = curMacro->getCellOffsetXInMacro(curCell);
            float offsetY = curMacro->getCellOffsetYInMacro(curCell);
            float fX = targetX - offsetX;
            float fY = targetY - offsetY;
            if (fY + curMacro->getTopOffset() > globalMaxY - eps)
            {
                fY = (globalMaxY - 2 * eps) - curMacro->getTopOffset();
            }
            else if (fY + curMacro->getBottomOffset() < globalMinY + eps)
            {
                fY = (globalMinY + 2 * eps) - curMacro->getBottomOffset();
            }

            if (fX + curMacro->getRightOffset() > globalMaxX - eps)
            {
                fX = (globalMaxX - 2 * eps) - curMacro->getRightOffset();
            }
            else if (fX + curMacro->getLeftOffset() < globalMinX + eps)
            {
                fX = (globalMinX + 2 * eps) - curMacro->getLeftOffset();
            }
            PUX = fX;
            PUY = fY;
        }
        else
        {
            PUX = -1;
            PUY = -1;
            assert(false && "should not reach here.");
        }
    }

    typedef struct Location
    {
        float X = -10;
        float Y = -10;
    } Location;

    inline std::vector<Location> &getCellId2location()
    {
        return cellId2location;
    }

    inline std::vector<Location> &getPinId2location()
    {
        return pinId2location;
    }

    /**
     * @brief Get the width of a bin in grid
     *
     * @return float
     */
    inline float getBinGridW()
    {
        return binWidth;
    }

    /**
     * @brief Get the height of a bin in grid
     *
     * @return float
     */
    inline float getBinGridH()
    {
        return binHeight;
    }

    /**
     * @brief record the bin information for a cell (BELtype, column/row, resource demand)
     *
     */
    typedef struct CellBinInfo
    {
        int sharedTypeId = -1;
        int X = -1;
        int Y = -1;
        float occupation = -1;
    } CellBinInfo;

    /**
     * @brief set the legalization of some PlacementUnit objects
     *
     * @param PU2X X of PlacementUnits
     * @param PU2Y Y of PlacementUnits
     */
    inline void setPULegalXY(std::map<PlacementInfo::PlacementUnit *, float> &PU2X,
                             std::map<PlacementInfo::PlacementUnit *, float> &PU2Y)
    {
        for (auto tmpPair : PU2X) // only update elements in PU2X and PU2Y
        {
            PULegalXY.first[tmpPair.first] = tmpPair.second;
        }
        for (auto tmpPair : PU2Y)
        {

            PULegalXY.second[tmpPair.first] = tmpPair.second;
        }
    }

    /**
     * @brief set the sites occupied by the PlacementUnit objects
     *
     * @param PU2Sites a mapping from PlaceuementUnit objects to device sites
     */
    inline void
    setPULegalSite(std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> &PU2Sites)
    {
        for (auto tmpPair : PU2Sites) // only update elements in PU2X and PU2Y
        {
            PU2LegalSites[tmpPair.first] = tmpPair.second;
        }
    }

    /**
     * @brief get the sites occupied by the legalized PlacementUnit objects
     *
     * @return std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>>&
     */
    inline std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> &getPULegalSite()
    {
        return PU2LegalSites;
    }

    /**
     * @brief get the locations (pair of X,Y) of the legalized PlacementUnit objects
     *
     * @return std::pair<std::map<PlacementInfo::PlacementUnit *, float>, std::map<PlacementInfo::PlacementUnit *,
     * float>>&
     */
    inline std::pair<std::map<PlacementInfo::PlacementUnit *, float>, std::map<PlacementInfo::PlacementUnit *, float>> &
    getPULegalXY()
    {
        return PULegalXY;
    }

    /**
     * @brief forget all the legalization information
     *
     */
    void resetPULegalInformation()
    {
        PULegalXY.first = std::map<PlacementInfo::PlacementUnit *, float>();
        PULegalXY.second = std::map<PlacementInfo::PlacementUnit *, float>();
        PU2LegalSites.clear();
    }

    /**
     * @brief remove the legalization information of a PlacementUnit object
     *
     * @param curPU
     */
    inline void deleteLegalizationInfoFor(PlacementInfo::PlacementUnit *curPU)
    {
        if (PU2LegalSites.find(curPU) != PU2LegalSites.end())
            PU2LegalSites.erase(curPU);
        if (PULegalXY.first.find(curPU) != PULegalXY.first.end())
            PULegalXY.first.erase(curPU);
        if (PULegalXY.second.find(curPU) != PULegalXY.second.end())
            PULegalXY.second.erase(curPU);
    }

    /**
     * @brief Set the cell bin Information of a design cell
     *
     * @param cellId the Id of the design cell
     * @param sharedTypeId which BEL type the design cell is
     * @param X the column in the grid of the bin which the cell is located in
     * @param Y the row in the grid of the bin which the cell is located in
     * @param occupation how much resource is cost by the design cell
     */
    inline void setCellBinInfo(int cellId, int sharedTypeId, int X, int Y, float occupation)
    {
        assert((unsigned int)cellId < cellId2CellBinInfo.size());
        cellId2CellBinInfo[cellId].sharedTypeId = sharedTypeId;
        cellId2CellBinInfo[cellId].X = X;
        cellId2CellBinInfo[cellId].Y = Y;
        cellId2CellBinInfo[cellId].occupation = occupation;
    }

    /**
     * @brief update the bin information of a design cell when it is moved to a new location
     *
     * When a cell is moved to a new location, corresponding bins should be updated accordingly.
     *
     * @param cellId  the Id of the design cell
     * @param fX the X coordinate the cell is moved to
     * @param fY the Y coordinate the cell is moved to
     */
    inline void transferCellBinInfo(int cellId, float fX, int fY)
    {
        assert((unsigned int)cellId < cellId2CellBinInfo.size());
        int binIdX, binIdY;
        getGridXY(fX, fY, binIdX, binIdY);
        assert(binIdY >= 0);
        assert((unsigned int)binIdY < SharedBELTypeBinGrid[cellId2CellBinInfo[cellId].sharedTypeId].size());
        assert(binIdX >= 0);
        assert((unsigned int)binIdX < SharedBELTypeBinGrid[cellId2CellBinInfo[cellId].sharedTypeId][binIdY].size());
        if (cellId2CellBinInfo[cellId].X == binIdX && cellId2CellBinInfo[cellId].Y == binIdY)
            return;

        assert(cellId2CellBinInfo[cellId].occupation >= 0);
        SharedBELTypeBinGrid[cellId2CellBinInfo[cellId].sharedTypeId][cellId2CellBinInfo[cellId].Y]
                            [cellId2CellBinInfo[cellId].X]
                                ->removeCell(designInfo->getCells()[cellId], cellId2CellBinInfo[cellId].occupation);
        SharedBELTypeBinGrid[cellId2CellBinInfo[cellId].sharedTypeId][binIdY][binIdX]->addCell(
            designInfo->getCells()[cellId], cellId2CellBinInfo[cellId].occupation);
        cellId2CellBinInfo[cellId].X = binIdX;
        cellId2CellBinInfo[cellId].Y = binIdY;
    }

    /**
     * @brief Get the Displacement from a given location to a device site (y2xRatio is considered.)
     *
     * @param fX given X
     * @param fY given Y
     * @param curSite target device site
     * @return float
     */
    inline float getDisplacement(float fX, float fY, DeviceInfo::DeviceSite *curSite)
    {
        return std::fabs(fX - curSite->X()) + y2xRatio * std::fabs(fY - curSite->Y());
    }

    /**
     * @brief find neibor device sites of a given cell from bin grid
     *
     * @param curCell target cell
     * @param targetX target location X
     * @param targetY target location Y
     * @param displacementThreshold the displacement threshold from the sites to the target location
     * @param siteNumThreshold if the number of sites exceed this threshold, stop the searching
     * @return std::vector<DeviceInfo::DeviceSite *>*
     */
    inline std::vector<DeviceInfo::DeviceSite *> *findNeiborSiteFromBinGrid(DesignInfo::DesignCell *curCell,
                                                                            float targetX, float targetY,
                                                                            float displacementThreshold,
                                                                            int siteNumThreshold)
    {
        // please note that the input DesignCell is only used to find the corresponding binGrid for site search.
        std::vector<DeviceInfo::DeviceSite *> *res = new std::vector<DeviceInfo::DeviceSite *>(0);

        int binIdX, binIdY;
        getGridXY(targetX, targetY, binIdX, binIdY);

        while (res->size() == 0)
        {
            auto sharedTypeIds = getPotentialBELTypeIDs(curCell->getCellType());

            for (auto sharedTypeId : sharedTypeIds)
            {
                assert((unsigned int)curCell->getCellId() < cellId2CellBinInfo.size());
                assert(binIdY >= 0);
                assert((unsigned int)binIdY < SharedBELTypeBinGrid[sharedTypeId].size());
                assert(binIdX >= 0);
                assert((unsigned int)binIdX < SharedBELTypeBinGrid[sharedTypeId][binIdY].size());

                std::vector<std::vector<PlacementBinInfo *>> &curBinGrid = SharedBELTypeBinGrid[sharedTypeId];

                std::queue<std::pair<int, int>> binXYqueue;
                std::set<std::pair<int, int>> reachedBinXYs;
                binXYqueue.emplace(binIdX, binIdY);
                reachedBinXYs.emplace(binIdX, binIdY);

                while (binXYqueue.size() > 0)
                {
                    std::pair<int, int> curXY = binXYqueue.front();
                    binXYqueue.pop();
                    int curbinIdX = curXY.first, curbinIdY = curXY.second;

                    PlacementBinInfo *curBin = curBinGrid[curbinIdY][curbinIdX];
                    float bin2TargetXYDistance = curBin->getManhattanDistanceTo(targetX, targetY);
                    if (bin2TargetXYDistance > displacementThreshold)
                        continue;
                    int findSiteCnt = 0;
                    for (auto curSite : curBin->getCorrespondingSites())
                    {
                        if (!curSite->isOccupied() && !curSite->isMapped())
                        {
                            if (getDisplacement(targetX, targetY, curSite) < displacementThreshold)
                            {
                                findSiteCnt++;
                                res->push_back(curSite);
                            }
                        }
                    }

                    if (res->size() < (unsigned int)siteNumThreshold)
                    {
                        for (int nextY = curbinIdY - 1; nextY <= curbinIdY + 1; nextY++)
                        {
                            for (int nextX = curbinIdX - 1; nextX <= curbinIdX + 1; nextX++)
                            {
                                if (!(nextY >= 0))
                                    continue;
                                if (!((unsigned int)nextY < SharedBELTypeBinGrid[sharedTypeId].size()))
                                    continue;
                                if (!(nextX >= 0))
                                    continue;
                                if (!((unsigned int)nextX < SharedBELTypeBinGrid[sharedTypeId][binIdY].size()))
                                    continue;
                                PlacementBinInfo *nextBin = curBinGrid[nextY][nextX];
                                float nextBin2TargetXYDistance = nextBin->getManhattanDistanceTo(targetX, targetY);
                                if (nextBin2TargetXYDistance > displacementThreshold)
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
            }
            displacementThreshold *= 1.5;
        }

        return res;
    }

    inline std::vector<CellBinInfo> &getCellId2CellBinInfo()
    {
        return cellId2CellBinInfo;
    }

    inline std::vector<DesignInfo::DesignCell *> &getCells()
    {
        return designInfo->getCells();
    }

    inline std::vector<std::vector<PlacementNet *>> &getPlacementUnitId2Nets()
    {
        return placementUnitId2Nets;
    }

    /**
     * @brief update the B2B net model for the placement and get the total HPWL of all the nets in the design
     *
     * @return double
     */
    double updateB2BAndGetTotalHPWL()
    {
        double totalHPWL = 0.0;
        int numNet = placementNets.size();

#pragma omp parallel for
        for (int netId = 0; netId < numNet; netId++)
        {
            auto net = placementNets[netId];
            net->updateNetBounds(true, true);
        }

        //#pragma omp parallel for reduction(+ : totalHPWL)
        for (int netId = 0; netId < numNet; netId++)
        {
            auto net = placementNets[netId];
            totalHPWL += net->getHPWL(y2xRatio);
        }
        return totalHPWL;
    }

    /**
     * @brief get the total HPWL of all the nets in the design without updating the B2B net model for the placement
     *
     * @return double
     */
    double getTotalHPWL()
    {
        double totalHPWL = 0.0;
        int numNet = placementNets.size();
        //#pragma omp parallel for reduction(+ : totalHPWL)
        for (int netId = 0; netId < numNet; netId++)
        {
            auto net = placementNets[netId];
            totalHPWL += net->getHPWL(y2xRatio);
        }
        return totalHPWL;
    }

    /**
     * @brief Set the progress ratio, indicating the progress of the placement convergence,
     *
     * the progress ratio is usually HPWL_lower / HPWL_upper
     *
     * @param p
     */
    inline void setProgress(float p)
    {
        placementProressRatio = p;
    }

    /**
     * @brief Get the Progress ratio of the placement
     *
     * @return float
     */
    inline float getProgress()
    {
        return placementProressRatio;
    }

    /**
     * @brief dump the congestion mesh grid for evaluation
     *
     * @param dumpFileName
     */
    void dumpCongestion(std::string dumpFileName);

    /**
     * @brief dump the placement commands to place cells in Vivado (do not use this temporarily)
     *
     * (do not use this temporarily)
     * We move this functionality to the packer/placer.
     * There is a conterpart function in ParallelPack, which is relatively complete.
     * We will collect those information back to PlacementInfo in later implementation.
     *
     * @param dumpFile
     */
    void dumpVivadoPlacementTclWithPULegalizationInfo(std::string dumpFile);

    /**
     * @brief dump the PlacementUnit objects and some placement parameters as a checkpoint
     *
     * @param dumpFile
     */
    void dumpPlacementUnitInformation(std::string dumpFile);

    /**
     * @brief load the data of the PlacementUnit objects and some placement parameters from a checkpoint file
     *
     * @param locationFile
     */
    void loadPlacementUnitInformation(std::string locationFile);

    /**
     * @brief Set the Pseudo Net Weight according to a given value
     *
     * This pseudo net weight will be used in the global placement iteration
     *
     * @param weight
     */
    inline void setPseudoNetWeight(float weight)
    {
        oriPseudoNetWeight = weight;
    }

    /**
     * @brief Get the Pseudo Net Weight object
     *
     * usually it is used to set the configuration of placer or dump placement information
     *
     * @return float
     */
    inline float getPseudoNetWeight()
    {
        assert(oriPseudoNetWeight > 0 && "should be set before get");
        return oriPseudoNetWeight;
    }

    /**
     * @brief Get the Macro Pseudo Net Enhance Counter
     *
     * The legalization pseudo nets for macros are strengthened as this counter is increased, so we can force the macros
     * get closer and closer to their legal location.
     *
     * @return int
     */
    inline int getMacroPseudoNetEnhanceCnt()
    {
        if (JSONCfg.find("DirectMacroLegalize") != JSONCfg.end())
        {
            if (JSONCfg["DirectMacroLegalize"] == "true")
            {
                return macroPseudoNetEnhanceCnt;
            }
        }
        assert(macroPseudoNetEnhanceCnt > 0 && "should be set before get");
        return macroPseudoNetEnhanceCnt;
    }

    /**
     * @brief Get the Macro Legalization Weight
     *
     * it is the legalization pseudo nets for macros.
     *
     * @return float
     */
    inline float getMacroLegalizationWeight()
    {
        assert(macroLegalizationWeight > 0 && "should be set before get");
        return macroLegalizationWeight;
    }

    /**
     * @brief Set the Macro Legalization Parameters
     *
     * We have specific pseudo net parameters for macro legalizations. We need to set them if loading a check point or
     * re-configuring the placement.
     *
     * @param cnt MacroPseudoNetEnhanceCnt
     * @param _macroLegalizationWeight  MacroPseudoNetEnhanceCnt
     */
    inline void setMacroLegalizationParameters(int cnt, float _macroLegalizationWeight)
    {
        macroPseudoNetEnhanceCnt = cnt;
        macroLegalizationWeight = _macroLegalizationWeight;
    }

    /**
     * @brief reset the LUTFFDeterminedOccupation object
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     *
     */
    void resetLUTFFDeterminedOccupation()
    {
        designInfo->resetLUTFFDeterminedOccupation();
    }

    /**
     * @briefget the Determined Occupation of a specific cell
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     * @param cellId target cell
     * @return int
     */
    inline int getDeterminedOccupation(int cellId)
    {
        return designInfo->getDeterminedOccupation(cellId);
    }

    /**
     * @brief Set the Determined Occupation of a specific cell
     *
     * LUTFFDeterminedOccupation is used to record the final resource demand of a LUT/FF after final packing
     *
     * @param cellId target cell
     * @param occupation resource demand of the cell after packing
     */
    inline void setDeterminedOccupation(int cellId, int occupation)
    {
        designInfo->setDeterminedOccupation(cellId, occupation);
    }

    /**
     * @brief Get the Pair Pin Num of two LUTs
     *
     * Two LUTs can share the input pins of a BEL in CLB. However, device architecture might have requirements on their
     * demands of the number of input pins. This function will return the total number of input pins for the two LUTs.
     *
     * @param LUTA LUT Cell A
     * @param LUTB LUT Cell A
     * @return unsigned int
     */
    inline unsigned int getPairPinNum(DesignInfo::DesignCell *LUTA, DesignInfo::DesignCell *LUTB)
    {
        if (LUTA->getInputPins().size() == 6 || LUTB->getInputPins().size() == 6 || LUTA->isLUT6() || LUTB->isLUT6())
            return 12;

        int pinNumA = 0;
        int totalPin = 0;
        int netIds[5]; // be aware that a LUT might have pins connected to the same net and they should be treated as
                       // different inputs.

        for (auto tmpPin : LUTA->getInputPins())
        {
            if (!tmpPin->isUnconnected())
            {
                netIds[pinNumA] = tmpPin->getNet()->getElementIdInType();
                pinNumA++;
            }
        }
        totalPin = pinNumA;
        for (auto tmpPin : LUTB->getInputPins())
        {
            if (!tmpPin->isUnconnected())
            {
                bool matched = false;
                for (int i = 0; i < pinNumA; i++)
                {
                    if (netIds[i] >= 0 && netIds[i] == tmpPin->getNet()->getElementIdInType())
                    {
                        netIds[i] = -1;
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                {
                    totalPin++;
                }
            }
        }
        return totalPin;
    }

    /**
     * @brief calculate the proportion of the PlacementUnit objects with high interconnection density
     *
     * if a PlacementUnit connects to more than 30 nets, count it.
     * calculate the proportion of the PlacementUnit objects with high interconnection density
     *
     */
    inline void calculateNetNumDistributionOfPUs()
    {
        int manyNetCnt = 0;
        for (auto tmpPU : placementUnits)
        {
            if (tmpPU->getNetsSetPtr()->size() >= 30)
            {
                manyNetCnt++;
            }
        }
        PUWithManyNetsRatio = (float)manyNetCnt / (float)placementUnits.size();
    }

    /**
     * @brief get the proportion of the PlacementUnit objects with high interconnection density
     *
     * @return float
     */
    inline float getPUWithManyNetsRatio()
    {
        assert(PUWithManyNetsRatio >= 0);
        return PUWithManyNetsRatio;
    }

    /**
     * @brief record the minimum HPWL during placement procedure
     *
     * @param val
     */
    inline void setMinHPWL(float val)
    {
        minHPWL = val;
    }

    inline float getMinHPWL()
    {
        return minHPWL;
    }

    /**
     * @brief check the utlization of the clock regions on the device
     *@param dump dump the clock utilization
     */
    void checkClockUtilization(bool dump);

    /**
     * @brief Get the Long Paths in the net list for later optimization
     *
     * @return std::vector<std::vector<PlacementUnit *>>&
     */
    inline std::vector<std::vector<PlacementUnit *>> &getLongPaths()
    {
        return longPaths;
    }

    /**
     * @brief make the PlacementUnits in the long path closer to each other
     *
     */
    void optimizeLongPaths();

    /**
     * @brief call timing info to build simple timing graph
     *
     */
    void buildSimpleTimingGraph()
    {
        simplePlacementTimingInfo->buildSimpleTimingGraph();
    }

  private:
    CompatiblePlacementTable *compatiblePlacementTable = nullptr;
    std::vector<PlacementUnit *> placementUnits;
    std::vector<PlacementUnpackedCell *> placementUnpackedCells;
    std::vector<PlacementMacro *> placementMacros;
    std::vector<PlacementUnit *> fixedPlacementUnits;
    std::set<DesignInfo::DesignCell *> cellInMacros;
    std::map<int, PlacementUnit *> cellId2PlacementUnit;
    std::vector<PlacementUnit *> cellId2PlacementUnitVec;
    std::vector<CellBinInfo> cellId2CellBinInfo;
    std::vector<Location> cellId2location;
    std::vector<Location> pinId2location;
    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;
    PlacementTimingInfo *simplePlacementTimingInfo = nullptr;
    /**
     * @brief a mapping from PlaceuementUnit objects to legalized locations
     *
     */
    std::pair<std::map<PlacementInfo::PlacementUnit *, float>, std::map<PlacementInfo::PlacementUnit *, float>>
        PULegalXY;
    /**
     * @brief a mapping from PlaceuementUnit objects to device sites
     *
     */
    std::map<PlacementInfo::PlacementUnit *, std::vector<DeviceInfo::DeviceSite *>> PU2LegalSites;

    /**
     * @brief left boundary of the device
     */
    float globalMinX;

    /**
     * @brief bottom boundary of the device
     */
    float globalMinY;

    /**
     * @brief right boundary of the device
     */
    float globalMaxX;

    /**
     * @brief top boundary of the device
     */
    float globalMaxY;

    /**
     * @brief left boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    float startX;

    /**
     * @brief bottom boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    float startY;

    /**
     * @brief right boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    float endX;

    /**
     * @brief bottom boundary of the bin grid
     *
     *  the coverage of bin grid is a bit larger than the device.
     */
    float endY;

    float eps = 1e-5;
    std::vector<std::vector<std::vector<PlacementBinInfo *>>> SharedBELTypeBinGrid;

    /**
     * @brief Bin Grid for LUTs and FFs, mainly for searching neighbor elements during packing
     *
     */
    std::vector<std::vector<PlacementBinInfo *>> LUTFFBinGrid;

    /**
     * @brief Bin Grid includes all types of sites, mainly for congestion evalution
     *
     */
    std::vector<std::vector<PlacementBinInfo *>> globalBinGrid;
    std::vector<std::vector<PlacementSiteBinInfo *>> siteGridForMacros;
    float binWidth;
    float binHeight;

    std::vector<PlacementNet *> placementNets;
    std::vector<std::vector<PlacementNet *>> placementUnitId2Nets;

    std::vector<PlacementNet *> clockNets;
    std::vector<std::vector<int>> clockRegionUtilization;

    std::set<PlacementUnit *> PUSetContainingFF;
    std::vector<PlacementUnit *> PUsContainingFF;
    std::vector<std::vector<PlacementUnit *>> longPaths;

    /**
     * @brief the retangular clock region coverage of a clock net
     *
     */
    typedef struct _ClockNetCoverage
    {
        PlacementNet *clockNet = nullptr;

        /**
         * @brief the left column in the grid of clock regions
         *
         */
        int leftRegionX;

        /**
         * @brief the right column in the grid of clock regions
         *
         */
        int rightRegionX;

        /**
         * @brief the top row in the grid of clock regions
         *
         */
        int topRegionY;

        /**
         * @brief the bottom row in the grid of clock regions
         *
         */
        int bottomRegionY;
    } ClockNetCoverage;
    std::vector<ClockNetCoverage> clockNetCoverages;

    /**
     * @brief the progress ratio, indicating the progress of the placement convergence.
     *
     * the progress ratio is usually HPWL_lower / HPWL_upper
     */
    float placementProressRatio = 0.01;

    std::map<std::string, std::string> &JSONCfg;
    std::string cellType2fixedAmoFileName;
    std::string cellType2sharedCellTypeFileName;
    std::string sharedCellType2BELtypeFileName;

    /**
     * @brief a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
     * in X-coordinate
     */
    float y2xRatio = 1.0;

    int dumpPlacementUnitLocationCnt = 0;

    float oriPseudoNetWeight = -1;
    int macroPseudoNetEnhanceCnt = -1;
    float macroLegalizationWeight = -1;

    float lastProgressWhenLUTFFUtilAdjust = -1.0;
    float PUWithManyNetsRatio = -1;
    float minHPWL = 1e8;
    bool LUTFFUtilizationAdjusted = false;
};

std::ostream &operator<<(std::ostream &os, PlacementInfo::PlacementMacro *curMacro);
std::ostream &operator<<(std::ostream &os, PlacementInfo::PlacementUnpackedCell *curUnpackedCell);
std::ostream &operator<<(std::ostream &os, PlacementInfo::PlacementUnit *curPU);
#endif