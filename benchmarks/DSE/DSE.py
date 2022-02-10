import os

arg1 = 2
while (arg1 < 8):
    overallTcl = open(
        "/home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl", "w")
    outputPath = "/home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/"
    outputName = "OpenPiton_"+str(arg1).replace(".", "p")

    templateFile = open("../benchmarks/testConfig/OpenPiton_template.json")
    outputFile = open("../benchmarks/testConfig/OpenPiton.json", "w")
    for line in templateFile.readlines():
        print(line.replace("XXXXXX", str(arg1)), end="", file=outputFile)
    outputFile.close()
    templateFile.close()

    os.system(
        "./AMFPlacer ../benchmarks/testConfig/OpenPiton.json > "+outputPath+outputName+".placerLog")

    print("open_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-Vivado/midOpenPiton/OpenPiton.dcp", file=overallTcl)
    print("source /home/zslwyuan-laptop/Documents/AMF-Placer/build/dumpData_OpenPiton/DumpCLBPacking-first-0.tcl", file=overallTcl)
    print("write_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/" +
          outputName+".dcp -force", file=overallTcl)
    overallTcl.close()
    os.system("vivado -mode batch -source /home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl -journal " +
              outputPath+outputName+".jou"
              + " -log " + outputPath+outputName+".log")
    arg1 = arg1 + 0.75
