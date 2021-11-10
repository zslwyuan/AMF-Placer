/**
 * @file PlacementTimingOptimizer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _PlacementTimingOptimizer
#define _PlacementTimingOptimizer

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "PlacementInfo.h"
#include "PlacementTimingInfo.h"
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

class PlacementTimingOptimizer
{
  public:
    PlacementTimingOptimizer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg);
    ~PlacementTimingOptimizer()
    {
    }

    void enhanceNetWeight_LevelBased(int levelThr);
    void propogateArrivalTime();
    void conductStaticTimingAnalysis();
    void setPinsLocation();
    void clusterLongPathInOneClockRegion(int pathLenThr, float clusterThrRatio);
    void moveDriverIntoBetterClockRegion(int pathLenThr, float clusterThrRatio);
    void dumpClockRegionClusters();

    void initPois()
    {
        pois.push_back(std::exp(-lambdaPois));
        for (int i = 1; i < poisN; i++)
        {
            pois.push_back(pois[i - 1] * lambdaPois / i);
        }
        for (int i = 1; i < poisN; i++)
        {
            pois[i] = std::pow(pois[i] / pois[100], 0.25);
        }
    }

    inline float getPois(int v)
    {
        if (v > 200)
        {
            return 0;
        }
        else
        {
            return pois[v];
        }
    }

    inline float getEffectFactor()
    {
        return effectFactor;
    }

    const float timingC[10] = {150.38575401, -620.94694989, -274.2735654, 494.72583191, 234.67951055};

    inline float getDelayByModel(float X1, float Y1, float X2, float Y2)
    {
        int clockRegionX0, clockRegionY0;
        deviceInfo->getClockRegionByLocation(X1, Y1, clockRegionX0, clockRegionY0);
        int clockRegionX1, clockRegionY1;
        deviceInfo->getClockRegionByLocation(X2, Y2, clockRegionX1, clockRegionY1);

        float X = std::fabs(X1 - X2) * 2;
        float Y = std::fabs(Y1 - Y2);

        return (timingC[0] + std::pow(X, 0.3) * timingC[1] + std::pow(Y, 0.3) * timingC[2] +
                std::pow(X, 0.5) * timingC[3] + std::pow(Y, 0.5) * timingC[4]) /
                   1000.0 +
               std::abs(clockRegionX1 - clockRegionX0) * 0.5;
    }

  private:
    PlacementInfo *placementInfo = nullptr;
    PlacementTimingInfo *timingInfo = nullptr;
    DesignInfo *designInfo;
    DeviceInfo *deviceInfo;

    // settings
    std::map<std::string, std::string> &JSONCfg;

    bool verbose = false;
    float y2xRatio = 1;

    inline float getDis(float x1, float y1, float x2, float y2)
    {
        return std::abs(x1 - x2) + y2xRatio * std::abs(y1 - y2);
    }

    float xDelayUnit = 0.13; // ns/unit
    float yDelayUnit = 0.10; // ns/unit

    int poisN = 1000;
    float lambdaPois = 100;
    int enhanceNetWeight_LevelBased_Cnt = 0;
    float effectFactor = 0;
    std::vector<float> pois;
    std::vector<std::vector<int>> clockRegionclusters;
};

#endif