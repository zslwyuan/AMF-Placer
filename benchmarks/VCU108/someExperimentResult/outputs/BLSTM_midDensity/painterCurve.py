# encoding=utf-8
from matplotlib import pyplot
import matplotlib.pyplot as plt
import os

def loadData(curFile):
    yList = []
    xList = []
    for line in curFile.readlines():
        if (line.find(" Spreader Iteration# ")<0):
            continue
        tmpLine = line.replace("\n","")
        tmpLine = tmpLine[tmpLine.find("HPWL="):]
        tmpLine = tmpLine.split(" ")
        xList.append(float(tmpLine[3]))
        yList.append(float(tmpLine[0].replace("HPWL=","")))
    return xList, yList

fileNames = os.listdir("./")

for fileName in fileNames:
    if (fileName.find(".py")>=0):
        continue
    if (fileName.find("config")<0):
        continue
    print("processing " + fileName)
    tmpFile = open(fileName,'r')
    std0X,std0Y = loadData(tmpFile)
    plt.plot(std0X, std0Y, marker=',', mec='r', mfc='w',label=fileName)
plt.legend()  # 让图例生效
plt.xlabel("runtime(s)") #X轴标签
plt.ylabel("HPWL") #Y轴标签
plt.margins(0)
plt.show()

exit()
std0File = open("standard.txt",'r')
std1File = open("standard1.txt",'r')
simpleExanpandFile = open("simpleExpand.txt",'r')
ripplePseudoNetFile = open("RippleFPGAPseudoNetWeight.txt",'r')
forgetLastSpreadLocFile = open("forgetLastSpreadLoc.txt",'r')

std0X,std0Y = loadData(std0File)
std1X,std1Y = loadData(std1File)
SEX,SEY = loadData(simpleExanpandFile)
RPX,RPY = loadData(ripplePseudoNetFile)
FGX,FGY = loadData(forgetLastSpreadLocFile)

plt.plot(std0X, std0Y, marker='o', mec='r', mfc='w',label='ourPlacer')
plt.plot(std1X, std1Y, marker='*', ms=10,label='ourPlacer')

plt.plot(SEX, SEY, marker=',', mec='r', mfc='w',label='simpleExpand')
#plt.plot(RPX,RPY, marker='.', ms=10,label='RippleFPGA PseudoNetWeight')
plt.plot(FGX,FGY, marker='.', ms=10,label='forget location in last iteration')

plt.legend()  # 让图例生效
plt.xlabel("runtime(s)") #X轴标签
plt.ylabel("HPWL") #Y轴标签
plt.margins(0)
plt.show()

