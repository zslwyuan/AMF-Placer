#ifndef _PlacementTimingInfo
#define _PlacementTimingInfo

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "dumpZip.h"
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
#include <algorithm>

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
    PlacementTimingInfo(DesignInfo *designInfo, DeviceInfo *deviceInfo, std::map<std::string, std::string> &JSONCfg);
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
        TimingGraph()
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
             * @brief Get the inward edges to this TimingNode
             *
             * @return std::vector<TimingEdge *>&
             */
            inline std::vector<TimingEdge *> &getInEdges()
            {
                return inEdges;
            }

          private:
            /**
             * @brief the pointer linked to the design element (pin or cell)
             *
             */
            nodeType *designNode;
            int id;
            float latestArrival = 0.0;      // ns
            float arrivalConstaint = 100.0; // ns

            /**
             * @brief the node can have internal delay (e.g., cell delay)
             *
             */
            float innerDelay = 0.0;
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
         * @brief Set the Longest Path Length for each TimingNode in the TimingGraph and get a sorted vector of
         * TimingNodes
         *
         */
        void setLongestPathLength()
        {
            for (auto tmpNode : nodes)
            {
                tmpNode->calcLongestPath();
            }

            pathLenSortedNodes = nodes;
            std::sort(pathLenSortedNodes.begin(), pathLenSortedNodes.end(), [](TimingNode *a, TimingNode *b) -> bool {
                return a->getLongestPathLength() > b->getLongestPathLength();
            });
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
         * @brief find the sucessors of a node in the long paths
         *
         * @param startNodeId start node Id
         * @param sizeThr the number limitation to avoid huge cluster
         * @return std::vector<int>
         */
        std::vector<int> BFSFromNode(int startNodeId, unsigned sizeThr, std::set<int> &exceptionCells);

        inline std::vector<TimingNode *> &getPathLenSortedNodes()
        {
            return pathLenSortedNodes;
        }

      private:
        std::vector<TimingNode *> nodes;
        std::vector<TimingNode *> pathLenSortedNodes;
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

  private:
    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;

    // settings
    std::map<std::string, std::string> &JSONCfg;

    TimingGraph<DesignInfo::DesignCell> *simpleTimingGraph = nullptr;
    bool verbose = false;
};

#endif