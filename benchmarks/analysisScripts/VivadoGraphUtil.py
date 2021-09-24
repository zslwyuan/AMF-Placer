import networkx as nx


class VivadoCell(object):
    def __init__(self, id, name, refType, pins, netStr, drivepinStr):
        self.id = id
        self.name = name
        self.refType = refType
        self.pins = pins
        self.netStr = netStr
        self.drivepinStr = drivepinStr
        self.drivepins_fromOthers = set()

    def bindPinDriveObjs(self, pinName2Obj):
        self.drivepins = []
        assert(len(self.drivepinStr) == len(self.pins))
        for i in range(len(self.drivepinStr)):
            tmpName = self.drivepinStr[i]
            pin = self.pins[i]
            if ((tmpName in pinName2Obj.keys()) and (not pin is None)):
                self.drivepins.append(pinName2Obj[tmpName])
            else:
                if (tmpName.find("VCC") < 0 and tmpName.find("GND") < 0 and len(tmpName) > 0):
                    if (tmpName.find("en_sig_1/G") < 0):
                        print("pin might have problem: ", tmpName)
                self.drivepins.append(None)
                continue
            if (pin.dir):
                self.drivepins_fromOthers.add(self.drivepins[-1])


class VivadoPin(object):
    def __init__(self, name, refName, nameOnCell, dir, cell):
        self.name = name
        self.dir = dir
        self.cell = cell
        self.refName = refName
        self.nameOnCell = nameOnCell


class VivadoNet(object):
    def __init__(self, name, inputpins, outputpins):
        self.name = name
        self.inputpins = inputpins
        self.outputpins = outputpins


class VivadoCoreCluster(object):
    def __init__(self, coreType, coreNodes, patternStr, nodeInCluster):
        self.coreType = coreType
        self.coreNodes = coreNodes
        self.patternStr = patternStr
        self.nodeInCluster = nodeInCluster


class VivadoPatternCluster(object):
    def __init__(self, initPatternStr, unextendedNodes, extendedNodes, clusterId):
        self.patternExtensionTrace = initPatternStr.replace(
            "\'", "").replace("\\", "")
        self.unextendedNodes = unextendedNodes
        self.extendedNodes = extendedNodes
        self.disabled = False
        self.clusterId = clusterId


class VivadoPatternClusterSeq(object):
    def __init__(self, initPatternStr, patternClusters):
        self.patternExtensionTrace = initPatternStr.replace(
            "\'", "").replace("\\", "")
        self.patternClusters = patternClusters


def loadCellInfoFromFile(textFile):

    firstCell = True

    VivadoCells = []
    cellName2Obj = dict()
    pinName2Obj = dict()
    idCnt = 0
    for line in textFile.decode('utf-8').split("\n"):

        # print(line)

        if (line.find("curCell=> ") >= 0):

            if (not firstCell):
                if (not (curCell is None)):
                    if (not curCell.name in cellName2Obj.keys()):
                        VivadoCells.append(curCell)
                        cellName2Obj[curCell.name] = curCell
                    else:
                        print("WARNING duplicate cell:", curCell.name)
            firstCell = False

            name_type = line.replace("curCell=> ", "").split(" type=> ")
            name = name_type[0]
            refType = name_type[1]
            if (name in cellName2Obj.keys()):
                curCell = None
            else:
                curCell = VivadoCell(idCnt, name, refType, [], [], [])
                idCnt += 1

            continue

        if (line.find("   pin=> ") >= 0):
            if (curCell is None):
                continue
            pin_refpin_dir_net_drivepin = line.replace("   pin=> ", "").replace(" refpin=> ", ";").replace(
                " dir=> ", ";").replace(" net=> ", ";").replace(" drivepin=> ", ";").split(";")

            if (len(pin_refpin_dir_net_drivepin) > 5):
                assert(False)  # we don't expect multi-driver

            if (pin_refpin_dir_net_drivepin[4].replace("\n", "") != ""):
                curCell.netStr.append(
                    pin_refpin_dir_net_drivepin[3].replace("\n", ""))
                curCell.drivepinStr.append(
                    pin_refpin_dir_net_drivepin[4].replace("\n", ""))

                if (pin_refpin_dir_net_drivepin[2] == "OUT"):
                    if (pin_refpin_dir_net_drivepin[0] != pin_refpin_dir_net_drivepin[4].replace("\n", "")):
                        curPin = VivadoPin(pin_refpin_dir_net_drivepin[4].replace(
                            "\n", ""), pin_refpin_dir_net_drivepin[1], pin_refpin_dir_net_drivepin[0], pin_refpin_dir_net_drivepin[2] == "IN", curCell)
                    else:
                        curPin = VivadoPin(pin_refpin_dir_net_drivepin[0], pin_refpin_dir_net_drivepin[1],
                                           pin_refpin_dir_net_drivepin[0], pin_refpin_dir_net_drivepin[2] == "IN", curCell)
                else:
                    curPin = VivadoPin(pin_refpin_dir_net_drivepin[0], pin_refpin_dir_net_drivepin[1],
                                       pin_refpin_dir_net_drivepin[0],  pin_refpin_dir_net_drivepin[2] == "IN", curCell)

                if (curPin.name == ""):
                    continue

                if (not curPin.name in pinName2Obj.keys()):
                    pinName2Obj[curPin.name] = curPin
                curCell.pins.append(curPin)

    VivadoCells.append(curCell)
    cellName2Obj[curCell.name] = curCell

    for curCell in VivadoCells:
        curCell.bindPinDriveObjs(pinName2Obj)

    return VivadoCells


def VivadoGraphExctractionAndInitialPatternDetect(VivadoCells):
    inputPatterns = dict()
    inputPatterns_Cells = dict()
    cellTypes = dict()
    outputPatternCnt = dict()

    netlist = []
    netset = set()
    netstr2Tuple = dict()
    netstr2netnum = dict()
    cellname2id = dict()
    nodetype = dict()
    nodename = dict()
    node2id = dict()
    for tmpCell in VivadoCells:
        outputPatternCnt[tmpCell.id] = dict()

    VivadoGraph = nx.DiGraph()
    edgeAttributes = []
    for tmpCell in VivadoCells:

        nodetype[tmpCell.id] = tmpCell.refType
        nodename[tmpCell.id] = tmpCell.name
        node2id[tmpCell.id] = tmpCell.id
        cellname2id[tmpCell.name] = tmpCell.id
        VivadoGraph.add_node(tmpCell.id)

        if (not(tmpCell.refType in cellTypes.keys())):
            cellTypes[tmpCell.refType] = 0
        cellTypes[tmpCell.refType] += 1

        driverTypeCnt = dict()
        driverCheck = set()

        for pin, net, drivepin in zip(tmpCell.pins, tmpCell.netStr, tmpCell.drivepins):

            if (not (drivepin is None)):
                if (pin.dir and net != ""):
                    # if (tmpCell.name == "design_1_i/face_detect_0/inst/grp_processImage_fu_371/SI_V_10_49_1_reg_26649_reg"):
                    #     print("pin=>", pin.name, "drivepin.cell=>", drivepin.cell.name, "drivepin=>", drivepin.name)
                    tmp_netTuple = (drivepin.cell.id, tmpCell.id)
                    edgeAttributes.append(
                        (drivepin.cell.id, tmpCell.id, drivepin.cell.refType+"-"+drivepin.refName))
                    tmp_netTuple_Str = tmp_netTuple
                    if (not(tmp_netTuple_Str in netset)):
                        netset.add(tmp_netTuple_Str)
                        netstr2Tuple[tmp_netTuple_Str] = tmp_netTuple
                        netstr2netnum[tmp_netTuple_Str] = 0
                    netstr2netnum[tmp_netTuple_Str] += 1

    for key in netstr2netnum.keys():
        netlist.append(
            (netstr2Tuple[key][0], netstr2Tuple[key][1], netstr2netnum[key]))

    VivadoGraph.add_weighted_edges_from(netlist)

    for a, b, driverPinType in edgeAttributes:
        VivadoGraph[a][b]['driverPinType'] = driverPinType

    nx.set_node_attributes(VivadoGraph, name="name", values=nodename)
    nx.set_node_attributes(VivadoGraph, name="type", values=nodetype)
    nx.set_node_attributes(VivadoGraph, name="id", values=node2id)
    print("#nodes:", VivadoGraph.number_of_nodes())
    print("#edges:", VivadoGraph.number_of_edges())

    return VivadoGraph


def getInitalSingleCorePatterns(VivadoGraph, careTypeList, coreType="CARRY", checkDirection="both", allowOverlap=True, onlyRecordInput=False):

    careTypeList = set(careTypeList)

    singleCorePattern = None

    if (checkDirection == "both"):
        inCheck = True
        outCheck = True
    elif (checkDirection == "in"):
        inCheck = True
        outCheck = False
    elif (checkDirection == "out"):
        inCheck = False
        outCheck = True
    else:
        assert(False)

    singleCorePattern = dict()

    traversedNodes = set()

    for nodeId in VivadoGraph.nodes():
        curNode = VivadoGraph.nodes()[nodeId]

        nodeType = curNode['type']
        if (nodeType.find(coreType) >= 0):

            if ((nodeId in traversedNodes) and (not allowOverlap)):
                continue

            nodeInPattern = set()
            nodeInPattern.add(nodeId)
            traversedNodes.add(nodeId)
            predTypeCnt = dict()
            if (inCheck):
                for pred in VivadoGraph.predecessors(nodeId):
                    if ((pred in traversedNodes) and (not allowOverlap)):
                        continue
                    nodeInPattern.add(pred)
                    traversedNodes.add(pred)
                    predNode = VivadoGraph.nodes()[pred]
                    if (not predNode['type'] in careTypeList):
                        continue
                    if (not (predNode['type'] in predTypeCnt.keys())):
                        predTypeCnt[predNode['type']] = 0
                    predTypeCnt[predNode['type']
                                ] += VivadoGraph[pred][nodeId]['weight']

            predTypeList = []
            for key in predTypeCnt.keys():
                predTypeList.append(key+"-"+str(predTypeCnt[key]))
            predTypeList.sort()
            predPatternStr = '-'.join(predTypeList) + ">="

            succTypeCnt = dict()
            if (outCheck):
                for succ in VivadoGraph.successors(nodeId):
                    if ((succ in traversedNodes) and (not allowOverlap)):
                        continue
                    if (not onlyRecordInput):
                        nodeInPattern.add(succ)
                        traversedNodes.add(succ)
                    succNode = VivadoGraph.nodes()[succ]
                    if (not succNode['type'] in careTypeList):
                        continue
                    if (not (succNode['type'] in succTypeCnt.keys())):
                        succTypeCnt[succNode['type']] = 0
                    succTypeCnt[succNode['type']
                                ] += VivadoGraph[nodeId][succ]['weight']

            succTypeList = []
            for key in succTypeCnt.keys():
                succTypeList.append(key+"-"+str(succTypeCnt[key]))
            succTypeList.sort()
            succPatternStr = ">=" + '-'.join(succTypeList)

            overallPatternStr = predPatternStr + nodeType + succPatternStr

            if (coreType == "MUXF7" and overallPatternStr.find("MUXF8") >= 0):
                continue

            if (not (overallPatternStr in singleCorePattern.keys())):
                singleCorePattern[overallPatternStr] = []

            singleCorePattern[overallPatternStr].append(
                (curNode, nodeInPattern))

    numSingleCorePattern = dict()
    for key in singleCorePattern.keys():
        numSingleCorePattern[key] = len(singleCorePattern[key])

    sortedSingleCorePattern = []
    for w in sorted(numSingleCorePattern, key=numSingleCorePattern.get, reverse=True):
        # if (len(singleCorePattern[w]) >= 5):
        #     print(w, len(singleCorePattern[w]))
        sortedSingleCorePattern.append((w, singleCorePattern[w]))

    return singleCorePattern, sortedSingleCorePattern


def chainBFS(VivadoGraph, nodeId):

    coreType = VivadoGraph.nodes()[nodeId]['type']
    curId = nodeId
    reachSet = set()
    curSet = set({curId})

    while (len(curSet) > 0):
        reachSet = reachSet | curSet
        nextSet = set()
        for curId in curSet:
            for succ in VivadoGraph.successors(curId):
                if (coreType == VivadoGraph.nodes()[succ]['type']):
                    if (not (succ in reachSet)):
                        nextSet.add(succ)
            for pred in VivadoGraph.predecessors(curId):
                if (coreType == VivadoGraph.nodes()[pred]['type']):
                    if (not (pred in reachSet)):
                        nextSet.add(pred)
        curSet = nextSet

    return reachSet


def clusterNodeChain(VivadoGraph, coreType="CARRY8"):

    chainedSet = set()
    chains = []
    for nodeId in VivadoGraph.nodes():
        curNode = VivadoGraph.nodes()[nodeId]
        nodeType = curNode['type']
        if (nodeType.find(coreType) >= 0):
            if (nodeId in chainedSet):
                continue
            coreNodes = chainBFS(VivadoGraph, nodeId)
            chainedSet = chainedSet | coreNodes
            chains.append(VivadoCoreCluster(nodeType, coreNodes, None, None))

    return chains


def clusterNodeWithCommonFanin(VivadoGraph, VivadoCells, targetType="RAM32M16"):

    clusteredSet = set()

    allNodesInType = []
    for nodeId in VivadoGraph.nodes():
        curNode = VivadoGraph.nodes()[nodeId]
        nodeType = curNode['type']
        if (nodeType.find(targetType) >= 0):
            allNodesInType.append(nodeId)

    graphForCluster = nx.Graph()
    for nodeId in allNodesInType:
        graphForCluster.add_node(nodeId)

    edges = []
    for listId, nodeId in enumerate(allNodesInType):
        for other_nodeId in allNodesInType[listId+1:]:
            driverPins = (VivadoCells[nodeId].drivepins_fromOthers) & (
                VivadoCells[other_nodeId].drivepins_fromOthers)
            edges.append((nodeId, other_nodeId, len(driverPins)))

    edges.sort(key=lambda tup: tup[2], reverse=True)

    node2NumCommonFanin = dict()

    for edge in edges:
        nodeA = edge[0]
        nodeB = edge[1]
        numCommonFanin = edge[2]
        if (not nodeA in node2NumCommonFanin.keys()):
            node2NumCommonFanin[nodeA] = numCommonFanin
        if (not nodeB in node2NumCommonFanin.keys()):
            node2NumCommonFanin[nodeB] = numCommonFanin

        if (node2NumCommonFanin[nodeA] == node2NumCommonFanin[nodeB]):
            graphForCluster.add_edge(nodeA, nodeB)

    clusterSet_list = []

    for edge in edges:
        nodeA = edge[0]
        nodeB = edge[1]

        if (not nodeA in clusteredSet):
            overlapDriverPin = None
            newCluster = set()
            clusteredSet.add(nodeA)
            newCluster.add(nodeA)
            for nodeId in graphForCluster.neighbors(nodeA):
                if (not nodeId in clusteredSet):
                    driverPins = (VivadoCells[nodeId].drivepins_fromOthers) & (
                        VivadoCells[nodeA].drivepins_fromOthers)
                    if (overlapDriverPin is None):
                        overlapDriverPin = driverPins

                    if (overlapDriverPin == driverPins):
                        clusteredSet.add(nodeId)
                        newCluster.add(nodeId)

            clusterSet_list.append(newCluster)
            continue

        if (not nodeB in clusteredSet):
            overlapDriverPin = None
            newCluster = set()
            clusteredSet.add(nodeB)
            newCluster.add(nodeB)
            for nodeId in graphForCluster.neighbors(nodeB):
                if (not nodeId in clusteredSet):
                    driverPins = (VivadoCells[nodeId].drivepins_fromOthers) & (
                        VivadoCells[nodeB].drivepins_fromOthers)
                    if (overlapDriverPin is None):
                        overlapDriverPin = driverPins

                    if (overlapDriverPin == driverPins):
                        clusteredSet.add(nodeId)
                        newCluster.add(nodeId)

            clusterSet_list.append(newCluster)
            continue

    clusters = []
    # print("comps:")
    for comp in clusterSet_list:  # nx.connected_components(graphForCluster#
        clusters.append(VivadoCoreCluster(targetType, comp, None, None))
        # for id in comp:
        #     print(node2NumCommonFanin[id], VivadoGraph.nodes()[id]['name'])
        # print("================================")

    # exit()
    return clusters


def clusterExtendPatterns(VivadoGraph, chains, largeCluserIntoPattern=False, allowOverlap=False, largeCluserThredhold=2):

    patternStr2Chains = dict()
    traversedNodes = set()

    largeCnt = 0
    for chain in chains:
        corePatternStr = chain.coreType + "-" + str(len(chain.coreNodes))

        if (largeCluserIntoPattern and len(chain.coreNodes) > largeCluserThredhold):
            corePatternStr += "largeParallel("+str(largeCnt)+")"
            largeCnt += 1
            for nodeId in chain.coreNodes:

                if ((nodeId in traversedNodes) and (not allowOverlap)):
                    continue
                traversedNodes.add(nodeId)

                nodeInPattern = set([nodeId])

                newCluster = VivadoCoreCluster(
                    chain.coreType, set(), None, None)
                # predTypeCnt = dict()
                # for pred in VivadoGraph.predecessors(nodeId):
                #     if ((pred in traversedNodes) and (not allowOverlap)):
                #         continue
                #     traversedNodes.add(pred)
                #     nodeInPattern.add(pred)
                #     predNode = VivadoGraph.nodes()[pred]
                #     if (not (predNode['type'] in predTypeCnt.keys())):
                #         predTypeCnt[predNode['type']] = 0
                #     predTypeCnt[predNode['type']] += VivadoGraph[pred][nodeId]['weight']

                # predTypeList = []
                # for key in predTypeCnt.keys():
                #     predTypeList.append(key+"-"+str(predTypeCnt[key]))
                # predTypeList.sort()
                # predPatternStr = '-'.join(predTypeList) + ">="

                # succTypeCnt = dict()
                # for succ in VivadoGraph.successors(nodeId):
                #     if ((succ in traversedNodes) and (not allowOverlap)):
                #         continue
                #     traversedNodes.add(succ)
                #     nodeInPattern.add(succ)
                #     succNode = VivadoGraph.nodes()[succ]
                #     if (not (succNode['type'] in succTypeCnt.keys())):
                #         succTypeCnt[succNode['type']] = 0
                #     succTypeCnt[succNode['type']] += VivadoGraph[nodeId][succ]['weight']

                # succTypeList = []
                # for key in succTypeCnt.keys():
                #     succTypeList.append(key+"-"+str(succTypeCnt[key]))
                # succTypeList.sort()
                # succPatternStr = ">=" + '-'.join(succTypeList)

                # overallPatternStr = predPatternStr + corePatternStr + succPatternStr
                overallPatternStr = ">=" + corePatternStr + ">="
                newCluster.patternStr = overallPatternStr
                newCluster.nodeInCluster = nodeInPattern

                if (not (overallPatternStr in patternStr2Chains.keys())):
                    patternStr2Chains[overallPatternStr] = []

                patternStr2Chains[overallPatternStr].append(newCluster)

        else:
            nodeInPattern = chain.coreNodes.copy() - traversedNodes

            predTypeCnt = dict()
            for nodeId in chain.coreNodes - traversedNodes:
                for pred in VivadoGraph.predecessors(nodeId):
                    if ((pred in traversedNodes) and (not allowOverlap)):
                        continue
                    traversedNodes.add(pred)
                    nodeInPattern.add(pred)
                    predNode = VivadoGraph.nodes()[pred]
                    if (not (predNode['type'] in predTypeCnt.keys())):
                        predTypeCnt[predNode['type']] = 0
                    predTypeCnt[predNode['type']
                                ] += VivadoGraph[pred][nodeId]['weight']

            predTypeList = []
            for key in predTypeCnt.keys():
                predTypeList.append(key+"-"+str(predTypeCnt[key]))
            predTypeList.sort()
            predPatternStr = '-'.join(predTypeList) + ">="

            succTypeCnt = dict()
            for nodeId in chain.coreNodes - traversedNodes:
                for succ in VivadoGraph.successors(nodeId):
                    if ((succ in traversedNodes) and (not allowOverlap)):
                        continue
                    traversedNodes.add(succ)
                    nodeInPattern.add(succ)
                    succNode = VivadoGraph.nodes()[succ]
                    if (not (succNode['type'] in succTypeCnt.keys())):
                        succTypeCnt[succNode['type']] = 0
                    succTypeCnt[succNode['type']
                                ] += VivadoGraph[nodeId][succ]['weight']

            succTypeList = []
            for key in succTypeCnt.keys():
                succTypeList.append(key+"-"+str(succTypeCnt[key]))
            succTypeList.sort()
            succPatternStr = ">=" + '-'.join(succTypeList)

            overallPatternStr = predPatternStr + corePatternStr + succPatternStr

            chain.patternStr = overallPatternStr
            chain.nodeInCluster = nodeInPattern

            if (not (overallPatternStr in patternStr2Chains.keys())):
                patternStr2Chains[overallPatternStr] = []

            patternStr2Chains[overallPatternStr].append(chain)

    numPatternStr2Chains = dict()
    for key in patternStr2Chains.keys():
        numPatternStr2Chains[key] = len(patternStr2Chains[key])

    sortedPatternStr2Chains = []
    for w in sorted(numPatternStr2Chains, key=numPatternStr2Chains.get, reverse=True):
        # if (len(patternStr2Chains[w]) >= 2):
        #     print(w, len(patternStr2Chains[w]))
        sortedPatternStr2Chains.append((w, patternStr2Chains[w]))

    return patternStr2Chains, sortedPatternStr2Chains


def printOutSimplePatterns(VivadoGraph, singleCorePattern):
    numSingleCorePattern = dict()
    for key in singleCorePattern.keys():
        numSingleCorePattern[key] = len(singleCorePattern[key])

    for w in sorted(numSingleCorePattern, key=numSingleCorePattern.get, reverse=True):

        nodeInPattern = set()
        overlappedSet = set()
        for curnode, neighbornodes in singleCorePattern[w]:
            nodeInPattern = nodeInPattern | neighbornodes

        for curNodeIdX, (curnodeX, neighbornodesX) in enumerate(singleCorePattern[w]):
            for curNodeIdY, (curnodeY, neighbornodesY) in enumerate(singleCorePattern[w]):
                if (curNodeIdX <= curNodeIdY):
                    break
                overlappedSet = overlappedSet | (
                    neighbornodesX & neighbornodesY)

        cnt = 0
        for comp in nx.algorithms.weakly_connected_components(VivadoGraph.subgraph(nodeInPattern)):
            cnt += 1

        if (numSingleCorePattern[w] <= 1):
            continue

        print("pattern: ", w, ":", numSingleCorePattern[w], ":", cnt)
        print("     overlap nodes:")
        for nodeId in overlappedSet:
            print("        nodeName:", VivadoGraph.nodes()[nodeId]['name'], "type", VivadoGraph.nodes()[
                  nodeId]['type'], "degree", VivadoGraph.degree(nodeId))
        print("     anchor nodes:")
        for curnode, neighbornodes in singleCorePattern[w]:
            print("        nodeName:",
                  curnode['name'], " id:", curnode['id'], "type: ", curnode['type'])
            for nNode in neighbornodes:
                print("              NeighborNodeName:", VivadoGraph.nodes()[
                      nNode]['name'], " id:", VivadoGraph.nodes()[nNode]['id'], "type: ", VivadoGraph.nodes()[nNode]['type'])
            for nodeId in neighbornodes:
                print(
                    "                    highlight_objects -color red [get_cells ", VivadoGraph.nodes()[nodeId]['name'], "]")


def printOutChainPatterns(VivadoGraph, patternStr2Chains):

    print("======================================\nprinting out chains' patterns")

    numpatternStr2Chains = dict()
    for key in patternStr2Chains.keys():
        numpatternStr2Chains[key] = len(patternStr2Chains[key])

    for w in sorted(numpatternStr2Chains, key=numpatternStr2Chains.get, reverse=True):

        nodeInPattern = set()
        overlappedSet = set()
        for chain in patternStr2Chains[w]:
            nodeInPattern = nodeInPattern | chain.nodeInCluster

        for curChainIdX, curChainX in enumerate(patternStr2Chains[w]):
            for curChainIdY, curChainY in enumerate(patternStr2Chains[w]):
                if (curChainIdX <= curChainIdY):
                    break
                overlappedSet = overlappedSet | (
                    curChainX.nodeInCluster & curChainY.nodeInCluster)

        cnt = 0
        for comp in nx.algorithms.weakly_connected_components(VivadoGraph.subgraph(nodeInPattern)):
            cnt += 1

        if (numpatternStr2Chains[w] <= 1):
            continue

        print("pattern: ", w, ":", numpatternStr2Chains[w], ":", cnt)
        for comp in nx.algorithms.weakly_connected_components(VivadoGraph.subgraph(nodeInPattern)):
            for nodeId in comp:
                print(
                    "                    highlight_objects -color red [get_cells ", VivadoGraph.nodes()[nodeId]['name'], "]")
            break
        print("     overlap nodes:")
        for nodeId in overlappedSet:
            print("        nodeName:", VivadoGraph.nodes()[nodeId]['name'], "type", VivadoGraph.nodes()[
                  nodeId]['type'], "degree", VivadoGraph.degree(nodeId))
        print("     anchor chains:")
        for chain in patternStr2Chains[w]:
            print("        coreNodes:", chain.coreNodes)
            for nodeId in chain.coreNodes:
                print("                    ", VivadoGraph.nodes()
                      [nodeId]['name'], " id:", nodeId)
            for nodeId in chain.coreNodes:
                print(
                    "                    highlight_objects -color red [get_cells ", VivadoGraph.nodes()[nodeId]['name'], "]")


def instantiatePatternClusters(VivadoGraph, sortedSingleCorePattern, lastClusterId):
    res = []
    clusterColorIdInitial = dict()
    for node in VivadoGraph.nodes():
        clusterColorIdInitial[node] = -1
    nx.set_node_attributes(
        G=VivadoGraph, values=clusterColorIdInitial, name="clusterColorId")

    for w, seqs in sortedSingleCorePattern:
        patternClusters = []
        if (len(seqs) == 0):
            assert(False)
        if (isinstance(seqs[0], VivadoCoreCluster)):
            for chain in seqs:
                unextendedNodes = set(chain.nodeInCluster)-set(chain.coreNodes)
                extendedNodes = set(chain.coreNodes)
                if (len(unextendedNodes) == 0):
                    unextendedNodes = extendedNodes
                patternClusters.append(VivadoPatternCluster(initPatternStr=chain.patternStr,
                                                            unextendedNodes=unextendedNodes, extendedNodes=extendedNodes, clusterId=lastClusterId))
                for nodeInSet in patternClusters[-1].extendedNodes | patternClusters[-1].unextendedNodes:
                    if (VivadoGraph.nodes()[nodeInSet]['clusterColorId'] < 0):
                        VivadoGraph.nodes()[
                            nodeInSet]['clusterColorId'] = lastClusterId
                lastClusterId += 1
        else:
            for curNode, neighborNodes in seqs:
                unextendedNodes = set(neighborNodes)-set([curNode['id']])
                extendedNodes = set([curNode['id']])
                if (len(unextendedNodes) == 0):
                    unextendedNodes = extendedNodes
                patternClusters.append(VivadoPatternCluster(
                    initPatternStr=w, unextendedNodes=unextendedNodes, extendedNodes=extendedNodes, clusterId=lastClusterId))
                for nodeInSet in patternClusters[-1].extendedNodes | patternClusters[-1].unextendedNodes:
                    if (VivadoGraph.nodes()[nodeInSet]['clusterColorId'] < 0):
                        VivadoGraph.nodes()[
                            nodeInSet]['clusterColorId'] = lastClusterId
                lastClusterId += 1
        res.append(VivadoPatternClusterSeq(
            initPatternStr=w, patternClusters=patternClusters))

    return res, lastClusterId


def accumulateClusterForce(Patterns):
    pass
    # for pattern in Patterns:
    #     for cluster in pattern:
    #         accumulateInterconnectionForceAmongNodes(cluster)


def printOutPatterns(patterns):
    print("printOutPatterns:")
    cntSum = 0
    for curPatternSeq in patterns:
        print(curPatternSeq.patternExtensionTrace.replace("\'", "").replace("\\", ""), len(curPatternSeq.patternClusters), len(
            curPatternSeq.patternClusters[0].extendedNodes | curPatternSeq.patternClusters[0].unextendedNodes))
        cntSum += len(curPatternSeq.patternClusters)
    print("Total Clusters:", cntSum)
    return cntSum


def BFSCheckEdgeAttributes(VivadoGraph, VivadoCells, curCell):
    edgeAttributeCnt = dict()
    cellQ = [curCell]
    depQ = [0]
    visitedSet = {curCell}
    nextSet = set([curCell])

    while (len(cellQ) > 0):
        curCell = cellQ.pop()
        curDepth = depQ.pop()
        visitedSet.add(curCell)

        if(curDepth >= 5):
            continue

        # if (len(set(VivadoGraph.successors(curCell)))<4):
        #     for succ in VivadoGraph.successors(curCell):
        #         if (not (succ in nextSet)):
        #             if (VivadoGraph[curCell][succ]['driverPinType'].find("[")>=0):
        #                 if (VivadoGraph[curCell][succ]['driverPinType'] in edgeAttributeCnt.keys()):
        #                     edgeAttributeCnt[VivadoGraph[curCell][succ]['driverPinType']] += 1
        #                 else:
        #                     edgeAttributeCnt[VivadoGraph[curCell][succ]['driverPinType']] = 1
        #             else:
        #                 nextSet.add(succ)
        #                 depQ.append(curDepth+1)
        #                 cellQ.append(succ)
        if (len(set(VivadoGraph.predecessors(curCell))) < 32):
            for pred in VivadoGraph.predecessors(curCell):
                if (not (pred in nextSet)):
                    if (VivadoGraph[pred][curCell]['driverPinType'].find("[") >= 0):
                        if (VivadoGraph[pred][curCell]['driverPinType'] in edgeAttributeCnt.keys()):
                            edgeAttributeCnt[VivadoGraph[pred]
                                             [curCell]['driverPinType']] += 1
                        else:
                            edgeAttributeCnt[VivadoGraph[pred]
                                             [curCell]['driverPinType']] = 1
                    else:
                        nextSet.add(pred)
                        depQ.append(curDepth+1)
                        cellQ.append(pred)

    sortedEdgeAttributeCnt = []
    for w in sorted(edgeAttributeCnt, key=edgeAttributeCnt.get, reverse=True):
        sortedEdgeAttributeCnt.append((w, edgeAttributeCnt[w]))

    if (len(sortedEdgeAttributeCnt) > 4):
        sortedEdgeAttributeCnt = sortedEdgeAttributeCnt[:4]
    return sortedEdgeAttributeCnt


def loadClusters(name2id, clusterFileName):
    clusters = []
    id2cluster = dict()
    clusterCnt = 0
    clusterFile = open(clusterFileName, "r")

    for line in clusterFile.readlines():
        clusterCellNames = line.split(" ")
        ids = set()
        for name in clusterCellNames:
            if (name != ""):
                if (name in name2id.keys()):
                    ids.add(name2id[name])
                    id2cluster[name2id[name]] = clusterCnt
        clusterCnt += 1
        clusters.append(ids)
    return clusters, id2cluster


def loadClocks(clockFileName):
    clockNames = []
    clusterFile = open(clockFileName, "r")
    for line in clusterFile.readlines():
        clockNames.append(line.replace("\n", "").replace("/O", ""))
    return clockNames


def loadFixedBlocks(fixedUnitFileName):
    fixedUnitNames = []
    fixedUnitFile = open(fixedUnitFileName, "r")
    for line in fixedUnitFile.readlines():
        fixedUnitNames.append(line.replace("\n", "").split(" ")[1])
    return fixedUnitNames
