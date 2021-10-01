/**
 * @file strPrint.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "strPrint.h"
#include <chrono>
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

std::chrono::time_point<std::chrono::steady_clock> oriTime;

void print_cmd(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 33, "HiFPlacer CMD: ", 0, tmp_string.c_str());
}

void print_info(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 34, "HiFPlacer INFO: ", 0, tmp_string.c_str());
}

void print_status(std::string tmp_string)
{
    auto nowTime = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - oriTime).count();
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s (elapsed time: %.3lf s)\n", 40, 32, "HiFPlacer STATUS: ", 0,
           tmp_string.c_str(), (double)millis / 1000);
}

void print_error(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "HiFPlacer ERROR: ", 0, tmp_string.c_str());
}

void print_warning(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "HiFPlacer WARNING: ", 0, tmp_string.c_str());
}

std::string to_string_align3(int __val)
{
    return __gnu_cxx::__to_xstring<std::string>(&std::vsnprintf, 4 * sizeof(int), "%3d", __val);
}

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(double percentage)
{
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
