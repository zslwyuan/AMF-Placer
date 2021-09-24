
#ifndef _PLACERSYSINFO
#define _PLACERSYSINFO

#include <libgen.h>       // dirname
#include <linux/limits.h> // PATH_MAX
#include <string>
#include <unistd.h> // readlink

std::string getExePath();
#endif