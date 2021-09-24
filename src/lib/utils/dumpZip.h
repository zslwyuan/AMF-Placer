#ifndef _DUMPZIP
#define _DUMPZIP
#include <zlib.h>
#include <string>
#include <iostream>
#include <sstream>

void writeStrToGZip(std::string fileName, std::stringstream &data);
#endif