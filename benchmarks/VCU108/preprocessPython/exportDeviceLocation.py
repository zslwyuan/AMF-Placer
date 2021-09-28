import matplotlib.cm as cm
import matplotlib as matplotlib
from matplotlib.colors import Normalize
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image
import numpy as np
from matplotlib.patches import Patch
import zipfile
import os
import sys
gw = 0.3

assert(len(sys.argv) == 3)

targetPath = sys.argv[1]
deviceName = sys.argv[2]


class siteInfo(object):
    def __init__(self, siteName, tileName, clockRegionName, siteType, tileType):
        self.siteName = siteName
        self.tileName = tileName
        self.clockRegionName = clockRegionName
        self.siteType = siteType
        self.tileType = tileType
        self.L = not (tileName.find("_R_") >= 0 or tileName.find(
            "DSP") >= 0 or tileName.find("PCIE_X") == 0)

        self.X = int(tileName[tileName.rfind("_X")+2:tileName.rfind("Y")])

        if (tileName.find("CLE_M_R_X84") >= 0):
            self.L = True

        self.Y = int(tileName[tileName.rfind("Y")+1:])

        if (tileName.find("DSP") >= 0):
            if (int(siteName[siteName.rfind("Y")+1:]) % 2 == 1):
                self.siteType += "_T"
            else:
                self.siteType += "_B"

        if (siteType.find("BUFCE_LEAF_X16") >= 0):
            if (int(siteName[siteName.rfind("Y")+1:]) % 2 == 1):
                self.Y += 2/3 + gw/2 - gw*0.3/2
            else:
                self.Y += 1/3 + gw/2 - gw*0.3/2
        if (siteType.find("BUFCE_ROW") >= 0):
            self.Y += 1/2 + gw/2 - gw*0.3/2

        self.siteX = int(siteName[siteName.rfind("_X")+2:siteName.rfind("Y")])
        self.siteY = int(siteName[siteName.rfind("Y")+1:])

        if (siteType == "HPIOB" or siteType == "HRIO" or siteType == "BITSLICE_RX_TX"):
            self.Y = self.siteY//26 * 30 + ((self.siteY) % 26+1)/28*30
        if (siteType == "BITSLICE_CONTROL" or siteType == "BITSLICE_TX"):
            self.Y = self.siteY//4 * 30 + ((self.siteY) % 4+0.5)/5*30
        if (siteType == "RIU_OR"):
            self.Y = self.siteY//2 * 30 + ((self.siteY) % 2+0.5)/3*30
        if (siteType == "PLLE3_ADV"):
            self.Y += 13

        self.centerX = 0
        self.centerY = 0

        self.BELs = []


def processGTH_RSites():
    sites_in_GTH_R = dict()

    for curSite in sites:
        if (curSite.tileName.find("GTH_R") == 0):
            if (not curSite.tileName in sites_in_GTH_R.keys()):
                sites_in_GTH_R[curSite.tileName] = []
            sites_in_GTH_R[curSite.tileName].append(curSite)

    for tileName in sites_in_GTH_R.keys():
        BUFG_GT_SYNCs = []
        GTHE3_CHANNELs = []
        BUFG_GTs = []
        GTHE3_COMMON = []
        for curSite in sites_in_GTH_R[tileName]:
            if (curSite.siteType == "BUFG_GT_SYNC"):
                BUFG_GT_SYNCs.append(curSite)
            elif (curSite.siteType == "GTHE3_CHANNEL"):
                GTHE3_CHANNELs.append(curSite)
            elif (curSite.siteType == "BUFG_GT"):
                BUFG_GTs.append(curSite)
            elif (curSite.siteType == "GTHE3_COMMON"):
                GTHE3_COMMON.append(curSite)

        BUFG_GT_SYNCs = sorted(
            BUFG_GT_SYNCs, key=lambda curSite: curSite.siteY)
        GTHE3_CHANNELs = sorted(
            GTHE3_CHANNELs, key=lambda curSite: curSite.siteY)
        BUFG_GTs = sorted(BUFG_GTs, key=lambda curSite: curSite.siteY)

        GTHE3_COMMON[0].Y = GTHE3_COMMON[0].Y + (59+gw)/2 - 1
        BUFG_GT_SYNCs[5].Y = GTHE3_COMMON[0].Y + 1

        GTHE3_CHANNELs[0].Y = GTHE3_COMMON[0].Y - 5
        GTHE3_CHANNELs[1].Y = GTHE3_CHANNELs[0].Y + 2
        GTHE3_CHANNELs[3].Y = GTHE3_COMMON[0].Y + 5
        GTHE3_CHANNELs[2].Y = GTHE3_CHANNELs[3].Y - 2

        for curSite in BUFG_GT_SYNCs:
            curSite.X += 0.15

        initOffset = 12
        step = 5/12
        for curSite in BUFG_GTs[:12]:
            curSite.Y += initOffset
            initOffset += step

        initOffset = 46
        step = 5/12
        for curSite in reversed(BUFG_GTs[12:]):
            curSite.Y += initOffset
            initOffset -= step

        initOffset = 13
        step = 3/5
        for curSite in BUFG_GT_SYNCs[:5]:
            curSite.Y += initOffset
            initOffset += step

        initOffset = 45
        step = 3/5
        for curSite in reversed(BUFG_GT_SYNCs[6:]):
            curSite.Y += initOffset
            initOffset -= step


def processGTY_QuadSites():
    sites_in_GTY_Quad = dict()

    for curSite in sites:
        if (curSite.tileName.find("GTY_QUAD_LEFT_FT") == 0):
            if (not curSite.tileName in sites_in_GTY_Quad.keys()):
                sites_in_GTY_Quad[curSite.tileName] = []
            sites_in_GTY_Quad[curSite.tileName].append(curSite)

    for tileName in sites_in_GTY_Quad.keys():
        BUFG_GT_SYNCs = []
        GTYE3_CHANNELs = []
        BUFG_GTs = []
        GTYE3_COMMON = []
        for curSite in sites_in_GTY_Quad[tileName]:
            if (curSite.siteType == "BUFG_GT_SYNC"):
                BUFG_GT_SYNCs.append(curSite)
            elif (curSite.siteType == "GTYE3_CHANNEL"):
                GTYE3_CHANNELs.append(curSite)
            elif (curSite.siteType == "BUFG_GT"):
                BUFG_GTs.append(curSite)
            elif (curSite.siteType == "GTYE3_COMMON"):
                GTYE3_COMMON.append(curSite)

        BUFG_GT_SYNCs = sorted(
            BUFG_GT_SYNCs, key=lambda curSite: curSite.siteY)
        GTYE3_CHANNELs = sorted(
            GTYE3_CHANNELs, key=lambda curSite: curSite.siteY)
        BUFG_GTs = sorted(BUFG_GTs, key=lambda curSite: curSite.siteY)

        GTYE3_COMMON[0].Y = GTYE3_COMMON[0].Y + (59+gw)/2 - 1
        BUFG_GT_SYNCs[5].Y = GTYE3_COMMON[0].Y + 1

        GTYE3_CHANNELs[0].Y = GTYE3_COMMON[0].Y - 5
        GTYE3_CHANNELs[1].Y = GTYE3_CHANNELs[0].Y + 2
        GTYE3_CHANNELs[3].Y = GTYE3_COMMON[0].Y + 5
        GTYE3_CHANNELs[2].Y = GTYE3_CHANNELs[3].Y - 2

        for curSite in BUFG_GT_SYNCs:
            curSite.X -= 0.15

        initOffset = 12
        step = 5/12
        for curSite in BUFG_GTs[:12]:
            curSite.Y += initOffset
            initOffset += step

        initOffset = 46
        step = 5/12
        for curSite in reversed(BUFG_GTs[12:]):
            curSite.Y += initOffset
            initOffset -= step

        initOffset = 13
        step = 3/5
        for curSite in BUFG_GT_SYNCs[:5]:
            curSite.Y += initOffset
            initOffset += step

        initOffset = 45
        step = 3/5
        for curSite in reversed(BUFG_GT_SYNCs[6:]):
            curSite.Y += initOffset
            initOffset -= step


# deviceInfoFile = open("VCU108DeviceSite","r")

# site=> HPIOBDIFFINBUF_X1Y59 tile=> HPIO_L_X51Y120 type=> HPIOBDIFFINBUF
# site=> RIU_OR_X1Y8 tile=> XIPHY_L_X52Y120 type=> RIU_OR
# site=> SLICE_X96Y475 tile=> CLEL_L_X58Y475 type=> SLICEL

# lines = deviceInfoFile.readlines()

archive = zipfile.ZipFile(targetPath+"/"+deviceName +
                          "/"+deviceName+"_DeviceSite.zip", 'r')
lines = archive.read(deviceName+"_DeviceSite").decode('utf-8').split("\n")

plotPriority = dict()


sites = []
siteTypes = set()
siteType2id = dict()

insertedSite = set()
siteName2site = dict()

for line in lines:
    # puts $fo  "bel=> $curBEL site=> $curSite tile=> $curTile type=> $siteType"
    #           "bel=> $curBEL site=> $curSite tile=> $curTile clockRegion=> $clockRegion sitetype=> $siteType tiletype=> $tileType"
    bel_site_tile_clockRegion_sitetype_tiletype = line.replace("\n", "").replace("bel=> ", "").replace(
        " site=> ", ";").replace(" tile=> ", ";").replace(" sitetype=> ", ";").replace(" clockRegion=> ", ";").replace(" tiletype=> ", ";").split(";")
    if (len(bel_site_tile_clockRegion_sitetype_tiletype) < 2):
        continue

    if (bel_site_tile_clockRegion_sitetype_tiletype[1] in insertedSite):
        siteName2site[bel_site_tile_clockRegion_sitetype_tiletype[1]].BELs.append(
            bel_site_tile_clockRegion_sitetype_tiletype[0])
        continue
    else:
        insertedSite.add(bel_site_tile_clockRegion_sitetype_tiletype[1])
    tmpSiteInfo = siteInfo(bel_site_tile_clockRegion_sitetype_tiletype[1], bel_site_tile_clockRegion_sitetype_tiletype[2], bel_site_tile_clockRegion_sitetype_tiletype[3],
                           bel_site_tile_clockRegion_sitetype_tiletype[4], bel_site_tile_clockRegion_sitetype_tiletype[5])
    sites.append(tmpSiteInfo)
    siteName2site[tmpSiteInfo.siteName] = tmpSiteInfo
    siteName2site[bel_site_tile_clockRegion_sitetype_tiletype[1]].BELs.append(
        bel_site_tile_clockRegion_sitetype_tiletype[0])
    siteTypes.add(tmpSiteInfo.siteType)
    if (not tmpSiteInfo.siteType in siteType2id.keys()):
        siteType2id[tmpSiteInfo.siteType] = len(siteTypes)

cmap = cm.hsv
norm = Normalize(vmin=0, vmax=len(siteTypes))

plotPriority = dict()
for name in siteTypes:
    plotPriority[name] = 0
plotPriority['RAMBFIFO36'] = -1

priorityArr = []
for curSite in sites:
    priorityArr.append(plotPriority[curSite.siteType])
priorityArr = np.array(priorityArr)
order = np.argsort(priorityArr)


height = dict()
for name in siteTypes:
    height[name] = gw
height['RAMBFIFO36'] = 4 + gw
height['RAMB181'] = height['RAMBFIFO36']/2 - gw/4
height['RAMBFIFO18'] = height['RAMBFIFO36']/2 - gw/4
height['DSP48E2_T'] = height['RAMBFIFO36']/2 - gw/4
height['DSP48E2_B'] = height['RAMBFIFO36']/2 - gw/4
#height['PCIE_3_1'] = 6 + gw
height['BUFCE_LEAF_X16'] = gw*0.3
height['BUFCE_ROW'] = gw*0.3

weight = dict()
for name in siteTypes:
    weight[name] = gw
weight['RAMBFIFO36'] = gw*1.3
weight['BUFG_GT'] = gw*0.3
weight['BUFG_GT_SYNC'] = gw*0.3
weight['BUFCE_LEAF_X16'] = gw*0.3
weight['BUFCE_ROW'] = gw*0.3

yoffset = dict()
for name in siteTypes:
    yoffset[name] = 0
yoffset['RAMB181'] = 2 + gw/2 + gw/4/2
yoffset['RAMBFIFO18'] = gw/4/2
yoffset['DSP48E2_T'] = 2 + gw/2 + gw/4/2
yoffset['DSP48E2_B'] = gw/4/2
#yoffset['PCIE_3_1'] = (59+gw-height['PCIE_3_1'])/2

# insert HPIO/HRIO bank
for curSite in sites:
    if (curSite.X >= 34):
        curSite.X += 1
    if (curSite.X >= 53):
        curSite.X += 1
    if (curSite.tileName.find("HRIO_L_X") == 0):
        curSite.X += 1
    if (curSite.tileName.find("HPIO_L_X") == 0):
        curSite.X += 1

processGTH_RSites()
processGTY_QuadSites()

fig, ax = plt.subplots(1)
ax.set_xlim([0, 90])
ax.set_ylim([0, 500])
plt.axis('scaled')
arr = np.arange(len(siteTypes)+1)
# arr = np.concatenate((arr[0::2],arr[1::2]))
np.random.shuffle(arr)


legend_elements = []

sites = np.array(sites)
sites = sites[order]

addedPatchTypes = set()
for curSite in sites:
    if (curSite.L):
        rect = patches.Rectangle(((curSite.X-0.25-weight[curSite.siteType]/2), curSite.Y-gw/2 + yoffset[curSite.siteType]),
                                 weight[curSite.siteType], height[curSite.siteType],    color=cmap(norm(arr[siteType2id[curSite.siteType]])))
        curSite.centerX = (
            curSite.X-0.25-weight[curSite.siteType]/2) + weight[curSite.siteType]/2
        curSite.centerY = curSite.Y-gw/2 + \
            yoffset[curSite.siteType] + height[curSite.siteType]/2
    else:
        rect = patches.Rectangle(((curSite.X+0.25-weight[curSite.siteType]/2),   curSite.Y-gw/2 + yoffset[curSite.siteType]),
                                 weight[curSite.siteType], height[curSite.siteType], color=cmap(norm(arr[siteType2id[curSite.siteType]])))
        curSite.centerX = (
            curSite.X+0.25-weight[curSite.siteType]/2) + weight[curSite.siteType]/2
        curSite.centerY = curSite.Y-gw/2 + \
            yoffset[curSite.siteType] + height[curSite.siteType]/2

    if (not curSite.siteType in addedPatchTypes):
        addedPatchTypes.add(curSite.siteType)
        legend_elements.append(Patch(color=cmap(
            norm(arr[siteType2id[curSite.siteType]])), label=curSite.siteType))

#     ax.add_patch(rect)

# ax.legend(handles=legend_elements, bbox_to_anchor=(
#     1.05, 1.0), loc='upper left')
# plt.tight_layout()
# plt.show()


exportfile = open(targetPath+"/"+deviceName+"/" +
                  deviceName+"_exportSiteLocation", "w")

for curSite in sites:
    cx = curSite.centerX
    cy = curSite.centerY
    if (curSite.siteType.find("DSP") >= 0):
        curSite.siteType = curSite.siteType[:-2]
    if (curSite.siteName.find("DSP") >= 0 or curSite.siteName.find("RAMB") >= 0):
        if (curSite.siteY % 2 == 1):
            cy = curSite.centerY + (1-gw)/2

    print("site=> " + curSite.siteName
          + " tile=> " + curSite.tileName
          + " clockRegionName=> " + curSite.clockRegionName
          + " sitetype=> " + curSite.siteType
          + " tiletype=> " + curSite.tileType
          + " centerx=> " + str(cx)
          + " centery=> " + str(cy)
          + " BELs=> " + str(curSite.BELs).replace(" ", "").replace("\'", ""), file=exportfile)

exportfile.close()
os.system("zip -j "+targetPath+"/"+deviceName+"/" +
          deviceName+"_exportSiteLocation.zip "+targetPath+"/"+deviceName+"/" +
          deviceName+"_exportSiteLocation")
