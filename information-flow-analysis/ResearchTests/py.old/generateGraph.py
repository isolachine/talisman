from math import *
# from collections import defaultdict
import sys
import heapq

# TODO:
# add debug bool
# rewrite for better debugging



p1 = 0.001
# p1 = (p2/(1-p2)) ** 3
c1 = -log(p1)
# c2 = -log(p2/(1-p2))
# c2 = -log(p2)
# c3 = -log(1-p2)
# print p1, p2, (1-p2)
# print c1, c2, c3
# c1 = 1
# c2 = 0

# p2 = P( suc | o, E) where o is the trace and E is the set of error nodes.

# using p2 = p( #(e in suc_paths) / #(e in total_paths) )


# f = open("trace/nodes")
f = open(sys.argv[1])
all_nodes = {}
nds={}
for line in f.xreadlines():
    node, detail = line.strip().split(" = ", 2)
    filename, line, col, scope, inlinedAt = detail[1:-1].split("; ", 5)
    all_nodes[int(node)] = (filename, int(line), int(col), scope, inlinedAt)
    nds[int(node)] = {"suc": 0, "fail": 0}

f.close()

# f = open("trace/traces")
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
                failed_traces.add(frozenset(trace))
                # failed_traces.append(trace)
                for n in frozenset(trace):
                    nds[n]["fail"] += 1
            else:
                # succeeded_traces.append(trace)
                succeeded_traces.add(frozenset(trace))
                for n in frozenset(trace):
                    nds[n]["suc"] += 1

for k in nds:
    nds[k]["prop"] = 1.0 * nds[k]["suc"] / (nds[k]["suc"] + nds[k]["fail"]) if (nds[k]["suc"] + nds[k]["fail"]) else 0

# for generating more reasonable p2 value
# using proportion here
def p2(n):
    # f = len(filter(lambda t: n in t, failed_traces))
    # s = len(filter(lambda t: n in t, succeeded_traces))
    # return 1.0 * s / (f+s if (f+s) else 1)
    return nds[n]["prop"]
    # return 0.8

for n in nds:
    if p2(n) and p2(n) < 1:
        c2 = -log(p2(n))
        c3 = -log(1-p2(n))
        print n, all_nodes[n], nds[n]["prop"], c2, len(filter(lambda t: n in t, succeeded_traces)), c3, len(filter(lambda t: n in t, failed_traces))


# Need to keep the failure path..
# General Idea: to get the min-cut of wrong paths.
#               nodes in more correct paths would be less likely wrong.

# Use A* search to find such min-cut sets

# all possible nodes to be in min cut set.
print "succ traces: ", len(succeeded_traces)
print "fail traces: ", len(failed_traces)

possible_nodes = reduce(lambda x,y: set(x) | set(y), failed_traces, set())
# print failed_traces
print reduce(lambda x,y: set(x) & set(y), failed_traces)
heap = []
# Calculate such nodes and add to heap
for n in possible_nodes:
    c2 = p2(n)
    # c3 = -log(1-p2(n))
    c3=0
    g = c1 + c2 * len(filter(lambda t: n in t, succeeded_traces)) + c3 * len(filter(lambda t: n in t, failed_traces))
    # g = c1 + c2 * len(filter(lambda t: n in t, succeeded_traces)) + c3 * len(filter(lambda t: n in t, failed_traces))
    remaining_paths = set(filter(lambda p: n not in p, map(frozenset, failed_traces)))
    h = 0 if not remaining_paths else 1 if reduce(lambda x,y: set(x) & set(y), remaining_paths) else 2
    heapq.heappush(heap, (g+h*c1, {n}, remaining_paths))

done = None
while True:
    h, nodes, paths = heapq.heappop(heap)
    if not paths:
        if done and h > done:
            break
        if not done:
            print "heapvalue    c1           c2           c3           nodes        nodes_details"
        # print h, (h-c1*len(nodes))/c2, nodes, map(lambda x: all_nodes[x], nodes)
        n=next(iter(nodes))
        c2 = -log(p2(n))
        c3 = -log(1-p2(n))
        print '%-12f %-12f %-12f %-12f %-12s %s' % (h, c1, c2, c3, str(nodes), str(map(lambda x: all_nodes[x], nodes)))
        done = h
    else:
        remaining_nodes = reduce(lambda x,y: x|y, map(set, paths))
        for n in remaining_nodes:
            ns = nodes | {n}
            # print c1 * len(ns), c2 * len(filter(lambda t: ns & t, succeeded_traces))
            c2 = -log(p2(n))
            c3 = -log(1-p2(n))
            g = c1 * len(ns) + c2 * len(filter(lambda t: ns & set(t), succeeded_traces)) + c3 * len(filter(lambda t: ns & set(t), failed_traces))
            remaining_paths = set(filter(lambda p: not(ns & p), map(frozenset, failed_traces)))
            h = 0 if not remaining_paths else 1 if reduce(lambda x,y: x & y, remaining_paths) else 2
            heapq.heappush(heap, (g+h*c1, ns, remaining_paths))

# for n in heap:
#     if len(n[1]) == 1 and len(n[2]) == 0:
#         print n
