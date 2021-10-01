/**
 * @file QPSolverWrapper.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "QPSolverWrapper.h"

#include <cmath>

void QPSolverWrapper::QPSolve(QPSolverWrapper *&curSolver)
{
    osqp::OsqpSolver &osqpSolver = curSolver->osqpSolver;

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower | Eigen::Upper> &CGSolver = curSolver->CGSolver;
    std::vector<Eigen::Triplet<float>> &objectiveMatrixTripletList = curSolver->solverData.objectiveMatrixTripletList;
    Eigen::VectorXd &objectiveVector = curSolver->solverData.objectiveVector;

    // min_x 0.5 * x'Px + q'x
    // s.t.  l <= Ax <= u

    // objective_matrix is P.
    // objective_vector is q.
    // constraint_matrix is A.
    // lower_bounds is l.
    // upper_bounds is u.

    Eigen::SparseMatrix<double> objective_matrix(objectiveVector.size(), objectiveVector.size());
    for (unsigned int i = 0; i < objectiveVector.size(); i++)
        objectiveMatrixTripletList.push_back(Eigen::Triplet<float>(i, i, curSolver->solverData.objectiveMatrixDiag[i]));
    objective_matrix.setFromTriplets(objectiveMatrixTripletList.begin(), objectiveMatrixTripletList.end());

    if (curSolver->solverSettings.useUnconstrainedCG)
    {
        /////////////////////////////////////////////////////////////////////////
        // Conjugate Gradient (does not support constraint yet.)
        if (curSolver->solverSettings.verbose)
            print_status("Unconstrained CG Solver Started.");
        CGSolver.setMaxIterations(curSolver->solverSettings.maxIters);
        CGSolver.setTolerance(curSolver->solverSettings.tolerence);
        CGSolver.compute(objective_matrix);
        if (curSolver->solverSettings.solutionForward)
            curSolver->solverData.oriSolution =
                CGSolver.solveWithGuess(-objectiveVector, curSolver->solverData.oriSolution);
        else
            curSolver->solverData.solution =
                CGSolver.solveWithGuess(-objectiveVector, curSolver->solverData.oriSolution);
        if (curSolver->solverSettings.verbose)
            print_status("Unconstrained CG Solver Done.");
    }
    else
    {
        /////////////////////////////////////////////////////////////////////////
        // OSQP (support constraints but runtime x2~3)
        Eigen::SparseMatrix<double> constraint_matrix(objectiveVector.size(), objectiveVector.size());
        std::vector<Eigen::Triplet<float>> constraints;
        for (unsigned int i = 0; i < objectiveVector.size(); i++)
        {
            constraints.push_back(Eigen::Triplet<float>(i, i, 1.0));
        }
        constraint_matrix.setFromTriplets(constraints.begin(), constraints.end());

        osqp::OsqpInstance instance;
        instance.objective_matrix = objective_matrix;
        instance.objective_vector = objectiveVector;
        instance.constraint_matrix = constraint_matrix;
        instance.lower_bounds.resize(objectiveVector.size());
        for (unsigned int i = 0; i < objectiveVector.size(); i++)
        {
            instance.lower_bounds[i] = curSolver->solverSettings.lowerbound;
        }
        instance.upper_bounds.resize(objectiveVector.size());
        for (unsigned int i = 0; i < objectiveVector.size(); i++)
        {
            instance.upper_bounds[i] = curSolver->solverSettings.upperbound;
        }

        osqp::OsqpSettings settings;
        settings.verbose = false;

        if (curSolver->solverSettings.verbose)
            print_status("OSQP Solver initializing.");

        auto status = osqpSolver.Init(instance, settings, curSolver->solverSettings.MKLorNot);
        assert(status.ok());

        if (curSolver->solverSettings.verbose)
            print_status("OSQP Solver Started.");

        status = osqpSolver.SetPrimalWarmStart(curSolver->solverData.oriSolution);
        assert(status.ok());

        osqp::OsqpExitCode exit_code = osqpSolver.Solve();
        assert(exit_code == osqp::OsqpExitCode::kOptimal);

        // if need to trace the objective, uncomment the line below
        // double optimal_objective = osqpSolver.objective_value();

        if (curSolver->solverSettings.solutionForward)
            curSolver->solverData.oriSolution = osqpSolver.primal_solution();
        else
            curSolver->solverData.solution = osqpSolver.primal_solution();

        if (curSolver->solverSettings.verbose)
            print_status("OSQP Solver Done.");
    }
}
