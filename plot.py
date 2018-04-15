#!/usr/bin/python3

import datetime
import pandas as pd
import os
import sys
from numpy import random
import matplotlib.pyplot as plt
from IPython.display import display

#file = r'test.txt'
file0 = str(sys.argv[1])
file1 = str(sys.argv[2])
file2 = str(sys.argv[3])
# file3 = str(sys.argv[4])

colname1 = '16 byte'
colname2 = '64 byte'
colname3 = '1024 byte'

colnames=[colname1, colname2, colname3]
df0 = pd.read_csv(file0, header=None, names=[colname1])
df0.info()
df1 = pd.read_csv(file1, header=None, names=[colname2])
df1.info()
df2 = pd.read_csv(file2, header=None, names=[colname3])
# df3 = pd.read_csv(file3, header=None, names=[colname4])
# df3.info()

#merge some data frames
#result = pd.concat([df0, df1], axis=1)
result = pd.concat([df0, df1, df2], axis=1)
# result = pd.concat([df0, df1, df2, df3], axis=1)

display(result.head())
# display(result.tail())

#file output string
tracefile = file0.split("#")[2]
print(tracefile)

rwindow = 20000

color_dict = {colname1: '#2ECC71', colname2: '#F39C12', colname3: '#3498DB'}
date = datetime.datetime.now()

#------------ mem trace output #------------
#------------ Choose title and stuff #------------

result.rolling(window=rwindow,center=False).mean().plot(stacked=False, color=[color_dict.get(x, '#333333') for x in result.columns])
#result.rolling(window=rwindow,center=False).mean().plot.hist(stacked=False, color=[color_dict.get(x, '#333333') for x in result.columns])


plt.title('32kb L1 cache memory access latency on tar_unzip')
plt.xlabel('Number of accesses')
plt.ylabel('Latency in nanoseconds')
plt.xticks(rotation=20)
plt.tight_layout()

plt.savefig(date.strftime('%d-%m-%Y_%H-%M-%S')+'_'+tracefile+'-line_fetch_ns_rwindow='+str(rwindow)+'.png')
plt.show()
