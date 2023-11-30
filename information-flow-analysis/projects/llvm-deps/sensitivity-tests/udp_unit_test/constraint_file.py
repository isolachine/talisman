"""
    Generate information flow graph in .dot format from constraint file.
"""
import sys
import re

TAG = "\\[(SCG|UB|TB)\\]\\|.*"


def main():
    """Main function for generation of graph"""

    if len(sys.argv) <= 1:
        print("Usage: constraint_graph.py [dat_file]")
        print("\t [dat_file]: The *.dat file for use")

    else:
        pattern = re.compile(TAG)
        with open("./call-stack", "w") as stack:
            with open(sys.argv[1], "r") as dat:
                for line in dat:
                    if pattern.match(line):
                        stack.write(line)
                dat.close()
        stack.close()


if __name__ == "__main__":
    main()
