#ifndef _SIMPLEJSON
#define _SIMPLEJSON

#include "strPrint.h"
#include "stringCheck.h"
#include <algorithm>
#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

inline bool exists_test(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

std::map<std::string, std::string> parseJSONFile(std::string JSONFileName)
{
    assert(exists_test(JSONFileName));
    std::ifstream infile(JSONFileName.c_str());

    std::string line;

    std::string entireStr;
    while (std::getline(infile, line))
    {
        if (line.find("//") == std::string::npos)
            entireStr += line;
    }

    std::map<std::string, std::string> res;
    res.clear();

    entireStr = std::string(entireStr.begin() + 1, entireStr.end() - 1);
    std::vector<std::string> jsonBlocks;
    strSplit(entireStr, jsonBlocks, ",");

    for (auto jsonBlock : jsonBlocks)
    {

        replaceAll(jsonBlock, "    ", "");
        replaceAll(jsonBlock, "\"", "");
        while (jsonBlock.find(": ") != std::string::npos)
            replaceAll(jsonBlock, ": ", ":");
        while (jsonBlock.find(" :") != std::string::npos)
            replaceAll(jsonBlock, " :", ":");
        std::vector<std::string> elements;
        strSplit(jsonBlock, elements, ":");

        res[elements[0]] = elements[1];
    }

    if (res.find("dumpDirectory") != res.end())
    {
        std::map<std::string, std::string>::iterator it;
        for (it = res.begin(); it != res.end(); it++)
        {
            if (it->first.find("dump") != std::string::npos || it->first.find("Dump") != std::string::npos)
            {
                res[it->first] = res["dumpDirectory"] + "/" + it->second;
            }
        }
    }

    print_warning("Placer configuration is loaded and the information is shown below, please check:");

    for (auto pair : res)
    {
        std::cout << "   \"" << pair.first << "\"   ====   \"" << pair.second << "\"\n";
    }

    return res;
}

#endif
