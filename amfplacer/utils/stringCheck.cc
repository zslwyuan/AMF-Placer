/**
 * @file stringCheck.cc
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

#include "stringCheck.h"
#include <cstring>
#include <iostream>
#include <vector>

bool strContains(std::string target, std::string substring)
{
    return (target.find(substring) != std::string::npos);
}

void strSplit(const std::string &s, std::vector<std::string> &sv, const char *delim)
{
    sv.clear();
    char *buffer = new char[s.size() + 1];
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);
    char *p = std::strtok(buffer, delim);
    do
    {
        sv.push_back(p);
    } while ((p = std::strtok(NULL, delim)));
    delete[] buffer;
    return;
}

void replaceAll(std::string &str, const std::string from, const std::string to)
{
    if (from.empty())
        return;
    std::string::size_type start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
