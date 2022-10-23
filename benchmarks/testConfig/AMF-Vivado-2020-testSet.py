import os
import numpy as np

'''
benchmarkNames = ["OpenPiton","digitRecognition",
                  "minimap_GENE",
                  "MemN2N","BLSTM_midDensity",
                  "halfsqueezenet", "faceDetect",
                  "optimsoc"]
'''

testSet = ["noBlockAware", "noWNSGPOpt", "noSectorGuided", "useBigWindow"]
benchmarkNames = ["digitRecognition", "faceDetect", "halfsqueezenet",
                  "BLSTM_midDensity", "MemN2N",  "minimap_GENE", "OpenPiton"]
targetPath = "/home/tingyuan/Documents/AMF-Placer-DCP-Test/expSet/"

for testName in testSet:

    os.system("cd "+targetPath+";mkdir "+testName)
    for benchmarkName in benchmarkNames:
        print("testing ", benchmarkName, "with", testName)

        outputPath = targetPath+"/"+testName+"/"

        outputName = benchmarkName
        os.system("mkdir dumpData_"+benchmarkName)

        if (benchmarkName != " "):

            os.system("cd dumpData_"+benchmarkName+";rm *")
            os.system("./AMFPlacer ../benchmarks/testConfig/"+testName+"/"+benchmarkName +
                      ".json > "+outputPath+"/log_"+testName+"_"+benchmarkName)
            os.system("tail log_"+benchmarkName+" -n 300")

    # for benchmarkName in benchmarkNames:
        overallTcl = open(
            "/home/tingyuan/Documents/AMF-Placer2/overall.tcl", "w")
        outputName = benchmarkName

        errorCode = 0
        if (errorCode == 0):
            print(
                "open_checkpoint /home/tingyuan/Documents/Benchmark-DCP/"+benchmarkName+".dcp", file=overallTcl)
            print("source /home/tingyuan/Documents/AMF-Placer2/build/dumpData_" +
                  benchmarkName+"/DumpCLBPacking-first-0.tcl", file=overallTcl)
            print("report_datasheet -name timing_2 -file " + outputPath +
                  outputName+"2020_timing_report.txt", file=overallTcl)
            print("write_checkpoint "+outputPath+"/" +
                  outputName+"2020.dcp -force", file=overallTcl)
            overallTcl.close()
            os.system("vivado -mode batch -source /home/tingyuan/Documents/AMF-Placer2/overall.tcl -journal " +
                      outputPath+outputName+"2020.jou"
                      + " -log " + outputPath+outputName+"2020.log")
