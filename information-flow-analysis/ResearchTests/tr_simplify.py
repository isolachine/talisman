import sys
import heapq
import os
from math import *
import argparse


parser = argparse.ArgumentParser(description='Trace Simplification')

"""
parser.add_argument('f_nodes', metavar='<nodes>', type=argparse.FileType('r'),
                    help='filepath to nodes')
parser.add_argument('f_error', metavar='<error>', type=argparse.FileType('r'),
                    help='filepath to error info')
parser.add_argument('ver', metavar='<version>', type=int, 
                    help='version number')
"""

parser.add_argument('f_traces', metavar='<traces>', type=argparse.FileType('r'),
                    help='filepath to traces')

args = parser.parse_args()

#################################################################################################

# Read in the traces
# [fail/success]_[trace]_[node/pair]
f_tr_s = set()
f_tr_p = set()
s_tr_s = set()
s_tr_p = set()
nds_s = {}
nds_p = {}

c = 0
for line in args.f_traces.xreadlines():
    if line.strip():
        if line[0] != ",":
            failed = int(bool(eval(line))) # 0 == succ, 1 == fail
        else:
            tr_s = eval(line[1:])
            l = len(tr_s)
            tr_p = tuple((tr_s[i], -1 if i == l-1 else tr_s[i+1]) for i in range(l))
            if failed:
                f_tr_s.add(tr_s)
                f_tr_p.add(tr_p)
            else:
                s_tr_s.add(tr_s)
                s_tr_p.add(tr_p)

            for n in frozenset(tr_s):
                if not nds_s.get(n, False): nds_s[n] = [0,0]
                nds_s[n][failed] += 1
            for n in frozenset(tr_p):
                if not nds_p.get(n, False): nds_p[n] = [0,0]
                nds_p[n][failed] += 1

args.f_traces.close()

#======================================================================================#
# Output the simplified traces (in same dir)
# Two files for single nodes and paired nodes info
# 	first line is a dictionary of node info, 
# 	each following line is set of nodes -- failed trace

with open(args.f_traces.name+".s", "w") as f:
    f.write(str(nds_s))
    f.write("\n")
    for tr in f_tr_s:
        f.write(str(frozenset(tr)))
        f.write("\n")

with open(args.f_traces.name+".p", "w") as f:
    f.write(str(nds_p))
    f.write("\n")
    for tr in f_tr_p:
        f.write(str(frozenset(tr)))
        f.write("\n")


