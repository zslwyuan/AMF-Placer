import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from io import StringIO

s = StringIO("""     HPWL\\time
cfg0\\912029 \\335.512
cfg1\\1.27426e+06 \\ 615.727
cfg2\\962136  \\374.442
cfg3\\1152483 \\754.724
cfg4\\1454848\\ 351.247
cfg5\\2846114 \\913.073
cfg6\\2121689\\ 651.014
cfg7\\1271695 \\1171.372
cfg8\\4690386 \\ 1535.283
.\\0 \\0
cfg0\\551076\\ 133.908
cfg1\\709485 \\ 295.330
cfg2\\573970 \\151.544
cfg3\\736784 \\511.956
cfg4\\842922\\ 138.335
cfg5\\1.75518e+06 \\392.619
cfg6\\1.32177e+06 \\185.504
cfg7\\691540 \\354.45
cfg8\\2.45886e+06 \\598.132
.\\0 \\0
cfg0\\1138380 \\350.290
cfg1\\ 8023992 \\ 710.040
cfg2\\1293786\\ 407.616
cfg3\\1147925\\ 547.088
cfg4\\1480986 \\383.991
cfg5\\2697352 \\1134.162
cfg6\\1448042 \\553.062
cfg7\\1173574 \\614.172
cfg8\\2301880 \\2741.470
.\\0 \\0
cfg0\\272778 \\82.668
cfg1\\337984 \\ 140.5
cfg2\\329617 \\142.81
cfg3\\348965 \\244.906
cfg4\\403503 \\70.326
cfg5\\551929 \\76.041
cfg6\\554137\\ 81.824
cfg7\\390822 \\314.727
cfg8\\525427 \\201.3
.\\0 \\0
cfg0\\394481 \\62.483
cfg1\\459377 \\ 97.684
cfg2\\430806 \\79.728
cfg3\\477222\\ 83.866
cfg4\\417996 \\57.784
cfg5\\511602 \\84.041
cfg6\\1.53514e+06 \\201.975
cfg7\\500033 \\147.599
cfg8\\1.42135e+06 \\194.503""")

df = pd.read_csv(s, index_col=0, delimiter='\\\\', skipinitialspace=True)
print(df)
fig = plt.figure() # Create matplotlib figure

ax = fig.add_subplot(111) # Create matplotlib axes
ax2 = ax.twinx() # Create another axes that shares the same x-axis as ax.

width = 0.4
df['HPWL'][0:10] /= 912029
df['time'][0:10] /= 335.512

df['HPWL'][10:20] /= 551076
df['time'][10:20] /= 133.908

df['HPWL'][20:30] /= 1138380
df['time'][20:30] /= 350.290

df['HPWL'][30:40] /= 272778
df['time'][30:40] /= 70.326

df['HPWL'][40:50] /= 394481
df['time'][40:50] /= 57.784
# df['HPWL'][0:10] /= 4690386
# df['time'][0:10] /= 1535.283

# df['HPWL'][10:20] /= 2.45886e+06
# df['time'][10:20] /= 598.132

# df['HPWL'][20:30] /= 8023992
# df['time'][20:30] /= 2741.470

# df['HPWL'][30:40] /= 554137
# df['time'][30:40] /= 314.727

# df['HPWL'][40:50] /= 1.53514e+06
# df['time'][40:50] /= 201.975
print(df)
df.HPWL.plot(kind='bar', color='red', ax=ax, width=width, position=1)
df.time.plot(kind='bar', color='blue', ax=ax2, width=width, position=0)

ax.set_ylabel('HPWL (normalized)')
ax2.set_ylabel('Placement Runtime (normalized)')
ax.legend( loc = 'upper left')
ax2.legend( loc = 'upper right')

plt.show()