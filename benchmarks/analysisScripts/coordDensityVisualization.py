from joblib import Parallel, delayed
import multiprocessing
from codecs import BOM_UTF8, BOM_UTF16_BE, BOM_UTF16_LE, BOM_UTF32_BE, BOM_UTF32_LE
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from scipy.stats import gaussian_kde
import numpy as np
import mpl_scatter_density
import matplotlib.pyplot as plt
import gzip
import zlib
from bs4 import UnicodeDammit
import time


def gzdecode(data):
    return gzip.decompress(data).decode('utf8')


BOMS = (
    (BOM_UTF8, "UTF-8"),
    (BOM_UTF32_BE, "UTF-32-BE"),
    (BOM_UTF32_LE, "UTF-32-LE"),
    (BOM_UTF16_BE, "UTF-16-BE"),
    (BOM_UTF16_LE, "UTF-16-LE"),
)


def check_bom(data):
    return [encoding for bom, encoding in BOMS if data.startswith(bom)]


def processInput(i):
    from scipy.stats import gaussian_kde
    import numpy as np
    import mpl_scatter_density
    import matplotlib.pyplot as plt
    filename = ("DumpLUTFFCoordTrace-GeneralSpreader-SLICEL_LUT-"+str(i)+".gz")
    file = gzip.open(filename, 'rb')
    content = file.read().decode()

    arrayList = []
    x = []
    y = []
    cnt = 0
    for line in content.split("\n")[:-1]:
        eles = line.split(' ')[:2]
        x.append(float(eles[0]))
        y.append(float(eles[1]))
        cnt += 1

    fig = plt.figure(figsize=(15, 30))
    ax = fig.add_subplot(1, 1, 1, projection='scatter_density')
    ax.scatter_density(x, y)
    ax.set_xlim(-10, 90)
    ax.set_ylim(-10, 490)
    # plt.show()
    fig.savefig("DumpLUTFFCoordTrace-GeneralSpreader-SLICEL_LUT-"+str(i)+'.png')
    print("saved "+"DumpLUTFFCoordTrace-GeneralSpreader-SLICEL_LUT-"+str(i)+'.png')
    file.close()


num_inputs = int(input())
inputs = range(0, num_inputs)
num_cores = multiprocessing.cpu_count()

results = Parallel(n_jobs=num_cores)(delayed(processInput)(i) for i in inputs)
print(results)
