/**
 * @file dumpZip.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _DUMPZIP
#define _DUMPZIP
#include <zlib.h>
#include <string>
#include <iostream>
#include <sstream>

void writeStrToGZip(std::string fileName, std::stringstream &data);
#endif