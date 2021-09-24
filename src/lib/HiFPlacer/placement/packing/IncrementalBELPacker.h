#ifndef _INCREMENTALBELPACKER
#define _INCREMENTALBELPACKER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "PlacementInfo.h"
#include "KDTree/KDTree.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class IncrementalBELPacker
{
  public:
    IncrementalBELPacker(DesignInfo *designInfo, DeviceInfo *deviceInfo, PlacementInfo *placementInfo,
                         std::map<std::string, std::string> &JSONCfg)
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

    void isLUTsPackable(PlacementInfo::PlacementUnpackedCell *LUTA, PlacementInfo::PlacementUnpackedCell *LUTB);
    void LUTFFPairing(float disThreshold);
    void FFPairing(float disThreshold);

    void dumpPairedLUTFF();

    inline float getCellDistance(PlacementInfo::Location &A, PlacementInfo::Location &B)
    {
        return fabs(A.X - B.X) + y2xRatio * fabs(A.Y - B.Y);
    }

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
    std::map<std::string, std::string> &JSONCfg;
    std::vector<int> placementNetId2LUTPlacementUnitId;

    std::vector<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> LUTFFPairs;
    std::vector<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> FF_FFPairs;
    int LUTFFPairDumpCnt = 0;
    float y2xRatio = 1.0;
};

#endif