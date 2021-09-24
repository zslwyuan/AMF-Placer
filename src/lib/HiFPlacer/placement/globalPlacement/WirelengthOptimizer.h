#ifndef _WIRELENGTHOPTIMIZER
#define _WIRELENGTHOPTIMIZER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "Eigen/Eigen"
#include "Eigen/SparseCore"
#include "MacroLegalizer.h"
#include "PlacementInfo.h"
#include "QPSolverWrapper.h"
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
 * @brief WirelengthOptimizer builds numerical models based on the element locations and calls solvers to find an
 * optimal solution of the placement.
 *
 */
class WirelengthOptimizer
{
  public:
    /**
     * @brief Construct a new Wirelength Optimizer object
     *
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param JSONCfg the user-defined placement configuration
     * @param verbose whether the WirelengthOptimizer prints out more information or dumps log files
     */
    WirelengthOptimizer(PlacementInfo *placementInfo, std::map<std::string, std::string> &JSONCfg, bool verbose = true);
    ~WirelengthOptimizer()
    {
        if (xSolver)
            delete xSolver;
        if (ySolver)
            delete ySolver;
    }
    void GlobalPlacementQPSolve(float pesudoNetWeight, bool firstIteration = true,
                                bool forwardSolutionToNextIteration = false, bool enableMacroPseudoNet2Site = false,
                                bool considerNetNum = true, bool enableUserDefinedClusterOpt = false,
                                bool timingOpt = false);
    void updateB2BNetWeight(float pesudoNetWeight, bool enableMacroPseudoNet2Site = false, bool considerNetNum = true,
                            bool enableUserDefinedClusterOpt = false, bool timingOpt = false);
    static void updateB2BNetWeightWorker(PlacementInfo *placementInfo,
                                         std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                         std::vector<float> &objectiveMatrixDiag, Eigen::VectorXd &objectiveVector,
                                         float generalNetWeight, float y2xRatio, bool updateX, bool updateY);
    void reloadPlacementInfo();

    inline void setMacroLegalizationParameters(int cnt, float macroLegalizationWeight)
    {
        macroPseudoNetCnt = cnt;
        oriMacroLegalizationWeight = macroLegalizationWeight;
    }

    inline int getMacroPseudoNetEnhanceCnt()
    {
        return macroPseudoNetCnt;
    }

    inline float getMacroLegalizationWeight()
    {
        return oriMacroLegalizationWeight;
    }

  private:
    static void QPSolve(QPSolverWrapper *&curSolver);
    void solverLoadData();
    void solverLoadFixedData();
    void solverWriteBackData();
    void addPseudoNetForMacros(float pesudoNetWeight, bool considerNetNum);
    void addExtraWeightToNetsInLongPath(float pesudoNetWeight);
    void addPseudoNet2LoctionForAllPUs(float pesudoNetWeight, bool considerNetNum);
    void updatePseudoNetForUserDefinedClusters(float pesudoNetWeight);
    void adjustHighFanoutPULocation(float pesudoNetWeight);
    void addPseudoNet_LevelBased(int levelThr, float pesudoNetWeight, double disExpected);
    inline double manhattanDis(double x0, double y0, double x1, double y1)
    {
        return std::abs(x0 - x1) + y2xRatio * std::abs(y0 - y1);
    }

    PlacementInfo *placementInfo;
    QPSolverWrapper *xSolver = nullptr, *ySolver = nullptr;
    std::map<std::string, std::string> &JSONCfg;
    bool verbose;

    float generalNetWeight = 1.0;
    float y2xRatio = 1.0;
    bool useUnconstrainedCG = true;
    bool MKLorNot = false;
    bool directMacroLegalize = false;
    float oriMacroLegalizationWeight = 1.0;
    float userDefinedClusterFadeOutFactor = 1.0;

    int macroPseudoNetCnt = 0;
};

#endif