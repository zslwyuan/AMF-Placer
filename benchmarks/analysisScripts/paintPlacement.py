from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from codecs import BOM_UTF8, BOM_UTF16_BE, BOM_UTF16_LE, BOM_UTF32_BE, BOM_UTF32_LE

import networkx as nx
import VivadoGraphUtil
import networkx as nx
import VivadoGraphUtil
import zipfile
import gzip
from PIL import Image
from PIL import ImageOps
import argparse
from os import listdir
from os.path import isfile, join


"""
usage example:  python paintPlacement.py -d xxxx/minimap2_allCellPinNet.zip -t xxxxx/dumpData_minimap_GENE -o xxxx/dumpData_minimap_GENE
-d indicate the design information archive, e.g.   benchmarks/VCU108/design/minimap2/minimap2_allCellPinNet.zip
-t indicate the path where the placement trace archives are dumped. (the trace files are required to be named as DumpAllCoordTrace-xxx.gz currently)
-o indicate the path where you want to store the output images (png) generated according to the trace files
"""

parser = argparse.ArgumentParser()

parser.add_argument(
    "-t", "--TraceDirectory", help="The directory that contains the DumpAllCoordTrace-xxx.gz files", required=True)
parser.add_argument(
    "-d", "--DesignInfoFile", help="The Input Archive File Path", required=True)
parser.add_argument(
    "-o", "--OutputDirectory", help="The directory specified for the output trace figures and video", required=True)
args = parser.parse_args()

patternStr = "DumpAllCoordTrace-"

window = 0                                             # glut window number
width, height = 800,  2000                             # window size

benchmarkName = args.DesignInfoFile.split(
    "/")[-1].split(".")[0].replace("_allCellPinNet", "")

print("extracting "+(benchmarkName)+"_allCellPinNet from "+args.DesignInfoFile)
archive = zipfile.ZipFile(
    args.DesignInfoFile, 'r')
textFile = archive.read(""+(benchmarkName)+"_allCellPinNet")

VivadoCells = VivadoGraphUtil.loadCellInfoFromFile(textFile)
VivadoGraph = VivadoGraphUtil.VivadoGraphExctractionAndInitialPatternDetect(
    VivadoCells)
del textFile
name2node = dict()
for node in VivadoCells:
    name2node[node.name] = node


def draw_rect(x, y, width, height):
    # start drawing a rectangle
    glBegin(GL_QUADS)
    glVertex2f(x, y)                                   # bottom left point
    glVertex2f(x + width, y)                           # bottom right point
    glVertex2f(x + width, y + height)                  # top right point
    glVertex2f(x, y + height)                          # top left point
    glEnd()


def refresh2d(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0.0, width, 0.0, height, 0.0, 1.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def draw():                                            # ondraw is called all the time
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)  # clear the screen
    glLoadIdentity()                                   # reset position
    refresh2d(width, height)                           # set mode to 2d
    glColor3f(0.0, 0.0, 0.0)
    draw_rect(0, 0, width, height*1.5)

    for curNum, name in enumerate(names):
        if (not name in name2node.keys()):
            continue
        node = name2node[name]
        tX = x[curNum] / 80.0 * width
        tY = y[curNum] / 480.0 * height
        if (node.refType.find('LUT') >= 0):
            glColor3f(1.0, 0.0, 0.0)
            draw_rect(tX, tY, 2, 2)
        elif (node.refType[0] == 'F'):
            glColor3f(0.0, 1.0, 0.0)
            draw_rect(tX, tY, 2, 2)
        elif (node.refType.find('CARRY') >= 0):
            glColor3f(0.0, 0.0, 1.0)
            draw_rect(tX, tY, 2, 8)
        elif (node.refType.find('MUX') >= 0):
            glColor3f(1.0, 0.0, 1.0)
            draw_rect(tX, tY, 2, 2)
        elif (node.refType.find('DSP') >= 0):
            glColor3f(0.0, 1.0, 1.0)
            draw_rect(tX, tY, 5, 20)
        elif (node.refType.find('RAMB') >= 0):
            glColor3f(1.0, 1.0, 0.0)
            draw_rect(tX, tY, 5, 20)
        elif (node.refType.find('RAM') >= 0):
            glColor3f(1.0, 0.5, 0.5)
            draw_rect(tX, tY, 3, 3)
        else:
            glColor3f(1.0, 1.0, 1.0)
            draw_rect(tX, tY, 2, 2)

    glFlush()
    glPixelStorei(GL_PACK_ALIGNMENT, 1)
    data = glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE)
    image = Image.frombytes("RGBA", (width, height), data)
    # in my case image is flipped top-bottom for some reason
    image = ImageOps.flip(image)
    image.save(targetImgName, 'PNG')


glutInit()                                             # initialize glut
glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
glutInitWindowSize(width, height)                      # set window size
# set window position
glutInitWindowPosition(0, 0)
# create window with title
window = glutCreateWindow("placement")

onlyfiles = [f for f in listdir(args.TraceDirectory) if isfile(
    join(args.TraceDirectory, f))]

for filename in onlyfiles:

    if (filename.find(patternStr) < 0 or filename.find(".gz") < 0):
        continue
    fileId = int(filename.split(patternStr)[1].replace(".gz", ""))
    filename = args.TraceDirectory + "/"+patternStr+str(fileId)+".gz"
    targetImgName = args.OutputDirectory + \
        "/"+patternStr+str(fileId)+".png"

    print("handling archive file: "+filename)
    print("file id: "+str(fileId))
    print("The output image file: "+targetImgName)

    file = gzip.open(filename, 'rb')
    content = file.read().decode()

    x = []
    y = []
    names = []
    cnt = 0
    for line in content.split("\n")[:-1]:
        eles = line.replace('\n', '').split(' ')
        x.append(float(eles[0]))
        y.append(float(eles[1]))
        names.append(eles[2])
        cnt += 1

    # initialization
    draw()
    # set draw function callback
    # glutDisplayFunc(draw)
    # glutIdleFunc(draw)                                     # draw all the time
    # glutMainLoop()

# ffmpeg -framerate 5 -i DumpAllCoordTrace-%d.png video.mp4
# ffmpeg -i video.mp4 -vcodec libx264 -crf 40 output.mp4
