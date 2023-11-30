""" Generating the shortest path from start node to end node.
    Requires the networkx package and graphviz support for python.
"""

import re
import sys
import networkx
from lattice import RLLabel, RLLattice


def print_path(graph, start, end):
    """Print a path from the starting node to the end node in graph."""
    print(start, " -> ", end)
    print("-----------")
    path = networkx.shortest_path(G=graph, source=start, target=end)
    for vertices in path:
        if "label" in graph.nodes[vertices].keys():
            print(vertices, graph.nodes[vertices]["label"])
        else:
            print(vertices)
    print("\n")


def main():
    """main function"""

    graph = networkx.drawing.nx_pydot.read_dot(sys.argv[1])
    lattice = RLLattice(sys.argv[2])

    source_set = []
    sink_set = []
    for node in graph.nodes:
        if "CONST[" in node:
            source_set.append(node)
            sink_set.append(node)

    for start in source_set:
        for end in sink_set:
            if networkx.has_path(graph, source=start,
                                 target=end) and start != end:
                left = re.sub(r"(\[(Src|Snk)Idx:\d+\])", "", start)
                right = re.sub(r"(\[(Src|Snk)Idx:\d+\])", "", end)
                left = RLLabel(left)
                right = RLLabel(right)
                if not lattice.check_sub(left, right):
                    print_path(graph, start, end)

    # for e in sys.argv:
    #     print(e)
    # if len(sys.argv) != 3:
    #     sys.exit("Error: Provide start and end node addresses.")
    # print("===========")

    # START = sys.argv[1]
    # END = sys.argv[2]

    # G = networkx.drawing.nx_pydot.read_dot("./tmp.dot")
    # start = START
    # end = ""
    # edges = []
    # print(start)
    # print("-----------")
    # for path in networkx.shortest_path(G, source=START, target=END):
    #     end = path
    #     if start == end:
    #         continue
    #     # edges.append(start + "&#45;&gt;" + end)
    #     print(end)
    #     start = end

    # # print(edges)

    # print("-----------")
    # print(end)
    # with open("./tmp.svg", "w") as tmp:
    #     with open("./g.svg", "r+") as svg:
    #         replace = False
    #         count = 0
    #         for line in svg:
    #             if not replace:
    #                 tmp.write(line)
    #                 for e in edges:
    #                     if e in line:
    #                         replace = True
    #                         print(line)
    #             else:
    #                 count = count + 1
    #                 tmp.write(line.replace('"black"', '"red"'))
    #                 if count == 4:
    #                     count = 0
    #                     replace = False
    #     svg.close()
    # tmp.close()


if __name__ == "__main__":
    main()
