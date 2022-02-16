import os
import numpy as np

for startPackingD in [0.25, 0.5, 1]:
    for deltaPackingD in [0.25, 0.33, 0.5]:
        for hpwlPackingWeight in [0.1, 0.05, 0.2]:
            overallTcl = open(
                "/home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl", "w")
            outputPath = "/home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/"
            outputName = "OpenPiton_"+str(startPackingD).replace(".", "p")+"_"+str(
                deltaPackingD).replace(".", "p")+"_"+str(hpwlPackingWeight).replace(".", "p")

            templateFile = open(
                "../benchmarks/testConfig/OpenPiton_template.json")
            outputFile = open("../benchmarks/testConfig/OpenPiton.json", "w")
            for line in templateFile.readlines():
                print(line.replace("arg1", str(startPackingD)).replace(
                    "arg2", str(deltaPackingD)).replace("arg3", str(hpwlPackingWeight)), end="", file=outputFile)
            outputFile.close()
            templateFile.close()

            errorCode = os.system(
                "./AMFPlacer ../benchmarks/testConfig/OpenPiton.json > "+outputPath+outputName+".placerLog")

            if (errorCode == 0):
                print(
                    "open_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-Vivado/midOpenPiton/OpenPiton.dcp", file=overallTcl)
                print("source /home/zslwyuan-laptop/Documents/AMF-Placer/build/dumpData_OpenPiton/DumpCLBPacking-first-0.tcl", file=overallTcl)
                print("write_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/" +
                      outputName+".dcp -force", file=overallTcl)
                overallTcl.close()
                os.system("vivado -mode batch -source /home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl -journal " +
                          outputPath+outputName+".jou"
                          + " -log " + outputPath+outputName+".log")
