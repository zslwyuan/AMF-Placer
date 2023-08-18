/**
 * @file PlacementTimingInfo.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief  This header file contains the classes of data which record the timing information related to placement.
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

#ifndef _PlacementTimingInfo
#define _PlacementTimingInfo

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "dumpZip.h"
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief PlacementTimingInfo is the container which record the timing information related to placement.
 *
 */
class PlacementTimingInfo
{
  public:
    /**
     * @brief Construct a new Placement Timing Info object based on the information of design and device
     *
     * @param designInfo pointer of design information
     * @param deviceInfo pointer of device information
     * @param JSONCfg  the user-defined placement configuration
     */
    PlacementTimingInfo(DesignInfo *designInfo, DeviceInfo *deviceInfo, std::map<std::string, std::string> JSONCfg);
    ~PlacementTimingInfo()
    {
        if (simpleTimingGraph)
            delete simpleTimingGraph;
    }

    /**
     * @brief a directed graph for timing analysis
     *
     * @tparam nodeType indicate the type of a node in the graph, usually it is a DesignCell* or DesignPin*
     */
    template <typename nodeType> class TimingGraph
    {
      public:
        /**
         * @brief Construct a new empty Timing Graph object
         *
         */
        TimingGraph(PlacementTimingInfo *timingInfo) : timingInfo(timingInfo)
        {
            nodes.clear();
            edges.clear();
        }
        ~TimingGraph()
        {
            for (auto edge : edges)
                delete edge;
            for (auto node : nodes)
                delete node;
        }

        class TimingEdge;

        /**
         * @brief TimingNode is the node in TimingGraph, which could be pin or cell in the design netlist
         *
         */
        class TimingNode
        {
          public:
            /**
             * @brief Construct a new Timing Node object
             *
             * @param designNode node element, could be cell or pin
             * @param id unique id for the node
             */
            TimingNode(nodeType *designNode, int id) : designNode(designNode), id(id)
            {
                inEdges.clear();
                outEdges.clear();
            };
            ~TimingNode(){};

            inline int getId()
            {
                return id;
            }

            inline nodeType *getDesignNode()
            {
                return designNode;
            }

            /**
             * @brief indicate that this node is a register node
             *
             */
            inline void setIsRegister()
            {
                isRegister = true;
            }

            /**
             * @brief check if the node is a register node
             *
             * @return true
             * @return false
             */
            inline bool checkIsRegister()
            {
                return isRegister;
            }

            inline void addInEdge(TimingEdge *tmpEdge)
            {
                inEdges.push_back(tmpEdge);
            }

            inline void addOutEdge(TimingEdge *tmpEdge)
            {
                outEdges.push_back(tmpEdge);
            }

            /**
             * @brief Set the data path forward level of the node for later propagation
             *
             * @param _forwardLevel
             */
            inline void setForwardLevel(int _forwardLevel)
            {
                forwardLevel = _forwardLevel;
                destLevel = _forwardLevel;
            }

            inline void setBackwardLevel(int _backwardLevel)
            {
                backwardLevel = _backwardLevel;
            }

            /**
             * @brief calculate the length of the longest path containing this TimingNode
             *
             */
            inline void calcLongestPath()
            {
                longestPathLength = forwardLevel + backwardLevel + 1;
            }

            /**
             * @brief Get the distance toward the farthest predecessor register based on the path length (instead of
             * delay)
             *
             * @return int
             */
            inline int getForwardLevel()
            {
                return forwardLevel;
            }

            /**
             * @brief Get the distance toward the farthest successor register based on the path length (instead of
             * delay)
             *
             * @return int
             */
            inline int getBackwardLevel()
            {
                return backwardLevel;
            }

            /**
             * @brief Get the length of the longest path containing this TimingNode
             *
             * @return int
             */
            inline int getLongestPathLength()
            {

                if (forwardLevel >= 0 && backwardLevel >= 0)
                    return longestPathLength;
                else
                    return -1;
            }

            inline void setDestLevel(int _destLevel)
            {
                destLevel = _destLevel;
            }

            inline int getDestLevel()
            {
                return destLevel;
            }

            /**
             * @brief Get the outward edges from this TimingNode
             *
             * @return std::vector<TimingEdge *>&
             */
            inline std::vector<TimingEdge *> &getOutEdges()
            {
                return outEdges;
            }

            /**
             * @brief sort the outward edges by their sink node backward level
             *
             */
            void sortOutEdgesByBackwardLevel()
            {
                std::sort(outEdges.begin(), outEdges.end(), [](TimingEdge *a, TimingEdge *b) -> bool {
                    return a->getSink()->getBackwardLevel() < b->getSink()->getBackwardLevel();
                });
            }

            /**
             * @brief Get the inward edges to this TimingNode
             *
             * @return std::vector<TimingEdge *>&
             */
            inline std::vector<TimingEdge *> &getInEdges()
            {
                return inEdges;
            }

            /**
             * @brief sort the inward edges by their source node forward level
             *
             */
            void sortInEdgesByForwardLevel()
            {
                std::sort(inEdges.begin(), inEdges.end(), [](TimingEdge *a, TimingEdge *b) -> bool {
                    return a->getSource()->getForwardLevel() < b->getSource()->getForwardLevel();
                });
            }

            /**
             * @brief Get the latest arrival time to the output of this timing node
             *
             * @return float
             */
            inline float getLatestInputArrival()
            {
                return latestInputArrival;
            }

            /**
             * @brief Set the latest arrival time to the output of this timing node
             *
             * @param _latestInputArrival
             */
            inline void setLatestInputArrival(float _latestInputArrival)
            {
                latestInputArrival = _latestInputArrival;
            }

            inline void setLatestOutputArrival(float _latestOutputArrival)
            {
                latestOutputArrival = _latestOutputArrival;
            }

            inline float getLatestOutputArrival()
            {
                return latestOutputArrival;
            }

            /**
             * @brief Get the slowest predecessor node Id
             *
             * @return int
             */
            inline int getSlowestPredecessorId()
            {
                return slowestPredecessorId;
            }

            /**
             * @brief Set the slowest predecessor node Id
             *
             * @param _slowestPredecessorId
             */
            inline void setSlowestPredecessorId(int _slowestPredecessorId)
            {
                slowestPredecessorId = _slowestPredecessorId;
            }

            /**
             * @brief Set the inner delay
             *
             * @param _innerDelay
             */
            inline void setInnerDelay(float _innerDelay)
            {
                innerDelay = _innerDelay;
            }

            /**
             * @brief Get the inner delay
             *
             * @return float
             */
            inline float getInnerDelay()
            {
                return innerDelay;
            }

            /**
             * @brief Get the required arrival time
             *
             * @return float
             */
            inline float getRequiredArrivalTime()
            {
                return requiredArrival;
            }

            /**
             * @brief Set the required arrival time
             *
             * @param _requiredArrival
             */
            inline void setRequiredArrivalTime(float _requiredArrival)
            {
                requiredArrival = _requiredArrival;
            }

            /**
             * @brief Set the required arrival time
             *
             * @param _requiredArrival
             */
            inline void setInitialRequiredArrivalTime(float _requiredArrival)
            {
                if (clockPeriod > 0)
                    requiredArrival = clockPeriod;
                else
                    requiredArrival = _requiredArrival;
            }

            /**
             * @brief Get the earliest successor node Id
             *
             * @return int
             */
            inline int getEarlestSuccessorId()
            {
                return slowestPredecessorId;
            }

            /**
             * @brief Set the earliest successor node Id
             *
             * @param _earliestSuccessorId
             */
            inline void setEarlestSuccessorId(int _earliestSuccessorId)
            {
                earliestSuccessorId = _earliestSuccessorId;
            }

            inline void setClockPeriod(float _clockPeriod)
            {
                clockPeriod = _clockPeriod;
            }

            inline float getClockPeriod()
            {
                return clockPeriod;
            }

            inline void setClusterId(int _clusterId)
            {
                clusterId = _clusterId;
            }

            inline float getClusterId()
            {
                return clusterId;
            }

          private:
            /**
             * @brief the pointer linked to the design element (pin or cell)
             *
             */
            nodeType *designNode = nullptr;
            int id;
            float latestInputArrival = 0.0; // ns
            float latestOutputArrival = 0.0;
            float requiredArrival = 10.0; // ns
            float clockPeriod = -10;
            int slowestPredecessorId = -1;
            int earliestSuccessorId = -1;
            int clusterId = -1;

            /**
             * @brief the node can have internal delay (e.g., cell delay)
             *
             */
            float innerDelay = 0.1;
            bool isRegister = false;
            std::vector<TimingEdge *> inEdges;
            std::vector<TimingEdge *> outEdges;

            /**
             * @brief the distance toward the farthest predecessor register based on the path length (instead of
             * delay)
             *
             */
            int forwardLevel = -1;

            /**
             * @brief the distance toward the farthest successor register based on the path length (instead of
             * delay)
             *
             */
            int backwardLevel = -1;
            int destLevel = -1;

            /**
             * @brief the length of the longest path containing this TimingNode
             *
             */
            int longestPathLength = 100000000;
        };

        /**
         * @brief TimingEdge records a directed interconnection relationship between two TimingNode. It is a
         * point-to-point information instead of HyperEdge.
         *
         */
        class TimingEdge
        {
          public:
            /**
             * @brief Construct a new Timing Edge object
             *
             * @param srcNode inward TimingNode
             * @param sinkNode outward TimingNode
             * @param srcPin inward DesignPin, each edge should be binded to pins for later evaluation
             * @param sinkPin outward DesignPin, each edge should be binded to pins for later evaluation
             * @param net the related DesignNet
             * @param id the unique id for this TimingEdge
             */
            TimingEdge(TimingNode *srcNode, TimingNode *sinkNode, DesignInfo::DesignPin *srcPin,
                       DesignInfo::DesignPin *sinkPin, DesignInfo::DesignNet *net = nullptr, int id = -1)
                : srcNode(srcNode), sinkNode(sinkNode), srcPin(srcPin), sinkPin(sinkPin), net(net), id(id)
            {
                assert(srcPin);
                assert(sinkPin);
                assert(id >= 0);
            }
            ~TimingEdge(){};

            inline int getId()
            {
                return id;
            }

            inline void setDelay(float _delay)
            {
                delay = _delay;
            }

            inline float getDelay()
            {
                return delay;
            }

            inline TimingNode *getSink()
            {
                return sinkNode;
            }

            inline TimingNode *getSource()
            {
                return srcNode;
            }

            inline DesignInfo::DesignPin *getSinkPin()
            {
                return sinkPin;
            }

            inline DesignInfo::DesignPin *getSourcePin()
            {
                return srcPin;
            }

          private:
            TimingNode *srcNode = nullptr;
            TimingNode *sinkNode = nullptr;
            DesignInfo::DesignPin *srcPin = nullptr;
            DesignInfo::DesignPin *sinkPin = nullptr;
            DesignInfo::DesignNet *net = nullptr;
            int id;
            float delay = 0; // ns
        };

        /**
         * @brief insert a TimingNode into this TimingGraph
         *
         * @param timingNode a given TimingNode
         */
        inline void insertTimingNode(TimingNode *timingNode)
        {
            nodes.push_back(timingNode);
        }

        inline std::vector<TimingNode *> &getNodes()
        {
            return nodes;
        }

        inline std::vector<TimingEdge *> &getEdges()
        {
            return edges;
        }

        /**
         * @brief add a TimingEdge into TimingGraph based on some related information
         *
         * @param idA TimingNode A id
         * @param idB  TimingNode B id
         * @param srcPin  source DesignPin
         * @param sinkPin sink DesignPin
         * @param net related DesignNet
         * @param delay the delay of this interconnection
         */
        inline void addEdgeBetween(int idA, int idB, DesignInfo::DesignPin *srcPin, DesignInfo::DesignPin *sinkPin,
                                   DesignInfo::DesignNet *net = nullptr, float delay = 0.0)
        {
            auto newEdge = new TimingEdge(nodes[idA], nodes[idB], srcPin, sinkPin, net, edges.size());
            newEdge->setDelay(delay);
            edges.push_back(newEdge);

            nodes[idB]->addInEdge(newEdge);
            nodes[idA]->addOutEdge(newEdge);
        }

        /**
         * @brief find loop from a node in timing graph (for debug)
         *
         * @param nodeInPath
         * @param startNode
         * @param curNode
         * @param level
         */
        void findALoopFromNode(std::vector<int> &nodeInPath, int startNode, int curNode, int level)
        {
            if (level > 0 && curNode == startNode)
            {
                for (auto nodeId : nodeInPath)
                {
                    std::cout << nodes[nodeId]->getDesignNode()->getName() << "  =>" << nodes[nodeId]->checkIsRegister()
                              << "\n";
                    std::cout << nodes[nodeId]->getDesignNode() << "\n";
                }
                exit(0);
            }
            for (auto outEdge : nodes[curNode]->getOutEdges())
            {
                int nextId = outEdge->getSink()->getId();

                if (!nodes[nextId]->checkIsRegister())
                {

                    nodeInPath.push_back(nextId);
                    findALoopFromNode(nodeInPath, startNode, nextId, level + 1);
                    nodeInPath.pop_back();
                }
            }
        }

        /**
         * @brief propogate the forward level of each TimingNode
         *  forward level of a TimingNode is the distance toward the farthest predecessor register based on the path
         * length (instead of delay)
         */
        void forwardLevelization();

        /**
         * @brief propogate the backward level of each TimingNode
         *  backward level of a TimingNode is the distance toward the farthest successor register based on the path
         * length (instead of delay)
         */
        void backwardLevelization();

        /**
         * @brief propogate the timing delay along the TimingEdge
         *
         */
        void propogateArrivalTime();

        /**
         * @brief back propogate the required arrival time
         *
         */
        void backPropogateRequiredArrivalTime();

        void updateCriticalPath()
        {
            maxDelay = 0;
            maxDelayId = -1;
            for (unsigned int i = 0; i < getNodes().size(); i++)
            {
                if (getNodes()[i]->getLatestInputArrival() > maxDelay)
                {
                    maxDelay = getNodes()[i]->getLatestInputArrival();
                    maxDelayId = i;
                }
            }
        }

        inline int getCriticalEndPoint()
        {
            return maxDelayId;
        }

        inline float getCriticalPathDelay()
        {
            return maxDelay;
        }

        /**
         * @brief backtrace the longest delay path from the node
         *
         * @param curNodeId
         * @return std::vector<int>
         */
        std::vector<int> backTraceDelayLongestPathFromNode(int curNodeId);

        bool backTraceDelayLongestPathFromNode(int curNodeId, std::vector<int> &isCovered, std::vector<int> &resPath,
                                               int converThr = 30);

        /**
         * @brief Set the Longest Path Length for each TimingNode in the TimingGraph and get a sorted vector of
         * TimingNodes
         *
         */
        void setLongestPathLength()
        {
            for (auto tmpNode : nodes)
            {
                tmpNode->calcLongestPath();
                tmpNode->getDesignNode()->setTimingLength(tmpNode->getLongestPathLength());
            }

            pathLenSortedNodes = nodes;
            std::sort(pathLenSortedNodes.begin(), pathLenSortedNodes.end(), [](TimingNode *a, TimingNode *b) -> bool {
                return (a->getLongestPathLength() == b->getLongestPathLength())
                           ? (a->getForwardLevel() < b->getForwardLevel())
                           : (a->getLongestPathLength() > b->getLongestPathLength());
            });
        }

        void sortedEndpointByDelay()
        {
            delaySortedTimingEndpointNodes.clear();
            for (auto id : forwardlevel2NodeIds[0])
            {
                if (nodes[id]->getInEdges().size() == 0)
                    continue;
                delaySortedTimingEndpointNodes.push_back(nodes[id]);
            }
            std::sort(delaySortedTimingEndpointNodes.begin(), delaySortedTimingEndpointNodes.end(),
                      [](TimingNode *a, TimingNode *b) -> bool {
                          return (a->getLatestInputArrival() > b->getLatestInputArrival());
                      });
        }

        inline std::vector<TimingNode *> &getSortedTimingEndpoints()
        {
            return delaySortedTimingEndpointNodes;
        }

        /**
         * @brief find the longest path from a register to the target node (id)
         *
         * @param targetId
         * @return std::vector<int>
         */
        std::vector<int> traceBackFromNode(int targetId);

        /**
         * @brief find the longest path from the target node (id) to a register
         *
         * @param targetId
         * @return std::vector<int>
         */
        std::vector<int> traceForwardFromNode(int targetId);

        /**
         * @brief BFS  the sucessors(predecessors) of a node in the long paths
         *
         * @param startNodeId start node Id
         * @param sizeThr the number limitation to avoid huge cluster
         * @return std::vector<int>
         */
        std::vector<int> BFSFromNode(int startNodeId, int pathLenThr, unsigned sizeThr, std::set<int> &exceptionCells);

        /**
         * @brief DFS the sucessors(predecessors) of a node in the long paths
         *
         * @param startNodeId start node Id
         * @param sizeThr the number limitation to avoid huge cluster
         * @param fanoutThr limit the node fanout during DFS
         * @return std::vector<int>
         */
        std::vector<int> DFSFromNode(int startNodeId, int pathLenThr, unsigned sizeThr, std::set<int> &exceptionCells,
                                     int fanoutThr = 10000000);

        inline std::vector<TimingNode *> &getPathLenSortedNodes()
        {
            return pathLenSortedNodes;
        }

        /**
         * @brief Get the long path threshold level
         *
         * @return int
         */
        inline int getLongPathThresholdLevel()
        {
            return longPathThresholdLevel;
        }

        /**
         * @brief Get the medium path threshold level
         *
         * @return int
         */
        inline int getMediumPathThresholdLevel()
        {
            return mediumPathThresholdLevel;
        }

        inline void setLongPathThrRatio(float _r)
        {
            longPathThrRatio = _r;
        }

        /**
         * @brief Get the clock period
         *
         * TODO: enable multiple clock
         *
         * @return float
         */
        inline float getClockPeriod()
        {
            return clockPeriod;
        }

        /**
         * @brief Set the clock period
         *
         * TODO: enable multiple clock
         *
         * @param _clockPeriod
         */
        inline void setClockPeriod(float _clockPeriod)
        {
            clockPeriod = _clockPeriod;
        }

      private:
        PlacementTimingInfo *timingInfo = nullptr;
        std::vector<TimingNode *> nodes;
        std::vector<TimingNode *> pathLenSortedNodes;
        std::vector<TimingNode *> delaySortedTimingEndpointNodes;
        std::vector<TimingEdge *> edges;

        /**
         * @brief levelized nodes in difference forward level
         *
         */
        std::vector<std::vector<int>> forwardlevel2NodeIds;

        /**
         * @brief levelized nodes in difference backward level
         *
         */
        std::vector<std::vector<int>> backwardlevel2NodeIds;

        float longPathThrRatio = 0.95;
        float mediumPathThrRatio = 0.8;
        int longPathThresholdLevel = 1;
        int mediumPathThresholdLevel = 1;

        float clockPeriod = 10.0; // ns

        float maxDelay = 0;
        int maxDelayId = -1;
    };

    /**
     * @brief build a simple timing graph, where the inner delay between pin paris for an element will be identical
     *
     */
    void buildSimpleTimingGraph();

    /**
     * @brief Get the Simple Timing Info object which regard design cells as timing nodes
     *
     * @return std::vector<TimingGraph<DesignInfo::DesignCell>::TimingNode *>&
     */
    inline std::vector<TimingGraph<DesignInfo::DesignCell>::TimingNode *> &getSimplePlacementTimingInfo()
    {
        return simpleTimingGraph->getNodes();
    }

    inline std::vector<TimingGraph<DesignInfo::DesignCell>::TimingNode *> &getSimplePlacementTimingInfo_PathLenSorted()
    {
        return simpleTimingGraph->getPathLenSortedNodes();
    }

    /**
     * @brief Get the Simple Placement Timing Graph object
     *
     * @return TimingGraph<DesignInfo::DesignCell>*
     */
    inline TimingGraph<DesignInfo::DesignCell> *getSimplePlacementTimingGraph()
    {
        return simpleTimingGraph;
    }

    inline int getLongPathThresholdLevel()
    {
        return simpleTimingGraph->getLongPathThresholdLevel();
    }

    inline int getMediumPathThresholdLevel()
    {
        return simpleTimingGraph->getMediumPathThresholdLevel();
    }

    inline DeviceInfo *getDeviceInfo()
    {
        return deviceInfo;
    }

    void setDSPInnerDelay();

  private:
    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;

    // settings
    std::map<std::string, std::string> JSONCfg;

    TimingGraph<DesignInfo::DesignCell> *simpleTimingGraph = nullptr;
    bool verbose = false;

    float clockPeriod = 10.0; // ns
    bool DSPCritical = false;

    std::map<DesignInfo::DesignNet *, float> clockNet2Period;
    std::map<int, float> cellId2Period;
};

#endif