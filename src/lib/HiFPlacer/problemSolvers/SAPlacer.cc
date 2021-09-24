#include "SAPlacer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

double SAPlacer::evaluateClusterPlacement(const std::vector<std::vector<std::vector<int>>> &grid2clusters,
                                          const std::vector<std::pair<int, int>> &cluster2XY)
{

    float regionW = deviceW / gridW;
    float regionH = deviceH / gridH;

    double resWL = 0;

    for (unsigned int clusterA = 1; clusterA < clusterAdjMat.size(); clusterA++)
        for (unsigned int clusterB = 0; clusterB < clusterA; clusterB++)
        {
            if (clusterAdjMat[clusterA][clusterB] > 0.00001)
            {
                resWL += clusterAdjMat[clusterA][clusterB] *
                         (fabs(cluster2XY[clusterA].first - cluster2XY[clusterB].first) * regionW +
                          y2xRatio * fabs(cluster2XY[clusterA].second - cluster2XY[clusterB].second) * regionH);
            }
        }
    for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
    {
        for (unsigned int fixedUnitId = 0; fixedUnitId < cluster2FixedUnitMat[clusterA].size(); fixedUnitId++)
        {
            if (cluster2FixedUnitMat[clusterA][fixedUnitId] > 0.00001)
            {
                resWL += connectionToFixedFactor * cluster2FixedUnitMat[clusterA][fixedUnitId] *
                         (fabs((cluster2XY[clusterA].first * regionW + regionW / 2) - fixedX[fixedUnitId]) +
                          y2xRatio * fabs((cluster2XY[clusterA].second * regionH + regionH / 2) - fixedY[fixedUnitId]));
            }
        }
    }

    for (int gridY = 0; gridY < gridH; gridY++)
        for (int gridX = 0; gridX < gridW; gridX++)
        {
            if (grid2clusters[gridY][gridX].size() > 1)
            {
                // resWL *= 10;
                for (unsigned int clusterAId = 1; clusterAId < grid2clusters[gridY][gridX].size(); clusterAId++)
                    for (unsigned int clusterBId = 0; clusterBId < clusterAId; clusterBId++)
                    {
                        int clusterA = grid2clusters[gridY][gridX][clusterAId];
                        int clusterB = grid2clusters[gridY][gridX][clusterBId];
                        resWL +=
                            5000 *
                            std::pow(std::min(1.1, (clusterWeights[clusterA] + clusterWeights[clusterB]) / 20000.0),
                                     2) *
                            (regionW + y2xRatio * regionH);
                    }
            }
        }

    return resWL;
}

double SAPlacer::incrementalEvaluateClusterPlacement(const std::vector<std::vector<std::vector<int>>> &grid2clusters,
                                                     const std::vector<std::pair<int, int>> &cluster2XY)
{
    std::vector<bool> placed;
    placed.clear();
    float regionW = deviceW / gridW;
    float regionH = deviceH / gridH;

    double resWL = 0;

    for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
    {
        placed.push_back(cluster2XY[clusterA].first >= 0 && cluster2XY[clusterA].second >= 0);
    }

    for (unsigned int clusterA = 1; clusterA < clusterAdjMat.size(); clusterA++)
        for (unsigned int clusterB = 0; clusterB < clusterA; clusterB++)
        {
            if (placed[clusterA] && placed[clusterB])
            {
                if (clusterAdjMat[clusterA][clusterB] > 0.00001)
                {
                    resWL += clusterAdjMat[clusterA][clusterB] *
                             (fabs(cluster2XY[clusterA].first - cluster2XY[clusterB].first) * regionW +
                              y2xRatio * fabs(cluster2XY[clusterA].second - cluster2XY[clusterB].second) * regionH);
                }
            }
        }
    for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
    {
        for (unsigned int fixedUnitId = 0; fixedUnitId < cluster2FixedUnitMat[clusterA].size(); fixedUnitId++)
        {
            if (placed[clusterA])
            {
                if (cluster2FixedUnitMat[clusterA][fixedUnitId] > 0.00001)
                {
                    resWL +=
                        connectionToFixedFactor * cluster2FixedUnitMat[clusterA][fixedUnitId] *
                        (fabs((cluster2XY[clusterA].first * regionW + regionW / 2) - fixedX[fixedUnitId]) +
                         y2xRatio * fabs((cluster2XY[clusterA].second * regionH + regionH / 2) - fixedY[fixedUnitId]));
                }
            }
        }
    }

    for (int gridY = 0; gridY < gridH; gridY++)
        for (int gridX = 0; gridX < gridW; gridX++)
        {
            if (grid2clusters[gridY][gridX].size() > 1)
            {
                // resWL *= 10;
                for (unsigned int clusterAId = 1; clusterAId < grid2clusters[gridY][gridX].size(); clusterAId++)
                    for (unsigned int clusterBId = 0; clusterBId < clusterAId; clusterBId++)
                    {
                        int clusterA = grid2clusters[gridY][gridX][clusterAId];
                        int clusterB = grid2clusters[gridY][gridX][clusterBId];
                        assert(placed[clusterA] && placed[clusterB]);
                        resWL +=
                            5000 *
                            std::pow(std::min(1.1, (clusterWeights[clusterA] + clusterWeights[clusterB]) / 20000.0),
                                     2) *
                            (regionW + y2xRatio * regionH);
                    }
            }
        }

    return resWL;
}

void swapVectors(std::vector<int> &a, std::vector<int> &b)
{
    std::vector<int> c = a;
    a = b;
    b = c;
}

void shuffleVectors(std::vector<std::vector<int>> &a, boost::mt19937 &rng)
{
    int N = a.size();
    for (int i = N - 1; i > 0; --i)
    {                            // gist, note, i>0 not i>=0
        int r = rng() % (i + 1); // gist, note, i+1 not i. "rng() % (i+1)" means
                                 // generate rand numbers from 0 to i
        swapVectors(a[i], a[r]);
    }
}

void SAPlacer::randomSwap(const std::vector<std::vector<std::vector<int>>> &grid2clusters,
                          std::vector<std::vector<std::vector<int>>> &new_grid2clusters,
                          const std::vector<std::pair<int, int>> &cluster2XY,
                          std::vector<std::pair<int, int>> &new_cluster2XY, float temperature, boost::mt19937 &rng)
{
    int gridY0 = rng() % (gridH * gridW) / gridW;
    int gridX0 = rng() % (gridH * gridW) % gridW;

    int gridY1 = rng() % (gridH * gridW) / gridW;
    int gridX1 = rng() % (gridH * gridW) % gridW;

    while ((gridX1 == gridX0 && gridY1 == gridY0) ||
           (grid2clusters[gridY1][gridX1].size() + grid2clusters[gridY0][gridX0].size()) == 0)
    {
        gridY0 = rng() % (gridH * gridW) / gridW;
        gridX0 = rng() % (gridH * gridW) % gridW;

        gridY1 = rng() % (gridH * gridW) / gridW;
        gridX1 = rng() % (gridH * gridW) % gridW;
    }

    new_grid2clusters = grid2clusters;
    new_cluster2XY = cluster2XY;

    std::vector<int> clustersMixed;
    clustersMixed.clear();
    for (auto id : grid2clusters[gridY0][gridX0])
        clustersMixed.push_back(id);
    for (auto id : grid2clusters[gridY1][gridX1])
        clustersMixed.push_back(id);

    // std::random_shuffle(clustersMixed.begin(), clustersMixed.end(), myrandom);
    for (unsigned int i = 0; i < clustersMixed.size(); i++)
    {
        for (unsigned int j = i + 1; j < clustersMixed.size(); j++)
        {
            if (rng() % 2)
            {
                int tmp = clustersMixed[i];
                clustersMixed[i] = clustersMixed[j];
                clustersMixed[j] = tmp;
            }
        }
    }

    while (true)
    {
        new_grid2clusters[gridY0][gridX0].clear();
        new_grid2clusters[gridY1][gridX1].clear();
        for (auto id : clustersMixed)
        {
            if (rng() % 2)
            {
                new_grid2clusters[gridY1][gridX1].push_back(id);
                new_cluster2XY[id] = std::pair<int, int>(gridX1, gridY1);
            }
            else
            {
                new_grid2clusters[gridY0][gridX0].push_back(id);
                new_cluster2XY[id] = std::pair<int, int>(gridX0, gridY0);
            }
        }
        std::sort(new_grid2clusters[gridY0][gridX0].begin(), new_grid2clusters[gridY0][gridX0].end());
        std::sort(new_grid2clusters[gridY1][gridX1].begin(), new_grid2clusters[gridY1][gridX1].end());
        if (new_grid2clusters[gridY0][gridX0].size() != grid2clusters[gridY0][gridX0].size())
        {
            break;
        }
        else
        {
            bool changed = false;
            for (unsigned int i = 0; i < grid2clusters[gridY0][gridX0].size(); i++)
            {
                if (grid2clusters[gridY0][gridX0][i] != new_grid2clusters[gridY0][gridX0][i])
                {
                    changed = true;
                    break;
                }
            }
            if (changed)
                break;
        }
    }

    // if (randGen() % 5 == 0) // shuffle multiple cluster?
    // {
    //     if (randGen() % 2) // shuffle row
    //     {
    //         int rowId = randGen() % gridH;
    //         int colA = randGen() % gridW;
    //         int colB = randGen() % gridW;
    //         swapVectors(new_grid2clusters[rowId][colA], new_grid2clusters[rowId][colB]);
    //     }
    //     else
    //     {
    //         int columnId = randGen() % gridW;
    //         int rowA = randGen() % gridH;
    //         int rowB = randGen() % gridH;
    //         swapVectors(new_grid2clusters[rowA][columnId], new_grid2clusters[rowB][columnId]);
    //     }
    //     for (int gridY = 0; gridY < gridH; gridY++)
    //         for (int gridX = 0; gridX < gridW; gridX++)
    //             for (auto clusterId : new_grid2clusters[gridY][gridX])
    //             {
    //                 new_cluster2XY[clusterId] = std::pair<int, int>(gridX, gridY);
    //             }
    // }
    // else
    if (rng() % 20 == 0 && temperature > 0.5) // shuffle multiple cluster?
    {
        if (rng() % 2) // shuffle row
        {
            int rowId = rng() % gridH;
            shuffleVectors(new_grid2clusters[rowId], rng);
        }
        else
        {
            int columnId = rng() % gridW;
            std::vector<std::vector<int>> tmpVec(gridH);
            for (int tmpI = 0; tmpI < gridH; tmpI++)
                tmpVec[tmpI] = new_grid2clusters[tmpI][columnId];
            shuffleVectors(tmpVec, rng);
            for (int tmpI = 0; tmpI < gridH; tmpI++)
                new_grid2clusters[tmpI][columnId] = tmpVec[tmpI];
        }
        for (int gridY = 0; gridY < gridH; gridY++)
            for (int gridX = 0; gridX < gridW; gridX++)
                for (auto clusterId : new_grid2clusters[gridY][gridX])
                {
                    new_cluster2XY[clusterId] = std::pair<int, int>(gridX, gridY);
                }
    }

    return;
}

void SAPlacer::randomShuffleRowColumn(const std::vector<std::vector<std::vector<int>>> &grid2clusters,
                                      std::vector<std::vector<std::vector<int>>> &new_grid2clusters,
                                      const std::vector<std::pair<int, int>> &cluster2XY,
                                      std::vector<std::pair<int, int>> &new_cluster2XY, boost::mt19937 &rng)
{
    new_grid2clusters = grid2clusters;
    new_cluster2XY = cluster2XY;

    if (rng() % 2) // shuffle row
    {
        int rowId = rng() % gridH;
        shuffleVectors(new_grid2clusters[rowId], rng);
    }
    else
    {
        int columnId = rng() % gridW;
        std::vector<std::vector<int>> tmpVec(gridH);
        for (int tmpI = 0; tmpI < gridH; tmpI++)
            tmpVec[tmpI] = new_grid2clusters[tmpI][columnId];
        shuffleVectors(tmpVec, rng);
        for (int tmpI = 0; tmpI < gridH; tmpI++)
            new_grid2clusters[tmpI][columnId] = tmpVec[tmpI];
    }
    for (int gridY = 0; gridY < gridH; gridY++)
        for (int gridX = 0; gridX < gridW; gridX++)
            for (auto clusterId : new_grid2clusters[gridY][gridX])
            {
                new_cluster2XY[clusterId] = std::pair<int, int>(gridX, gridY);
            }

    return;
}

float SAPlacer::probabilituFunc(double oriE, double newE, float T)
{
    if (newE < oriE)
        return 1.0;
    return exp(-10 * (newE / SACalibrationOffset - oriE / SACalibrationOffset) / T);
}

void SAPlacer::worker(SAPlacer *saPlacer, std::vector<std::vector<std::vector<int>>> &init_grid2clusters,
                      std::vector<std::pair<int, int>> &init_cluster2XY,
                      std::vector<std::vector<std::vector<int>>> &opt_grid2clusters,
                      std::vector<std::pair<int, int>> &opt_cluster2XY, int &totalIterNum, int &workers_randomSeed,
                      double &resE)
{
    std::vector<std::pair<int, int>> cur_cluster2XY = init_cluster2XY;
    std::vector<std::vector<std::vector<int>>> cur_grid2clusters = init_grid2clusters;
    std::vector<std::pair<int, int>> new_cluster2XY;
    std::vector<std::vector<std::vector<int>>> new_grid2clusters;

    boost::mt19937 rng(workers_randomSeed);

    double oriE = saPlacer->evaluateClusterPlacement(init_grid2clusters, init_cluster2XY);
    resE = oriE;
    // double inputE = oriE;

    int SAIterNum = (totalIterNum - 1);
    for (int k = SAIterNum + 10000; k >= 0; k--)
    {

        float temperature = (float)(k + 1) / SAIterNum;
        new_cluster2XY.clear();
        new_grid2clusters.clear();

        if (k > SAIterNum)
            saPlacer->randomShuffleRowColumn(cur_grid2clusters, new_grid2clusters, cur_cluster2XY, new_cluster2XY, rng);
        else
            saPlacer->randomSwap(cur_grid2clusters, new_grid2clusters, cur_cluster2XY, new_cluster2XY, temperature,
                                 rng);
        double newE = saPlacer->evaluateClusterPlacement(new_grid2clusters, new_cluster2XY);

        float thr = (float)rng() / (float)rng.max();
        float P = saPlacer->probabilituFunc(oriE, newE, temperature);
        if (P >= thr || k > SAIterNum)
        {
            oriE = newE;
            cur_grid2clusters = new_grid2clusters;
            cur_cluster2XY = new_cluster2XY;
        }

        if (resE > newE)
        {
            resE = newE;
            opt_grid2clusters = new_grid2clusters;
            opt_cluster2XY = new_cluster2XY;
        }
    }

    // assert(inputE > resE);
}

void SAPlacer::greedyPlaceACluster(const std::vector<std::pair<int, int>> &init_cluster2XY,
                                   const std::vector<std::vector<std::vector<int>>> &init_grid2clusters,
                                   std::vector<std::pair<int, int>> &res_cluster2XY,
                                   std::vector<std::vector<std::vector<int>>> &res_grid2clusters, int clusterIdToPlace)
{
    double res = 10e+10;
    for (int gridY = 0; gridY < gridH; gridY++)
        for (int gridX = 0; gridX < gridW; gridX++)
        {
            std::vector<std::pair<int, int>> new_cluster2XY;
            std::vector<std::vector<std::vector<int>>> new_grid2clusters;
            new_cluster2XY = init_cluster2XY;
            new_grid2clusters = init_grid2clusters;
            new_cluster2XY[clusterIdToPlace].first = gridX;
            new_cluster2XY[clusterIdToPlace].second = gridY;
            new_grid2clusters[gridY][gridX].push_back(clusterIdToPlace);
            double tmpRes = incrementalEvaluateClusterPlacement(new_grid2clusters, new_cluster2XY);
            if (tmpRes < res)
            {
                res = tmpRes;
                res_cluster2XY = new_cluster2XY;
                res_grid2clusters = new_grid2clusters;
            }
        }
}

int SAPlacer::greedyFindNextClusterToPlace(std::vector<std::pair<int, int>> &tmp_cluster2XY,
                                           std::vector<std::vector<std::vector<int>>> &tmp_grid2clusters)
{
    std::vector<bool> placed;
    std::vector<int> unplacedClusterIds;
    unplacedClusterIds.clear();
    placed.clear();

    for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
    {
        placed.push_back(tmp_cluster2XY[clusterA].first >= 0 && tmp_cluster2XY[clusterA].second >= 0);
        if (!(tmp_cluster2XY[clusterA].first >= 0 && tmp_cluster2XY[clusterA].second >= 0))
        {
            unplacedClusterIds.push_back(clusterA);
        }
    }

    int highConnectCluster = -1;
    float connectRatio = 0;
    for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
    {
        if (placed[clusterA])
        {
            for (unsigned int clusterB = 0; clusterB < clusterAdjMat.size(); clusterB++)
            {
                if (!placed[clusterB])
                {
                    if (clusterAdjMat[clusterA][clusterB] > 0.00001)
                    {
                        if (clusterAdjMat[clusterA][clusterB] > connectRatio)
                        {
                            connectRatio = clusterAdjMat[clusterA][clusterB];
                            highConnectCluster = clusterB;
                        }
                    }
                }
            }
        }
    }

    if (highConnectCluster < 0)
    {
        assert(unplacedClusterIds.size());
        return unplacedClusterIds[random() % unplacedClusterIds.size()];
    }
    else
    {
        return highConnectCluster;
    }
}

void SAPlacer::greedyInitialize(std::vector<std::pair<int, int>> &init_cluster2XY,
                                std::vector<std::vector<std::vector<int>>> &init_grid2clusters, int initOffset)
{
    int tmpCluster = initOffset % clusterAdjMat.size();
    for (unsigned int clusterId = 0; clusterId < clusterAdjMat.size(); clusterId++)
    {
        init_cluster2XY.push_back(std::pair<int, int>(-1, -1));
    }

    std::vector<std::pair<int, int>> new_cluster2XY;
    std::vector<std::vector<std::vector<int>>> new_grid2clusters;
    new_cluster2XY = init_cluster2XY;
    new_grid2clusters = init_grid2clusters;

    greedyPlaceACluster(init_cluster2XY, init_grid2clusters, new_cluster2XY, new_grid2clusters, tmpCluster);

    init_cluster2XY = new_cluster2XY;
    init_grid2clusters = new_grid2clusters;
    int iterNum = clusterAdjMat.size() - 1;
    while (iterNum--)
    {
        int nextClusterId = greedyFindNextClusterToPlace(init_cluster2XY, init_grid2clusters);
        std::vector<std::pair<int, int>> new_cluster2XY;
        std::vector<std::vector<std::vector<int>>> new_grid2clusters;
        new_cluster2XY = init_cluster2XY;
        new_grid2clusters = init_grid2clusters;

        greedyPlaceACluster(init_cluster2XY, init_grid2clusters, new_cluster2XY, new_grid2clusters, nextClusterId);

        init_cluster2XY = new_cluster2XY;
        init_grid2clusters = new_grid2clusters;
    }
}

void SAPlacer::solve()
{
    resE = 1e13;
    std::string dumpSAFile = "";
    if (verbose)
        dumpSAFile = placerName + "DumpFile";

    std::ofstream outfile0;
    // bool dump = false;
    // if (dumpSAFile != "")
    // {
    //     dump = true;
    //     outfile0 = std::ofstream(dumpSAFile.c_str());
    // }

    std::vector<std::pair<int, int>> init_cluster2XY;
    std::vector<std::vector<std::vector<int>>> init_grid2clusters;

    std::vector<std::thread> threads;
    std::vector<std::vector<std::pair<int, int>>> workers_cluster2XY;
    std::vector<std::vector<std::vector<std::vector<int>>>> workers_grid2clusters;
    std::vector<double> works_E;
    std::vector<int> workers_randomSeed;

    // assign each restarted task to a thread
    int workerIterNum = Kmax / restartNum;
    std::vector<std::pair<int, double>> seedAndE;
    seedAndE.clear();

    int initOffset = 0;
    for (int restartI = restartNum; restartI > 0; restartI -= nJobs)
    {

        // generate initial cluster placement
        init_cluster2XY.clear();
        init_grid2clusters =
            std::vector<std::vector<std::vector<int>>>(gridH, std::vector<std::vector<int>>(gridW, std::vector<int>()));

        initOffset++;
        greedyInitialize(init_cluster2XY, init_grid2clusters, initOffset);
        // for (unsigned int clusterId = 0; clusterId < clusterAdjMat.size(); clusterId++)
        // {
        //     int gridY = (clusterId + initOffset + (random() % 2)) % (gridH * gridW) / gridW;
        //     int gridX = (clusterId + initOffset + (random() % 2)) % (gridH * gridW) % gridW;
        //     init_grid2clusters[gridY][gridX].push_back(clusterId);
        //     init_cluster2XY.push_back(std::pair<int, int>(gridX, gridY));
        // }
        for (unsigned int clusterA = 0; clusterA < clusterAdjMat.size(); clusterA++)
        {
            assert(init_cluster2XY[clusterA].first >= 0 && init_cluster2XY[clusterA].second >= 0);
        }
        for (int gridY = 0; gridY < gridH; gridY++)
            for (int gridX = 0; gridX < gridW; gridX++)
            {
                std::sort(init_grid2clusters[gridY][gridX].begin(), init_grid2clusters[gridY][gridX].end());
            }

        SACalibrationOffset = evaluateClusterPlacement(init_grid2clusters, init_cluster2XY);

        printProgress(1 - ((double)restartI / restartNum));
        threads.clear();
        workers_cluster2XY.clear();
        workers_grid2clusters.clear();
        works_E.clear();
        workers_randomSeed.clear();

        for (int threadId = restartI; threadId >= 0 && threadId > restartI - nJobs; threadId--)
        {
            workers_cluster2XY.push_back(init_cluster2XY);
            workers_grid2clusters.push_back(init_grid2clusters);
            works_E.push_back(0);
            workers_randomSeed.push_back(threadId);
        }
        for (unsigned int threadId = 0; threadId < workers_cluster2XY.size(); threadId++)
        {
            threads.push_back(std::thread(worker, this, std::ref(init_grid2clusters), std::ref(init_cluster2XY),
                                          std::ref(workers_grid2clusters[threadId]),
                                          std::ref(workers_cluster2XY[threadId]), std::ref(workerIterNum),
                                          std::ref(workers_randomSeed[threadId]), std::ref(works_E[threadId])));
        }
        for (unsigned int threadId = 0; threadId < threads.size(); threadId++)
        {
            threads[threadId].join();
            seedAndE.emplace_back(workers_randomSeed[threadId], works_E[threadId]);
        }

        if (works_E[0] < resE)
        {
            resE = works_E[0];
            res_grid2clusters = workers_grid2clusters[0];
            res_cluster2XY = workers_cluster2XY[0];
        }

        for (unsigned int threadId = 1; threadId < threads.size(); threadId++)
        {
            if (works_E[threadId] < resE)
            {
                resE = works_E[threadId];
                res_grid2clusters = workers_grid2clusters[threadId];
                res_cluster2XY = workers_cluster2XY[threadId];
            }
        }
    }
    printf("\n");
    print_info("SA optimization ratio (finalE/initE) = " + std::to_string(resE / SACalibrationOffset));
    print_info("SA optimization initE = " + std::to_string(SACalibrationOffset));
    for (int restartI = restartNum - 1; restartI >= 0; restartI--)
    {
        std::cout << " " << seedAndE[restartI].first << "(" << seedAndE[restartI].second << ")";
        if (restartI % 10 == 0)
            std::cout << "\n";
    }
    std::cout << "\n";
    int occupiedRegionNum = 0;
    for (int gridY = 0; gridY < gridH; gridY++)
        for (int gridX = 0; gridX < gridW; gridX++)
        {
            if (res_grid2clusters[gridY][gridX].size() > 0)
            {
                occupiedRegionNum++;
            }
        }
    print_info("SA final occupied region num = " + std::to_string(occupiedRegionNum));

    if (dumpSAFile != "")
        outfile0.close();
    print_info("SAPlace handle " + std::to_string(init_cluster2XY.size()) +
               " cluster(s) and the final placement cost is " + std::to_string(resE));
}
