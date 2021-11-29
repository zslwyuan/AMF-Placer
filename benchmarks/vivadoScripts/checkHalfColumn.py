inputFile = open("clockInfoCheck", 'r')

lines = inputFile.readlines()

i = 0

clockInHalfColumns = [set() for i in range(0, 40)]
clockNum = 0
while (i < len(lines)):
    if (lines[i].find("=============") >= 0):
        curLine = lines[i].replace("=", "").replace("\n", "").split(" ")
        rowY = int(curLine[2])
        clockNum = int(curLine[4])

        for j in range(0, clockNum):
            halfColId = rowY//15
            clockInHalfColumns[halfColId].add(lines[i+1+j])
            if (len(clockInHalfColumns[halfColId]) > 12):
                print("halfColumn ", halfColId, " exceeed limit with ",
                      len(clockInHalfColumns[halfColId]), "clocks in it")
                for clockName in clockInHalfColumns[halfColId]:
                    print(clockName)

    i += clockNum+1
