""" Update gaps table
"""

import sys
import re
from lattice import RLLabel

cols = ["subject", "object", "operation"]
explicit_gaps = set()
src_records = [set() for i in range(6)]


def update_tags(left, right, state, counts, src_idx, sink_idx):
    """Update tags"""
    col = -1
    if left.join(right).compartment["purpose"] != left.compartment["purpose"]:
        for src in left.compartment["purpose"]:
            for dst in right.compartment["purpose"]:
                if src != dst:
                    if src == cols[0]:
                        if dst == cols[1]:
                            col = 0
                        elif dst == cols[2]:
                            col = 1
                    elif src == cols[1]:
                        if dst == cols[0]:
                            col = 2
                        elif dst == cols[2]:
                            col = 3
                    elif src == cols[2]:
                        if dst == cols[0]:
                            col = 4
                        elif dst == cols[1]:
                            col = 5
                # print(col)
                if col >= 0:
                    record = left.__str__() + right.__str__() + '[' + str(
                        src_idx) + ',' + str(sink_idx) + ']'
                    if record not in src_records[col]:
                        src_records[col].add(record)
                        if state == 1:
                            counts[col][0] += 1
                        elif state == 2:
                            counts[col][1] += 1
    others = [0] * 4
    if left.level["value"] > right.level["value"]:
        others[0] = 1
    if left.level["source"] > right.level["source"]:
        if left.level["source"] == 1:
            others[1] = 1
        elif right.level["source"] == 1:
            others[2] = 1
        else:
            others[3] = 1

    for i, other in enumerate(others):
        if other == 1:
            if state == 1:
                counts[i + 6][0] += 1
            elif state == 2:
                counts[i + 6][1] += 1


def main():
    """main function"""

    encoding = "UTF-8"
    state = 0
    delim = "  ->  "
    new_table = []
    tags = [""] * 10
    counts = [[0] * 2 for i in range(10)]
    augment = False
    name = ''

    with open(sys.argv[2], "r", encoding=encoding) as gaps:
        with open(sys.argv[1], "r", encoding=encoding) as table:
            for i, line in enumerate(gaps):
                if i == 0:
                    name = line.strip().strip("[]")
                    if name.endswith(".z"):
                        augment = True
                        name = name[:-2]
                    print(name)
                elif line.startswith("explicit:"):
                    state = 1
                elif line.startswith("implicit:"):
                    state = 2
                if delim in line:
                    if state == 1:
                        explicit_gaps.add(line)
                    elif state == 2:
                        if line in explicit_gaps:
                            # print(line)
                            continue
                    line = line.strip().split(delim)
                    # print(line)
                    src_idx = int(
                        re.search(r"\[SrcIdx:(\d+)\]", line[0]).group(1))
                    sink_idx = int(
                        re.search(r"\[SnkIdx:(\d+)\]", line[1]).group(1))
                    line[0] = re.sub(r"(\[SrcIdx:\d+\])", "", line[0])
                    line[1] = re.sub(r"(\[SnkIdx:\d+\])", "", line[1])
                    left = RLLabel(line[0])
                    right = RLLabel(line[1])
                    update_tags(left, right, state, counts, src_idx, sink_idx)

            for i, cnt in enumerate(counts):
                tags[i] = (str(cnt[0]) if cnt[0] > 0 else '-') + "/" + (str(
                    cnt[1]) if cnt[1] > 0 else '-')

            for i, line in enumerate(table):
                if name in line:
                    # print(tags)
                    if augment:
                        line = [
                            tag.strip()
                            for tag in re.sub(r"-", "0", line).split("|")
                        ]
                        for j, cell in enumerate(line[2:]):
                            if cell == "":
                                line[j + 2] = "0/0"
                        print(line)
                        for j, tag in enumerate(tags):
                            tag = re.sub(r"-", "0", tag).split("/")
                            original = re.sub(r"-", "0",
                                              line[j + 2]).split("/")
                            tag[0] = int(original[0]) + counts[j][0]
                            tag[1] = int(original[1]) + counts[j][1]
                            line[j + 2] = (str(tag[0]) if tag[0] > 0 else
                                           '-') + "/" + (str(tag[1]) if
                                                         tag[1] > 0 else '-')
                        line = "|".join(line[:-1])
                        # print(line)
                    else:
                        line = "|" + name + "|"
                        line += "|".join(tags)
                    line += "|\n"
                line = re.sub(r"-/-", "", line)
                new_table.append(line)
            table.close()
        gaps.close()
    # print(src_records)
    with open(sys.argv[1], "w", encoding=encoding) as table:
        table.writelines(new_table)
        table.close()


if __name__ == "__main__":
    main()
