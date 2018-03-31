#!/usr/bin/python3

import pandas as pd
import os
import sys
from numpy import random
import matplotlib.pyplot as plt
from IPython.display import display

#Location = r'test.txt'
Location = str(sys.argv[1])
df1 = pd.read_csv(Location, header = None, names=['ns'])
df1.info()

#display(df1.head())
#display(df1.tail())

rwindow = 20000
df1.rolling(window=rwindow,center=False).mean().plot()

plt.title('L1 DCACHE timeline')
plt.xlabel('Read/Write memory access')
plt.ylabel('Time (ns)')
plt.savefig('fetch_ns_rwindow='+str(rwindow)+'.pdf')
plt.show()
