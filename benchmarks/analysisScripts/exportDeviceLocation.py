import matplotlib.cm as cm
import matplotlib as matplotlib
from matplotlib.colors import Normalize
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image
import numpy as np
from matplotlib.patches import Patch
import zipfile
import matplotlib.image as img 
  

tmpFile = open("tiles","r")
# reading png image file 
im = img.imread('ping3.png') 

outputFile = open("highlight3.tcl","w")

cnt = 0
colorindex=0
outputStr = "highlight -color_index "+ str(colorindex%20+1) +" [get_tiles {"
for line in tmpFile.readlines():
    
    tileX = int(line[line.rfind("_X")+2:line.rfind("Y")])
    tileY = int(line[line.rfind("Y")+1:])
    if (tileX>79 or tileY>479):
        continue
    if (im[479-tileY,tileX,0]<0.9):
        outputStr += line.replace("\n","") + " "

        cnt +=1
        if (cnt>10):
            cnt = 0
            colorindex += 1
            outputStr += "}]"
            print(outputStr,file=outputFile)
            outputStr = "highlight -color_index "+ str(colorindex%20+1) +" [get_tiles {"

if (cnt != 0):
    outputStr += "}]"
    print(outputStr,file=outputFile)

outputFile.close()