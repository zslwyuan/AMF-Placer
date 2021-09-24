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
