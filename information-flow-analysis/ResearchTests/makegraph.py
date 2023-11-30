#!/usr/bin/python
import sys

f = open(sys.argv[1])

f.readline()
f.readline()
s = []

for line in f.readlines():
  a,b,c = line.strip().split("\t")
  if c[0] != '>': s.append(int(c))
f.close()

s.sort()

f = open(sys.argv[1]+".plot", "w+")

for i in range(len(s)):
  if i== len(s)-1 or s[i] != s[i+1]:
    f.write("%f %f\n" % (s[i]/float(202), (i+1)/float(37)))



