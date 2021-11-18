/**
 * @file WirelengthOptimizer.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of GlobalPlacer class and its internal modules and APIs which
 * build numerical models based on the element locations and calls solvers to find an optimal solution of the placement.
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _WIRELENGTHOPTIMIZER
#define _WIRELENGTHOPTIMIZER

#include "DesignInfo.h"
#include "DeviceInfo.h"
#include "Eigen/Eigen"
#include "Eigen/SparseCore"
#include "MacroLegalizer.h"
#include "PlacementInfo.h"
#include "QPSolverWrapper.h"
#include "PlacementTimingOptimizer.h"
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

    /**
     * @brief use quadratic model to estimate the HPWL and some timing/user-defined pseudo nets are involved for
     * specific optimization
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     * @param firstIteration indicates whether this is the first iteration of current round of WL optimization to
     * control the data loading
     * @param forwardSolutionToNextIteration if this round of WL optimization includes multiple iterations, do we
     * forward the location data obtained in last iteration for later processing?
     * @param enableMacroPseudoNet2Site enable the legalization pseudo net to force macros move to the legal site
     * @param considerNetNum whether add the interconnection-density-aware factor to pseudo net weights
     * @param enableUserDefinedClusterOpt whether check user-defined cluster information to add pseudo nets
     * @param timingOpt whether turn on the timing-oriented optimizations
     */
    void GlobalPlacementQPSolve(float pesudoNetWeight, bool firstIteration = true,
                                bool forwardSolutionToNextIteration = false, bool enableMacroPseudoNet2Site = false,
                                bool considerNetNum = true, bool enableUserDefinedClusterOpt = false,
                                PlacementTimingOptimizer *timingOptimizer = nullptr);

    /**
     * @brief update the net weights in the quadratic model according to B2B net HPWL model.
     *
     * In this procedure, we will not only consider the conventional nets/pseudonets in previous QP placer, but also
     * include the legalization pseudo nets and timing-oriented pseudo nets to comprehensively optimize the placement in
     * this part. Meanwhile, we allow users to define some clusters and let the WL optimizer take them into
     * consideration during optimization.
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     * @param enableMacroPseudoNet2Site enable the legalization pseudo net to force macros move to the legal site
     * @param considerNetNum whether add the interconnection-density-aware factor to pseudo net weights
     * @param enableUserDefinedClusterOpt whether check user-defined cluster information to add pseudo nets
     * @param timingOpt whether turn on the timing-oriented optimizations
     */
    void updateB2BNetWeight(float pesudoNetWeight, bool enableMacroPseudoNet2Site = false, bool considerNetNum = true,
                            bool enableUserDefinedClusterOpt = false,
                            PlacementTimingOptimizer *timingOptimizer = nullptr);

    /**
     * @brief a worker funtion for multi-threading net weight updating
     *
     *  min_x 0.5 * x'Px + q'x
     *  s.t.  l <= Ax <= u
     *
     * @param placementInfo the PlacementInfo for this placer to handle
     * @param objectiveMatrixTripletList a list of Eigen::Triplet used to describe the sparse matrix P of QP model
     * @param objectiveMatrixDiag a vector used to store the diagonal values in the sparse matrix P of
     * QP model
     * @param objectiveVector  a vector (i.e., Eigen::VectorXd) used to store the linear factors q' of QP model
     * @param generalNetWeight a common factor indicate the overall strength of the nets in the QP model from external
     * setting
     * @param y2xRatio a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning in
     * X-coordinate
     * @param updateX update the X-coordinate term in the quadratic problem
     * @param updateY update the X-coordinate term in the quadratic problem
     */
    static void updateB2BNetWeightWorker(PlacementInfo *placementInfo,
                                         std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList,
                                         std::vector<float> &objectiveMatrixDiag, Eigen::VectorXd &objectiveVector,
                                         float generalNetWeight, float y2xRatio, bool updateX, bool updateY);

    /**
     * @brief re-initialize some parameters and optimizer configuration according to the PlacementInfo
     *
     * PlacementInfo might change due to the loading of checkpoint or re-initialization of bin grid. Therefore, we need
     * to let the WL optimizer to know the changes.
     *
     */
    void reloadPlacementInfo();

    /**
     * @brief Set the macro legalization parameters
     *
     * The convergence of the placement is determined by some parameters and when we load the checkpoint from files, we
     * also need to load the corresponding parameters for the optimizers.
     *
     * @param cnt the number of the conducted macro iterations
     * @param macroLegalizationWeight the net weight for macro legalization
     */
    inline void setMacroLegalizationParameters(int cnt, float macroLegalizationWeight)
    {
        macroPseudoNetCnt = cnt;
        oriMacroLegalizationWeight = macroLegalizationWeight;
    }

    /**
     * @brief Get the number of the conducted macro iterations
     *
     * @return int
     */
    inline int getMacroPseudoNetEnhanceCnt()
    {
        return macroPseudoNetCnt;
    }

    /**
     * @brief Get the net weight for macro legalization
     *
     * @return float
     */
    inline float getMacroLegalizationWeight()
    {
        return oriMacroLegalizationWeight;
    }

    inline void clearNetPinEnhanceRate()
    {
        netPinEnhanceRate.clear();
    }

  private:
    /**
     * @brief call the cooresponding solver to solve the QP problem defined in the given QPSolverWrapper
     *
     * @param curSolver a given QPSolverWrapper
     */
    static void QPSolve(QPSolverWrapper *&curSolver);

    /**
     * @brief load to placement location from PlacementInfo to the solver
     *
     */
    void solverLoadData();

    /**
     * @brief load to placement location of fixed PlacementUnits from PlacementInfo to the solver
     *
     */
    void solverLoadFixedData();

    /**
     * @brief write placement location from  the solver to the PlacementInfo
     *
     */
    void solverWriteBackData();

    /**
     * @brief  add the legalization pseudo nets to force macros move to the legal sites
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     * @param considerNetNum whether add the interconnection-density-aware factor to pseudo net weights
     */
    void addPseudoNetForMacros(float pesudoNetWeight, bool considerNetNum);

    /**
     * @brief as convential QP placers do, we add pseudo nets between anchors and PlacementUnit to constrain the
     * movement of PlacementUnit for WL optimization.
     *
     * Please note that we can add the interconnect-aware factor into the pseudo nets so the macros can move slower
     * compared to the fine-grained elements.
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     * @param considerNetNum whether add the interconnection-density-aware factor to pseudo net weights
     */
    void addPseudoNet2LoctionForAllPUs(float pesudoNetWeight, bool considerNetNum);

    /**
     * @brief add pseudo net for user-defined clusters
     *
     * We allow users to specify some clusters in the design and WL optimizer will try to push them togather during
     * solving the QP problem
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     */
    void updatePseudoNetForUserDefinedClusters(float pesudoNetWeight);

    /**
     * @brief add pseudo net for timing optimization based on the length of timing path and the span of the nets
     *
     * @param levelThr the threshold of the level of the timing path to trigger to timing optimzation
     * @param timingWeight the common weight factor for pseudo nets for timing
     * @param disExpected the expected total length of timing paths
     */
    void addPseudoNet_LevelBased(int levelThr, float timingWeight, double disExpected);

    /**
     * @brief add pseudo net for timing optimization based on the timing slack of each elements in the design netlist
     *
     * @param timingWeight the common weight factor for pseudo nets for timing
     * @param slackPowFactor a factor for the sensitivity of negative timing slack
     * @param timingOptimizer the handler of timing-related analysis
     */
    void addPseudoNet_SlackBased(float timingWeight, double slackPowFactor, PlacementTimingOptimizer *timingOptimizer);

    /**
     * @brief add pseudo nets for clock region
     *
     * we find that when the nets route across the boundaries of clock regions will lead to high delay, therefore, we
     * will try to force the long path located in the same clock regions for the timing optimization.
     *
     * @param pesudoNetWeight the common weight factor for pseudo nets
     */
    void updatePseudoNetForClockRegion(float pesudoNetWeight);

    /**
     * @brief evaluate the Mahattan distance between two locations
     *
     * @param x0
     * @param y0
     * @param x1
     * @param y1
     * @return double
     */
    inline double manhattanDis(double x0, double y0, double x1, double y1)
    {
        return std::abs(x0 - x1) + y2xRatio * std::abs(y0 - y1);
    }

    PlacementInfo *placementInfo;

    QPSolverWrapper *xSolver = nullptr;
    QPSolverWrapper *ySolver = nullptr;
    std::map<std::string, std::string> &JSONCfg;
    bool verbose;

    /**
     * @brief a common factor indicate the overall strength of the nets in the QP model from external
     * setting
     *
     */
    float generalNetWeight = 1.0;

    /**
     * @brief a factor to tune the weights of the net spanning in Y-coordinate relative to the net spanning
     * in X-coordinate
     *
     */
    float y2xRatio = 1.0;

    /**
     * @brief indicate whether wirelength optimizer uses Eigen3, which cannot solve QP problem with constraints. If
     * false, OSQP solver which can set constraints for the quadratic model, will be involved to replace Eigen3.
     *
     */
    bool useUnconstrainedCG = true;

    /**
     * @brief  indicate whether wirelength optimizer is based on MKL library when using OSQP placer, which can set
     * constraints for the quadratic model
     *
     */
    bool MKLorNot = false;

    /**
     * @brief indicate whether we use direct macro legalization instread of the progressive legalization (2-phase
     * legalization)
     *
     * If we conduct direct macro legalization, pseudo net for legalization will be removed and macro will be placed to
     * the cooresponding location according to the legalization result.
     *
     */
    bool directMacroLegalize = false;

    /**
     * @brief the fade-out factor for the user-defined clusters
     *
     * The user-defined clusters might not be perfect so we might need to gradually reduce their terms in the QP problem
     * by this factor
     *
     */
    float userDefinedClusterFadeOutFactor = 1.0;

    /**
     * @brief the net weight for macro legalization
     *
     */
    float oriMacroLegalizationWeight = 1.0;

    /**
     * @brief the number of the conducted macro iterations
     *
     */
    int macroPseudoNetCnt = 0;

    std::map<DesignInfo::DesignNet *, std::vector<float>> netPinEnhanceRate;
};

#endif