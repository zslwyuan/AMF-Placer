/**
 * @file sysInfo.cc
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

#include "sysInfo.h"
#include <assert.h>
#include <cstring>
#include <iostream>

std::string getExePath()
{
    char result[PATH_MAX];
    //ssize_t count =
    //assert(readlink("/proc/self/exe", result, PATH_MAX) > 0);

    //const char *path;
    //path = dirname(result);
    //return std::string(path);
    return std::string("/home/ccip/boost");
}