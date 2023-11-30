#!/usr/bin/python
import sys, os

f = open(sys.argv[1])

f.readline()
f.readline()
s = []
maxi = 0

for line in f.readlines():
  a,b,c,d = line.strip().split("\t")
  maxi = max(int(d), maxi)
  if c[0] != '>': 
    s.append(int(c))
f.close()

s.sort()
fdir, fname = os.path.split(sys.argv[1])

f = open(fdir+os.path.splitext(fname)[0]+".plot", "w+")

for i in range(len(s)):
  if i== len(s)-1 or s[i] != s[i+1]:
    f.write("%f %f\n" % (s[i]/float(maxi), (i+1)/float(len(s))))



