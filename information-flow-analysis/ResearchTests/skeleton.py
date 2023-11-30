import sys
import heapq
import os
from math import *

import argparse

BASE = ('node', 'edge')
METHOD = ('const', 'freq', 'loc')


parser = argparse.ArgumentParser(description='Trace Analysis')

parser.add_argument('-d', '--debug', dest='DEBUG', action='store_true',
                    default=False, help='print Debug Info (default: False)')
parser.add_argument('-x2', dest='x2', action='store_false',
                    default=True, help='set this flag to ignore p2 (succeeded traces)')
parser.add_argument('-x3', dest='x3', action='store_false',
                    default=True, help='set this flag to ignore p3 (failed traces)')

parser.add_argument('f_nodes', metavar='<nodes>', type=argparse.FileType('r'),
                    help='filepath to nodes')
parser.add_argument('f_traces', metavar='<traces>', type=argparse.FileType('r'),
                    help='filepath to traces')
parser.add_argument('f_error', metavar='<error>', type=argparse.FileType('r'),
                    help='filepath to error info')
parser.add_argument('ver', metavar='<version>', type=int, 
                    help='version number')

parser.add_argument('-b', '--base', required=True, choices=BASE,
                    help='[Required] analysis based on node or edge')
parser.add_argument('-m', '--method', required=True, choices=METHOD,
                    help='[Required] method to calculate p2 (and p3)')

args = parser.parse_args()

# print args

DEBUG, x2, x3 = args.DEBUG, args.x2, args.x3
m = METHOD.index(args.method)

p2 = [
  (lambda n: 0.2),		# const
  (lambda n: (nds[n][3]+1e-10)/(1+2e-10)),  # freq (P2 = fail%)
  (lambda n: (nds[n][4]+1e-10)/(1+2e-10)),  # freq (P2 = succ%)
  (lambda n: nds[n][-1]+1e-20)  # loc
][m]

def node_at_line(n):
    if isinstance(n, tuple): n = n[0]
    return all_nodes[n][1]

def node_info(n):
    if isinstance(n, tuple): n = n[0]
    return all_nodes[n]

f_out = open(os.path.dirname(os.path.dirname(args.f_nodes.name))+"/result_log/"+args.base+"_"+args.method + ("" if x2 else "_x2") + ("" if x3 else "_x3") + ".log", 
             "w" if args.ver == 1 else "a")
if args.ver == 1:
    f_out.write("Calculate likelyhood with base = %s\tmethod = %s\n" %(args.base, args.method)+
			"Test\tFailsAt\tRank\n")
#################################################################################################


# Read in the nodes
all_nodes = {}
nds={}
for line in args.f_nodes.xreadlines():
    node, detail = line.strip().split(" = ", 2)
    filename, line, col, scope, inlinedAt = detail[1:-1].split("; ", 5)
    all_nodes[int(node)] = (filename, eval(line), eval(col), scope, inlinedAt)
    if DEBUG: print all_nodes[int(node)]
args.f_nodes.close()


# Read in error info
errorInfo = eval(args.f_error.read())
args.f_error.close()
failsAt = errorInfo[int(args.ver)]
if failsAt[0] == -1:
    print "Error: MISSING LINE %s" % str(failsAt[1:])
    f_out.write("%s\t%s\t>%d\t%d\n" % (args.ver, failsAt,len(all_nodes),len(all_nodes)))
    exit()


# Read in the simplified trace info

failed_traces = set()
nds = {}
# succeeded_traces = set()
first = True
for line in args.f_traces.xreadlines():
#    print line
    if first:
        nds = eval(line)
        first = False
    elif line.strip():
        failed_traces.add(eval(line))
args.f_traces.close()



tol = len(all_nodes)
###############################################################################################

for k in nds:
    nds[k].append(nds[k][0] + nds[k][1])                            # nds[k][2] = total #occurs
    nds[k].append(float(nds[k][1]) / nds[k][2] if nds[k][2] else 0) # nds[k][3] = fail%
    nds[k].append(float(nds[k][0]) / nds[k][2] if nds[k][2] else 0) # nds[k][4] = succ%

###############################################################################################

possible_nodes = reduce(lambda x,y: set(x).union(y), failed_traces, set())
heap = []


for n in possible_nodes:
#    print p2(n)
    c2 = x2 and -log(p2(n))
    c3 = x3 and -log(1-p2(n))
#    nds[n]["kE"] = len(filter(lambda t: n in t, succeeded_traces))
#    nds[n]["fE"] = len(filter(lambda t: n in t, failed_traces))
#    g = c2 * nds[n]["kE"] + c3 * nds[n]["fE"]
    g = c2 * nds[n][0] + c3 * nds[n][1]
    remaining_paths = filter(lambda p: n not in p, failed_traces)
    h = 0 if not remaining_paths else 1 if reduce(lambda x,y: set(x).intersection(y), remaining_paths) else 2
    heapq.heappush(heap, ((h, g), {n}, remaining_paths, c2, c3))


first = True
foundAt = set()
for count in range(1, tol+1):
    if not heap: break
    h, nodes, paths, c2, c3 = heapq.heappop(heap)
    if not paths:
        if DEBUG:
            if first:
                first = None
                print "heapvalue       c2           c3           nodes        nodes_details"
            print '(%1d, %10f) %-12f %-12f %-12s %s' % (h[0], h[1], c2, c3, nodes, str(map(node_info, nodes)))
	if isinstance(node_at_line(next(iter(nodes))), tuple):
            foundAt = reduce(lambda x,y: x|y, map(lambda x: set(range(x[0],x[1]+1)), map(node_at_line, nodes)))
        else:
            foundAt = set(map(node_at_line, nodes))
        if foundAt & set(failsAt):
            break

"""
    # the else part should not be used for now... (all errors should be size one)
    else:
        remaining_nodes = reduce(lambda x,y: x|y, map(set, paths))
        for n in remaining_nodes:
            ns = nodes | {n}
            c2 = x2 and -log(p2(n))
            c3 = x3 and -log(1-p2(n))
            g = c1 * len(ns) + c2 * len(filter(lambda t: ns & set(t), succeeded_traces)) \
                + c3 * len(filter(lambda t: ns & set(t), failed_traces))
            remaining_paths = filter(lambda t: not set(ns).intersect(t), failed_traces)
            h = 0 if not remaining_paths else 1 if reduce(lambda x,y: x & y, remaining_paths) else 2
            heapq.heappush(heap, (g+h*c1, ns, remaining_paths))
"""

if foundAt & set(failsAt):
    print "The true error (line: %s) is found at rank %d" % (failsAt, count)
    f_out.write("%s\t%s\t%d\t%d\n" % (args.ver, failsAt, count, len(all_nodes)))
else:
    print "The true error (line: %s) falls out of rank %d" % (failsAt, tol)
    f_out.write("%s\t%s\t>%d\t%d\n" % (args.ver, failsAt, tol, len(all_nodes)))


if DEBUG:
#    print "Failed Traces:\n", 
#    for t in failed_traces: print t
#    print "succ trace patterns: ", len(succeeded_traces)
    print "fail trace patterns: ", len(failed_traces)
#    print nds
#    import time
#    time.sleep(1)

