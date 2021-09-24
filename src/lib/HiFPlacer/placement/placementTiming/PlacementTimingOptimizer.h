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
    void setEdgesDelay();
    void setPinsLocation();

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
    std::vector<float> pois;
};

#endif