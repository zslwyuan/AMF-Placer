import argparse


parser = argparse.ArgumentParser()

parser.add_argument(
    "-o", "--Output", help="The Output File Path", required=True)
parser.add_argument(
    "-i", "--Input", help="The Input File Path", required=True)

args = parser.parse_args()

inputFile = open(args.Input, 'r')
outputFile = open(args.Output, 'w')

lines = inputFile.readlines()
cellList = []
targetLineId = 0
for i, line in enumerate(lines):
    if (line.find(" with block Id: ") >= 0):
        cellList.append(line.split(" with block Id: ")[0])

print("unplace_cell {"+' '.join(cellList) +
      "}\nplace_design\nroute_design", file=outputFile, end='')

inputFile.close()
outputFile.close()
