import matplotlib.cm as cm
import matplotlib as matplotlib
from matplotlib.colors import Normalize
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image
import numpy as np
from matplotlib.patches import Patch
import re

deviceInfoFile = open("PCIEPin2Sw", "r")
refpinnamefile = open("pcierefpinname", "r")
# pin=> XIL_UNCONN_OUT99 swtile=> INT_INTERFACE_PCIE_L_X77Y23

lines = deviceInfoFile.readlines()
exportfile = open("PCIEPin2SwXY", "w")

devicePinNames = set()

for line in lines:
    # puts $fo  "bel=> $curBEL site=> $curSite tile=> $curTile type=> $siteType"
    pin_SW = line.replace("\n", "").replace(
        "pin=> ", "").replace(" swtile=> ", ";").split(";")

    pinName = pin_SW[0]

    if (re.search("[0-9]+", pinName)):
        if (pinName.find("_") >= 0):
            lastWord = pinName[pinName.rfind("_")+1:]
            prevWord = pinName[:pinName.rfind("_")]
            tmploc = re.search("[0-9]+", lastWord)
            if (not tmploc is None):
                tmploc = re.search("[0-9]+", lastWord).span()[0]
                lastWord = lastWord[:tmploc]+"["+lastWord[tmploc:]+"]"
            pinName = prevWord+lastWord
        else:
            tmploc = re.search("[0-9]+", pinName).span()[0]
            pinName = pinName[:tmploc]+"["+pinName[tmploc:]+"]"

    pinName = pinName.replace("_", "")

    SWName = pin_SW[1]
    X = 0
    Y = int(SWName[SWName.rfind("Y")+1:])
    devicePinNames.add(pinName)
    # print("pin=> "+pinName+" swX=> "+str(X)+" swY=> "+str(Y))
    print("pin=> "+pinName+" swX=> "+str(X)+" swY=> "+str(Y), file=exportfile)

lines = refpinnamefile.readlines()

designRefPinNames = set(lines[0].replace("\n", "").split(" "))

print("designRefPinNames-devicePinNames: ", designRefPinNames-devicePinNames)

if (len(designRefPinNames-devicePinNames) > 0):
    print("WARNING!!!!!!!!!!!!!!!!!")
    print("The pins shown above fail to match with the design pins!")
exportfile.close()
