/**
 * @file strPrint.h
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

#ifndef _STRPRINT
#define _STRPRINT

#include <chrono>
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

extern std::chrono::time_point<std::chrono::steady_clock> oriTime;

void print_cmd(std::string tmp_string);
void print_info(std::string tmp_string);
void print_status(std::string tmp_string);
void print_error(std::string tmp_string);
void print_warning(std::string tmp_string);
void printProgress(double percentage);
void setOriTime();
std::string to_string_align3(int __val);

#endif
