#!/usr/bin/env python
""" This file parses output from taint/vulnerable branch analysis

After discovering positives from the analysis it can be helpful to view the
results side by side with the source. This code runes through and gets the
source for each reported line.


Usage:
post_analysis.py <result_file>

Sample Usage:
post_analysis.py tmp.dat > positives_with_source.txt
"""
import re
import sys

from collections import defaultdict


def parse_number(num):
    try:
        return int(num)
    except ValueError:
        return float(num)


def filter_lines_from_results(fname):
    """Filters through vulnerable branch dat file for tainted lines"""
    match = "[.a-zA-Z0-9/_-]+.(c|h|cpp|hpp) line [0-9]+"
    match2 = "[.a-zA-Z0-9/_-]+.(c|h|cpp|hpp) at [0-9]+"
    filtered_lines = []
    filtered_lines2 = []
    with open(fname, "r") as f:
        for line in f:
            if re.match(match, line):
                filtered_lines.append(line)
            if re.match(match2, line):
                filtered_lines2.append(line)

    return [filtered_lines, filtered_lines2]


def get_lines_from_source(positives, positives2):
    """Opens file and extracts source associated with tainted result"""
    results = defaultdict(list)
    for k, v in positives.items():
        with open(k, "r") as f:
            lines = f.readlines()  # this may get large for large source files
        for num in sorted(v):
            # Each file has list of tuples with this structure (linenum, source code)
            results[k].append((num + 1, lines[num].strip()))

    results2 = defaultdict(list)
    for k, v in positives2.items():
        with open(k, "r") as f:
            lines = f.readlines()  # this may get large for large source files
        for num in sorted(v):
            # Each file has list of tuples with this structure (linenum, source code)
            results2[k].append((num + 1, lines[num].strip()))

    return [results, results2]


def output_results(lines):
    """Writes out to stdout ex. fn.c line # - source"""
    for fname, result_pair in lines[0].items():
        for line, src in result_pair:
            print("{} line {:4d} - {}".format(fname, line, src))
    for fname, result_pair in lines[1].items():
        for line, src in result_pair:
            print("{} line {:4d} - {}".format(fname, line, src))


def update_table(lines):
    """Writes out to stdout ex. fn.c line # - source"""
    start = int(sys.argv[2])
    end = int(sys.argv[3])
    row_idx = int(sys.argv[5])
    interested_file = sys.argv[6]
    min, sec = divmod(parse_number(sys.argv[7]), 60)
    running_time = "%02d:%02d" % (min, sec)

    count = 0
    for fname, result_pair in lines[0].items():
        for line, src in result_pair:
            # print("{} line {:4d} - {}".format(fname, line, src))
            if line >= start and line <= end:
                if fname == interested_file:
                    count = count + 1

    table = []
    with open("../../paper/conference/benchmark-table.tex", "r") as f:
        for i, line in enumerate(f):
            l = line.split("&")
            if len(l) == 12:
                table.append(
                    [
                        s.replace("\\", "")
                        .replace("hspace{0.25cm}", "")
                        .replace("textbf{", "")
                        .replace("}", "")
                        .strip()
                        for s in l
                    ]
                )

    for idx, label in enumerate(table[0]):
        if label == sys.argv[4]:
            table[row_idx][idx] = count

    with open("../../paper/conference/benchmark-table.tex", "w") as f:
        f.write((" & ".join(table[0]) + " \\\\\n").replace("%", "\%"))
        f.write("\\midrule\n")
        for i in range(1, 4):
            table[i][0] = "\\textbf{" + table[i][0] + "}"
            f.write((" & ".join(map(str, table[i])) + " \\\\\n").replace("%", "\%"))
        f.write("\\textbf{OpenSSL 1.1.1q} \\\\\n")
        for i in range(4, 8):
            table[i][0] = "\\hspace{0.25cm}" + table[i][0]
            f.write((" & ".join(map(str, table[i])) + " \\\\\n").replace("%", "\%"))
        f.close()

    count = 0
    for fname, result_pair in lines[1].items():
        for line, src in result_pair:
            # print("{} line {:4d} - {}".format(fname, line, src))
            if line >= start and line <= end:
                if fname == interested_file:
                    count = count + 1

    table = []
    with open("../../paper/conference/benchmark-array-access.tex", "r") as f:
        for i, line in enumerate(f):
            l = line.split("&")
            if len(l) == 12:
                table.append(
                    [
                        s.replace("\\", "")
                        .replace("hspace{0.25cm}", "")
                        .replace("textbf{", "")
                        .replace("}", "")
                        .strip()
                        for s in l
                    ]
                )

    for idx, label in enumerate(table[0]):
        if label == sys.argv[4]:
            table[row_idx][idx] = count

    with open("../../paper/conference/benchmark-array-access.tex", "w") as f:
        f.write((" & ".join(table[0]) + " \\\\\n").replace("%", "\%"))
        f.write("\\midrule\n")
        for i in range(1, 4):
            table[i][0] = "\\textbf{" + table[i][0] + "}"
            f.write((" & ".join(map(str, table[i])) + " \\\\\n").replace("%", "\%"))
        f.write("\\textbf{OpenSSL 1.1.1q} \\\\\n")
        for i in range(4, 8):
            table[i][0] = "\\hspace{0.25cm}" + table[i][0]
            f.write((" & ".join(map(str, table[i])) + " \\\\\n").replace("%", "\%"))
        f.close()

    running_time_table = []
    with open("../../paper/conference/benchmark-running-time.tex", "r") as f:
        for i, line in enumerate(f):
            l = line.split("&")
            if len(l) == 12:
                running_time_table.append(
                    [
                        s.replace("\\", "")
                        .replace("hspace{0.25cm}", "")
                        .replace("textbf{", "")
                        .replace("}", "")
                        .strip()
                        for s in l
                    ]
                )

    for idx, label in enumerate(running_time_table[0]):
        if label == sys.argv[4]:
            running_time_table[row_idx][idx] = running_time

    with open("../../paper/conference/benchmark-running-time.tex", "w") as f:
        f.write((" & ".join(running_time_table[0]) + " \\\\\n").replace("%", "\%"))
        f.write("\\midrule\n")
        for i in range(1, 4):
            running_time_table[i][0] = "\\textbf{" + running_time_table[i][0] + "}"
            f.write(
                (" & ".join(map(str, running_time_table[i])) + " \\\\\n").replace(
                    "%", "\%"
                )
            )
        f.write("\\textbf{OpenSSL 1.1.1q} \\\\\n")
        for i in range(4, 8):
            running_time_table[i][0] = "\\hspace{0.25cm}" + running_time_table[i][0]
            f.write(
                (" & ".join(map(str, running_time_table[i])) + " \\\\\n").replace(
                    "%", "\%"
                )
            )
        f.close()


def main():
    """Looks at results from Vulnerable/Taint Analysis and records the associated
    line with it
    """
    positives = defaultdict(list)  # only report each line once: set
    positives2 = defaultdict(list)  # only report each line once: set
    # to keep repeated lines use: list

    for line in filter_lines_from_results(sys.argv[1])[0]:
        s = line.strip().split()
        if len(s) == 3:
            fn, _, ln = s
            # Subtract 1 because indices of lines starts at 0
            positives[fn].append(int(ln) - 1)

    for line in filter_lines_from_results(sys.argv[1])[1]:
        s = line.strip().split()
        if len(s) == 3:
            fn, _, ln = s
            # Subtract 1 because indices of lines starts at 0
            positives2[fn].append(int(ln) - 1)

    res = get_lines_from_source(positives, positives2)
    output_results(res)
    update_table(res)


if __name__ == "__main__":
    main()
