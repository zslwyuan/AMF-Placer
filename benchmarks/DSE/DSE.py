import os


for arg1 in [2, -1,  4]:
    for arg2 in [2, -1,  1]:
        for arg3 in [-1, 1, 2]:
            overallTcl = open(
                "/home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl", "w")
            outputPath = "/home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/"
            outputName = "OpenPiton_"+str(arg1)+"_"+str(arg2)+"_"+str(arg3)

            os.system(
                "./AMFPlacer ../benchmarks/testConfig/OpenPiton.json "+str(arg1)+" "+str(arg2)+" "+str(arg3)+" > "+outputPath+outputName+".placerLog")

            print("open_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-Vivado/midOpenPiton/OpenPiton.dcp", file=overallTcl)
            print("source /home/zslwyuan-laptop/Documents/AMF-Placer/build/dumpData_OpenPiton/DumpCLBPacking-first-0.tcl", file=overallTcl)
            print("write_checkpoint /home/zslwyuan-laptop/Documents/AMF-Placer-DCP/dse/" +
                  outputName+".dcp -force", file=overallTcl)
            overallTcl.close()
            os.system("vivado -mode batch -source /home/zslwyuan-laptop/Documents/AMF-Placer/overall.tcl -journal " +
                      outputPath+outputName+".jou"
                      + " -log " + outputPath+outputName+".log")
