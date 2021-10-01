/**
 * @file stringCheck.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _STRINGCHECK
#define _STRINGCHECK

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

bool strContains(std::string target, std::string substring);

void strSplit(const std::string &s, std::vector<std::string> &sv, const char *delim);

void replaceAll(std::string &str, const std::string from, const std::string to);

#endif