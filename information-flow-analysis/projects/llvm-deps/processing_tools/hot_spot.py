""" Generating the hot nodes from tainted nodes.
    Requires networkx and pygraphviz packages support.
"""
import sys
import re
import time
import networkx
import os
import platform


def main():
    """main function"""

    start_time = total_time = time.time()
    print("-------------------------")
    print("Start hot nodes lookup for %s" % sys.argv[1], flush=True)

    graph = networkx.drawing.nx_pydot.read_dot(sys.argv[1])
    print(f"{(time.time() - start_time):.2f}s : Processing graph", flush=True)
    start_time = time.time()

    # Let's remove unnecessary nodes to reduce the search space
    remove_nodes = []
    for node in graph.nodes:
        if "label" in graph.nodes[node].keys():
            label = graph.nodes[node]["label"]
            if "*SummSource*" in label and graph.in_degree(node) == 0:
                # print(node, graph.nodes[node]["label"], sep=" | ")
                remove_nodes.append(node)
            if "*SummSink*" in label and graph.out_degree(node) == 0:
                # print(node, graph.nodes[node]["label"], sep=" | ")
                remove_nodes.append(node)

    # print(len(graph.nodes))
    for node in remove_nodes:
        graph.remove_node(node)
    # print(len(graph.nodes), "removed: ", len(remove_nodes))

    # for n in graph.nodes:
    #     if "label" in graph.nodes[n].keys():
    #         print(n, graph.nodes[n]["label"])
    #     else:
    #         print(n)

    networkx.drawing.nx_pydot.write_dot(graph, "remove.dot")
    os.system("dot -Tsvg remove.dot -o g.svg")
    if "WSL" in platform.release():
        os.system("wslview g.svg")
    else:
        os.system("open g.svg")

    existing_functions = []

    for node in graph.nodes:
        if "label" in graph.nodes[node].keys():
            label = graph.nodes[node]["label"]
            if "Function" in label and "Arg" in label:
                graph.nodes[node]["function"] = re.search(
                    r"Function::(.*?)&", label
                ).group(1)
                graph.nodes[node]["arg"] = re.search(r"&Arg::(.*?)::", label).group(1)
                existing_functions.append(graph.nodes[node]["function"])

    existing_functions = list(dict.fromkeys(existing_functions))
    existing_functions.sort()
    print("\nAll existing functions\n-------------------------")
    for func in existing_functions:
        print(func)
    print("-------------------------\n")

    print(f"{(time.time() - start_time):.2f}s : Finding Func & Arg nodes", flush=True)
    start_time = time.time()

    candidates = set()

    taint_string = "CONST[private]"
    sink_string = "CONST[public]"
    # taints = graph.successors(taint_string)

    # taint_set = set()
    # for taint in taints:
    #     taint_set.add(taint)

    # for node in graph.nodes:
    #     if "label" in graph.nodes[node].keys():
    #         label = graph.nodes[node]["label"]
    #         if "|key|" in label:
    #             taint_set.add(node)

    # for taint in taint_set:
    #     print(graph.nodes[taint]["label"])

    # print()

    # IMPORTANT[11/21/2023]: The following way of looking for candidates is not
    # working properly. See sensitivity-tests/context_sensitivity.
    # Commenting out for a better appraoch...
    # --------------------------------------------------------------------------
    # # for node in taint_set:
    # successors = networkx.dfs_preorder_nodes(graph, source=taint_string)
    # for succ in successors:
    #     if (
    #         "function" in graph.nodes[succ].keys()
    #         and "*MultipleSource*" not in graph.nodes[succ]["label"]
    #     ):
    #         # if "function" not in graph.nodes[node].keys():
    #         # print("1", graph.nodes[succ]["label"])
    #         candidates.add(succ)
    #         # elif (
    #         #     graph.nodes[node]["function"] !=
    #         #               graph.nodes[succ]["function"]
    #         #     or graph.nodes[node]["arg"] != graph.nodes[succ]["arg"]
    #         # ):
    #         #     # print("2", graph.nodes[succ]["label"])
    #         #     candidates.add(succ)
    # --------------------------------------------------------------------------

    # WARNING: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.dag.ancestors.html#ancestors
    # the ancestor function is only guaranteed to work for DAGs. May be problematic.
    # ancestors = networkx.ancestors(graph, sink_string)
    # print("ancestors: ", ancestors, len(ancestors), len(graph.nodes))
    for node in graph.nodes:
        if "function" in graph.nodes[node].keys():
            candidates.add(node)

    # tmp_candidates = set()
    # for candidate in candidates:
    #     successors = networkx.dfs_preorder_nodes(graph, source=candidate)
    #     if sink_string in successors:
    #         tmp_candidates.add(candidate)
    # candidates = tmp_candidates

    for candidate in candidates:
        print(graph.nodes[candidate]["label"])
    print()

    print(f"{(time.time() - start_time):.2f}s : Finding candidates", flush=True)
    start_time = time.time()

    hotspot = set()

    for candidate in candidates:
        print(graph.nodes[candidate]["label"])
        for pred in graph.predecessors(candidate):
            if pred != taint_string:
                print("  --  ", graph.nodes[pred]["label"])

                # total_in = 0
                # for succ in graph.successors(pred):
                #     total_in += graph.in_degree(succ) - 1
                #     print("  ----  ", graph.nodes[succ]["label"])
                #     for ppred in graph.predecessors(succ):
                #         if "label" in graph.nodes[ppred].keys():
                #             print("  ------  ", graph.nodes[ppred]["label"])
                #         else:
                #             print("  ------  ", ppred)

                # if total_in > 1:
                # if taint_string not in graph.predecessors(pred):
            if graph.in_degree(candidate) > 1:
                hotspot.add(candidate)
                print("********* added *********")
        print("=====================")
    # roots = [n for n, d in graph.in_degree() if d == 0]
    # roots.remove(taint_string)

    # print(f"{(time.time() - start_time):.2f}s : Finding roots", flush=True)
    # start_time = time.time()

    # for root in roots:
    #     next_nodes = networkx.dfs_preorder_nodes(graph, source=root,
    #       depth_limit=1)
    #     for node in next_nodes:
    #         if node in candidates:
    #             roots.remove(root)

    # print(f"{(time.time() - start_time):.2f}s : Trimming roots", flush=True)
    # start_time = time.time()

    # pool = set()
    # hotspot = set()
    # for root in roots:
    #     for node in networkx.dfs_preorder_nodes(graph, source=root):
    #         pool.add(node)

    # print(f"{(time.time() - start_time):.2f}s : Getting the pool",
    #   flush=True)
    # start_time = time.time()

    # for node in pool:
    #     if node in candidates:
    #         hotspot.add(node)

    print(f"{(time.time() - start_time):.2f}s : Finding hotspots")
    print(f"{(time.time() - total_time):.2f}s : Total running time", flush=True)

    functions = []
    print("\nHotspot nodes\n-------------------------")
    for node in hotspot:
        functions.append(graph.nodes[node]["function"])
        print(node, graph.nodes[node]["function"], graph.nodes[node]["arg"], sep=" | ")
    print("-------------------------")

    functions = list(dict.fromkeys(functions))
    functions.sort()

    print("\nUnique hotspot functions\n-------------------------")
    for func in functions:
        print(func)
    print("-------------------------\n")

    print("Total hotspot nodes:             %d" % len(hotspot))
    print("Total existing functions:        %d" % len(existing_functions))
    print("Total unique hotspot functions:  %d" % len(functions), flush=True)


if __name__ == "__main__":
    main()
