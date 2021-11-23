/**
 * @file GeneralSpreader.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of GeneralSpreader class and its internal modules and APIs which
 * account for the cell spreading, which controls the cell density of specific resource type, under the device
 * constraints for specific regions.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _GENERALSPREADER
#define _GENERALSPREADER

#include "const.h"
#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "PlacementInfo.h"
#include "dumpZip.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief GeneralSpreader accounts for the cell spreading, which controls the cell density of specific resource type,
 * under the device constraints for specific regions
 *
 */
class GeneralSpreader
{
  public:
    /**
     * @brief Construct a new General Spreader object
     *
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg  the user-defined placement configuration
     * @param sharedCellType a string indicating the cell type which should be handled by this spreader
     * @param currentIteration the current global placement iteration number, which might be used for spreading settings
     * @param capacityShrinkRatio shrink the area supply to a specific ratio
     * @param verbose option to enable detailed information display in terminal
     */
    GeneralSpreader(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg,
                    std::string &sharedCellType, int currentIteration, float capacityShrinkRatio, bool verbose = true);
    ~GeneralSpreader()
    {
    }

    /**
     * @brief spread cells with a given forgetting ratio
     *
     * @param forgetRatio forget the original according to a given extent ratio. Lower forget ratio will make cell
     * spreading less sensitive (mreo stable).
     * @param enableClockRegionAware spread but limit in specific clock region
     * @param spreadRegionBinSizeLimit the maximum size of the spread region for a overflow bin
     */
    void spreadPlacementUnits(float forgetRatio, bool enableClockRegionAware = false,
                              unsigned int spreadRegionBinSizeLimit = 1000000);
    void dumpLUTFFCoordinate();

    /**
     * @brief SpreadRegion is an object that record cell spreading region information, including boundaries, cells,
     * bins, and spreading boxes.
     *
     */
    class SpreadRegion
    {
      public:
        /**
         * @brief Construct a new Spread Region object
         *
         * @param curBin the initial bin for the SpreadRegion object, which should be overflowed.
         * @param placementInfo PlacementInfo so this object can access the corresponding placement database
         * @param binGrid the reference of the binGrid for cell spreading. A bin grid is used to record the density of
         * cells on the device
         * @param capacityShrinkRatio shrink the area supply to a specific ratio
         */
        SpreadRegion(PlacementInfo::PlacementBinInfo *curBin, PlacementInfo *placementInfo,
                     std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &binGrid, float capacityShrinkRatio)
            : placementInfo(placementInfo), binGrid(binGrid), capacityShrinkRatio(capacityShrinkRatio)
        {
            topBinY = bottomBinY = curBin->Y();
            leftBinX = rightBinX = curBin->X();
            cellsInRegion.clear();
            binsInRegion.clear();
            cellsInRegionVec.clear();
            binSetInRegion.clear();
            binsInRegion.push_back(curBin);
            binSetInRegion.insert(curBin);
            for (auto curCell : curBin->getCells())
            {
                cellsInRegion.insert(curCell);
                cellsInRegionVec.push_back(curCell);
            }
            totalCapacity = curBin->getCapacity();
            totalUtilization = curBin->getUtilization();
            overflowRatio = totalUtilization / totalCapacity;
        }
        ~SpreadRegion()
        {
        }

        /**
         * @brief check whether the SpreadRegion is overlapped with a given boundary
         *
         * @param tmpRegionTopBinY
         * @param tmpRegionBottomBinY
         * @param tmpRegionLeftBinX
         * @param tmpRegionRightBinX
         * @return true when overlapped
         * @return false when not overlapped
         */
        inline bool isRegionOverlap(int tmpRegionTopBinY, int tmpRegionBottomBinY, int tmpRegionLeftBinX,
                                    int tmpRegionRightBinX)
        {
            if (leftBinX > tmpRegionRightBinX || tmpRegionLeftBinX > rightBinX)
                return false;
            if (topBinY < tmpRegionBottomBinY || tmpRegionTopBinY < bottomBinY)
                return false;
            return true;
        }

        /**
         * @brief check whether overlapped with another SpreadRegion
         *
         * @param anotherRegion another SpreadRegion object
         * @return true when overlapped
         * @return false when not overlapped
         */
        inline bool isRegionOverlap(SpreadRegion *anotherRegion)
        {
            if (leftBinX > anotherRegion->right() || anotherRegion->left() > rightBinX)
                return false;
            if (topBinY < anotherRegion->bottom() || anotherRegion->top() < bottomBinY)
                return false;
            return true;
        }

        /**
         * @brief add cell bins into SpreadRegion with a new boundary and update region information including
         * boundary and the sets of cells and bins
         *
         * @param newRegionTopBinY
         * @param newRegionBottomBinY
         * @param newRegionLeftBinX
         * @param newRegionRightBinX
         * @param coveredBinSet
         */
        void addBinRegion(int newRegionTopBinY, int newRegionBottomBinY, int newRegionLeftBinX, int newRegionRightBinX,
                          std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet);

        /**
         * @brief Get the bins in the SpreadRegion
         *
         * @return std::vector<PlacementInfo::PlacementBinInfo *>&
         */
        inline std::vector<PlacementInfo::PlacementBinInfo *> &getBinsInRegion()
        {
            return binsInRegion;
        }

        /**
         * @brief check whether the resource in this region is less than the requirement
         *
         * @return true when the resource in this region is less than the requirement
         * @return false when the resource in this region is more than the requirement
         */
        inline bool isOverflow()
        {
            return overflowRatio > 1.0;
        }

        /**
         * @brief four direction options to expand the SpreadRegion
         *
         */
        enum dirType
        {
            expandLeft,
            expandRight,
            expandUp,
            expandDown
        };

        /**
         * @brief a struct to describe the expanding operation for the SpreadRegion
         *
         */
        struct expandOp
        {
            int topYOp, bottomYOp, leftXOp, rightXOp;

            expandOp()
            {
            }

            expandOp(int topYOp, int bottomYOp, int leftXOp, int rightXOp)
                : topYOp(topYOp), bottomYOp(bottomYOp), leftXOp(leftXOp), rightXOp(rightXOp)
            {
            }

            inline expandOp &operator=(const expandOp &a)
            {
                topYOp = a.topYOp;
                bottomYOp = a.bottomYOp;
                leftXOp = a.leftXOp;
                rightXOp = a.rightXOp;
                return *this;
            }

            /**
             * @brief overload operator+ to expand the SpreadRegion by given steps in the pre-set direction
             *
             * @param x the expand steps (currently we only support the expansion by 1 bin)
             * @return expandOp
             */
            inline expandOp operator+(int x) const
            {
                assert(x == 1);
                if (topYOp)
                    return expandOp(topYOp + 1, bottomYOp, leftXOp, rightXOp);
                if (bottomYOp)
                    return expandOp(topYOp, bottomYOp - 1, leftXOp, rightXOp);
                if (leftXOp)
                    return expandOp(topYOp, bottomYOp, leftXOp - 1, rightXOp);
                if (rightXOp)
                    return expandOp(topYOp, bottomYOp, leftXOp, rightXOp + 1);
                assert(false && "should not reach here");
                return expandOp(topYOp * x, bottomYOp * x, leftXOp * x, rightXOp * x);
            }
        };

        // topBinY, bottomBinY, leftBinX, rightBinX;
        expandOp expandOps[4] = {{0, 0, -1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}, {0, -1, 0, 0}};
        expandOp actualExpandOps[4];

        /**
         * @brief check whether an expansion in a given direction will make the SpreadRegion overlapped with some bins
         * in another SpreadRegion
         *
         * @param newTopBinY the top boundary after expansion
         * @param newBottomBinY the bottom boundary after expansion
         * @param newLeftBinX the left boundary after expansion
         * @param newRightBinX the right boundary after expansion
         * @param tmpDir a given direction
         * @param coveredBinSet a set of bins which have been included in other SpreadRegion
         * @return true when the expansion will be overlapped with some bins in another SpreadRegion
         * @return false when the expansion will NOT be overlapped with some bins in another SpreadRegion
         */
        inline bool isCovered(int newTopBinY, int newBottomBinY, int newLeftBinX, int newRightBinX, dirType tmpDir,
                              std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
        {
            if (tmpDir == expandUp)
            {
                for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    if (coveredBinSet.find(binGrid[newTopBinY][binX]) != coveredBinSet.end())
                        return true;
                }
            }
            else if (tmpDir == expandDown)
            {
                for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    if (coveredBinSet.find(binGrid[newBottomBinY][binX]) != coveredBinSet.end())
                        return true;
                }
            }
            else if (tmpDir == expandLeft)
            {
                for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                {
                    if (coveredBinSet.find(binGrid[binY][newLeftBinX]) != coveredBinSet.end())
                        return true;
                }
            }
            else if (tmpDir == expandRight)
            {
                for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                {
                    if (coveredBinSet.find(binGrid[binY][newRightBinX]) != coveredBinSet.end())
                        return true;
                }
            }
            return false;
        }

        /**
         * @brief evaluation the utilization and capacity of the incoming bins which will be included in the
         * SpreadRegion after an expansion .
         *
         * @param newTopBinY the top boundary after expansion
         * @param newBottomBinY the bottom boundary after expansion
         * @param newLeftBinX the left boundary after expansion
         * @param newRightBinX the right boundary after expansion
         * @param tmpDir a given direction
         * @param tmpUtilization the resultant utilization
         * @param tmpCapacity the resultant capacity
         */
        inline void getDirCapacityAndUtilization(int newTopBinY, int newBottomBinY, int newLeftBinX, int newRightBinX,
                                                 dirType tmpDir, float &tmpUtilization, float &tmpCapacity)
        {
            // tmpCapacity = 0.0;
            // tmpUtilization = 0.0;
            if (tmpDir == expandUp)
            {
                //  for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    tmpCapacity += binGrid[newTopBinY][binX]->getCapacity();
                    tmpUtilization += binGrid[newTopBinY][binX]->getUtilization();
                }
            }
            else if (tmpDir == expandDown)
            {
                //  for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    tmpCapacity += binGrid[newBottomBinY][binX]->getCapacity();
                    tmpUtilization += binGrid[newBottomBinY][binX]->getUtilization();
                }
            }
            else if (tmpDir == expandLeft)
            {
                for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                //       for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    tmpCapacity += binGrid[binY][newLeftBinX]->getCapacity();
                    tmpUtilization += binGrid[binY][newLeftBinX]->getUtilization();
                }
            }
            else if (tmpDir == expandRight)
            {
                for (int binY = newBottomBinY; binY <= newTopBinY; binY++)
                //         for (int binX = newLeftBinX; binX <= newRightBinX; binX++)
                {
                    tmpCapacity += binGrid[binY][newRightBinX]->getCapacity();
                    tmpUtilization += binGrid[binY][newRightBinX]->getUtilization();
                }
            }
            if (tmpCapacity < 1e-5)
                tmpCapacity = 1e-5;
            if (tmpUtilization < 1e-5)
                tmpUtilization = 2e-5;
        }

        /**
         * @brief find the legal expansion direction
         *
         * evaluate the utilization/capacity of 4 legal directions
         *
         * @param coveredBinSet a set of bins which have been included in other SpreadRegion
         * @return true when there is legal direction to expand the SpreadRegion
         * @return false when there is NO legal direction to expand the SpreadRegion
         */
        inline bool smartFindExpandDirection(std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
        {
            int possibleCnt = 0;

            // iterate the 4 direction
            for (int tmpDir = 0; tmpDir < 4; tmpDir++)
            {
                // initialize the expansion result (boundary, utlization, capacity, legal or not)
                unsigned int newTopBinY = topBinY + expandOps[tmpDir].topYOp;
                int newBottomBinY = bottomBinY + expandOps[tmpDir].bottomYOp;
                int newLeftBinX = leftBinX + expandOps[tmpDir].leftXOp;
                unsigned int newRightBinX = rightBinX + expandOps[tmpDir].rightXOp;
                dir2utilization[tmpDir] = 1e-1;
                dir2capacity[tmpDir] = 1e-5;
                legalDir[tmpDir] = false;
                if (newTopBinY < binGrid.size() && newBottomBinY >= 0)
                {
                    if (newRightBinX < binGrid[newTopBinY].size() && newLeftBinX >= 0)
                    {
                        // don't include the bins in other existing SpreadRegion
                        if (!isCovered(newTopBinY, newBottomBinY, newLeftBinX, newRightBinX,
                                       static_cast<dirType>(tmpDir), coveredBinSet))
                        {
                            // evaluate the utilization and capacity
                            getDirCapacityAndUtilization(newTopBinY, newBottomBinY, newLeftBinX, newRightBinX,
                                                         static_cast<dirType>(tmpDir), dir2utilization[tmpDir],
                                                         dir2capacity[tmpDir]);

                            // capacity greater than 0 => legal direction
                            if (dir2capacity[tmpDir] > 2e-5)
                            {
                                actualExpandOps[tmpDir] = expandOps[tmpDir];
                                legalDir[tmpDir] = true;
                                possibleCnt++;
                            }

                            // if the capacity is legal, then try to further expand 1 more bin step in this direction
                            if (!legalDir[tmpDir])
                            {
                                actualExpandOps[tmpDir] = expandOps[tmpDir] + 1; // <== further expand
                                unsigned int newTopBinY = topBinY + actualExpandOps[tmpDir].topYOp;
                                int newBottomBinY = bottomBinY + actualExpandOps[tmpDir].bottomYOp;
                                int newLeftBinX = leftBinX + actualExpandOps[tmpDir].leftXOp;
                                unsigned int newRightBinX = rightBinX + actualExpandOps[tmpDir].rightXOp;
                                if (newTopBinY < binGrid.size() && newBottomBinY >= 0)
                                {
                                    if (newRightBinX < binGrid[newTopBinY].size() && newLeftBinX >= 0)
                                    {
                                        if (!isCovered(newTopBinY, newBottomBinY, newLeftBinX, newRightBinX,
                                                       static_cast<dirType>(tmpDir), coveredBinSet))
                                        {
                                            getDirCapacityAndUtilization(newTopBinY, newBottomBinY, newLeftBinX,
                                                                         newRightBinX, static_cast<dirType>(tmpDir),
                                                                         dir2utilization[tmpDir], dir2capacity[tmpDir]);
                                            if (dir2capacity[tmpDir] > 2e-5)
                                            {
                                                legalDir[tmpDir] = true;
                                                possibleCnt++;
                                            }

                                            // if the capacity is legal, then try to further expand 1 more bin step in
                                            // this direction
                                            if (!legalDir[tmpDir])
                                            {
                                                actualExpandOps[tmpDir] =
                                                    (expandOps[tmpDir] + 1) + 1; // <== further expand
                                                unsigned int newTopBinY = topBinY + actualExpandOps[tmpDir].topYOp;
                                                int newBottomBinY = bottomBinY + actualExpandOps[tmpDir].bottomYOp;
                                                int newLeftBinX = leftBinX + actualExpandOps[tmpDir].leftXOp;
                                                unsigned int newRightBinX =
                                                    rightBinX + actualExpandOps[tmpDir].rightXOp;
                                                if (newTopBinY < binGrid.size() && newBottomBinY >= 0)
                                                {
                                                    if (newRightBinX < binGrid[newTopBinY].size() && newLeftBinX >= 0)
                                                    {
                                                        if (!isCovered(newTopBinY, newBottomBinY, newLeftBinX,
                                                                       newRightBinX, static_cast<dirType>(tmpDir),
                                                                       coveredBinSet))
                                                        {
                                                            getDirCapacityAndUtilization(
                                                                newTopBinY, newBottomBinY, newLeftBinX, newRightBinX,
                                                                static_cast<dirType>(tmpDir), dir2utilization[tmpDir],
                                                                dir2capacity[tmpDir]);
                                                            if (dir2capacity[tmpDir] > 2e-5)
                                                            {
                                                                legalDir[tmpDir] = true;
                                                                possibleCnt++;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return possibleCnt;
        }

        inline bool expandable(int tmpDir)
        {
            return legalDir[tmpDir];
        }

        /**
         * @brief select the expansion direction greedily
         *
         * evaluate the 4 potential directions and select the direction, which can significantly reduce the overflow, to
         * conduct expansion
         *
         * @param coveredBinSet a set of bins which have been included in other SpreadRegion
         */
        inline void smartExpand(std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
        {
            float hCapacity = dir2capacity[0] + dir2capacity[1];
            float vCapacity = dir2capacity[2] + dir2capacity[3];
            float hUtilization = dir2utilization[0] + dir2utilization[1];
            float vUtilization = dir2utilization[2] + dir2utilization[3];
            bool expanded = false;

            // try to expand in horizontal direction when horizontal utilization ratio is lower than vertical
            // utilization ratio
            if ((expandable(0) || expandable(1)) &&
                ((hUtilization / hCapacity < 0.9 * vUtilization / vCapacity) ||
                 (std::fabs(hUtilization + vUtilization) < 1e-4 && hCapacity > vCapacity)))
            {
                if (random() % 2)
                {
                    if (expandable(0))
                    {
                        if ((dir2utilization[0] + totalUtilization) / (dir2capacity[0] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding left can solve the overflow, then only expand to left direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[0].topYOp;
                            int newBottomBinY = bottomBinY + actualExpandOps[0].bottomYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[0].leftXOp;
                            addBinRegion(newTopBinY, newBottomBinY, newLeftBinX, leftBinX - 1, coveredBinSet);
                            return;
                        }
                    }
                    if (expandable(1))
                    {
                        if ((dir2utilization[1] + totalUtilization) / (dir2capacity[1] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding right can solve the overflow, then only expand to right direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[1].topYOp;
                            int newBottomBinY = bottomBinY + actualExpandOps[1].bottomYOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[1].rightXOp;
                            addBinRegion(newTopBinY, newBottomBinY, rightBinX + 1, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                }
                else
                {
                    if (expandable(1))
                    {
                        if ((dir2utilization[1] + totalUtilization) / (dir2capacity[1] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding right can solve the overflow, then only expand to right direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[1].topYOp;
                            int newBottomBinY = bottomBinY + actualExpandOps[1].bottomYOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[1].rightXOp;
                            addBinRegion(newTopBinY, newBottomBinY, rightBinX + 1, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                    if (expandable(0))
                    {
                        if ((dir2utilization[0] + totalUtilization) / (dir2capacity[0] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding left can solve the overflow, then only expand to left direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[0].topYOp;
                            int newBottomBinY = bottomBinY + actualExpandOps[0].bottomYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[0].leftXOp;
                            addBinRegion(newTopBinY, newBottomBinY, newLeftBinX, leftBinX - 1, coveredBinSet);
                            return;
                        }
                    }
                }

                // left and right
                unsigned int newTopBinY = topBinY + actualExpandOps[0].topYOp;
                int newBottomBinY = bottomBinY + actualExpandOps[0].bottomYOp;
                int newLeftBinX = leftBinX + actualExpandOps[0].leftXOp;
                unsigned int newRightBinX = rightBinX + actualExpandOps[0].rightXOp;
                if (expandable(0))
                {
                    addBinRegion(newTopBinY, newBottomBinY, newLeftBinX, leftBinX - 1, coveredBinSet);
                    expanded = true;
                }
                newTopBinY = topBinY + actualExpandOps[1].topYOp;
                newBottomBinY = bottomBinY + actualExpandOps[1].bottomYOp;
                newLeftBinX = leftBinX + actualExpandOps[1].leftXOp;
                newRightBinX = rightBinX + actualExpandOps[1].rightXOp;
                if (expandable(1))
                {
                    addBinRegion(newTopBinY, newBottomBinY, rightBinX + 1, newRightBinX, coveredBinSet);
                    expanded = true;
                }
            }
            else
            {
                if (random() % 2)
                {
                    if (expandable(2))
                    {
                        if ((dir2utilization[2] + totalUtilization) / (dir2capacity[2] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding upward can solve the overflow, then only expand to upward direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[2].topYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[2].leftXOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[2].rightXOp;
                            addBinRegion(newTopBinY, topBinY + 1, newLeftBinX, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                    if (expandable(3))
                    {
                        if ((dir2utilization[3] + totalUtilization) / (dir2capacity[3] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding downward can solve the overflow, then only expand to downward direction
                            int newBottomBinY = bottomBinY + actualExpandOps[3].bottomYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[3].leftXOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[3].rightXOp;
                            addBinRegion(bottomBinY - 1, newBottomBinY, newLeftBinX, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                }
                else
                {
                    if (expandable(3))
                    {
                        if ((dir2utilization[3] + totalUtilization) / (dir2capacity[3] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding downward can solve the overflow, then only expand to downward direction
                            int newBottomBinY = bottomBinY + actualExpandOps[3].bottomYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[3].leftXOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[3].rightXOp;
                            addBinRegion(bottomBinY - 1, newBottomBinY, newLeftBinX, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                    if (expandable(2))
                    {
                        if ((dir2utilization[2] + totalUtilization) / (dir2capacity[2] + totalCapacity) <
                            capacityShrinkRatio)
                        {
                            // if expanding upward can solve the overflow, then only expand to upward direction
                            unsigned int newTopBinY = topBinY + actualExpandOps[2].topYOp;
                            int newLeftBinX = leftBinX + actualExpandOps[2].leftXOp;
                            unsigned int newRightBinX = rightBinX + actualExpandOps[2].rightXOp;
                            addBinRegion(newTopBinY, topBinY + 1, newLeftBinX, newRightBinX, coveredBinSet);
                            return;
                        }
                    }
                }

                // up and down
                unsigned int newTopBinY = topBinY + actualExpandOps[2].topYOp;
                int newBottomBinY = bottomBinY + actualExpandOps[2].bottomYOp;
                int newLeftBinX = leftBinX + actualExpandOps[2].leftXOp;
                unsigned int newRightBinX = rightBinX + actualExpandOps[2].rightXOp;
                if (expandable(2))
                {
                    addBinRegion(newTopBinY, topBinY + 1, newLeftBinX, newRightBinX, coveredBinSet);
                    expanded = true;
                }
                newTopBinY = topBinY + actualExpandOps[3].topYOp;
                newBottomBinY = bottomBinY + actualExpandOps[3].bottomYOp;
                newLeftBinX = leftBinX + actualExpandOps[3].leftXOp;
                newRightBinX = rightBinX + actualExpandOps[3].rightXOp;
                if (expandable(3))
                {
                    addBinRegion(bottomBinY - 1, newBottomBinY, newLeftBinX, newRightBinX, coveredBinSet);
                    expanded = true;
                }
            }

            if (!expanded)
            {
                std::cout << "expandable(0,1,2,3):" << expandable(0) << expandable(1) << expandable(2) << expandable(3)
                          << "\ndir2capacity[0]:" << dir2capacity[0] << " dir2capacity[1]:" << dir2capacity[1]
                          << " dir2capacity[2]:" << dir2capacity[2] << " dir2capacity[3]:" << dir2capacity[3]
                          << "\ndir2utilization[0]:" << dir2utilization[0]
                          << " dir2utilization[1]:" << dir2utilization[1]
                          << " dir2utilization[2]:" << dir2utilization[2]
                          << " dir2utilization[3]:" << dir2utilization[3] << "\n"
                          << " hUtilization / hCapacity < vUtilization / vCapacity:"
                          << (hUtilization / hCapacity < vUtilization / vCapacity) << "\n";
                std::cout.flush();
                assert(false && "should not be unexpandable");
            }
        }

        /**
         * @brief find the expansion direction iteratively in pre-defined order
         *
         * @param coveredBinSet a set of bins which have been included in other SpreadRegion
         * @return true when there is legal direction to expand the SpreadRegion
         * @return false when there is NO legal direction to expand the SpreadRegion
         */
        inline bool simpleFindExpandDirection(std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
        {
            char failureCnt = 0;
            curDirectionIndex = random() % 4;
            while (failureCnt < 4)
            {

                unsigned int newTopBinY = topBinY + expandOps[curDirectionIndex].topYOp;
                int newBottomBinY = bottomBinY + expandOps[curDirectionIndex].bottomYOp;
                int newLeftBinX = leftBinX + expandOps[curDirectionIndex].leftXOp;
                unsigned int newRightBinX = rightBinX + expandOps[curDirectionIndex].rightXOp;
                if (newTopBinY < binGrid.size() && newBottomBinY >= 0)
                {
                    if (newRightBinX < binGrid[newTopBinY].size() && newLeftBinX >= 0)
                    {
                        if (!isCovered(newTopBinY, newBottomBinY, newLeftBinX, newRightBinX,
                                       static_cast<dirType>(curDirectionIndex), coveredBinSet))
                        {
                            curDirection = static_cast<dirType>(curDirectionIndex);
                            curDirectionIndex = (curDirectionIndex + 1) % 4;
                            return true;
                        }
                    }
                }
                curDirectionIndex = (curDirectionIndex + 1) % 4;
                failureCnt++;
            }
            return false;
        }

        /**
         * @brief select the the expansion direction which is found in a pre-defined order
         *
         * @param coveredBinSet a set of bins which have been included in other SpreadRegion
         */
        inline void simpleExpand(std::set<PlacementInfo::PlacementBinInfo *> &coveredBinSet)
        {
            unsigned int newTopBinY = topBinY + expandOps[curDirection].topYOp;
            int newBottomBinY = bottomBinY + expandOps[curDirection].bottomYOp;
            int newLeftBinX = leftBinX + expandOps[curDirection].leftXOp;
            unsigned int newRightBinX = rightBinX + expandOps[curDirection].rightXOp;
            if (curDirection == expandUp)
                addBinRegion(newTopBinY, newTopBinY, newLeftBinX, newRightBinX, coveredBinSet);
            else if (curDirection == expandDown)
                addBinRegion(newBottomBinY, newBottomBinY, newLeftBinX, newRightBinX, coveredBinSet);
            else if (curDirection == expandLeft)
                addBinRegion(newTopBinY, newBottomBinY, newLeftBinX, newLeftBinX, coveredBinSet);
            else if (curDirection == expandRight)
                addBinRegion(newTopBinY, newBottomBinY, newRightBinX, newRightBinX, coveredBinSet);
        }

        /**
         * @brief check whether SpreadRegion contains the given bin
         *
         * @param curBin a given bin
         * @return true when SpreadRegion contains the given bin
         * @return false when SpreadRegion does not contain the given bin
         */
        inline bool contains(PlacementInfo::PlacementBinInfo *curBin)
        {
            return binSetInRegion.find(curBin) != binSetInRegion.end();
        }

        /**
         * @brief Get the cells in the SpreadRegion
         *
         * please be aware that a SpreadRegion will only contain cells in a specific type defined by its constructor.
         *
         * @return std::vector<DesignInfo::DesignCell *>&
         */
        inline std::vector<DesignInfo::DesignCell *> &getCells()
        {
            return cellsInRegionVec;
        }

        /**
         * @brief get the top bin coordinate Y of the SpreadRegion in bin grid
         *
         * @return int
         */
        inline int top()
        {
            return topBinY;
        }

        /**
         * @brief get the bottom bin coordinate Y of the SpreadRegion in bin grid
         *
         * @return int
         */
        inline int bottom()
        {
            return bottomBinY;
        }

        /**
         * @brief get the left bin coordinate X of the SpreadRegion in bin grid
         *
         * @return int
         */
        inline int left()
        {
            return leftBinX;
        }

        /**
         * @brief get the right bin coordinate X of the SpreadRegion in bin grid
         *
         * @return int
         */
        inline int right()
        {
            return rightBinX;
        }

        /**
         * @brief Get the resource overflow ratio
         *
         * @return float
         */
        inline float getOverflowRatio()
        {
            return overflowRatio;
        }

        /**
         * @brief SubBox is the exact container which is the object for bi-partitioning-based cell spreading
         *
         * Each SubBox can be splited vertically or horizontally and cells can be assigned to the partitions accoringly
         * if it is allowed.
         *
         */
        class SubBox
        {
          public:
            /**
             * @brief Construct a new Sub Box object
             *
             * @param placementInfo PlacementInfo so this object can access the corresponding placement database
             * @param curRegion the parent SpreadRegion of this SubBox which guides the boundary setting of the subox
             * @param binGrid the bin grid which record the cell density distribution
             * @param capacityShrinkRatio shrink the area supply to a specific ratio
             * @param level current recursion level
             * @param dirIsH split vertically or horizontally (it is a priority setting instead of enforcement)
             */
            SubBox(PlacementInfo *placementInfo, SpreadRegion *curRegion,
                   std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &binGrid,
                   float capacityShrinkRatio = 1.0, int level = 100, bool dirIsH = true)
                : placementInfo(placementInfo), binGrid(binGrid), capacityShrinkRatio(capacityShrinkRatio),
                  topBinY(curRegion->top()), bottomBinY(curRegion->bottom()), leftBinX(curRegion->left()),
                  rightBinX(curRegion->right()), level(level), dirIsH(dirIsH)
            {
                cellIds.clear();
                std::vector<DesignInfo::DesignCell *> tmpCells;
                tmpCells.clear();
                for (auto curCell : curRegion->getCells())
                {
                    tmpCells.push_back(curCell);
                }
                std::sort(tmpCells.begin(), tmpCells.end(),
                          [](DesignInfo::DesignCell *a, DesignInfo::DesignCell *b) -> bool {
                              return a->getCellId() > b->getCellId();
                          });
                for (auto curCell : tmpCells)
                {
                    cellIds.push_back(curCell->getCellId());
                }
            }

            /**
             * @brief Construct a new Sub Box object
             *
             * @param parentBox inheret information from a parent SubBox
             * @param topBinY the top bin index
             * @param bottomBinY  the bottom bin index
             * @param leftBinX the left bin index
             * @param rightBinX the right bin index
             * @param cellRangeBegin the range begin for copying the cell ids to this child SubBox
             * @param cellRangeEnd the range end for copying the cell ids to this child SubBox
             */
            SubBox(SubBox *parentBox, int topBinY, int bottomBinY, int leftBinX, int rightBinX, int cellRangeBegin,
                   int cellRangeEnd)
                : placementInfo(parentBox->placementInfo), binGrid(parentBox->binGrid),
                  capacityShrinkRatio(parentBox->capacityShrinkRatio), topBinY(topBinY), bottomBinY(bottomBinY),
                  leftBinX(leftBinX), rightBinX(rightBinX), level(parentBox->getLevel() - 1), dirIsH(!parentBox->dirIsH)
            {
                if (cellRangeBegin <= cellRangeEnd)
                    cellIds = std::vector<int>(
                        {parentBox->cellIds.begin() + cellRangeBegin, parentBox->cellIds.begin() + cellRangeEnd + 1});
                else
                    cellIds.clear();
            }

            ~SubBox()
            {
            }

            PlacementInfo *placementInfo;
            std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &binGrid;
            float capacityShrinkRatio = 1.0;

            inline void addCellId(int cellId)
            {
                cellIds.push_back(cellId);
            }

            /**
             * @brief spread cells and partition the SubBox into smaller SubBoxes
             *
             */
            void spreadAndPartition();

            /**
             * @brief the partition/sort function for quick sorting by cell location (X or Y)
             *
             * @param cellIds the cell ids for location sorting
             * @param low lower bound of sorting range
             * @param high upper bound of sorting range
             * @param Xsort sort by X loction or Y location
             * @return int
             */
            inline int Partition(std::vector<int> &cellIds, int low, int high, bool Xsort)
            {
                if (Xsort)
                {
                    int pivot, index, i;
                    index = low;
                    pivot = high;
                    for (i = low; i < high; i++)
                    {
                        // finding index of pivot.
                        // if (a[i] < a[pivot])
                        if (placementInfo->getCellId2location()[cellIds[i]].X <
                            placementInfo->getCellId2location()[cellIds[pivot]].X)
                        {
                            std::swap(cellIds[i], cellIds[index]);
                            index++;
                        }
                    }
                    std::swap(cellIds[pivot], cellIds[index]);
                    return index;
                }
                else
                {
                    int pivot, index, i;
                    index = low;
                    pivot = high;
                    for (i = low; i < high; i++)
                    {
                        // finding index of pivot.
                        // if (a[i] < a[pivot])
                        if (placementInfo->getCellId2location()[cellIds[i]].Y <
                            placementInfo->getCellId2location()[cellIds[pivot]].Y)
                        {
                            std::swap(cellIds[i], cellIds[index]);
                            index++;
                        }
                    }
                    std::swap(cellIds[pivot], cellIds[index]);
                    return index;
                }
            }

            /**
             * @brief the partition/sort function for quick sorting by cell location (X or Y)
             *
             * @param cellIds the cell ids for location sorting
             * @param low lower bound of sorting range
             * @param high upper bound of sorting range
             * @param Xsort sort by X loction or Y location
             * @return int
             */
            inline int RandomPivotPartition(std::vector<int> &cellIds, int low, int high, bool Xsort)
            {
                // Random selection of pivot.
                int pvt, n;
                n = HiFPlacer_hashprimes[low & 0xff];
                pvt = low + n % (high - low + 1); // Randomizing the pivot value from sub-array.
                std::swap(cellIds[high], cellIds[pvt]);
                return Partition(cellIds, low, high, Xsort);
            }

            /**
             * @brief recursive implementation of quick sort for cell ids by X/Y location
             *
             * @param cellIds the cell ids for location sorting
             * @param p lower bound of sorting range
             * @param q upper bound of sorting range
             * @param Xsort sort by X loction or Y location
             */
            void quick_sort(std::vector<int> &cellIds, int p, int q, bool Xsort)
            {
                // recursively sort the list
                int pindex;
                if (p < q)
                {
                    pindex = RandomPivotPartition(cellIds, p, q, Xsort); // randomly choose pivot
                    // Recursively implementing QuickSort.
                    quick_sort(cellIds, p, pindex - 1, Xsort);
                    quick_sort(cellIds, pindex + 1, q, Xsort);
                }
            }

            inline int getLevel()
            {
                return level;
            }

            /**
             * @brief split horizontally the SubBox into smaller ones and assign cells to them
             *
             * @param boxA the obtained pointer of new smaller SubBox
             * @param boxB another obtained pointer of new smaller SubBox
             */
            void spreadCellsH(SubBox **boxA, SubBox **boxB);

            /**
             * @brief split vertically the SubBox into smaller ones and assign cells to them
             *
             * @param boxA the obtained pointer of new smaller SubBox
             * @param boxB another obtained pointer of new smaller SubBox
             */
            void spreadCellsV(SubBox **boxA, SubBox **boxB);

            /**
             * @brief get the top bin coordinate Y of the SubBox in bin grid
             *
             * @return int
             */
            inline int top()
            {
                return topBinY;
            }

            /**
             * @brief get the bottom bin coordinate Y of the SubBox in bin grid
             *
             * @return int
             */
            inline int bottom()
            {
                return bottomBinY;
            }

            /**
             * @brief get the left bin coordinate X of the SubBox in bin grid
             *
             * @return int
             */
            inline int left()
            {
                return leftBinX;
            }

            /**
             * @brief get the right bin coordinate X of the SubBox in bin grid
             *
             * @return int
             */
            inline int right()
            {
                return rightBinX;
            }

          private:
            int topBinY, bottomBinY, leftBinX, rightBinX;

            /**
             * @brief the cells in this SubBox
             *
             */
            std::vector<int> cellIds;
            int level;

          public:
            bool dirIsH;

            /**
             * @brief the minimum length of boundary
             *
             */
            int minExpandSize = 2;
        };

        static constexpr float eps = 1e-4;

      private:
        PlacementInfo *placementInfo;
        float totalCapacity;
        float totalUtilization;
        float overflowRatio;
        int topBinY, bottomBinY;
        int leftBinX, rightBinX;

        /**
         * @brief record the last selection of expansion direction for later expansion
         *
         */
        int curDirectionIndex = 0;

        /**
         * @brief make the direction into enum type for clearer identification
         *
         */
        dirType curDirection = expandLeft;

        /**
         * @brief a reference of the global bin grid for data accessing and updating of cell density
         *
         */
        std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &binGrid;

        /**
         * @brief the utilization of the four directions (absolute value)
         *
         */
        float dir2utilization[4];

        /**
         * @brief the capacity of the four directions (absolute value)
         *
         */
        float dir2capacity[4];

        /**
         * @brief a boolean array to record legal directions to expand the SpreadRegion
         *
         * legal direction means that
         *
         */
        bool legalDir[4];
        float capacityShrinkRatio = 1.0;

        /**
         * @brief a set of cells in the SpreadRegion
         *
         */
        std::set<DesignInfo::DesignCell *> cellsInRegion;

        /**
         * @brief a vector of cells in the SpreadRegion
         *
         * vector can ensure the iteration order to get rid of random factors in multi-threading
         */
        std::vector<DesignInfo::DesignCell *> cellsInRegionVec;

        /**
         * @brief a vector of bins in the SpreadRegion
         *
         * vector can ensure the iteration order to get rid of random factors in multi-threading
         */
        std::vector<PlacementInfo::PlacementBinInfo *> binsInRegion;

        /**
         * @brief a set of bins in the SpreadRegion
         *
         */
        std::set<PlacementInfo::PlacementBinInfo *> binSetInRegion;
    };

    /**
     * @brief update the information of the involved PlacementUnit(s)
     *
     * The inforation including cell location and the related bins should be updated.
     * Only the involved elements will lead to information update, which can reduce the runtime.
     *
     * @param involvedPUs a set of PlacementUnit which are involved in the cell spreading procedure
     * @param involvedCells a set of DesignCell which are involved in the cell spreading procedure
     * @param involvedPUVec a vector of PlacementUnit which are involved in the cell spreading procedure(ensure the
     * iteration order)
     * @param forgetRatio the forget ratio for the original location in last iteration
     * @param enableClockRegionAware spread but limit in specific clock region
     */
    void updatePlacementUnitsWithSpreadedCellLocations(std::set<PlacementInfo::PlacementUnit *> &involvedPUs,
                                                       std::set<DesignInfo::DesignCell *> &involvedCells,
                                                       std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec,
                                                       float forgetRatio, bool enableClockRegionAware);

    /**
     * @brief multi-threading workers for updating the information of the involved PlacementUnit(s)
     *
     * @param placementInfo PlacementInfo so this object can access the corresponding placement database
     * @param involvedPUs a set of PlacementUnit which are involved in the cell spreading procedure
     * @param involvedCells a set of DesignCell which are involved in the cell spreading procedure
     * @param involvedPUVec a vector of PlacementUnit which are involved in the cell spreading procedure(ensure the
     * iteration order)
     * @param forgetRatio the forget ratio for the original location in last iteration
     * @param startId the PlacementUnit range begin for this worker
     * @param endId  the PlacementUnit range end for this worker
     */
    static void updatePlacementUnitsWithSpreadedCellLocationsWorker(
        PlacementInfo *placementInfo, std::set<PlacementInfo::PlacementUnit *> &involvedPUs,
        std::set<DesignInfo::DesignCell *> &involvedCells, std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec,
        float forgetRatio, int startId, int endId);

    /**
     * @brief ensure the X/Y is in the legal range of the target device
     *
     * @param placementInfo PlacementInfo so this object can access the corresponding placement database
     * @param cellX inout reference to legalize X location for a given location X
     * @param cellY inout reference to legalize Y location for a given location Y
     */
    static void makeCellInLegalArea(PlacementInfo *placementInfo, float &cellX, float &cellY)
    {
        float eps = 1e-4;
        if (cellX < placementInfo->getGlobalMinX())
        {
            cellX = placementInfo->getGlobalMinX() + eps;
        }
        if (cellY < placementInfo->getGlobalMinY())
        {
            cellY = placementInfo->getGlobalMinY() + eps;
        }
        if (cellX > placementInfo->getGlobalMaxX())
        {
            cellX = placementInfo->getGlobalMaxX() - eps;
        }
        if (cellY > placementInfo->getGlobalMaxY())
        {
            cellY = placementInfo->getGlobalMaxY() - eps;
        }
    }

    /**
     * @brief record the spreaded location in PlacementInfo for later forget-ratio-based location updating
     *
     */
    void recordSpreadedCellLocations();

    void DumpCellsCoordinate(std::string dumpFileName, GeneralSpreader::SpreadRegion *curRegion);
    void DumpPUCoordinate(std::string dumpFileName, std::vector<PlacementInfo::PlacementUnit *> &involvedPUVec);

  private:
    PlacementInfo *placementInfo;
    std::map<std::string, std::string> &JSONCfg;
    std::string sharedCellType;
    int currentIteration = 0;
    float capacityShrinkRatio = 1.0;
    bool verbose;

    /**
     * @brief a reference of the global bin grid for data accessing and updating of cell density
     *
     */
    std::vector<std::vector<PlacementInfo::PlacementBinInfo *>> &binGrid;

    int dumpSiteGridDensityCnt = 0;
    int LUTFFCoordinateDumpCnt = 0;
    int nJobs = 1;

    /**
     * @brief simple expansion will iteratively try 4 directions to expand the SpreadRegion
     *
     * smart expansion will greedily select a promising direction to expand the SpreadRegion
     *
     */
    bool useSimpleExpland = false;
    bool enforceSimpleExpland = false;
    int dumpCnt = 0;

    /**
     * @brief a vector of the found overflow bins in the current placement
     *
     */
    std::vector<PlacementInfo::PlacementBinInfo *> overflowBins;

    /**
     * @brief a set of the found overflow bins in the current placement
     *
     */
    std::set<PlacementInfo::PlacementBinInfo *> overflowBinSet;

    /**
     * @brief a set of the bins covered by existing SpreadRegion(s)
     *
     */
    std::set<PlacementInfo::PlacementBinInfo *> coveredBinSet;

    void dumpSiteGridDensity(std::string dumpFileName);

    /**
     * @brief find the overflow bins in the placement accoridng to a given threshold
     *
     * @param overflowThreshold a given threshold
     */
    void findOverflowBins(float overflowThreshold);

    /**
     * @brief obtain a SpreadRegion by expanding a cell spreading window from an overflow bin
     *
     * @param curBin the initial bin for the SpreadRegion construction
     * @param capacityShrinkRatio shrink the area supply to a specific ratio
     * @param numBinThr the maximum number of bin in one
     * @return GeneralSpreader::SpreadRegion*
     */
    GeneralSpreader::SpreadRegion *expandFromABin(PlacementInfo::PlacementBinInfo *curBin, float capacityShrinkRatio,
                                                  unsigned int numBinThr = 1000000);

    /**
     * @brief the obtained SpreadRegion s which can be processed in parallel.
     *
     */
    std::vector<GeneralSpreader::SpreadRegion *> expandedRegions;
};

std::ostream &operator<<(std::ostream &os, GeneralSpreader::SpreadRegion::SubBox *curBox);
#endif