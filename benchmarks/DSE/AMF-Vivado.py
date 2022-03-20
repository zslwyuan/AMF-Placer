import os
import numpy as np

benchmarkNames = ["BLSTM_midDensity",
                  "halfsqueezenet", "faceDetect",
                  "optimsoc",
                  "digitRecognition",
                  "minimap_GENE",
                  "MemN2N",
                  "OpenPiton"]

for benchmarkName in benchmarkNames:
    overallTcl = open(
        "/home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl", "w")
    outputPath = "/home/zslwyuan-laptop/Documents/AMF-Placer-DCP-Test/dse/"
    outputName = benchmarkName

    # errorCode = os.system(
    #     "./AMFPlacer ../benchmarks/testConfig/"+benchmarkName+".json > "+outputPath+outputName+".placerLog")
    errorCode = 0
    if (errorCode == 0):
        print(
            "open_checkpoint /home/zslwyuan-laptop/Documents/Benchmark-DCP/"+benchmarkName+".dcp", file=overallTcl)
        print("source /home/zslwyuan-laptop/Documents/AMF-Placer/build/dumpData_" +
              benchmarkName+"/DumpCLBPacking-first-0.tcl", file=overallTcl)
        print("write_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-DCP-Test/dse/" +
              outputName+".dcp -force", file=overallTcl)
        overallTcl.close()
        os.system("vivado -mode batch -source /home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl -journal " +
                  outputPath+outputName+".jou"
                  + " -log " + outputPath+outputName+".log")
