import os
import sys
from os import listdir
from os.path import isfile, join
from pathlib import Path

mypathPrefix = "../benchmarks/testConfig/testConfigSets/config"
for i in range(0, 9):
    mypath = mypathPrefix+str(i)
    onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]
    for filename in onlyfiles:
        splitList = filename.split("/")
        benchmarkName = splitList[-1].replace(".json", "")
        configName = "config"+str(i)
        Path("../outputs/"+benchmarkName+"/").mkdir(parents=True, exist_ok=True)
        outputFileName = "../outputs/"+benchmarkName+"/"+configName
        print("running command: ", "./AMFPlacer " +
              mypath+"/"+filename + " > "+outputFileName)
        os.system("./AMFPlacer "+mypath+"/"+filename + " > "+outputFileName)
