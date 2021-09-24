import os
import sys
from os import listdir
from os.path import isfile, join, isdir
from pathlib import Path

from matplotlib import rcParams
rcParams['font.family']='sans-serif'
rcParams['font.sans-serif']=['Tahoma']
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from io import StringIO

mypathPrefix = "./outputs"
dirs = [f for f in listdir(mypathPrefix) if isdir(join(mypathPrefix, f))]
benchmark2cfgid2HPWLRuntime = dict()
for dirPath in dirs:
    splitList = dirPath.split("/")
    benchmarkName = splitList[-1]
    benchmark2cfgid2HPWLRuntime[benchmarkName] = dict()
    for i in range(0, 9):
        mypath = mypathPrefix+"/"+dirPath + "/config"+str(i)
        logFile = open(mypath, 'r')
        lines = []
        for line in logFile.readlines():
            lines.append(line)
        runtime = float(lines[-2].split(" ")[-2])
        HPWL = float(lines[-1].split(" ")[-1])
        benchmark2cfgid2HPWLRuntime[benchmarkName][i] = (HPWL, runtime)

print(benchmark2cfgid2HPWLRuntime)

string2CSV = "     HPWL\\time\n"
for benchmarkName in benchmark2cfgid2HPWLRuntime.keys():
    maxRuntime = 0
    maxHPWL = 0

    for id in benchmark2cfgid2HPWLRuntime[benchmarkName].keys():
        if (benchmark2cfgid2HPWLRuntime[benchmarkName][id][0] > maxHPWL):
            maxHPWL = benchmark2cfgid2HPWLRuntime[benchmarkName][id][0]
        if (benchmark2cfgid2HPWLRuntime[benchmarkName][id][1] > maxRuntime):
            maxRuntime = benchmark2cfgid2HPWLRuntime[benchmarkName][id][1]

    for id in benchmark2cfgid2HPWLRuntime[benchmarkName].keys():
        string2CSV += "cfg"+str(id)+"\\"+str(benchmark2cfgid2HPWLRuntime[benchmarkName][id][0]/maxHPWL)+"\\"+str(
            benchmark2cfgid2HPWLRuntime[benchmarkName][id][1]/maxRuntime)+"\n"
    string2CSV += ".\\0 \\0\n"

s = StringIO(string2CSV)

df = pd.read_csv(s, index_col=0, delimiter='\\\\', skipinitialspace=True)
print(df)
fig = plt.figure()  # Create matplotlib figure

for benchmarkName in benchmark2cfgid2HPWLRuntime.keys():
    print(benchmarkName)

ax = fig.add_subplot(111)  # Create matplotlib axes
ax2 = ax.twinx()  # Create another axes that shares the same x-axis as ax.

width = 0.4

df.HPWL.plot(kind='bar', color='red', ax=ax, width=width, position=1)
df.time.plot(kind='bar', color='blue', ax=ax2, width=width, position=0)

ax.set_ylabel('HPWL (normalized)')
ax2.set_ylabel('Placement Runtime (normalized)')
ax.legend(loc='upper left')
ax2.legend(loc='upper right')

plt.show()


string2CSV = "     HPWL\\HPWL(normalized)\\time(s)\\time(normalized)\n"
for benchmarkName in benchmark2cfgid2HPWLRuntime.keys():
    minRuntime = 200000000000.0
    minHPWL = 200000000000.0

    for id in benchmark2cfgid2HPWLRuntime[benchmarkName].keys():
        if (benchmark2cfgid2HPWLRuntime[benchmarkName][id][0] < minHPWL):
            minHPWL = benchmark2cfgid2HPWLRuntime[benchmarkName][id][0]
        if (benchmark2cfgid2HPWLRuntime[benchmarkName][id][1] < minRuntime):
            minRuntime = benchmark2cfgid2HPWLRuntime[benchmarkName][id][1]

    for id in benchmark2cfgid2HPWLRuntime[benchmarkName].keys():
        string2CSV += "cfg"+str(id)+"\\"+str(benchmark2cfgid2HPWLRuntime[benchmarkName][id][0])+"\\"+str(benchmark2cfgid2HPWLRuntime[benchmarkName][id][0]/minHPWL)   + "\\"+str(benchmark2cfgid2HPWLRuntime[benchmarkName][id][1])  + "\\"+str(benchmark2cfgid2HPWLRuntime[benchmarkName][id][1]/minRuntime)+"\n"
                    
    string2CSV += ".\\0 \\0 \\0 \\0\n"

s = StringIO(string2CSV)

df = pd.read_csv(s, index_col=0, delimiter='\\', skipinitialspace=True)

df.to_excel("output.xlsx")  