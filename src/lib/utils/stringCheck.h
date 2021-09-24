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