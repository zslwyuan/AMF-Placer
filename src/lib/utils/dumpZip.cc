/**
 * @file dumpZip.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "dumpZip.h"
#include <assert.h>
#include <sys/stat.h>

void writeStrToGZip(std::string fileName, std::stringstream &data)
{
    // we will use GZip from zlib
    gzFile gz_file;
    // open the file for writing in binary mode
    gz_file = gzopen(fileName.c_str(), "wb");

    assert(Z_NULL != gz_file && "The zip file should be created successfully and please check your path settings.");
    // Get the size of the stream
    unsigned long int file_size = sizeof(char) * data.str().size();
    // Write the data
    gzwrite(gz_file, (void *)(data.str().data()), file_size);
    // close the file
    gzclose(gz_file);
}

bool fileExists(const std::string &filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
