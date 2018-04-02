#!/usr/bin/python3

import datetime
import pandas as pd
import os
import sys
from numpy import random
import matplotlib.pyplot as plt
from IPython.display import display

#Location = r'test.txt'
associative = str(sys.argv[1])
direct = str(sys.argv[2])

df1 = pd.read_csv(associative, header = None, names=['associative'])
df2 = pd.read_csv(direct, header = None, names=['direct'])
df1.info()
df2.info()
#display(df1.head())
#display(df1.tail())

rwindow = 20000
result = pd.concat([df1, df2], axis=1)

plt.title('L1 DCACHE timeline')
plt.xlabel('Read/Write memory access')
plt.ylabel('Time (ns)')
color_dict = {'associative': '#3498DB', 'direct': '#F39C12'}
date = datetime.datetime.now()

result.rolling(window=rwindow,center=False).mean().plot(stacked=False, color=[color_dict.get(x, '#333333') for x in result.columns])
plt.savefig(date.strftime('%d-%m-%Y_%H-%M-%S')+'-line_fetch_ns_rwindow='+str(rwindow)+'.png')
#plt.show()
plt.clf()

result.rolling(window=rwindow,center=False).mean().plot(kind='area', stacked=False, color=[color_dict.get(x, '#333333') for x in result.columns])
plt.savefig(date.strftime('%d-%m-%Y_%H-%M-%S')+'-area_fetch_ns_rwindow='+str(rwindow)+'.png')
#plt.show()
