""" Update gaps table
"""

import sys
import re
from lattice import RLLabel


def main():
    """main function"""

    encoding = "UTF-8"
    state = 0
    delim = "  ->  "
    new_table = []
    tags = [""] * 4
    cols = []
    augment = False
    name = ""

    with open(sys.argv[2], "r", encoding=encoding) as gaps:
        with open(sys.argv[1], "r", encoding=encoding) as table:
            for i, line in enumerate(gaps):
                if i == 0:
                    name = line.strip().strip("[]")
                    if name.endswith(".z"):
                        augment = True
                        name = name[:-2]
                elif line.startswith("explicit:"):
                    state = 1
                elif line.startswith("implicit:"):
                    state = 2
                if delim in line:
                    line = line.strip().split(delim)
                    left = RLLabel(line[0])
                    right = RLLabel(line[1])
                    # print(left)
                    # print(right)
                    if left.compartment["purpose"] == right.compartment[
                            "purpose"]:
                        if left.level["value"] > right.level["value"]:
                            cols.append(0)
                        if left.level["source"] > right.level["source"]:
                            if left.level["source"] == 1:
                                cols.append(1)
                            elif right.level["source"] == 1:
                                cols.append(2)
                            else:
                                cols.append(3)

                        for col in cols:
                            if state == 1:
                                tags[col] += ":red_circle:"
                            elif state == 2 and (len(tags[col]) == 0
                                                 or tags[col].startswith(
                                                     ":large_blue_diamond:")):
                                tags[col] += ":large_blue_diamond:"

            for i, tag in enumerate(tags):
                exp = len(re.findall(r":red_circle:", tag))
                imp = len(re.findall(r":large_blue_diamond:", tag))
                if exp > 0:
                    tags[i] = ":red_circle:" + "*" + str(exp)
                elif imp > 0:
                    tags[i] = ":large_blue_diamond:" + "*" + str(imp)

            for i, line in enumerate(table):
                if name in line:
                    clean = True
                    print(tags)
                    if augment:
                        line = [tag.strip() for tag in line.split("|")]
                        for j, tag in enumerate(tags):
                            if ":red_circle:" in line[j + 2]:
                                count = int(
                                    re.search(r"\d+", line[j + 2]).group())
                                if ":red_circle:" in tag:
                                    count += int(
                                        re.search(r"\d+", tag).group())
                                line[j + 2] = ":red_circle:" + "*" + str(count)
                            elif ":large_blue_diamond:" in line[j + 2]:
                                count = int(
                                    re.search(r"\d+", line[j + 2]).group())
                                if ":red_circle:" in tag:
                                    line[j + 2] = tag
                                elif ":large_blue_diamond:" in tag:
                                    count += int(
                                        re.search(r"\d+", tag).group())
                                    line[j + 2] = (":large_blue_diamond:" +
                                                   "*" + str(count))
                            else:
                                line[j + 2] = tag
                        line = "|".join(line[:-1])
                        print(line)
                    else:
                        line = "|" + name + "|"
                        line += "|".join(tags) + "|"

                    if ":" in line:
                        clean = False

                    if clean:
                        line += ":white_check_mark:|\n"
                    else:
                        line += "|\n"
                    new_table.append(line)
                else:
                    new_table.append(line)
            table.close()
        gaps.close()

    with open(sys.argv[1], "w", encoding=encoding) as table:
        table.writelines(new_table)
        table.close()


if __name__ == "__main__":
    main()
