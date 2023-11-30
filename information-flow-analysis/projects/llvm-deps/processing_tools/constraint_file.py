"""
    Generate information flow graph in .dot format from constraint file.
"""
import re
import sys


def main():
    """Main function for generation of graph"""

    encoding = "UTF-8"

    if len(sys.argv) <= 1:
        print("Usage: constraint_graph.py [dat_file]")
        print("\t [dat_file]: The *.dat file for use")

    else:
        with open("./constraints-explicit.con", "w", encoding=encoding) as explicit:
            with open("./constraints-implicit.con", "w", encoding=encoding) as implicit:
                with open(sys.argv[1], "r", encoding=encoding) as dat:
                    exp_write = False
                    imp_write = False
                    for line in dat:
                        if re.match(r"^default:$|^default-sink:$|^source-sink:$", line):
                            exp_write = True
                            imp_write = True
                            continue
                        if re.match(r"^implicit:$", line):
                            exp_write = False
                            imp_write = True
                            continue
                        if "<:" in line and bool(exp_write):
                            explicit.write(line)
                            implicit.write(line)
                        if "<:" in line and not bool(exp_write) and bool(imp_write):
                            implicit.write(line)
                    dat.close()
                implicit.close()
            explicit.close()


if __name__ == "__main__":
    main()
