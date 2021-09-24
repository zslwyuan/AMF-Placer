#include "dumpZip.h"
#include <assert.h>
void writeStrToGZip(std::string fileName, std::stringstream &data)
{
    // we will use GZip from zlib
    gzFile gz_file;
    // open the file for writing in binary mode
    gz_file = gzopen(fileName.c_str(), "wb");

    assert(Z_NULL != gz_file);
    // Get the size of the stream
    unsigned long int file_size = sizeof(char) * data.str().size();
    // Write the data
    gzwrite(gz_file, (void *)(data.str().data()), file_size);
    // close the file
    gzclose(gz_file);
}
