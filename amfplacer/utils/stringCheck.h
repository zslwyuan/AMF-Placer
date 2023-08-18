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