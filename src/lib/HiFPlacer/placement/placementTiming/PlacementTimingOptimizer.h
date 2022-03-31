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

    void propogateArrivalTime();
    std::vector<int> findCriticalPath();
    std::vector<std::vector<int>> findCriticalPaths(float criticalRatio);
    float conductStaticTimingAnalysis(bool enforeOptimisticTiming = false);
    void incrementalStaticTimingAnalysis_forPUWithLocation(PlacementInfo::PlacementUnit *curPU, float targetX,
                                                           float targetY);
    void setPinsLocation();
    void clusterLongPathInOneClockRegion(int pathLenThr, float clusterThrRatio);
    void moveDriverIntoBetterClockRegion(int pathLenThr, float clusterThrRatio);
    void dumpClockRegionClusters();
    void stretchClockRegionColumns();

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

    inline void setEffectFactor(float _effectFactor)
    {
        effectFactor = _effectFactor;
    }

    inline float getDelayByModel(PlacementTimingInfo::TimingGraph<DesignInfo::DesignCell>::TimingNode *node1,
                                 PlacementTimingInfo::TimingGraph<DesignInfo::DesignCell>::TimingNode *node2, float X1,
                                 float Y1, float X2, float Y2)
    {
        float delay = getDelayByModel_conservative(X1, Y1, X2, Y2);
        // if (node1->getClusterId() >= 0 && node2->getClusterId() >= 0 &&
        //     node1->getClusterId() != node2->getClusterId() && delay > 0.05)
        //     delay -= 0.05;
        return delay;
    }

    inline float getDelayByModel(float X1, float Y1, float X2, float Y2)
    {
        return getDelayByModel_conservative(X1, Y1, X2, Y2);
    }

    const float timingC[10] = {150.38575401, -620.94694989, -274.2735654, 494.72583191, 234.67951055};

    inline float getDelayByModel_agressive(float X1, float Y1, float X2, float Y2)
    {
        if (conservativeTiming)
            return getDelayByModel_conservative(X1, Y1, X2, Y2);

        int clockRegionX0, clockRegionY0;
        deviceInfo->getClockRegionByLocation(X1, Y1, clockRegionX0, clockRegionY0);
        int clockRegionX1, clockRegionY1;
        deviceInfo->getClockRegionByLocation(X2, Y2, clockRegionX1, clockRegionY1);

        float X = std::fabs(X1 - X2) * 2;
        float Y = std::fabs(Y1 - Y2);

        float delay = (timingC[0] + std::pow(X, 0.3) * timingC[1] + std::pow(Y, 0.3) * timingC[2] +
                       std::pow(X, 0.5) * timingC[3] + std::pow(Y, 0.5) * timingC[4]) /
                          1000.0 +
                      std::abs(clockRegionX1 - clockRegionX0) * 0.5;

        if (delay < 0.05)
            delay = 0.05;

        return delay;
    }

    const float timingC0[10] = {95.05263521, -26.50563359, 77.42394117, 106.29195883, -14.975527};
    const float timingC1[10] = {123.05017047, -169.25614191, -117.28028144, 208.53573639, 174.2573465};
    const float timingC2[10] = {234.7694101, -433.99467294, -64.96319998, 373.78606257, 139.45226658};

    inline float getDelayByModel_conservative(float X1, float Y1, float X2, float Y2)
    {
        int clockRegionX0, clockRegionY0;
        deviceInfo->getClockRegionByLocation(X1, Y1, clockRegionX0, clockRegionY0);
        int clockRegionX1, clockRegionY1;
        deviceInfo->getClockRegionByLocation(X2, Y2, clockRegionX1, clockRegionY1);

        float X = std::fabs(X1 - X2);
        float Y = std::fabs(Y1 - Y2);

        if (X * X + Y * Y < 9)
        {
            X *= 2;
            float delay = (timingC0[0] + std::pow(X, 0.3) * timingC0[1] + std::pow(Y, 0.3) * timingC0[2] +
                           std::pow(X, 0.5) * timingC0[3] + std::pow(Y, 0.5) * timingC0[4]) /
                          1000.0;
            if (std::abs(clockRegionX1 - clockRegionX0) > 1 || clockRegionX1 == 2 || clockRegionX0 == 2)
                delay += std::abs(clockRegionX1 - clockRegionX0) * 0.5;

            if (delay < 0.05)
                delay = 0.05;
            return delay;
        }
        else if (X * X + Y * Y < 36)
        {
            X *= 2;
            float delay = (timingC1[0] + std::pow(X, 0.3) * timingC1[1] + std::pow(Y, 0.3) * timingC1[2] +
                           std::pow(X, 0.5) * timingC1[3] + std::pow(Y, 0.5) * timingC1[4]) /
                          1000.0;
            if (std::abs(clockRegionX1 - clockRegionX0) > 1 || clockRegionX1 == 2 || clockRegionX0 == 2)
                delay += std::abs(clockRegionX1 - clockRegionX0) * 0.5;

            if (delay < 0.05)
                delay = 0.05;
            return delay;
        }
        else
        {
            X *= 2;
            float delay = (timingC2[0] + std::pow(X, 0.3) * timingC2[1] + std::pow(Y, 0.3) * timingC2[2] +
                           std::pow(X, 0.5) * timingC2[3] + std::pow(Y, 0.5) * timingC2[4]) /
                          1000.0;
            if (std::abs(clockRegionX1 - clockRegionX0) > 1 || clockRegionX1 == 2 || clockRegionX0 == 2)
                delay += std::abs(clockRegionX1 - clockRegionX0) * 0.5;

            if (delay < 0.05)
                delay = 0.05;
            return delay;
        }
    }

    inline bool isConservativeTiming()
    {
        return conservativeTiming;
    }

    inline void pauseCounter()
    {
        enableCounter = false;
    }

    float getSlackThr();
    std::map<PlacementInfo::PlacementNet *, int> &getNetActualSlackPinNum()
    {
        return netActualSlackPinNum;
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
    int STA_Cnt = 0;
    float effectFactor = 0;
    bool clockRegionClusterTooLarge = false;
    std::vector<float> pois;
    std::vector<std::vector<int>> clockRegionclusters;
    std::map<PlacementInfo::PlacementNet *, int> netActualSlackPinNum;
    bool conservativeTiming = false;
    bool enableCounter = true;
};

#endif