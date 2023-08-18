/**
 * @file IncrementalBELPacker.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of IncrementalBELPacker class and its internal modules and APIs
 * which incrementally pack some LUTs/FFs during global placement based on their distance, interconnection density and
 * compatibility
 * @version 0.1
 * @date 2021-10-02
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

#ifndef _INCREMENTALBELPACKER
#define _INCREMENTALBELPACKER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "KDTree/KDTree.h"
#include "PlacementInfo.h"
#include "PlacementTimingOptimizer.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief  IncrementalBELPacker incrementally packs some LUTs/FFs during global placement based on their distance,
 * interconnection density and compatibility
 *
 */
class IncrementalBELPacker
{
  public:
    /**
     * @brief Construct a new Incremental BEL Packer object
     *
     * @param designInfo given design information
     * @param deviceInfo given device information
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg  the user-defined placement configuration
     */
    IncrementalBELPacker(DesignInfo *designInfo, DeviceInfo *deviceInfo, PlacementInfo *placementInfo,
                         std::map<std::string, std::string> JSONCfg)
        : designInfo(designInfo), deviceInfo(deviceInfo), placementInfo(placementInfo),
          compatiblePlacementTable(placementInfo->getCompatiblePlacementTable()),
          placementUnits(placementInfo->getPlacementUnits()),
          placementUnpackedCells(placementInfo->getPlacementUnpackedCells()),
          placementMacros(placementInfo->getPlacementMacros()),
          fixedPlacementUnits(placementInfo->getFixedPlacementUnits()), cellInMacros(placementInfo->getCellInMacros()),
          cellId2PlacementUnit(placementInfo->getCellId2PlacementUnit()),
          cellId2PlacementUnitVec(placementInfo->getCellId2PlacementUnitVec()), JSONCfg(JSONCfg)
    {
        if (JSONCfg.find("y2xRatio") != JSONCfg.end())
        {
            y2xRatio = std::stof(JSONCfg["y2xRatio"]);
        }
    }

    /**
     * @brief check whether two LUTs can be packed to share one BEL
     *
     * @param LUTA
     * @param LUTB
     */
    void isLUTsPackable(PlacementInfo::PlacementUnpackedCell *LUTA, PlacementInfo::PlacementUnpackedCell *LUTB);

    /**
     * @brief try to pair LUTs/FFs in the design netlist which are neighbors according to a given threshold and
     * connectivity
     *
     * @param disThreshold a given threshold
     */
    void LUTFFPairing(float disThreshold);

    /**
     * @brief try to pair FFs in the design netlist which are neighbors according to a given threshold and the control
     * set compatibility
     *
     * @param disThreshold a given threshold
     */
    void FFPairing(float disThreshold);

    void dumpPairedLUTFF();

    inline float getCellDistance(PlacementInfo::Location &A, PlacementInfo::Location &B)
    {
        return fabs(A.X - B.X) + y2xRatio * fabs(A.Y - B.Y);
    }

    /**
     * @brief FFLocation records the FF cell pointer and the location of the FF cell for kdt::KDTree construction which
     * can help to find neighbors for cells
     *
     */
    class FFLocation : public std::array<float, 2>
    {
      public:
        // dimension of the Point
        static const int DIM = 2;
        FFLocation()
        {
            assert(false);
        }
        FFLocation(PlacementInfo::PlacementUnpackedCell *unpackedCell) : unpackedCell(unpackedCell)
        {
            (*this)[0] = unpackedCell->X();
            (*this)[1] = unpackedCell->Y();
        }

        inline PlacementInfo::PlacementUnpackedCell *getUnpackedCell()
        {
            return unpackedCell;
        }

      private:
        PlacementInfo::PlacementUnpackedCell *unpackedCell;
    };

  private:
    typedef struct _PUWithScore
    {
        PlacementInfo::PlacementUnit *PU;
        float score;

        _PUWithScore(PlacementInfo::PlacementUnit *PU, float score) : PU(PU), score(score)
        {
        }
    } PUWithScore;

    typedef struct _CellWithScore
    {
        DesignInfo::DesignCell *cell;
        float score;

        _CellWithScore(DesignInfo::DesignCell *cell, float score) : cell(cell), score(score)
        {
        }
    } CellWithScore;

    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;
    PlacementInfo *placementInfo;
    PlacementInfo::CompatiblePlacementTable *compatiblePlacementTable;
    std::vector<PlacementInfo::PlacementUnit *> &placementUnits;
    std::vector<PlacementInfo::PlacementUnpackedCell *> &placementUnpackedCells;
    std::vector<PlacementInfo::PlacementMacro *> &placementMacros;
    std::vector<PlacementInfo::PlacementUnit *> &fixedPlacementUnits;
    std::set<DesignInfo::DesignCell *> &cellInMacros;
    std::map<int, PlacementInfo::PlacementUnit *> &cellId2PlacementUnit;
    std::vector<PlacementInfo::PlacementUnit *> &cellId2PlacementUnitVec;
    std::map<std::string, std::string> JSONCfg;
    std::vector<int> placementNetId2LUTPlacementUnitId;

    std::vector<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> LUTFFPairs;
    std::vector<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> FF_FFPairs;
    int LUTFFPairDumpCnt = 0;
    float y2xRatio = 1.0;
};

#endif