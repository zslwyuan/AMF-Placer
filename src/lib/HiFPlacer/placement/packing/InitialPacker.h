#ifndef _INITIALPACKER
#define _INITIALPACKER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "PlacementInfo.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief InitialPacker will identify macros from the design netlist based on pattern matching
 *
 */
class InitialPacker
{
  public:
    /**
     * @brief Construct a new Initial Packer object
     *
     * @param designInfo given design information
     * @param deviceInfo given device information
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg  the user-defined placement configuration
     */
    InitialPacker(DesignInfo *designInfo, DeviceInfo *deviceInfo, PlacementInfo *placementInfo,
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
    }

    /**
     * @brief extract the macros from the netlist to construction PlacmentMacro
     *
     */
    void pack();

    /**
     * @brief BFS to find the core cells of a macro based on some pre-defined patterns of cascaded cells
     *
     * @param portPattern a given set of port patterns on the element to detect cascading interconnection
     * @param startCell a start cell for the search initialization
     * @param exactMatch indicate whether the portPattern should be exactly matched otherwise, containment relationship
     * will be accepted as matched.
     * @return std::vector<DesignInfo::DesignCell *>
     */
    std::vector<DesignInfo::DesignCell *>
    BFSExpandViaSpecifiedPorts(std::string portPattern, DesignInfo::DesignCell *startCell, bool exactMatch = false);
    std::vector<DesignInfo::DesignCell *> BFSExpandViaSpecifiedPorts(std::vector<std::string> portPatterns,
                                                                     DesignInfo::DesignCell *startCell,
                                                                     bool exactMatch = false);

    /**
     * @brief detects DSP macros and clusters the related cells into PlacementMacro
     *
     * If two DSPs are interconnected via their "ACIN[", "BCIN[", "PCIN[" ports, they are cascaded.
     *
     */
    void findDSPMacros();

    /**
     * @brief detects BRAM macros and clusters the related cells into PlacementMacro
     *
     * If two BRAMs are interconnected via their "CAS" ports, they are cascaded.
     *
     */
    void findBRAMMacros();
    std::vector<DesignInfo::DesignCell *> checkCompatibleFFs(std::vector<DesignInfo::DesignCell *> FFs);
    void findCARRYMacros();
    void findMuxMacros();
    void loadOtherCLBMacros(std::string RAMMacroListFromVivadoFileName);
    void findUnpackedUnits();
    void loadFixedPlacementUnits(std::string fixedPlacementUnitsFromVivadoFileName);
    void loadNets();
    void LUTFFPairing();
    void dumpMacroHighLight();
    void enhanceIONets();

  private:
    class PackedControlSet
    {
      public:
        PackedControlSet()
        {
            FFs.clear();
        };

        PackedControlSet(DesignInfo::DesignCell *curFF)
            : CSId(curFF->getControlSetInfo()->getId()), CLK(curFF->getControlSetInfo()->getCLK()),
              SR(curFF->getControlSetInfo()->getSR()), CE(curFF->getControlSetInfo()->getCE())
        {
            FFs.clear();
            FFs.push_back(curFF);
        };

        PackedControlSet(const PackedControlSet &anotherControlSet)
        {
            FFs.clear();
            assert(anotherControlSet.getSize() > 0 && "the other one control set should not be empty.");
            CSId = anotherControlSet.getCSId();
            FFs = anotherControlSet.getFFs();
        };

        PackedControlSet &operator=(const PackedControlSet &anotherControlSet)
        {
            FFs.clear();
            assert(anotherControlSet.getSize() > 0 && "the other one control set should not be empty.");
            CSId = anotherControlSet.getCSId();
            FFs = anotherControlSet.getFFs();
            return *this;
        };

        ~PackedControlSet(){};

        inline unsigned int getSize() const
        {
            return FFs.size();
        }

        inline const std::vector<DesignInfo::DesignCell *> &getFFs() const
        {
            return FFs;
        }

        inline void addFF(DesignInfo::DesignCell *curFF)
        {
            if (CSId < 0)
            {
                if (!curFF->isVirtualCell())
                {
                    assert(curFF->getControlSetInfo());
                    CSId = curFF->getControlSetInfo()->getId();
                    CLK = curFF->getControlSetInfo()->getCLK();
                    SR = curFF->getControlSetInfo()->getSR();
                    CE = curFF->getControlSetInfo()->getCE();
                }
            }
            else
            {
                if (!curFF->isVirtualCell())
                {
                    assert(curFF->getControlSetInfo()->getId() == CSId);
                }
            }
            FFs.push_back(curFF);
        }

        inline void removeXthFF(int i)
        {
            FFs.erase(FFs.begin() + i);
        }

        inline int findFF(DesignInfo::DesignCell *curFF)
        {
            for (unsigned int i = 0; i < FFs.size(); i++)
            {
                if (FFs[i] == curFF)
                    return i;
            }
            return -1;
        }

        inline int getCSId() const
        {
            return CSId;
        }

        inline void setCSId(int _CSId)
        {
            CSId = _CSId;
        }

        inline DesignInfo::DesignNet *getCLK()
        {
            assert(CSId >= 0);
            return CLK;
        }

        inline DesignInfo::DesignNet *getSR()
        {
            assert(CSId >= 0);
            return SR;
        }

      private:
        int CSId = -1;
        DesignInfo::DesignNet *CLK = nullptr;
        DesignInfo::DesignNet *SR = nullptr;
        DesignInfo::DesignNet *CE = nullptr;
        std::vector<DesignInfo::DesignCell *> FFs;
    };

    class SiteBELMapping
    {
      public:
        SiteBELMapping()
        {
            Carry = nullptr;
            for (int i = 0; i < 2; i++)
            {
                MuxF8[i] = nullptr;
                for (int j = 0; j < 2; j++)
                {
                    MuxF7[i][j] = nullptr;
                    for (int k = 0; k < 4; k++)
                    {
                        LUTs[i][j][k] = nullptr;
                        FFs[i][j][k] = nullptr;
                    }
                }
            }
        }

        ~SiteBELMapping()
        {
        }

        DesignInfo::DesignCell *LUTs[2][2][4]; // [bottom_Or_Top][6 or 5][which Slot]
        DesignInfo::DesignCell *FFs[2][2][4];  // [bottom_Or_Top][FF or FF2][which Slot]
        DesignInfo::DesignCell *MuxF7[2][2];   // [bottom_Or_Top][which Slot]
        DesignInfo::DesignCell *MuxF8[2];
        DesignInfo::DesignCell *Carry;
    };

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

    void mapCarryRelatedRouteThru(PlacementInfo::PlacementMacro *CARRYChain, DesignInfo::DesignCell *coreCell,
                                  float CARRYChainSiteOffset);
};

#endif