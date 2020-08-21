adcValueTable = [
    [509,  532,  556,  579,  602],
    [1470, 1535, 1600, 1670, 1735],
    [1581, 1634, 1661, 1681, 1694],
    [1622, 1820, 1902, 1960, 2000]]

for i in range(0, 4):
    for j in range(0, 5):
        adcValueTable[i][j] = int(adcValueTable[i][j] * 748 / 499)
        #print(adcValueTable[i][j])

for i in range(0, 4):
    print(adcValueTable[i])

