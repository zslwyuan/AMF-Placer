from string import ascii_uppercase
import argparse
from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import numpy as np
import scipy.linalg
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt


def Average(lst):
    return sum(lst) / len(lst)


parser = argparse.ArgumentParser()

# parser.add_argument(
#     "-o", "--Output", help="The Output File Path", required=True)
parser.add_argument(
    "-i", "--Input", help="The Input File Path", required=True)

args = parser.parse_args()
inputFile = open(args.Input, 'r')
lines = inputFile.readlines()
lineId = 0

disXY2delay = dict()

XList = []
YList = []
delayList = []

while (lineId < len(lines)):
    pinDrivenLine = lines[lineId]
    pinDriverLine = lines[lineId+1]
    delayLine = lines[lineId+2]
    if (pinDrivenLine.find("=> SLICE_X") >= 0 and pinDriverLine.find("=> SLICE_X") >= 0):
        drivenDrivenSiteName = pinDrivenLine.replace("\n", "").split(" ")[3]
        drivenSiteX = int(drivenDrivenSiteName[drivenDrivenSiteName.rfind(
            "_X")+2:drivenDrivenSiteName.rfind("Y")])
        drivenSiteY = int(
            drivenDrivenSiteName[drivenDrivenSiteName.rfind("Y")+1:])
        drivenDriverSiteName = pinDriverLine.replace("\n", "").split(" ")[3]
        driverSiteX = int(drivenDriverSiteName[drivenDriverSiteName.rfind(
            "_X")+2:drivenDriverSiteName.rfind("Y")])
        driverSiteY = int(
            drivenDriverSiteName[drivenDriverSiteName.rfind("Y")+1:])

        disX = abs(drivenSiteX-driverSiteX)
        disY = abs(drivenSiteY-driverSiteY)
        tupleXY = (disX, disY)
        delay = int(delayLine.split(" ")[1])
        if (not tupleXY in disXY2delay.keys()):
            disXY2delay[tupleXY] = []
        disXY2delay[tupleXY].append(delay)
        XList.append(disX)
        YList.append(disY)
        delayList.append(delay)
    lineId += 3

XList = []
YList = []
delayList = []
for XYPair in disXY2delay.keys():
    XList.append(XYPair[0])
    YList.append(XYPair[1])
    delayList.append(min(disXY2delay[XYPair]))

z = np.array(delayList)
x = np.array(XList)
y = np.array(YList)

data = np.vstack((x, y, z))
data = data.T
print(data)

X, Y = np.meshgrid(np.arange(0.1, 40, 1), np.arange(0.1, 120, 1))
XX = X.flatten()
YY = Y.flatten()

# best-fit cubic curve
A = np.c_[np.ones(data.shape[0]), (data[:, :2])**0.3, data[:, :2]**0.5]
C, _, _, _ = scipy.linalg.lstsq(A, data[:, 2])

print("the factors for the 3-D fitting: ", C)


# evaluate it on a grid
Z = np.dot(np.c_[np.ones(XX.shape), (XX)**0.3, (YY)**0.3,
                 XX**0.5, YY**0.5], C).reshape(X.shape)

# plot points and fitted surface
fig = plt.figure()
ax = fig.gca(projection='3d')
ax.plot_surface(X, Y, Z, rstride=1, cstride=1, alpha=0.3)
ax.scatter(data[:, 0], data[:, 1], data[:, 2], c='r', s=5, alpha=0.1)
plt.xlabel('X')
plt.ylabel('Y')
ax.set_zlabel('Z')

plt.show()

while (True):
    XX = int(input("X"))
    YY = int(input("Y"))
    print("delay = ",  C[0] + XX**0.3*C[1]+YY **
          0.3*C[2]+XX**0.5*C[3]+YY**0.5*C[4])
