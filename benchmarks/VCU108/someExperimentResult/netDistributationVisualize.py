import matplotlib.pyplot as plt

tmpFile = open("netNumDisribution_faceDetection", 'r')

x = []
totalNum = 0
for line in tmpFile.readlines():
    if (int(line.split(" ")[0])<100 and int(line.split(" ")[0])>=30):
        x.append(int(line.split(" ")[0]))

    totalNum += 1

print("totalNum: ",totalNum)
print("macroNum: ", len(x))
print("ratio=", len(x)/totalNum)
plt.hist(x,bins=100)
plt.show()
