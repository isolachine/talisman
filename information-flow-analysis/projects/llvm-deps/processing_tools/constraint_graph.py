"""
    Generate information flow graph in .dot format from constraint file.
"""
import re
import sys
import os
import platform

CONST_STR = "CONST"
# HOTSPOT_SINK = ["*Constant*", "*SummSource*", "*SummSink*"]
# HOTSPOT_SOURCE = ["*Constant*", "*SummSink*"]
HOTSPOT_MATCH = ["*Constant*"]


class CE:
    """Constraint Element class"""

    def __init__(self, addr="", label=""):
        self.addr = addr
        label = re.sub(r"\|\|+", "|", label)
        label = re.sub(r"\|$", "", label)
        label = re.sub(r"(\w+\.\w+)\|(\d+)", r"\1, \2", label)
        self.label = label

    def __hash__(self):
        return hash((self.addr, self.label))

    def __eq__(self, other):
        if not isinstance(other, type(self)):
            return NotImplemented
        return self.addr == other.addr and self.label == other.label

    def full(self):
        """Full string form of a constraint element"""
        if CONST_STR in self.addr:
            return self.addr + "\n"
        return (
            self.addr
            + ' [shape=record,shape=Mrecord,label="{'
            + self.addr
            + "|"
            + self.label
            + '}"]\n'
        )


def main():
    """Main function for generation of graph"""

    encoding = "UTF-8"

    if len(sys.argv) <= 2:
        print("Usage: constraint_graph.py [con_file] [out_dot] -op [op]")
        print("\t [con_file]: The *.con file for use")
        print("\t [out_dot]: The *.dot file for output")
        print("\t -graph [0-1]: specifying if generating graph, defaults to 0")
        print("\t\t\t0: just dot file")
        print("\t\t\t1: also generating svg image")

    else:
        gen_graph = 0
        hotspot = 0
        for i in range(3, len(sys.argv)):
            if sys.argv[i] == "-graph":
                gen_graph = int(sys.argv[i + 1])
            if sys.argv[i] == "-hotspot":
                hotspot = int(sys.argv[i + 1])

        with open(sys.argv[2], "w", encoding=encoding) as tmp_file:
            tmp_file.write("digraph {\n")
            with open(sys.argv[1], "r", encoding=encoding) as file:
                ce_set = set()
                con_set = set()
                for line in file:
                    line = re.split(r" +<: +| +;+", line)
                    if line[0].startswith(CONST_STR):
                        try:
                            found = re.search(r"(\[SrcIdx:\d+\])", line[2]).group(1)
                        except AttributeError:
                            found = ""
                        ce_str = re.split(r"\[|\]", line[1])
                        ce1 = CE('"' + line[0] + found + '"')
                        if hotspot:
                            ce1 = CE('"' + "CONST[private]" + '"')
                        ce2 = CE(ce_str[0], ce_str[1])
                    elif line[1].startswith(CONST_STR):
                        try:
                            found = re.search(r"(\[SnkIdx:\d+\])", line[2]).group(1)
                        except AttributeError:
                            found = ""
                        ce_str = re.split(r"\[|\]", line[0])
                        ce1 = CE(ce_str[0], ce_str[1])
                        ce2 = CE('"' + line[1] + found + '"')
                        if hotspot:
                            ce2 = CE('"' + "CONST[public]" + '"')
                    else:
                        ce_str = re.split(r"\[|\]", line[0])
                        ce1 = CE(ce_str[0], ce_str[1])
                        ce_str = re.split(r"\[|\]", line[1])
                        ce2 = CE(ce_str[0], ce_str[1])

                    if (
                        hotspot
                        and not any(sub in ce1.label for sub in HOTSPOT_MATCH)
                        and not any(sub in ce2.label for sub in HOTSPOT_MATCH)
                    ) or not hotspot:
                        ce_set.add(ce1)
                        ce_set.add(ce2)
                        con_set.add(ce1.addr + " -> " + ce2.addr + "\n")
                for ele in ce_set:
                    tmp_file.write("\t" + ele.full())
                for con in con_set:
                    tmp_file.write("\t" + con)

                file.close()
            tmp_file.write("}")
            tmp_file.close()

        if gen_graph == 1:
            os.system("dot -Tsvg tmp.dot -o g.svg")
            if "WSL" in platform.release():
                os.system("wslview g.svg")
            else:
                os.system("open g.svg")


if __name__ == "__main__":
    main()
