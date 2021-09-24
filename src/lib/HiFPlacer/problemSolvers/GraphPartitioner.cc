#include "GraphPartitioner.h"

template <class NodeList, class NetList>
void GraphPartitioner<NodeList, NetList>::solve(int eachClusterDSPNum, int eachClusterBRAMNum)
{
    clusters.clear();
    std::vector<int> inputCluster;
    inputCluster.clear();
    for (unsigned int i = 0; i < nodeList.size(); i++)
    {
        inputCluster.push_back(i);
    }
    recursiveMinCutPartition(inputCluster, this, eachClusterDSPNum, eachClusterBRAMNum);
    sortClustersBySize();
}

template <class NodeList, class NetList> void GraphPartitioner<NodeList, NetList>::sortClustersBySize()
{
    // std::vector<std::vector<int>>
    int numClusters = clusters.size();
    for (int i = 0; i < numClusters; i++)
    {

        for (int j = i + 1; j < numClusters; j++)
        {
            if (clusters[i].size() > clusters[j].size())
            {
                std::vector<int> tmpNodes = clusters[i];
                clusters[i] = clusters[j];
                clusters[j] = tmpNodes;
            }
        }
    }
}

template <class NodeList, class NetList>
void GraphPartitioner<NodeList, NetList>::recursiveMinCutPartition(
    std::vector<int> &inputCluster, GraphPartitioner<NodeList, NetList> *graphPartitioner, int eachClusterDSPNum,
    int eachClusterBRAMNum)
{

    std::array<std::vector<int>, 2> outputClusters;

    sem_wait(&graphPartitioner->partitionSem);
    int cut = graphPartitioner->minCutBipartition(inputCluster, outputClusters, graphPartitioner, eachClusterDSPNum,
                                                  eachClusterBRAMNum);
    sem_post(&graphPartitioner->partitionSem);

    if (cut == 0)
    {
        graphPartitioner->clustersLock.lock();
        graphPartitioner->clusters.push_back(inputCluster);
        graphPartitioner->clustersLock.unlock();
        return;
    }
    if (outputClusters[0].size() == 0 && outputClusters[1].size() == 0)
    {
        graphPartitioner->clustersLock.lock();
        graphPartitioner->clusters.push_back(inputCluster);
        graphPartitioner->clustersLock.unlock();
        return;
    }
    assert(outputClusters[0].size() > 0 && outputClusters[1].size() > 0);

    std::thread t1(recursiveMinCutPartition, std::ref(outputClusters[0]), std::ref(graphPartitioner),
                   std::ref(eachClusterDSPNum), std::ref(eachClusterBRAMNum));
    std::thread t2(recursiveMinCutPartition, std::ref(outputClusters[1]), std::ref(graphPartitioner),
                   std::ref(eachClusterDSPNum), std::ref(eachClusterBRAMNum));
    t1.join();
    t2.join();
}

template <class NodeList, class NetList>
unsigned GraphPartitioner<NodeList, NetList>::minCutBipartition(const std::vector<int> &inputCluster,
                                                                std::array<std::vector<int>, 2> &outputClusters,
                                                                GraphPartitioner<NodeList, NetList> *graphPartitioner,
                                                                int eachClusterDSPNum, int eachClusterBRAMNum)
{

    static int cnt = 0;

    outputClusters[0].clear();
    outputClusters[1].clear();
    unsigned int totalWeight = 0;

    int totalDSPNum = 0;
    int totalBRAMNum = 0;
    for (auto PUId : inputCluster)
    {
        totalWeight += graphPartitioner->nodeList[PUId]->getWeight();
        totalBRAMNum += graphPartitioner->nodeList[PUId]->getBRAMNum();
        totalDSPNum += graphPartitioner->nodeList[PUId]->getDSPNum();
    }

    // Constraints
    unsigned minCellClusterSize = graphPartitioner->minClusterCellNum;

    if (totalDSPNum <= eachClusterDSPNum && totalBRAMNum <= eachClusterBRAMNum)
    {
        if (double(minCellClusterSize) > 0.8 * totalWeight)
            return 0;
        else
        {
            if (graphPartitioner->verbose)
                print_info("partitioning iter#" + std::to_string(cnt) + " #curNode: " +
                           std::to_string(inputCluster.size()) + " totalWeight=" + std::to_string(totalWeight) +
                           " minCellClusterSize > 0.8 * totalWeight => SHOULD CONTINUE PARTITIONING");
        }
    }
    else
    {
        graphPartitioner->cntLock.lock();
        if (graphPartitioner->verbose)
            print_info("partitioning iter#" + std::to_string(cnt) + " #curNode: " +
                       std::to_string(inputCluster.size()) + " #totalDSPNum: " + std::to_string(totalDSPNum) +
                       " #totalBRAMNum: " + std::to_string(totalBRAMNum) + " SHOULD CONTINUE PARTITIONING");
        graphPartitioner->cntLock.unlock();
    }

    if (0.4 * double(minCellClusterSize) > totalWeight)
        return 0;

    // map placement unit to hyperNodes for PaToH
    int numHyperNodes = 0; // num of placement unit
    std::vector<int> placementUnit2hyperNode(graphPartitioner->nodeList.size(), -1);

    for (auto instId : inputCluster)
        placementUnit2hyperNode[instId] = numHyperNodes++;

    int numPlacementNets = 0;
    std::vector<bool> isCsdNet(graphPartitioner->netList.size(), false);
    int numPlacementPins = 0;
    for (auto net : graphPartitioner->netList)
    {
        bool exist = false;
        for (auto tmpPU : net->getUnits())
        {
            if (placementUnit2hyperNode[tmpPU->getId()] != -1)
            {
                ++numPlacementPins;
                exist = true;
            }
        }
        if (exist)
        {
            ++numPlacementNets;
            isCsdNet[net->getId()] = true;
        }
    }

    unsigned int shareMemorySize = 6 + numHyperNodes + numPlacementNets + 1 + numPlacementPins + numHyperNodes + 2;
    ExternalProcessFunc *externalProc =
        new ExternalProcessFunc(getExePath() + "/partitionHyperGraph", shareMemorySize * 4, graphPartitioner->verbose);
    int *shareMemory = (int *)externalProc->getSharedMemory();
    shareMemory[0] = numHyperNodes;
    shareMemory[1] = numPlacementNets;
    shareMemory[2] = numPlacementPins;
    double final_imbal = std::max(0.5 - double(minCellClusterSize) / totalWeight, 0.4);
    *(double *)(shareMemory + 3) = final_imbal;
    int *cut = shareMemory + 5;
    int *cwghts = shareMemory + 6;
    int *xpins = cwghts + numHyperNodes;
    int *pins = xpins + numPlacementNets + 1;
    int *partvec = pins + numPlacementPins;
    int *partweights = partvec + numHyperNodes;

    int *hyperNode2placementUnit = new int[numHyperNodes];
    for (auto instId : inputCluster)
        hyperNode2placementUnit[placementUnit2hyperNode[instId]] = instId;

    for (int i = 0; i < numHyperNodes; ++i)
    {
        assert((unsigned int)hyperNode2placementUnit[i] < graphPartitioner->nodeList.size());
        cwghts[i] = graphPartitioner->nodeList[hyperNode2placementUnit[i]]->getWeight();
        partvec[i] = -1;
    }

    // map placement nets to hypernet for PaToH

    int maxCut = maxCutRate * numPlacementNets;

    // hyper net to hyper nodes
    xpins[0] = 0;
    int indexNet = 0, indexPin = 0;
    for (auto net : graphPartitioner->netList)
    {
        if (isCsdNet[net->getId()])
        {
            for (auto tmpPU : net->getUnits())
                if (placementUnit2hyperNode[tmpPU->getId()] != -1)
                    pins[indexPin++] = placementUnit2hyperNode[tmpPU->getId()];
            xpins[++indexNet] = indexPin;
        }
    }
    assert(indexPin == numPlacementPins && indexNet == numPlacementNets);

    // call external process to execute
    externalProc->execute();

    // Postprocessing
    for (int i = 0; i < numHyperNodes; ++i)
        outputClusters[partvec[i]].push_back(inputCluster[i]);

    int totalWeight0 = 0;
    for (auto PUId : outputClusters[0])
        totalWeight0 += graphPartitioner->nodeList[PUId]->getWeight();
    int totalWeight1 = 0;
    for (auto PUId : outputClusters[1])
        totalWeight1 += graphPartitioner->nodeList[PUId]->getWeight();

    unsigned minClusterTotalWeight = std::min(totalWeight0, totalWeight1);
    double imbal = 0.5 - double(minClusterTotalWeight) / totalWeight;

    graphPartitioner->cntLock.lock();
    if ((*cut > maxCut || ((float)outputClusters[0].size() / (float)outputClusters[1].size() < 0.5) ||
         ((float)outputClusters[1].size() / (float)outputClusters[0].size() < 0.5)) &&
        (minCellClusterSize * 2 > totalWeight && totalDSPNum <= 3 * eachClusterDSPNum &&
         totalBRAMNum <= 3 * eachClusterBRAMNum))
    {
        // if (graphPartitioner->verbose)
        print_warning("partitioned iter#" + std::to_string(cnt) + " #node: " + std::to_string(numHyperNodes) +
                      " #net: " + std::to_string(numPlacementNets) + " max_cut: " + std::to_string(maxCut) +
                      " max_imbal: " + std::to_string(final_imbal) + " get too large cut: " + std::to_string(*cut) +
                      " failed to find cut meeting requirements of min-cut or size balance. cluster[0].size=" +
                      std::to_string(outputClusters[0].size()) +
                      " cluster[1].size=" + std::to_string(outputClusters[1].size()));
        outputClusters[0].clear();
        outputClusters[1].clear();
    }
    else
    {
        if (graphPartitioner->verbose)
            print_info("partitioned iter#" + std::to_string(cnt) + " #node: " + std::to_string(numHyperNodes) +
                       " #net: " + std::to_string(numPlacementNets) + " max_cut: " + std::to_string(maxCut) +
                       " max_imbal: " + std::to_string(final_imbal) + " succeed with " +
                       std::to_string(partweights[0]) + " and " + std::to_string(partweights[1]) +
                       " (cut: " + std::to_string(*cut) + ", imbal: " + std::to_string(imbal) + ")" +
                       "cluster[0].size=" + std::to_string(outputClusters[0].size()) +
                       " cluster[1].size=" + std::to_string(outputClusters[1].size()));
    }

    ++cnt;
    graphPartitioner->cntLock.unlock();

    int res = *cut;
    delete[] hyperNode2placementUnit;
    delete externalProc;

    return res;
}

// declare involved templates
template class GraphPartitioner<std::vector<PlacementInfo::PlacementUnit *>,
                                std::vector<PlacementInfo::PlacementNet *>>;
template class GraphPartitioner<std::vector<PlacementInfo::ClusterUnit *>, std::vector<PlacementInfo::ClusterNet *>>;