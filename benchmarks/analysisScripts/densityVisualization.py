import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
import glob

densityFileList = glob.glob('Density-*')
print(densityFileList)

for fileName in densityFileList:
    file = open(fileName,'r')

    arrayList = []
    for line in file.readlines():
        curList = []
        for ele in line.split(' ')[:-1]:
            curList.append(float(ele))
        arrayList.append(curList)
    numpy_array = np.array(arrayList)
    im = plt.imshow(numpy_array, cmap=cm.jet, origin='lower',vmin=0, vmax=1)
    plt.colorbar(im)
    plt.savefig(fileName+'.png')
    plt.close()
    #plt.show() 