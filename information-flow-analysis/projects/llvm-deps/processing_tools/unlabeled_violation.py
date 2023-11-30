""" Generating the hot nodes from tainted nodes.
    Requires networkx and pygraphviz packages support.
"""
import sys
import time
import networkx


def main():
    """main function"""

    start_time = time.time()
    print("Start unlabeled source violation detection for %s" % sys.argv[1], flush=True)

    graph = networkx.drawing.nx_pydot.read_dot(sys.argv[1])
    print(f"{(time.time() - start_time):.2f}s : Reading graph", flush=True)
    start_time = time.time()

    for node in graph.nodes:
        if "label" in graph.nodes[node].keys():
            label = graph.nodes[node]["label"]
            preds = list(graph.predecessors(node))
            if len(preds) == 1:
                if (
                    "label" in graph.nodes[preds[0]].keys()
                    and "*SummSource*" in graph.nodes[preds[0]]["label"]
                    and len(list(graph.predecessors(preds[0]))) == 0
                ):
                    successors = networkx.dfs_preorder_nodes(graph, source=node)
                    for succ in successors:
                        if "CONST[" in succ:
                            print(label)
                            print("\t--", succ)
            elif len(preds) == 0 and "*SummSource*" not in graph.nodes[node]["label"]:
                successors = networkx.dfs_preorder_nodes(graph, source=node)
                for succ in successors:
                    if "CONST[" in succ:
                        print(label)
                        print("\t--", succ)


if __name__ == "__main__":
    main()
