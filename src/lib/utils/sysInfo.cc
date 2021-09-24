#include "sysInfo.h"
#include <assert.h>
std::string getExePath()
{
    char result[PATH_MAX];
    // ssize_t count =
    assert(readlink("/proc/self/exe", result, PATH_MAX) > 0);
    const char *path;
    path = dirname(result);
    return std::string(path);
}