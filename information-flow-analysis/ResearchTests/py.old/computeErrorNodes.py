from math import *
import sys
import heapq


if len(sys.argv) < 4:
    print "USAGE: python ./calculateErrorNodes.py <nodes> <traces> <errorInfo> <version>"
    print "\t<nodes>\t filepath that stores nodes"
    print "\t<traces>\t filepath that stores traces"
    print "\t<errorInfo>\t filepath that stores error Info, in a form Dict[version: line]"
    print "\t<version>\t number 1 - 41, which generates the corresponding nodes & traces"
    exit

###########################################################################
# p2 Algorithms
#
# Definition
# p2 = P(suc | o, E) where o is the trace and E is the set of error nodes.
#
# 0. p2 is uniform for all nodes. # [may not be correct]
# 1. p2(n) = p( #(e in suc_paths) / #(e in total_paths) ) to estimate
# 2. p2(n) = avg(f(tr, n))
# 3. p2(n) = product(f(tr, n))
###########################################################################


###########################################################################
# Flags and Constants
###########################################################################
debug = True
x1,x2,x3 = 1,1,1
p1 = 1e-20
c1 = x1 and -log(p1)
p3dep = True # whether p3 = 1-p2
tol = 200 # tolerance of the rank of true error in the heap
###########################################################################


###########################################################################
# File reading
###########################################################################

# Read in the nodes
f = open(sys.argv[1])
all_nodes = {}
nds={}
for line in f.xreadlines():
    node, detail = line.strip().split(" = ", 2)
    filename, line, col, scope, inlinedAt = detail[1:-1].split("; ", 5)
    all_nodes[int(node)] = (filename, int(line), int(col), scope, inlinedAt)
    nds[int(node)] = {"suc": 0, "fail": 0}
f.close()

# Read in the traces
f = open(sys.argv[2])
failed_traces = set()
succeeded_traces = set()
for line in f.xreadlines():
    if line.strip():
        if line[0] != ",":
            failed = eval(line)
        else:
            trace = eval(line[1:])
            if failed:
                failed_traces.add(trace)
                for n in frozenset(trace):
                    nds[n]["fail"] += 1
            else:
                succeeded_traces.add(trace)
                for n in frozenset(trace):
                    nds[n]["suc"] += 1
f.close()

# Read in the dictionary (which stores error info (version: line))
f = open(sys.argv[3])
errorInfo = eval(f.read())
f.close()
failsAt = errorInfo[int(sys.argv[4])]

###########################################################################
# Calculate the probability (frequency) of each node that a succeeded trace
# passing through it.
###########################################################################
for k in nds:
    nds[k]["freq"] = float(nds[k]["suc"]) / (nds[k]["suc"] + nds[k]["fail"]) \
        if (nds[k]["suc"] + nds[k]["fail"]) else 0

###########################################################################
# function to generating more reasonable p2 value
# return something between 0 and 1
###########################################################################
def p2(n):
    if p2method == 0:
        ret = p2value
    elif p2method == 1:
        ret = nds[n]["freq"]
    elif p2method == 2:
        if nds[n].get("len2tail", None) is None:
            l = [f(tr, n) for tr in succeeded_traces if n in tr]
            # print n, l
            nds[n]["len2tail"] = sum(l)/float(len(l)+1) # use the average for now
        ret = nds[n]["len2tail"]
    elif p2method == 3:
        pass
    # print ret
    return ret

def avg(l):
    return sum(l)/len(l) if l else 0

def f(tr, n):
    if isinstance(n, set):
        return 1.0 - avg([tr.index(node) for node in n if node in tr])/float(len(tr)) if n & set(tr) else 1.0
    else:
        # return 0.25 + tr[::-1].index(n)/float(2*len(tr)) if n in tr else 1.0
        return 0.75 - tr.index(n)/float(2*len(tr)) if n in tr else 1.0

###########################################################################

possible_nodes = reduce(lambda x,y: x | y, map(set, failed_traces), set())
heap = []
# Calculate such nodes and add to heap
for n in possible_nodes:
    p2 = reduce(lambda x,y: x*y, map(lambda tr: f(tr,n), succeeded_traces), 1.0)
    p3 = reduce(lambda x,y: x*y, map(lambda tr: 1-f(tr,n), failed_traces), 1.0)
    c2 = x2 and -log(p2 + 1e-20)
    c3 = x3 and -log(p3 + 1e-20) # added a small number to avoid domain error
    g = c1 + c2 + c3
    remaining_paths = set(filter(lambda p: n not in p, map(frozenset, failed_traces)))
    h = 0 if not remaining_paths else 1 if reduce(lambda x,y: x & y, remaining_paths) else 2
    heapq.heappush(heap, (g + h*c1, {n}, remaining_paths, c2, c3))

first = True
for count in range(1, tol+1):
    h, nodes, paths, c2, c3 = heapq.heappop(heap)
    if not paths:
        if first:
            first = None
            if debug:
                print "heapvalue    c1           c2           c3           nodes        nodes_details"
        if debug:
            print '%-12f %-12f %-12f %-12f %-12s %s' % (h, c1, c2, c3, nodes, str(map(lambda x: all_nodes[x], nodes)))
        foundAt = set(map(lambda x: all_nodes[x][1], nodes))
        if foundAt & set(failsAt):
            break

    # the else part should not be used for now... (all errors should be size one)
    else:
        remaining_nodes = reduce(lambda x,y: x|y, map(set, paths))
        for n in remaining_nodes:
            ns = nodes | {n}
            c2 = x2 and -log(reduce(lambda x,y: x*y, map(lambda tr: f(tr, ns), succeeded_traces), 1.0))
            c3 = x3 and -log(reduce(lambda x,y: x*y, map(lambda tr: 1-f(tr,ns), failed_traces), 1.0) + 1e-20)
            g = c1 * len(ns) + c2 + c3
            remaining_paths = set(filter(lambda p: not(ns & p), map(frozenset, failed_traces)))
            h = 0 if not remaining_paths else 1 if reduce(lambda x,y: x & y, remaining_paths) else 2
            heapq.heappush(heap, (g+h*c1, ns, remaining_paths, c2, c3))

##########################################################################################
# Print Debug info
if debug:
    print "Failed Traces:\n", reduce(lambda x,y: x & y, map(set, failed_traces))
    print "succ trace patterns: ", len(succeeded_traces)
    print "fail trace patterns: ", len(failed_traces)
    for n in nds:
        p2 = reduce(lambda x,y: x*y, map(lambda tr: f(tr,n), succeeded_traces), 1.0)
        p3 = reduce(lambda x,y: x*y, map(lambda tr: 1-f(tr,n), failed_traces), 1.0)
        c2 = x2 and -log(p2 + 1e-20)
        c3 = x3 and -log(p3 + 1e-20)
        print n, all_nodes[n], p2, p3, c2, c3
##########################################################################################

with open("/tmp/log", "a") as file:
    if foundAt & set(failsAt):
        print "The true error is found at rank %d" % count
        file.write("%s\t%s\t%d\n" % (sys.argv[4], failsAt, count))
    else:
        print "The true error falls out of rank %d, which should be in line: %s" % (tol, failsAt)
        file.write("%s\t%s\t>%d\n" % (sys.argv[4], failsAt, tol))
