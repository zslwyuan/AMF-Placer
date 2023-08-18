
#include "simpleJSON.h"


std::map<std::string, std::string> parseJSONFile(std::string JSONFileName)
{
    assert(exists_test(JSONFileName));
    std::ifstream infile(JSONFileName.c_str());
    assert(infile.good());
    std::string line;

    std::string entireStr;
    while (std::getline(infile, line))
    {
        if (line.find("//") == std::string::npos)
            entireStr += line;
        else
            entireStr += line.substr(0, line.find("//"));
    }

    std::map<std::string, std::string> res;
    res.clear();

    entireStr = std::string(entireStr.begin() + 1, entireStr.end() - 1);
    std::vector<std::string> jsonBlocks;
    strSplit(entireStr, jsonBlocks, ",");

    for (auto jsonBlock : jsonBlocks)
    {
        if (jsonBlock.find(":") == std::string::npos)
            continue;
        if (jsonBlock.find("\"") == std::string::npos)
            continue;

        int quote1Loc = jsonBlock.find("\"");
        int quote2Loc = jsonBlock.find("\"", quote1Loc + 1);
        int quote3Loc = jsonBlock.find("\"", quote2Loc + 1);
        int quote4Loc = jsonBlock.find("\"", quote3Loc + 1);

        assert(quote1Loc >= 0 && quote2Loc >= 0 && quote3Loc >= 0 && quote4Loc >= 0 &&
               "the json file has some syntax error");

        res[jsonBlock.substr(quote1Loc + 1, quote2Loc - quote1Loc - 1)] =
            jsonBlock.substr(quote3Loc + 1, quote4Loc - quote3Loc - 1);
    }

    if (res.find("dumpDirectory") == res.end())
    {
        res["dumpDirectory"] = "./";
    }
    else
    {
        res["dumpDirectory"] += "/";
    }

    std::map<std::string, std::string>::iterator it;
    for (it = res.begin(); it != res.end(); it++)
    {
        if (it->first == "dumpDirectory")
            continue;
        if (it->first.find("dump") != std::string::npos || it->first.find("Dump") != std::string::npos)
        {
            res[it->first] = res["dumpDirectory"] + "/" + it->second;
        }
    }

    print_warning("Placer configuration is loaded and the information is shown below, please check:");

    for (auto pair : res)
    {
        std::cout << "   \"" << pair.first << "\"   ====   \"" << pair.second << "\"\n";
    }

    return res;
}
