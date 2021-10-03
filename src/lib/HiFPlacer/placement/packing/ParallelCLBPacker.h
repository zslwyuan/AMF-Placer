/**
 * @file ParallelCLBPacker.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _PARALLELCLBPACKER_
#define _PARALLELCLBPACKER_

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "const.h"
#include "dumpZip.h"
#include "KDTree/KDTree.h"
#include "MaximalCardinalityMatching/MaximalCardinalityMatching.h"
#include "PlacementInfo.h"
#include "readZip.h"
#include "strPrint.h"
#include "stringCheck.h"
#include <assert.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <omp.h>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// implemented based on the paper's Algorithm 1:
// W. Li and D. Z. Pan, "A New Paradigm for FPGA Placement Without Explicit Packing,"
// in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 38, no. 11, pp. 2113-2126,
// Nov. 2019, doi: 10.1109/TCAD.2018.2877017.

/**
 * @brief a utility struct for the comparison between PlacementInfo::PlacementUnit according to PU ID
 *
 * The default STL set for PlacementUnit pointers will lead to random iteration order of the PlacementUnits in a set and
 * lead to slight variation in the final packing result.
 *
 */
struct Packing_PUcompare
{
    inline bool operator()(PlacementInfo::PlacementUnit *lhs, PlacementInfo::PlacementUnit *rhs) const
    {
        return lhs->getId() < rhs->getId();
    }
};

/**
 * @brief ParallelCLBPacker will finally pack LUT/FF/MUX/CARRY elements into legal CLB sites in a parallel approach.
 *
 * implemented based on the paper's Algorithm 1:
 * W. Li and D. Z. Pan, "A New Paradigm for FPGA Placement Without Explicit Packing,"
 * in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, vol. 38, no. 11, pp. 2113-2126,
 * Nov. 2019, doi: 10.1109/TCAD.2018.2877017.
 *
 * We also provide many detailed optimization techniques according to our observation, macro constraints, timing
 * demands, and the application characteristics.
 *
 */
class ParallelCLBPacker
{
  public:
    /**
     * @brief Construct a new Parallel CLB Packer object
     *
     * @param designInfo given design information
     * @param deviceInfo given device information
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg  the user-defined placement configuration
     * @param unchangedIterationThr specify how many iterations a PlacementUnit should stay at the top priority of a
     * site before we finally map it to the site
     * @param numNeighbor the threshold number of cells for site
     * @param deltaD the increase step of the neighbor search diameter
     * @param curD current neighbor search diameter
     * @param maxD the maximum constraint of the neighbor search diameter
     * @param PQSize the size of priority queue (the low-priority candidates will be removed)
     * @param HPWLWeight the factor of HPWL overhead in packing evaluation for a cell
     * @param packerName the name of this packer
     */
    ParallelCLBPacker(DesignInfo *designInfo, DeviceInfo *deviceInfo, PlacementInfo *placementInfo,
                      std::map<std::string, std::string> &JSONCfg, int unchangedIterationThr, int numNeighbor,
                      float deltaD, float curD, float maxD, int PQSize, float HPWLWeight, std::string packerName);

    ~ParallelCLBPacker()
    {
        for (auto packingSite : packingSites)
            delete packingSite;
    }

    class PackedControlSet
    {
      public:
        PackedControlSet()
        {
            FFs.clear();
        };

        // PackedControlSet(DesignInfo::DesignCell *curFF) : CSId(curFF->getControlSetInfo()->getId())
        // {
        //     FFs.clear();
        //     FFs.push_back(curFF);
        // };

        PackedControlSet(const PackedControlSet &anotherControlSet)
        {
            FFs.clear();
            assert((anotherControlSet.getSize() > 0 || anotherControlSet.getCSId() < 0) &&
                   "the other one control set should not be empty.");
            CSId = anotherControlSet.getCSId();
            FFs = anotherControlSet.getFFs();
            if (CSId >= 0)
            {
                CLK = anotherControlSet.getCLK();
                SR = anotherControlSet.getSR();
                CE = anotherControlSet.getCE();
                FFType = anotherControlSet.getFFType();
            }
            else
            {
                CLK = nullptr;
                SR = nullptr;
                CE = nullptr;
            }
        };

        PackedControlSet &operator=(const PackedControlSet &anotherControlSet)
        {
            FFs.clear();
            assert((anotherControlSet.getSize() > 0 || anotherControlSet.getCSId() < 0) &&
                   "the other one control set should not be empty.");
            CSId = anotherControlSet.getCSId();
            FFs = anotherControlSet.getFFs();
            if (CSId >= 0)
            {
                CLK = anotherControlSet.getCLK();
                SR = anotherControlSet.getSR();
                CE = anotherControlSet.getCE();
                FFType = anotherControlSet.getFFType();
            }
            else
            {
                CLK = nullptr;
                SR = nullptr;
                CE = nullptr;
            }
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

        inline void reset()
        {
            assert(FFs.size() == 0);
            CSId = -1;
            CLK = nullptr;
            SR = nullptr;
            CE = nullptr;
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
                    FFType = curFF->getOriCellType();
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

        inline DesignInfo::DesignNet *getCLK() const
        {
            assert(CSId >= 0);
            return CLK;
        }

        inline DesignInfo::DesignNet *getSR() const
        {
            assert(CSId >= 0);
            return SR;
        }

        inline DesignInfo::DesignNet *getCE() const
        {
            assert(CSId >= 0);
            return CE;
        }

        inline DesignInfo::DesignCellType getFFType() const
        {
            assert(CSId >= 0);
            return FFType;
        }

        inline bool compatibleWith(int inputCSId)
        {
            if (CSId == -1)
                return true;
            return inputCSId == CSId;
        }

      private:
        int CSId = -1;
        DesignInfo::DesignNet *CLK = nullptr;
        DesignInfo::DesignNet *SR = nullptr;
        DesignInfo::DesignNet *CE = nullptr;
        DesignInfo::DesignCellType FFType;
        std::vector<DesignInfo::DesignCell *> FFs;
    };

    class PackingCLBSite
    {
      public:
        PackingCLBSite(PlacementInfo *placementInfo, DeviceInfo::DeviceSite *CLBSite, int unchangedIterationThr,
                       int numNeighbor, float deltaD, float curD, float maxD, unsigned int PQSize, float y2xRatio,
                       float HPWLWeight, std::vector<PackingCLBSite *> &PUId2PackingCLBSite)
            : placementInfo(placementInfo), CLBSite(CLBSite), unchangedIterationThr(unchangedIterationThr),
              numNeighbor(numNeighbor), deltaD(deltaD), curD(curD), maxD(maxD), PQSize(PQSize), y2xRatio(y2xRatio),
              HPWLWeight(HPWLWeight), PUId2PackingCLBSite(PUId2PackingCLBSite), determinedClusterInSite(nullptr)
        {
            neighborPUs.clear();
            seedClusters.clear();
            seedClusters.clear();
            priorityQueue.clear();
            PU2TopCnt.clear();
            // PU2HPWLChange.clear();
        }

        ~PackingCLBSite()
        {
            if (determinedClusterInSite)
            {
                delete determinedClusterInSite;
            }
            for (auto tmpCluster : priorityQueue)
            {
                delete tmpCluster;
            }
        }

        class PackingCLBCluster
        {
          public:
            PackingCLBCluster()
            {
                assert(false && "PackingCLBCluster should not initialize without parameters \"parentPackingCLB\". This "
                                "problem might be caused by resizing vector to a longer one");
            }
            PackingCLBCluster(PackingCLBSite *parentPackingCLB) : parentPackingCLB(parentPackingCLB)
            {
                id = random();
                PUs.clear();
                FFControlSets.clear();
                FFControlSets.resize(4);
                singleLUTs.clear();
                pairedLUTs.clear();
                HPWLWeight = parentPackingCLB->getHPWLWeight();
                // nets.clear();
            }
            ~PackingCLBCluster(){};

            PackingCLBCluster(PackingCLBCluster *anotherPackingCLBCluster)
            {
                id = anotherPackingCLBCluster->getId();
                FFControlSets = anotherPackingCLBCluster->getFFControlSets();
                singleLUTs = anotherPackingCLBCluster->getSingleLUTs();
                pairedLUTs = anotherPackingCLBCluster->getPairedLUTs();
                PUs = anotherPackingCLBCluster->getPUs();
                scoreInSite = anotherPackingCLBCluster->getScoreInSite();
                parentPackingCLB = anotherPackingCLBCluster->getParentPackingCLB();
                // net2ConnectivityScore = anotherPackingCLBCluster->getNet2ConnectivityScore();
                HPWLChange = anotherPackingCLBCluster->getHPWLChange();
                totalConnectivityScore = anotherPackingCLBCluster->getTotalConnectivityScore();
                totalCellNum = anotherPackingCLBCluster->getTotalCellNum();
                totalLen = anotherPackingCLBCluster->getTotalLen();
                HPWLWeight = anotherPackingCLBCluster->getHPWLWeight();
                numMuxes = anotherPackingCLBCluster->getNumMuxes();
            }

            inline int getId() const
            {
                return id;
            }

            inline void refreshId()
            {
                id = random();
            }

            inline unsigned int getPairPinNum(DesignInfo::DesignCell *LUTA, DesignInfo::DesignCell *LUTB)
            {
                if (LUTA->getInputPins().size() == 6 || LUTB->getInputPins().size() == 6 || LUTA->isLUT6() ||
                    LUTB->isLUT6())
                    return 12;

                int pinNumA = 0;
                int totalPin = 0;
                int netIds[5]; // be aware that a LUT might have pins connected to the same net and they should be
                               // treated as different inputs.

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

            void maxCardinalityMatching(bool verbose = false);

            int getInternalPinsNum(PlacementInfo::PlacementNet *curNet);
            bool compatibleInOneHalfCLB(DesignInfo::ControlSetInfo *CSPtr, int anotherHalfCLB);
            bool addLUT(DesignInfo::DesignCell *curLUT);

            bool addToFFSet(DesignInfo::DesignCell *curFF, int halfCLB);
            bool addToFFSet(std::vector<DesignInfo::DesignCell *> curFFs, int halfCLB);
            bool addFF(DesignInfo::DesignCell *curFF, int enforceHalfCLB = -1, bool enforceMainFFSlot = false);
            bool addFFGroup(std::vector<DesignInfo::DesignCell *> curFFs, int enforceHalfCLB, bool enforceMainFFSlot,
                            bool isMuxMacro);
            void removeLUT(DesignInfo::DesignCell *curLUT)
            {
                if (singleLUTs.find(curLUT) != singleLUTs.end())
                {
                    singleLUTs.erase(curLUT);
                    return;
                }
                for (auto LUTPair : pairedLUTs)
                {
                    if (LUTPair.first == curLUT)
                    {
                        singleLUTs.insert(LUTPair.second);
                        pairedLUTs.erase(LUTPair);
                        return;
                    }
                    else if (LUTPair.second == curLUT)
                    {
                        singleLUTs.insert(LUTPair.first);
                        pairedLUTs.erase(LUTPair);
                        return;
                    }
                }
                assert(false && "should be erased successfully");
            }

            void removeFF(DesignInfo::DesignCell *curFF)
            {
                unsigned int i = -1;
                for (i = 0; i < FFControlSets.size(); i++)
                {
                    int findFFLoc = FFControlSets[i].findFF(curFF);
                    if (findFFLoc >= 0)
                    {
                        FFControlSets[i].removeXthFF(findFFLoc);
                        if (FFControlSets[i].getSize() == 0)
                        {
                            FFControlSets[i].reset();
                        }
                        return;
                    }
                }
                assert(false && "should not reach here");
            }

            void removePUToConstructDetCluster(PlacementInfo::PlacementUnit *tmpPU)
            {
                assert(PUs.find(tmpPU) != PUs.end());
                std::vector<DesignInfo::DesignCell *> cellsToRemove(0);
                if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                {
                    cellsToRemove.push_back(unpackCell->getCell());
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                {
                    for (auto tmpCell : curMacro->getCells())
                        cellsToRemove.push_back(tmpCell);
                }
                //  assert(checkCellCorrectness(tmpPU, false));
                for (auto curCell : cellsToRemove)
                {
                    if (curCell->isLUT())
                    {
                        removeLUT(curCell);
                    }
                    else if (curCell->isFF())
                    {
                        removeFF(curCell);
                    }
                    else
                    {
                        assert(curCell->isMux());
                        // assert(false && "unexpected type.");
                    }
                }
                PUs.erase(tmpPU);
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                {
                    if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX7 ||
                        tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_MUX8)
                    {
                        numMuxes--;
                    }
                }
                //  assert(checkCellCorrectness(tmpPU, false));
                hashed = false;
            }

            // the addPU implemented is based on the following paper's Algorithm 1:
            // G. Chen et al., “RippleFPGA: Routability-Driven Simultaneous Packing and Placement for Modern FPGAs,”
            // IEEE Trans. Comput.-Aided Des. Integr. Circuits Syst., vol. 37, no. 10, pp. 2022–2035,
            //  Oct. 2018, doi: 10.1109/TCAD.2017.2778058.

            inline bool canPUInSite(PlacementInfo::PlacementUnit *tmpPU)
            {
                if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                {
                    if (tmpMacro->getMacroType() == PlacementInfo::PlacementMacro::PlacementMacroType_LCLB)
                    {
                        if (parentPackingCLB->getCLBSite()->getSiteType() == "SLICEM")
                            return false;
                    }
                }
                return true;
            }

            bool addPU(PlacementInfo::PlacementUnit *tmpPU, bool allowOverlap = false);

            inline bool tryAddPU(PlacementInfo::PlacementUnit *tmpPU)
            {
                PackingCLBCluster *fakeCluster = new PackingCLBCluster(this);
                if (fakeCluster->addPU(tmpPU, false))
                {
                    FFControlSets = fakeCluster->getFFControlSets();
                    singleLUTs = fakeCluster->getSingleLUTs();
                    pairedLUTs = fakeCluster->getPairedLUTs();
                    PUs.insert(tmpPU);
                    delete fakeCluster;
                    return true;
                }
                else
                {
                    delete fakeCluster;
                    return false;
                }
            }

            void addPUFailReason(PlacementInfo::PlacementUnit *tmpPU);

            inline bool contains(PlacementInfo::PlacementUnit *tmpPU)
            {
                return PUs.find(tmpPU) != PUs.end();
            }

            inline const std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> &getPUs() const
            {
                return PUs;
            }

            inline const std::vector<PackedControlSet> &getFFControlSets() const
            {
                return FFControlSets;
            }

            inline const std::set<DesignInfo::DesignCell *> &getSingleLUTs() const
            {
                return singleLUTs;
            }

            inline void removeSingleLUT(DesignInfo::DesignCell *tmpLUT)
            {
                assert(singleLUTs.find(tmpLUT) != singleLUTs.end());
                singleLUTs.erase(tmpLUT);
            }

            inline bool tryRemoveSingleLUT(DesignInfo::DesignCell *tmpLUT)
            {
                if (singleLUTs.find(tmpLUT) != singleLUTs.end())
                {
                    singleLUTs.erase(tmpLUT);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            inline bool tryRemoveSingleLUTFromPairs(DesignInfo::DesignCell *tmpLUT)
            {
                for (auto pair : pairedLUTs)
                {
                    if (tmpLUT == pair.first)
                    {
                        singleLUTs.insert(pair.second);
                        pairedLUTs.erase(pair);
                        return true;
                    }
                    if (tmpLUT == pair.second)
                    {
                        singleLUTs.insert(pair.first);
                        pairedLUTs.erase(pair);
                        return true;
                    }
                }
                return false;
            }

            inline bool tryRemoveLUTPairFromPairs(DesignInfo::DesignCell *tmpLUTA, DesignInfo::DesignCell *tmpLUTB)
            {
                for (auto pair : pairedLUTs)
                {
                    if (tmpLUTA == pair.first)
                    {
                        assert(pair.second == tmpLUTB);
                        pairedLUTs.erase(pair);
                        return true;
                    }
                    if (tmpLUTA == pair.second)
                    {
                        assert(pair.first == tmpLUTB);
                        pairedLUTs.erase(pair);
                        return true;
                    }
                }
                return false;
            }

            inline const std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> &getPairedLUTs() const
            {
                return pairedLUTs;
            }

            inline bool isValid(const std::vector<PackingCLBSite *> &PUId2PackingCLBSite,
                                PackingCLBSite *parentPackingCLBSite)
            {
                for (auto tmpPU : PUs)
                {
                    if (PUId2PackingCLBSite[tmpPU->getId()])
                    {
                        if (PUId2PackingCLBSite[tmpPU->getId()] != parentPackingCLBSite)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }

            inline float getScoreInSite() const
            {
                return scoreInSite;
            }

            void updateScoreInSite();

            void incrementalUpdateScoreInSite(PlacementInfo::PlacementUnit *tmpPU);

            inline PackingCLBSite *getParentPackingCLB() const
            {
                return parentPackingCLB;
            }

            void clusterHash()
            {
                hashId = PUs.size();
                if (parentPackingCLB->getCarryMacro())
                {
                    hashId += ReguPlacer_hashprimes[(unsigned char)(~(parentPackingCLB->getCarrySiteOffset()) & 0xff)];
                    hashId %= 10001777;
                    hashId +=
                        ReguPlacer_hashprimes[(unsigned char)(~(parentPackingCLB->getCarryMacro()->getId()) & 0xff)];
                    hashId %= 10001777;
                }
                if (parentPackingCLB->getLUTRAMMacro())
                {
                    hashId +=
                        ReguPlacer_hashprimes[(unsigned char)(~(parentPackingCLB->getLUTRAMMacro()->getId()) & 0xff)];
                    hashId %= 10001777;
                }
                for (auto tmpPU : PUs)
                {
                    if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                    {
                        hashId += ReguPlacer_hashprimes[(unsigned char)(~(unpackCell->getCell()->getCellId()) & 0xff)] *
                                  unpackCell->getCell()->getCellId();
                        hashId %= 10001777;
                    }
                    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                    {
                        for (auto tmpCell : curMacro->getCells())
                        {
                            hashId += ReguPlacer_hashprimes[(unsigned char)(~(tmpCell->getCellId()) & 0xff)] *
                                      tmpCell->getCellId();
                            hashId %= 10001777;
                        }
                    }
                }
                hashed = true;
            }

            inline int clusterHashWithAdditionalPU(PlacementInfo::PlacementUnit *tmpPU)
            {

                int clusterHashId = getHash();

                if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                {
                    clusterHashId +=
                        ReguPlacer_hashprimes[(unsigned char)(~(unpackCell->getCell()->getCellId()) & 0xff)] *
                        unpackCell->getCell()->getCellId();
                    clusterHashId %= 10001777;
                }
                else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                {
                    for (auto tmpCell : curMacro->getCells())
                    {
                        clusterHashId += ReguPlacer_hashprimes[(unsigned char)(~(tmpCell->getCellId()) & 0xff)] *
                                         tmpCell->getCellId();
                        clusterHashId %= 10001777;
                    }
                }

                return clusterHashId;
            }

            inline int getHash()
            {
                if (!hashed)
                    clusterHash();
                return hashId;
            }

            inline int getHashConst() const
            {
                int hashId = 0;
                std::vector<DesignInfo::DesignCell *> cellsToCheck(0);
                for (auto tmpPU : PUs)
                {
                    if (auto unpackCell = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(tmpPU))
                    {
                        hashId += 28901 * unpackCell->getCell()->getCellId();
                        hashId %= 10001777;
                    }
                    else if (auto curMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(tmpPU))
                    {
                        for (auto tmpCell : curMacro->getCells())
                        {
                            hashId += 28901 * tmpCell->getCellId();
                            hashId %= 10001777;
                        }
                    }
                }
                return hashId;
            }

            inline float getHPWLChangeForPU(PlacementInfo::PlacementUnit *tmpPU)
            {
                return parentPackingCLB->getHPWLChangeForPU(tmpPU);
            }

            inline float getTotalConnectivityScore() const
            {
                return totalConnectivityScore;
            }
            inline float getHPWLChange() const
            {
                return HPWLChange;
            }
            inline int getTotalCellNum() const
            {
                return totalCellNum;
            }
            inline int getNumMuxes() const
            {
                return numMuxes;
            }
            inline int getTotalLen() const
            {
                return totalLen;
            }
            inline float getTotalCellWeight() const
            {
                float totalCellWeight = 0;
                for (auto curCell : singleLUTs)
                {
                    totalCellWeight +=
                        parentPackingCLB->getPlacementInfo()->getActualOccupationByCellId(curCell->getCellId());
                }
                for (auto pair : pairedLUTs)
                {
                    totalCellWeight +=
                        parentPackingCLB->getPlacementInfo()->getActualOccupationByCellId(pair.first->getCellId());
                    totalCellWeight +=
                        parentPackingCLB->getPlacementInfo()->getActualOccupationByCellId(pair.second->getCellId());
                }
                for (auto &CS : FFControlSets)
                {
                    for (auto curCell : CS.getFFs())
                    {
                        totalCellWeight +=
                            parentPackingCLB->getPlacementInfo()->getActualOccupationByCellId(curCell->getCellId());
                    }
                }
                return totalCellWeight;
            }
            inline float getHPWLWeight() const
            {
                return HPWLWeight;
            }

            inline bool containFF(DesignInfo::DesignCell *curFF)
            {
                for (unsigned int i = 0; i < FFControlSets.size(); i++)
                {
                    if (FFControlSets[i].findFF(curFF) >= 0)
                        return true;
                }
                return false;
            }

            void printMyself();

            bool checkCellCorrectness(PlacementInfo::PlacementUnit *tmpPU, bool isAddPU);

            bool checkNumMuxCompatibleInFFSet(int i, int addNum);

            inline int getPlacementUnitMaxPathLen(PlacementInfo::PlacementUnit *curPU)
            {

                auto &timingNodes =
                    parentPackingCLB->getPlacementInfo()->getTimingInfo()->getSimplePlacementTimingInfo();
                if (auto unpacked = dynamic_cast<PlacementInfo::PlacementUnpackedCell *>(curPU))
                {
                    if (unpacked->getCell()->isVirtualCell())
                        return 0;
                    return timingNodes[unpacked->getCell()->getCellId()]->getLongestPathLength();
                }
                else if (auto tmpMacro = dynamic_cast<PlacementInfo::PlacementMacro *>(curPU))
                {
                    int maxLen = 0;
                    for (auto tmpCell : tmpMacro->getCells())
                    {
                        if (tmpCell->isVirtualCell())
                            continue;
                        int len = timingNodes[tmpCell->getCellId()]->getLongestPathLength();
                        if (len > maxLen)
                            maxLen = len;
                    }
                    return maxLen;
                }
                return 0;
            }

          private:
            const unsigned int MaxNum_ControlSet = 4;
            const unsigned int MaxNum_FFinControlSet = 4;
            const unsigned int MaxNum_LUTSite = 8;
            int numMuxes = 0;

            PackingCLBSite *parentPackingCLB = nullptr;
            float scoreInSite = -100000000;
            int hashId = -3654;
            bool hashed = false;

            int id = -1;
            std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> PUs;

            float totalConnectivityScore = 0;
            float HPWLChange = 0;
            int totalCellNum = 0;
            int totalLen = 0;
            float HPWLWeight = 0.01;

            // int muxF7Limit = 2;
            // int muxF8Limit = 1;
            // std::map<PlacementInfo::PlacementNet *, float> net2ConnectivityScore;
            // std::set<PlacementInfo::PlacementNet *> nets;

            // please note that some of these LUT/FFs are belong to CARRY chain, which is not shown in PUs
            std::vector<PackedControlSet> FFControlSets;
            std::set<DesignInfo::DesignCell *> singleLUTs;
            std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> pairedLUTs;
        };

        inline bool isPQTopCompletelyAccptedByCells()
        {
            auto qTop = priorityQueue[0];
            for (auto tmpPU : qTop->getPUs())
            {
                if (PUId2PackingCLBSite[tmpPU->getId()] != this)
                {
                    return false;
                }
            }
            return true;
        }

        std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> *
        findNeiborPUsFromBinGrid(DesignInfo::DesignCellType curCellType, float targetX, float targetY,
                                 float displacementLowerbound, float displacementUpperbound, int PUNumThreshold,
                                 const std::vector<PackingCLBSite *> &PUId2PackingCLBSite, float y2xRatio,
                                 std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> *res = nullptr);

        inline std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> &getNeighborPUs()
        {
            return neighborPUs;
        }

        void refreshPrioryQueue();

        void removeInvalidClustersFromPQ();

        void removeInvalidClustersFromNeighborPUs();

        void removeClustersIncompatibleWithDetClusterFromPQ();

        void findNewClustersWithNeighborPUs();

        void updateStep(bool initial, bool debug = false);

        inline bool hasValidPQTop()
        {
            return priorityQueue.size();
        }

        const PackingCLBCluster *getPriorityQueueTop()
        {
            assert(priorityQueue.size());
            return priorityQueue[0];
        }

        inline DeviceInfo::DeviceSite *getCLBSite()
        {
            return CLBSite;
        }

        inline float getY2xRatio() const
        {
            return y2xRatio;
        }

        inline float getDetScore()
        {
            return detScore;
        }

        inline PackingCLBCluster *getDeterminedClusterInSite()
        {
            return determinedClusterInSite;
        }

        inline void setDeterminedClusterInSite(PackingCLBCluster *tmpCluster)
        {
            determinedClusterInSite = tmpCluster;
        }

        void updateConsistenPUsInTop();

        inline float getHPWLChangeForPU(PlacementInfo::PlacementUnit *tmpPU)
        {
            float changeHPWL = 0;
            for (auto tmpNet : *tmpPU->getNetsSetPtr())
            {
                if (tmpNet->getUnits().size() > 64) // ignore large net
                    continue;
                float newHPWL = tmpNet->getNewHPWLByTrying(tmpPU, CLBSite->X(), CLBSite->Y(), y2xRatio);
                float oriHPWL = tmpNet->getHPWL(y2xRatio);
                if (std::isnan(newHPWL) || std::isnan(oriHPWL))
                {
#pragma omp critical
                    {
                        // TODO: Why there is such BUG???
                        int o = 0;
                        std::cout << "curPU === \n" << tmpPU << "\n";
                        std::cout << "CLBSite->X()=" << CLBSite->X() << " CLBSite->Y()=" << CLBSite->Y()
                                  << " y2xRatio=" << y2xRatio << "\n";
                        for (auto tmpPU0 : tmpNet->getUnits())
                        {
                            auto tmpPinOffset = tmpNet->getPinOffsetsInUnit()[o];
                            std::cout << "PUXoffset=" << tmpPinOffset.x << " PUYoffset=" << tmpPinOffset.y << "\n";
                            std::cout << "PU=" << tmpPU0 << "\n";
                            o++;
                        }
                        float newHPWL1 = tmpNet->getNewHPWLByTrying(tmpPU, CLBSite->X(), CLBSite->Y(), y2xRatio);
                        float oriHPWL1 = tmpNet->getHPWL(y2xRatio);
                        std::cout << "newHPWL1=" << newHPWL1 << " oriHPWL1=" << oriHPWL1 << "\n";
                        assert(false);
                    }
                }
                changeHPWL += newHPWL - oriHPWL;
            }
            return changeHPWL;
        }

        inline float getHPWLWeight() const
        {
            return HPWLWeight;
        }

        inline void setDebug()
        {
            debug = true;
        }

        inline PlacementInfo *getPlacementInfo() const
        {
            return placementInfo;
        }

        inline void addCarry()
        {
            assert(determinedClusterInSite == nullptr);
            assert(CARRYChain);
            determinedClusterInSite = new PackingCLBCluster(this);
            for (auto prefixedSingleLUT : conflictLUTs)
            {
                assert(fixedLUTsInPairs.find(prefixedSingleLUT) == fixedLUTsInPairs.end());
                // these are fixed LUT Pairs, should not be added
                assert(determinedClusterInSite->addLUT(prefixedSingleLUT));
            }
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        if (slotMapping.FFs[i][j][k])
                        {
                            //  if (!slotMapping.FFs[i][j][k]->isVirtualCell())
                            bool succ = determinedClusterInSite->addFF(slotMapping.FFs[i][j][k], i * 2 + j);
                            if (!succ)
                            {
                                determinedClusterInSite->addFF(slotMapping.FFs[i][j][k], i * 2 + j);
                                std::cout << CARRYChain << " \n =====================================\n targetFF:"
                                          << slotMapping.FFs[i][j][k] << "\n";
                                std::cout << "slotMapping.FFs[i][j][k].CS=\n";
                                if (!slotMapping.FFs[i][j][k]->isVirtualCell())
                                {
                                    slotMapping.FFs[i][j][k]->getControlSetInfo()->display();
                                }
                                std::cout << "offset: " << CARRYChainSiteOffset << "\n";
                                std::cout << "currentCluster:\n";
                                determinedClusterInSite->printMyself();
                                std::cout << "FF-FFSet:\n";
                                std::cout << DesignInfo::FFSRCompatible(
                                    slotMapping.FFs[i][j][k]->getOriCellType(),
                                    determinedClusterInSite->getFFControlSets()[0].getFFType());
                                std::cout << "FF-FF:\n";
                                std::cout << DesignInfo::FFSRCompatible(
                                    slotMapping.FFs[i][j][k]->getOriCellType(),
                                    determinedClusterInSite->getFFControlSets()[0].getFFs()[0]->getCellType());
                                std::cout << "i,j,k: " << i << ", " << j << ", " << k << "\n";
                                std::cout.flush();
                            }
                            assert(succ);
                        }
                    }
                }
            }
            determinedClusterInSite->clusterHash();
            determinedClusterInSite->updateScoreInSite();
        }

        inline void addLUTRAMMacro()
        {
            assert(determinedClusterInSite == nullptr);
            assert(LUTRAMMacro);
            determinedClusterInSite = new PackingCLBCluster(this);
            determinedClusterInSite->clusterHash();
            determinedClusterInSite->updateScoreInSite();
        }

        inline bool checkIsPrePackedSite()
        {
            return isCarrySite || isLUTRAMSite;
        }

        inline bool checkIsCarrySite()
        {
            return isCarrySite;
        }

        inline bool checkIsMuxSite()
        {
            if (determinedClusterInSite)
            {
                return determinedClusterInSite->getNumMuxes() > 0;
            }
            return false;
        }

        inline bool checkIsLUTRAMSite()
        {
            return isLUTRAMSite;
        }

        inline PlacementInfo::PlacementMacro *getCarryMacro()
        {
            return CARRYChain;
        }

        inline PlacementInfo::PlacementMacro *getLUTRAMMacro()
        {
            return LUTRAMMacro;
        }

        inline int getCarrySiteOffset()
        {
            return CARRYChainSiteOffset;
        }

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

            const std::string MuxF8SlotNames[2] = {"F8MUX_BOT", "F8MUX_TOP"};
            const std::string MuxF7SlotNames[2][2] = {{"F7MUX_AB", "F7MUX_CD"}, {"F7MUX_EF", "F7MUX_GH"}};
        };

        void mapCarryRelatedCellsToSlots(PlacementInfo::PlacementMacro *_CARRYChain, float siteOffset);
        void mapLUTRAMRelatedCellsToSlots(PlacementInfo::PlacementMacro *_LUTRAMMacro);
        void finalMapToSlotsForCarrySite();

        /**
         * @brief map cells in MUXF8 macro to CLB slot
         *
         * @param muxF8Offset the offset of the MUXF8 in the CLB site. There is only two slots for MUXF8
         * @param MUXF8Macro the pointer of the MUXF8 needed to be mapped into current site
         */
        void mapMuxF8Macro(int muxF8Offset, PlacementInfo::PlacementMacro *MUXF8Macro);
        /**
         * @brief map cells in MUXF7 macro to CLB slot
         *
         * @param muxF7Offset the offset of the MUXF7 in the CLB site. There is only two slots for MUXF8
         * @param MUXF7Macro the pointer of the MUXF7 needed to be mapped into current site
         */
        void mapMuxF7Macro(int halfCLBOffset, PlacementInfo::PlacementMacro *MUXF7Macro);

        int findMuxFromHalfCLB(PlacementInfo::PlacementMacro *MUXF8Macro);
        void greedyMapMuxForCommonLUTFFSite();
        void greedyMapForCommonLUTFFSite();
        void finalMapToSlotsForCommonLUTFFSite();
        void finalMapToSlots()
        {
            if (determinedClusterInSite)
            {
                if (checkIsCarrySite())
                {
                    finalMapToSlotsForCarrySite();
                }
                else if (checkIsLUTRAMSite())
                {
                }
                else if (checkIsMuxSite())
                {
                    greedyMapMuxForCommonLUTFFSite();
                }
                else if (!checkIsPrePackedSite() && !checkIsMuxSite())
                {
                    // LUTS-FFs Packing
                    finalMapToSlotsForCommonLUTFFSite();
                }
                else
                {
                    // assert(false && "undefined packing situation");
                }
            }
        }

        std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> &getFixedPairedLUTs()
        {
            return fixedPairedLUTs;
        }
        std::set<DesignInfo::DesignCell *> &getConflictLUTs()
        {
            return conflictLUTs;
        }
        bool conflictLUTsContain(DesignInfo::DesignCell *tmpCell)
        {
            return conflictLUTs.find(tmpCell) != conflictLUTs.end();
        }

        const SiteBELMapping &getSlotMapping() const
        {
            return slotMapping;
        }

      private:
        PlacementInfo *placementInfo;
        DeviceInfo::DeviceSite *CLBSite;
        int unchangedIterationThr = 3;
        unsigned int numNeighbor = 10;
        float deltaD = 1.0;
        float curD = 0;
        float maxD = 10;
        unsigned int PQSize = 10;
        float y2xRatio = 1.0;
        float HPWLWeight = 0.01;

        int recordTop = -1;
        int unchangeIterationCnt = 0;
        std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> neighborPUs;
        // std::map<PlacementInfo::PlacementUnit *, float> PU2HPWLChange;
        std::vector<PackingCLBCluster *> seedClusters;
        std::vector<PackingCLBCluster *> priorityQueue;
        std::map<PlacementInfo::PlacementUnit *, int> PU2TopCnt;
        const std::vector<PackingCLBSite *> &PUId2PackingCLBSite;
        PackingCLBCluster *determinedClusterInSite = nullptr;
        float detScore = 0;

        bool isCarrySite = false;
        bool isLUTRAMSite = false;
        PlacementInfo::PlacementMacro *CARRYChain = nullptr;
        PlacementInfo::PlacementMacro *LUTRAMMacro = nullptr;
        int CARRYChainSiteOffset = -1;

        bool debug = false;

        std::set<DesignInfo::DesignCell *> mappedCells;
        std::set<DesignInfo::DesignCell *> mappedLUTs;
        std::set<DesignInfo::DesignCell *> mappedFFs;
        std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> fixedPairedLUTs;
        std::set<DesignInfo::DesignCell *> fixedLUTsInPairs;
        std::set<DesignInfo::DesignCell *> conflictLUTs;
        SiteBELMapping slotMapping;
    };

    void prePackLegalizedMacros(PlacementInfo::PlacementMacro *tmpMacro);
    void packCLBsIteration(bool initial, bool debug = false);
    void packCLBs(int packIterNum, bool doExceptionHandling, bool debug = false);

    typedef struct _siteWithScore
    {
        PackingCLBSite *site;
        float score;

        _siteWithScore(PackingCLBSite *site, float score) : site(site), score(score)
        {
        }
    } siteWithScore;

    class PULocation : public std::array<float, 2>
    {
      public:
        // dimension of the Point
        static const int DIM = 2;
        PULocation()
        {
            assert(false);
        }
        PULocation(PlacementInfo::PlacementUnit *tmpPU) : PU(tmpPU)
        {
            (*this)[0] = tmpPU->X();
            (*this)[1] = tmpPU->Y();
        }
        PULocation &operator=(const PULocation &anotherPULocation)
        {
            (*this)[0] = anotherPULocation[0];
            (*this)[1] = anotherPULocation[1];
            PU = anotherPULocation.getPU();
            return (*this);
        }

        inline PlacementInfo::PlacementUnit *getPU() const
        {
            return PU;
        }

      private:
        PlacementInfo::PlacementUnit *PU;
    };

    void exceptionHandling(bool verbose = false);

    std::vector<DeviceInfo::DeviceSite *> *findNeiborSitesFromBinGrid(DesignInfo::DesignCellType curCellType,
                                                                      float targetX, float targetY,
                                                                      float displacementLowerbound,
                                                                      float displacementUpperbound, float y2xRatio);

    bool exceptionPULegalize(PlacementInfo::PlacementUnit *curPU, float displacementThreshold, bool verbose);
    bool ripUpAndLegalizae(
        PackingCLBSite *curTargetPackingSite, PlacementInfo::PlacementUnit *curPU, float displacementThreshold,
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> &packingSite2DeterminedCluster, bool verbose);
    void checkPackedPUsAndUnpackedPUs();

    void setPULocationToPackedSite();
    void updatePackedMacro(bool setPUPseudoNetToCLBSite = false, bool setCLBFixed = false);
    void setPUsToBePacked();
    void dumpFinalPacking();
    void dumpDSPBRAMPlacementTcl(std::ofstream &outfileTcl);
    void dumpCLBPlacementTcl(std::ofstream &outfileTcl, bool packingRelatedToLUT6_2);
    void dumpPlacementTcl(std::string dumpTclFile);

  private:
    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;
    PlacementInfo *placementInfo;
    std::map<std::string, std::string> &JSONCfg;
    int unchangedIterationThr;
    int numNeighbor;
    float deltaD;
    float curD;
    float maxD;
    int PQSize;
    float HPWLWeight;
    std::string packerName;
    bool incrementalPacking;

    int DumpCLBPackingCnt = 0;

    std::vector<PackingCLBSite *> PUId2PackingCLBSite;
    std::vector<PackingCLBSite *> packingSites;
    std::vector<PackingCLBSite *> PUId2PackingCLBSiteCandidate;
    std::vector<PlacementInfo::PlacementUnit *> &placementUnits;
    std::vector<PlacementInfo::PlacementUnpackedCell *> &placementUnpackedCells;
    std::vector<PlacementInfo::PlacementMacro *> &placementMacros;
    std::set<DesignInfo::DesignCell *> &cellInMacros;
    std::map<int, PlacementInfo::PlacementUnit *> &cellId2PlacementUnit;

    std::map<DeviceInfo::DeviceSite *, PackingCLBSite *> deviceSite2PackingSite;
    std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> packedPUs;
    std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> unpackedPUs;
    std::vector<PlacementInfo::PlacementUnit *> unpackedPUsVec;
    std::map<PackingCLBSite *, PlacementInfo::PlacementUnit *> involvedPackingSite2PU;
    std::vector<PULocation> PUPoints;

    float y2xRatio = 1.0;
};
std::ostream &operator<<(std::ostream &os, const ParallelCLBPacker::PackingCLBSite::PackingCLBCluster *tmpCluster);
// inline bool operator<(const ParallelCLBPacker::PackingCLBSite::PackingCLBCluster &A,
//                       const ParallelCLBPacker::PackingCLBSite::PackingCLBCluster &B)
// {
//     return A.getScoreInSite() < B.getScoreInSite();
// }

#endif