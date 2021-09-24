import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
import glob

densityFileList = glob.glob('congestionInfo*')
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
    fig = plt.figure(figsize=(30,80))
    fig.tight_layout()
    im = plt.imshow(numpy_array, cmap=cm.jet, origin='lower',vmax=160)
    plt.colorbar(im)
    plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05)
    plt.savefig(fileName+'.png')
    plt.close()

    x = numpy_array.reshape((-1))
    plt.hist(x, bins=200,range=(110,160))
    plt.savefig(fileName+'hist.png')
    plt.close()
    #plt.show() 
