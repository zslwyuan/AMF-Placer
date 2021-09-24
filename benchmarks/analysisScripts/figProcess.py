from matplotlib import pyplot as plt
import numpy as np
import cv2
import glob

densityFileList = glob.glob('DumpLUTFFCoordTrace-*.png')
for fileName in densityFileList:
    #fileName.replace("DumpLUTFFCoordTrace-", "").replace()
    img = cv2.imread(fileName)
    if img is None:
        print("图片读入失败, 请检查图片路径及文件名")
        exit()

    height = img.shape[0]
    width = img.shape[1]

    img[:, :, 1] = img[:, :, 1]*4

    cv2.imwrite("./new/"+fileName, img)

# ffmpeg -framerate 5 -i DumpCARRYCoordTrace-%d.png video.mp4
# ffmpeg -i video.mp4 -vcodec libx264 -crf 40 output.mp4
