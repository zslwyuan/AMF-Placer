/**
 * @file sysInfo.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _PLACERSYSINFO
#define _PLACERSYSINFO

#include <libgen.h>       // dirname
#include <linux/limits.h> // PATH_MAX
#include <string>
#include <unistd.h> // readlink

std::string getExePath();
#endif