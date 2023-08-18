/**
 * @file QPSolverWrapper.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
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

#ifndef _QPSOLVER
#define _QPSOLVER

#include "Eigen/Eigen"
#include "Eigen/SparseCore"
//#include "osqp++/osqp++.h"
#include "strPrint.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class QPSolverWrapper
{
  public:
    typedef struct
    {
        std::vector<Eigen::Triplet<float>> objectiveMatrixTripletList;
        std::vector<float> objectiveMatrixDiag;
        Eigen::VectorXd objectiveVector;
        Eigen::VectorXd solution;
        Eigen::VectorXd oriSolution;
    } solverDataType;

    solverDataType solverData;
    // osqp::OsqpSolver osqpSolver;

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower | Eigen::Upper> CGSolver;
    typedef struct
    {
        bool useUnconstrainedCG = true;
        bool MKLorNot = false;
        float lowerbound;
        float upperbound;
        int maxIters = 500;
        float tolerence = 0.001;
        bool solutionForward = false;
        bool verbose = false;
    } solverSettingsType;
    solverSettingsType solverSettings;

    QPSolverWrapper(bool useUnconstrainedCG, bool MKLorNot, float lowerbound, float upperbound, int elementNum,
                    bool verbose)
    {
        solverSettings.useUnconstrainedCG = useUnconstrainedCG;
        solverSettings.MKLorNot = MKLorNot;
        solverSettings.lowerbound = lowerbound;
        solverSettings.upperbound = upperbound;
        solverSettings.verbose = verbose;
        solverData.solution.resize(elementNum);
        solverData.oriSolution.resize(elementNum);
        solverData.objectiveVector.resize(elementNum);
    }
    ~QPSolverWrapper()
    {
    }

    static void QPSolve(QPSolverWrapper *&curSolver);
};

#endif