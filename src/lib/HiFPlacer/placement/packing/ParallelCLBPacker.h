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
#include "KDTree/KDTree.h"
#include "MaximalCardinalityMatching/MaximalCardinalityMatching.h"
#include "PlacementInfo.h"
#include "const.h"
#include "dumpZip.h"
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
 * demands, and the application characteristics to improve the packing efficiency and quality.
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

    /**
     * @brief PackedControlSet stores the data of a combination of FFs within one control set (clock
     * enable/preset-reset/clock) that can be packed in a site.
     *
     */
    class PackedControlSet
    {
      public:
        PackedControlSet()
        {
            FFs.clear();
        };

        /**
         * @brief Construct a new Packed Control Set object by cloning another one
         *
         * @param anotherControlSet
         */
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

        /**
         * @brief  undate a new Packed Control Set object by cloning another one
         *
         * @param anotherControlSet
         * @return PackedControlSet&
         */
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

        /**
         * @brief Get the the number of FFs in this control set
         *
         * @return unsigned int
         */
        inline unsigned int getSize() const
        {
            return FFs.size();
        }

        /**
         * @brief get the FFs in this PackedControlSet
         *
         * @return const std::vector<DesignInfo::DesignCell *>&
         */
        inline const std::vector<DesignInfo::DesignCell *> &getFFs() const
        {
            return FFs;
        }

        /**
         * @brief clear the control set information in this PackedControlSet (only when there is no FF in this set)
         *
         */
        inline void reset()
        {
            assert(FFs.size() == 0);
            CSId = -1;
            CLK = nullptr;
            SR = nullptr;
            CE = nullptr;
        }

        /**
         * @brief add a FF into this PackedControlSet and check the compatibility
         *
         * @param curFF a given FF cell
         */
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

        /**
         * @brief remove a specify i-th FF from this PackedControlSet
         *
         * @param i a specified index of the FF to be removed
         */
        inline void removeXthFF(int i)
        {
            FFs.erase(FFs.begin() + i);
        }

        /**
         * @brief find the index in the list for a given FF cell pointer
         *
         * @param curFF a given FF cell
         * @return int
         */
        inline int findFF(DesignInfo::DesignCell *curFF)
        {
            for (unsigned int i = 0; i < FFs.size(); i++)
            {
                if (FFs[i] == curFF)
                    return i;
            }
            return -1;
        }

        /**
         * @brief get the control set id of this PackedControlSet.
         *
         * The control set determines whether two FFs can be packed.
         *
         * @return int
         */
        inline int getCSId() const
        {
            return CSId;
        }

        /**
         * @brief set the control set id of this PackedControlSet.
         *
         * The control set determines whether two FFs can be packed.
         *
         * @param _CSId the id of the target control set
         */
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

        /**
         * @brief check whether this PackedControlSet can be compatible with a given control set ID
         *
         * @param inputCSId  the id of the target control set
         * @return true
         * @return false
         */
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

    /**
     * @brief PackingCLBSite is a container for the packing information (parameters, candidates and packing status) of a
     * specific DeviceInfo::DeviceSite
     *
     */
    class PackingCLBSite
    {
      public:
        /**
         * @brief Construct a new Packing CLB Site object
         *
         * @param placementInfo the PlacementInfo for this placer to handle
         * @param CLBSite
         * @param unchangedIterationThr specify how many iterations a PlacementUnit should stay at the top priority of a
         * site before we finally map it to the site
         * @param numNeighbor the threshold number of cells for site
         * @param deltaD the increase step of the neighbor search diameter
         * @param curD current neighbor search diameter
         * @param maxD the maximum constraint of the neighbor search diameter
         * @param PQSize the size of priority queue (the low-priority candidates will be removed)
         * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
         * in X-coordinate
         * @param HPWLWeight the factor of HPWL overhead in packing evaluation for a cell
         * @param PUId2PackingCLBSite the reference of a map (actually a vector) recording the mapping of PlacementUnits
         * to the PackingCLBSites
         */
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

        /**
         * @brief PackingCLBCluster is a container of cells/PlacementUnits which can be packed in the corresponding CLB
         * site
         *
         */
        class PackingCLBCluster
        {
          public:
            /**
             * @brief Construct a new Packing CLB Cluster object (it should not be called.)
             *
             */
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
                numMuxes = anotherPackingCLBCluster->getNumMuxes();
            }

            /**
             * @brief Get the Id of the PackingCLBCluster (just for debug information don't use it in algorithm)
             *
             * @return int
             */
            inline int getId() const
            {
                return id;
            }

            /**
             * @brief refresh the Id of the PackingCLBCluster so we can know it is changed.
             *
             */
            inline void refreshId()
            {
                id = random();
            }

            /**
             * @brief check how many input pins will be needed if the two LUTs are packed.
             *
             * @param LUTA
             * @param LUTB
             * @return unsigned int
             */
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

            /**
             * @brief conduct maximun cardinality matching algorithm to pair LUTs
             *
             * @param verbose
             */
            void maxCardinalityMatching(bool verbose = false);

            /**
             * @brief Get the number of pins within this site for a given net (more pins are located in ont site will
             * reduce the demand of routing resource)
             *
             * @param curNet a given net
             * @return int
             */
            int getInternalPinsNum(PlacementInfo::PlacementNet *curNet);

            /**
             * @brief check whether a control set can be placed in a given half CLB
             *
             * Since in current architecture, FFs are packed in half CLBs in the site. The control sets of half CLB
             * pairs should be compatible.
             *
             * @param CSPtr a given control set pointer
             * @param anotherHalfCLB the other one half CLB id in this half CLB pair
             * @return true if the control set can be placed in the given half CLB
             * @return false if the control set CANNOT be placed in the given half CLB
             */
            bool compatibleInOneHalfCLB(DesignInfo::ControlSetInfo *CSPtr, int anotherHalfCLB);

            /**
             * @brief try to add a given LUT into this cluster
             *
             * @param curLUT a given LUT
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addLUT(DesignInfo::DesignCell *curLUT);

            /**
             * @brief try to add a given FF into a specific half CLB in this cluster
             *
             * @param curFF a given FF
             * @param halfCLB the target half CLB id in this cluster
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addToFFSet(DesignInfo::DesignCell *curFF, int halfCLB);

            /**
             * @brief  try to add a given list of FFs  into a specific half CLB in this cluster
             *
             * @param curFFs a given list of FFs
             * @param halfCLB the target half CLB id in this cluster
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addToFFSet(std::vector<DesignInfo::DesignCell *> curFFs, int halfCLB);

            /**
             * @brief try to add a given FF into this cluster
             *
             * @param curFF a given FF
             * @param enforceHalfCLB (default -1/no limit) limit the candidate half CLB id for this FF
             * @param enforceMainFFSlot (default false) constaint that the FF can be only placed in the main half CLB
             * (the one connected to LUT6 output pins in the CLB site). There are two types of half CLBs, we call those
             * connected to LUT6 output pins "main half CLB slots" and those connected to LUT5 output pins "secondary
             * half CLB slots". Some FFs (especially the virtual ones) should be mapped to the "main" ones.
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addFF(DesignInfo::DesignCell *curFF, int enforceHalfCLB = -1, bool enforceMainFFSlot = false);

            /**
             * @brief try to add a given list of FFs into this cluster
             *
             * @param curFFs  a given list of FFs
             * @param enforceHalfCLB (default -1/no limit) limit the candidate half CLB id for this FF
             * @param enforceMainFFSlot (default false) constaint that the FF can be only placed in the main half CLB
             * (the one connected to LUT6 output pins in the CLB site). There are two types of half CLBs, we call those
             * connected to LUT6 output pins "main half CLB slots" and those connected to LUT5 output pins "secondary
             * half CLB slots". Some FFs (especially the virtual ones) should be mapped to the "main" ones.
             * @param isMuxMacro
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addFFGroup(std::vector<DesignInfo::DesignCell *> curFFs, int enforceHalfCLB, bool enforceMainFFSlot,
                            bool isMuxMacro);

            /**
             * @brief remove a specific LUT from this candidate cluster
             *
             * @param curLUT a given LUT
             */
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

            /**
             * @brief remove a specific FF from this candidate cluster
             *
             * @param curFF  a given FF
             */
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

            /**
             * @brief remove some PlacementInfo::PlacementUnit from the cluster for later determined cluster
             * construction of this site
             *
             * @param tmpPU a given PlacementInfo::PlacementUnit to be removed
             */
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

            /**
             * @brief check whether the type of the given PlacementUnit is compatible with the site type
             *
             * @param tmpPU a given PlacementUnit
             * @return true the PU is compatible with the siet
             * @return false the PU is NOT compatible with the siet
             */
            inline bool isPUTypeCompatibleWithSiteType(PlacementInfo::PlacementUnit *tmpPU)
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

            /**
             * @brief  try to add a given PlacementUnit into this cluster
             *
             * the addPU implemented is based on the following paper's Algorithm 1:
             * G. Chen et al., “RippleFPGA: Routability-Driven Simultaneous Packing and Placement for Modern FPGAs,”
             * IEEE Trans. Comput.-Aided Des. Integr. Circuits Syst., vol. 37, no. 10, pp. 2022–2035,
             * Oct. 2018, doi: 10.1109/TCAD.2017.2778058.
             *
             * @param tmpPU a given PlacementUnit
             * @param allowOverlap whether it is successful if there is
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
            bool addPU(PlacementInfo::PlacementUnit *tmpPU, bool allowOverlap = false);

            /**
             * @brief without modifying the original cluster container, try to add a given PlacementUnit into this
             * cluster
             *
             * @param tmpPU a given PlacementUnit
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
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

            /**
             * @brief find/print the reason why the PlacementUnit fails to be added into this cluster
             *
             * @param tmpPU a given PlacementUnit
             */
            void addPUFailReason(PlacementInfo::PlacementUnit *tmpPU);

            /**
             * @brief check whether the cluster contains a specific PlacementUnit
             *
             * @param tmpPU  a given PlacementUnit
             * @return true if this attempt is successful
             * @return false if this attempt FAILED.
             */
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

            /**
             * @brief Get the set of single LUTs in this cluster (some other LUTs have been paired for packing)
             *
             * @return const std::set<DesignInfo::DesignCell *>&
             */
            inline const std::set<DesignInfo::DesignCell *> &getSingleLUTs() const
            {
                return singleLUTs;
            }

            /**
             * @brief remove a single LUT from the set of single LUTs in this cluster
             *
             * @param tmpLUT a given single LUT
             */
            inline void removeSingleLUT(DesignInfo::DesignCell *tmpLUT)
            {
                assert(singleLUTs.find(tmpLUT) != singleLUTs.end());
                singleLUTs.erase(tmpLUT);
            }

            /**
             * @brief try to remove a single LUT from the single LUT set
             *
             * @param tmpLUT a given LUT
             * @return true if the given LUT is found in the cluster set and can be removed
             * @return false  if the given LUT is NOT found in the cluster set
             */
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

            /**
             * @brief  try to remove a LUT from the set of paired LUTs
             *
             * @param tmpLUT a given LUT
             * @return true if the given LUT is found in the set of paired LUTs and can be removed
             * @return false  if the given LUT is NOT found in the set of paired LUTs
             */
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

            /**
             * @brief   try to remove a pair of LUTs from the set of paired LUTs
             *
             * @param tmpLUTA a LUT in the pair
             * @param tmpLUTB another LUT in the pair
             * @return true if the given LUTs are found in the set of paired LUTs and can be removed
             * @return false  if the given LUTs are NOT found in the set of paired LUTs
             */
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

            /**
             * @brief Get the set of the paired LUTs
             *
             * @return const std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>>&
             */
            inline const std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> &getPairedLUTs() const
            {
                return pairedLUTs;
            }

            /**
             * @brief check whether all the PlacementUnit in this cluster are valid (not bound to other sites) for this
             * site
             *
             * @param PUId2PackingCLBSite the mapping from PUs to sites
             * @param parentPackingCLBSite the site for this cluster
             * @return true if all the PUs can be mapped to this site
             * @return false if some of the PUs CANNOT be mapped to this site
             */
            inline bool areAllPUsValidForThisSite(const std::vector<PackingCLBSite *> &PUId2PackingCLBSite,
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

            /**
             * @brief Get the score if this cluster is mapped the site
             *
             * @return float
             */
            inline float getScoreInSite() const
            {
                return scoreInSite;
            }

            /**
             * @brief update the score of this cluster by considering HPWL, interconnection density, timing and etc
             *
             */
            void updateScoreInSite();

            /**
             * @brief incrementally update the score of this cluster by considering that only a given PlacementUnit will
             * be added into this cluster
             *
             * @param tmpPU a given PlacementUnit
             */
            void incrementalUpdateScoreInSite(PlacementInfo::PlacementUnit *tmpPU);

            /**
             * @brief Get the Parent Packing CLB site of this cluster (this can be used to get more device information)
             *
             * @return PackingCLBSite*
             */
            inline PackingCLBSite *getParentPackingCLB() const
            {
                return parentPackingCLB;
            }

            /**
             * @brief we use a hash function to encode the cluster to easily check duplicated clusters in the candidates
             *
             */
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

            /**
             * @brief incrementally update the hash function with an additional PlacementUnit. This hash will be used to
             * encode the cluster to easily check duplicated clusters in the candidates
             *
             * @param tmpPU
             * @return int
             */
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

            /**
             * @brief Get the hash code for this cluster
             *
             * @return int
             */
            inline int getHash()
            {
                if (!hashed)
                    clusterHash();
                return hashId;
            }

            /**
             * @brief Get the hash code of this cluster without changing the class variables of this cluster
             *
             * @return int
             */
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

            /**
             * @brief get the HPWL change if a given PlacementUnit is moved to this site
             *
             * @param tmpPU a given PlacementUnit
             * @return float
             */
            inline float getHPWLChangeForPU(PlacementInfo::PlacementUnit *tmpPU)
            {
                return parentPackingCLB->getHPWLChangeForPU(tmpPU);
            }

            /**
             * @brief Get the connectivity term in the cluster score object
             *
             * @return float
             */
            inline float getTotalConnectivityScore() const
            {
                return totalConnectivityScore;
            }

            /**
             * @brief Get the HWPL change term in the cluster score object
             *
             * @return float
             */
            inline float getHPWLChange() const
            {
                return HPWLChange;
            }

            /**
             * @brief Get the total number of cells in this cluster
             *
             * @return int
             */
            inline int getTotalCellNum() const
            {
                return totalCellNum;
            }

            /**
             * @brief Get the total number of MUX cells in this cluster
             *
             * @return int
             */
            inline int getNumMuxes() const
            {
                return numMuxes;
            }

            /**
             * @brief Get the maximum length of the paths which involve this cluster
             *
             * @return int
             */
            inline int getTotalLen() const
            {
                return totalLen;
            }

            /**
             * @brief Get the total weights of cells in the cluster (each cell will have different weight in the
             * placement)
             *
             * @return float
             */
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

            /**
             * @brief check whether the cluster contains a specific FF cell
             *
             * @param curFF a given FF cell
             * @return true if the cluster contains a specific FF cell
             * @return false if the cluster DOES NOT contain a specific FF cell
             */
            inline bool containFF(DesignInfo::DesignCell *curFF)
            {
                for (unsigned int i = 0; i < FFControlSets.size(); i++)
                {
                    if (FFControlSets[i].findFF(curFF) >= 0)
                        return true;
                }
                return false;
            }

            /**
             * @brief a API to print the information of cluster
             *
             */
            void printMyself();

            /**
             * @brief a verification function to check whether the cells in this cluster are REALLY LEGAL.
             *
             * @param tmpPU
             * @param isAddPU
             * @return true
             * @return false
             */
            bool checkCellCorrectness(PlacementInfo::PlacementUnit *tmpPU, bool isAddPU);

            /**
             * @brief check whether a specific number of Muxes can be compatible with a specific FFset for packing
             *
             * Since Mux will use some register input wires for the select signal, we have to check whether the packing
             * is possible for this situation
             *
             * @param i the id for the FF set
             * @param addNum the number of Muxes we want to add into the CLB
             * @return true if the given number of Muxes can be added.
             * @return false if the given number of Muxes CANNOT be added.
             */
            bool checkNumMuxCompatibleInFFSet(int i, int addNum);

            /**
             * @brief Get the max length of paths involving a given PlacementUnit
             *
             * (unused currently) during packing, we should consider timing factors and the critical path should be
             * assigned top priority.
             *
             * @param curPU a given PlacementUnit
             * @return int
             */
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

            /**
             * @brief the paraent CLB site for this cluster
             *
             */
            PackingCLBSite *parentPackingCLB = nullptr;

            /**
             * @brief the evaluation score of packing for this cluster
             *
             */
            float scoreInSite = -100000000;

            /**
             * @brief a hash id to record the elements in this cluster
             *
             * Some clusters have the same elements (hashid) but they might have different index due to the combination
             * (packing) order. We can use this hash id to remove duplicate candidates.
             *
             */
            int hashId = -3654;
            bool hashed = false;

            /**
             * @brief the unique id for each cluster
             *
             */
            int id = -1;
            std::set<PlacementInfo::PlacementUnit *, Packing_PUcompare> PUs;

            /**
             * @brief the connectivity score for this cluster
             *
             * We want more nets become the internal nets inside the CLB sites so the number of nets between sites can
             * be reduced.
             *
             */
            float totalConnectivityScore = 0;

            /**
             * @brief the HPWL term for the wirelength optimization
             *
             * We want the packing will not significantly increase the HPWL
             *
             */
            float HPWLChange = 0;

            /**
             * @brief the cell number term in the cluster score
             *
             * We want the large PlacementUnits can have a relatively higher priority in packing since the high
             * displacement of these elements might lead to bad routing.
             *
             */
            int totalCellNum = 0;

            /**
             * @brief the term of timing (paths) in the packing score
             *
             * We will accumulate the max length of paths for each elements in the cluster
             *
             */
            int totalLen = 0;

            /**
             * @brief the configurable weight for the wirelength in the cluster score
             *
             */
            float HPWLWeight = 0.01;

            // int muxF7Limit = 2;
            // int muxF8Limit = 1;
            // std::map<PlacementInfo::PlacementNet *, float> net2ConnectivityScore;
            // std::set<PlacementInfo::PlacementNet *> nets;

            /**
             * @brief the control set information for this cluster
             *
             * please note that some of these LUT/FFs are belong to CARRY chain, which is not shown in PUs
             *
             */
            std::vector<PackedControlSet> FFControlSets;

            /**
             * @brief the set of LUTs have not been paired with other LUTs in the clutser
             *
             */
            std::set<DesignInfo::DesignCell *> singleLUTs;

            /**
             * @brief the paired LUTs in the cluster
             *
             */
            std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> pairedLUTs;
        };

        /**
         * @brief check whether all the PlacementUnit in the top cluster in the priority queue have been assigned to
         * this CLB site
         *
         * @return true if all the PlacementUnits in the top cluster in the priority queue have been assigned to
         * this CLB site
         * @return false if some of the PlacementUnits in the top cluster in the priority queue have NOT been assigned
         * to this CLB site yet (maybe unassigned yet or maybe assigned to some other CLB sites)
         */
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

        /**
         * @brief SiteBELMapping is a contain recording the mapping between cells and BELs.
         *
         * We hold the cell information in arrays of slots.
         *
         */
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

        /**
         * @brief find the correspdnding FF control set id for a given Mux macro (this mux macro should have been mapped
         * to a control set in this site)
         *
         * @param MUXF8Macro a given Mux macro
         * @return int
         */
        int findMuxFromHalfCLB(PlacementInfo::PlacementMacro *MUXF8Macro);

        /**
         * @brief find the slots in the site for Muxes
         *
         */
        void greedyMapMuxForCommonLUTFFInSite();

        /**
         * @brief greedily find the exact slots for the LUTs/FFs in the site
         *
         */
        void greedyMapForCommonLUTFFInSite();

        /**
         * @brief finally map LUTs/FFs to the exact slots in the sites
         *
         */
        void finalMapToSlotsForCommonLUTFFInSite();

        /**
         * @brief finally map the elements (CARRY/MUX/LUT/FF) packed in this site into the slots in the site
         *
         */
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
                    greedyMapMuxForCommonLUTFFInSite();
                }
                else if (!checkIsPrePackedSite() && !checkIsMuxSite())
                {
                    // LUTS-FFs Packing
                    finalMapToSlotsForCommonLUTFFInSite();
                }
                else
                {
                    // assert(false && "undefined packing situation");
                }
            }
        }

        /**
         * @brief Get the fixed pairs of LUTs which should NOT be broken
         *
         * @return std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>>&
         */
        std::set<std::pair<DesignInfo::DesignCell *, DesignInfo::DesignCell *>> &getFixedPairedLUTs()
        {
            return fixedPairedLUTs;
        }

        /**
         * @brief Get the LUTs which CANNOT be paired
         *
         * @return std::set<DesignInfo::DesignCell *>&
         */
        std::set<DesignInfo::DesignCell *> &getConflictLUTs()
        {
            return conflictLUTs;
        }

        /**
         * @brief check whether a given cell is unpackable
         *
         * some driver LUTs of CARRY/MUX cannot be paired
         *
         * @param tmpCell the given cell
         * @return true if the cell CANNOT be paired with other LUTs
         * @return false if the cell CAN be paired with other LUTs
         */
        bool conflictLUTsContain(DesignInfo::DesignCell *tmpCell)
        {
            return conflictLUTs.find(tmpCell) != conflictLUTs.end();
        }

        /**
         * @brief Get the slot(BEL) mapping of the cells
         *
         * @return const SiteBELMapping&
         */
        const SiteBELMapping &getSlotMapping() const
        {
            return slotMapping;
        }

      private:
        PlacementInfo *placementInfo;
        DeviceInfo::DeviceSite *CLBSite;

        /**
         * @brief specify how many iterations a PlacementUnit should stay at the top priority of a
         * site before we finally map it to the site
         *
         */
        int unchangedIterationThr = 3;

        /**
         * @brief the threshold number of cells for site
         *
         */
        unsigned int numNeighbor = 10;

        /**
         * @brief the increase step of the neighbor search diameter
         *
         */
        float deltaD = 1.0;

        /**
         * @brief current neighbor search diameter
         *
         */
        float curD = 0;

        /**
         * @brief the maximum constraint of the neighbor search diameter
         *
         */
        float maxD = 10;

        /**
         * @brief the size of priority queue (the low-priority candidates will be removed)
         *
         */
        unsigned int PQSize = 10;

        /**
         * @brief  a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
         * in X-coordinate
         *
         */
        float y2xRatio = 1.0;

        /**
         * @brief the factor of HPWL overhead in packing evaluation for a cell
         *
         */
        float HPWLWeight = 0.01;

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

    /**
     * @brief helper struct for candidate site sorting
     *
     */
    typedef struct _siteWithScore
    {
        PackingCLBSite *site;
        float score;

        _siteWithScore(PackingCLBSite *site, float score) : site(site), score(score)
        {
        }
    } siteWithScore;

    /**
     * @brief PULocation is a helper class to find the neighbor PlacementUnits with KD-Tree
     *
     */
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

    /**
     * @brief Load the information of some packed macros like LUTRAM/Crossing-Clock-Domain FFs/Carry Chains have been
     * legalized.
     *
     * @param tmpMacro
     */
    void prePackLegalizedMacros(PlacementInfo::PlacementMacro *tmpMacro);

    /**
     * @brief update the packing cluster candidates for each CLB site and determine some mapping from elements to sites
     * according to the "confidence".
     *
     * @param initial indicate whether it is the first round of the packing iteration
     * @param debug whether print out debugging information
     */
    void packCLBsIteration(bool initial, bool debug = false);

    /**
     * @brief packing the PlacementUnits (which are compatible to CLB sites) into CLB sites
     *
     * @param packIterNum the number of packing iteration
     * @param doExceptionHandling conduct exception handling if some PlacementUnits fail to be legalized during the
     * parallel procedure
     * @param debug whether print out debugging information
     */
    void packCLBs(int packIterNum, bool doExceptionHandling, bool debug = false);

    /**
     * @brief handle the PlacementUnits that cannot be packed during the parallel procedure
     *
     * @param verbose whether dumping information for debugging
     */
    void exceptionHandling(bool verbose = false);

    /**
     * @brief find the neighbors of specific cell type with given coordinate center
     *
     * @param curCellType the given cell type
     * @param targetX center X
     * @param targetY center Y
     * @param displacementLowerbound the lower bound threshold of neighbors' displacement from the center (the neighbors
     * with low displacement might be tried by previous procedure)
     * @param displacementUpperbound  the upper bound threshold of neighbors' displacement from the center
     * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
     * in X-coordinate
     * @return std::vector<DeviceInfo::DeviceSite *>*
     */
    std::vector<DeviceInfo::DeviceSite *> *findNeiborSitesFromBinGrid(DesignInfo::DesignCellType curCellType,
                                                                      float targetX, float targetY,
                                                                      float displacementLowerbound,
                                                                      float displacementUpperbound, float y2xRatio);

    /**
     * @brief try to find a legal location for the given PlacementUnit when most of PlacementUnits are packed into CLB
     * site
     *
     * @param curPU a PlacementUnit which has NOT been legalized/packed
     * @param displacementThreshold the displacement threshold to find the neighbor site candidate
     * @param verbose whether print out debugging information
     * @return true if the PlacementUnit is legalized/packed successfully
     * @return false  if the PlacementUnit CANNOT be legalized/packed successfully in this iteration
     */
    bool exceptionPULegalize(PlacementInfo::PlacementUnit *curPU, float displacementThreshold, bool verbose);

    /**
     * @brief try to rip up the packing for a given CLB site and pack the given PlacementUnit in the site. The evicted
     * PlacementUnits which are originally packed in this site and cannot be packed now will try to find other CLB sites
     * to pack
     *
     * @param curTargetPackingSite a given CLB site
     * @param curPU a given PlacementUnit
     * @param displacementThreshold the displacement threshold for the evicted PlacementUnits to find the neighbor site
     * candidates
     * @param packingSite2DeterminedCluster the mapping between PlacementUnit and CLB sites
     * @param verbose whether print out debugging information
     * @return true if such re-packing is sucessful for the involved CLB sites and PlacementUnits
     * @return false if such re-packing FAILS for the involved CLB sites and PlacementUnits
     */
    bool ripUpAndLegalizae(
        PackingCLBSite *curTargetPackingSite, PlacementInfo::PlacementUnit *curPU, float displacementThreshold,
        std::map<PackingCLBSite *, PackingCLBSite::PackingCLBCluster *> &packingSite2DeterminedCluster, bool verbose);

    /**
     * @brief check the packing status for all the PlacementUnits
     *
     */
    void checkPackedPUsAndUnpackedPUs();

    /**
     * @brief update the location of PlacementUnits according to the packing result
     *
     */
    void setPULocationToPackedSite();

    /**
     * @brief Update the macros in PlacementInfo by regarding those elements in one CLB site as a macro
     *
     * @param setPUPseudoNetToCLBSite  whether set the legalization pseudo nets for those packed PlacementUnits after
     * updateing
     * @param setCLBFixed whether fix the locations of the packed PlacementUnits after updateing
     */
    void updatePackedMacro(bool setPUPseudoNetToCLBSite = false, bool setCLBFixed = false);

    /**
     * @brief set the packed attribute for the packed PlacementUnits
     *
     */
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

    /**
     * @brief specify how many iterations a PlacementUnit should stay at the top priority of a
     * site before we finally map it to the site
     *
     */
    int unchangedIterationThr;

    /**
     * @brief the threshold number of cells for site
     *
     */
    int numNeighbor;

    /**
     * @brief the increase step of the neighbor search diameter
     *
     */
    float deltaD;

    /**
     * @brief current neighbor search diameter
     *
     */
    float curD;

    /**
     * @brief the maximum constraint of the neighbor search diameter
     *
     */
    float maxD;

    /**
     * @brief the size of priority queue (the low-priority candidates will be removed)
     *
     */
    int PQSize;

    /**
     * @brief the factor of HPWL overhead in packing evaluation for a cell
     *
     */
    float HPWLWeight;
    std::string packerName;

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