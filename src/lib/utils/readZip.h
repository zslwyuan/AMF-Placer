/**
 * @file readZip.h
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

#ifndef _READZIP
#define _READZIP

#include <cstdio>
#include <iostream>

// create a FILEBUF to read the unzip file pipe

struct FILEbuf : std::streambuf
{
    FILEbuf(FILE *fp) : fp_(fp)
    {
    }
    int underflow()
    {
        if (this->gptr() == this->egptr())
        {
            int size = fread(this->buffer_, 1, int(s_size), this->fp_);
            if (0 < size)
            {
                this->setg(this->buffer_, this->buffer_, this->buffer_ + size);
            }
        }
        return this->gptr() == this->egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr());
    }
    FILE *fp_;
    enum
    {
        s_size = 1024
    };
    char buffer_[s_size];
};

#endif
