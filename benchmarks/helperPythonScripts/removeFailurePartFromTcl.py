import argparse


parser = argparse.ArgumentParser()

parser.add_argument(
    "-o", "--Output", help="The Output File Path", required=True)
parser.add_argument(
    "-i", "--Input", help="The Input File Path", required=True)
parser.add_argument("-e", "--ErrorLocation",
                    help="The error BEL location which triggers the error in Vivado", required=True)


args = parser.parse_args()

inputFile = open(args.Input, 'r')
outputFile = open(args.Output, 'w')
targetBELStr = args.ErrorLocation

lines = inputFile.readlines()

targetLineId = 0
for i, line in enumerate(lines):
    if (line.find(targetBELStr) >= 0):
        targetLineId = i

startPrintOut = False
for line in lines[targetLineId:]:
    if (line.find("set result ") >= 0):
        startPrintOut = True
    if (startPrintOut):
        print(line, file=outputFile, end='')

inputFile.close()
outputFile.close()
