/**
 * @file main.cc
 * @author Tingyuan Liang (tliang@connect.ust.hk)
 * @brief AMF-Placer Main file
 * @version 0.1
 * @date 2021-05-31
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and Technology. All rights reserved.
 *
 */

#include "AMFPlacer.h"

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config JSON file> " << std::endl;
        return 1;
    }

    auto placer = new AMFPlacer(argv[1]);
    placer->run();
    delete placer;

    return 0;
}