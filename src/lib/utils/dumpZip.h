/**
 * @file dumpZip.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#ifndef _DUMPZIP
#define _DUMPZIP
#include <iostream>
#include <sstream>
#include <string>
#include <zlib.h>

void writeStrToGZip(std::string fileName, std::stringstream &data);
bool fileExists(const std::string &filename);
#endif