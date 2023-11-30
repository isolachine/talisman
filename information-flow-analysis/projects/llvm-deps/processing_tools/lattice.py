"""The lattice library"""
import re
from itertools import chain, combinations
import copy


class Expr:

    def __init__(self, name, desc="", file="", line=""):
        self.name = name
        self.desc = desc
        self.file = file
        self.line = line

    def __eq__(self, other):
        return isinstance(other, Expr) and self.name == other.name

    def __str__(self):
        return self.name + "[" + self.desc + "]"


class Label(Expr):

    def __init__(self, name, desc="", file="", line=""):
        self.name = name
        self.desc = desc
        self.file = file
        self.line = line
        self.level = {}
        self.compartment = {}

    def __str__(self):
        return self.name + "_LABEL_"


class RLLabel(Label):

    def __init__(self, lString="", level={}, compartment={}):
        self.name = "CONST"
        self.level = {}
        self.compartment = {}
        if lString != "":
            self.parseConstString(lString)
        else:
            self.level = level
            self.compartment = compartment

    def __eq__(self, other):
        return (isinstance(other, RLLabel) and self.level == other.level
                and self.compartment == other.compartment)

    def __str__(self):
        return "[" + self.level.__str__() + "][" + self.compartment.__str__(
        ) + "]"

    def __hash__(self):
        return hash(self.__str__())

    def compare(self, other):
        if self == other:
            return 0
        else:
            gt = False
            lt = False
            for key in self.level:
                if self.level[key] > other.level[key]:
                    gt = True
                elif self.level[key] < other.level[key]:
                    lt = True
            for key in self.compartment:
                if (len(self.compartment[key] - other.compartment[key]) > 0
                        and len(other.compartment[key] - self.compartment[key])
                        > 0):
                    gt = True
                    lt = True
                elif self.compartment[key] > other.compartment[key]:
                    gt = True
                elif self.compartment[key] < other.compartment[key]:
                    lt = True
            if gt and lt:
                return -2
            elif gt and (not lt):
                return 1
            elif (not gt) and lt:
                return -1
            else:
                raise Exception("This shouldn't happen, \
                        it should be dealt with in the previous branch")

    def join(self, other):
        if self == other:
            return self
        else:
            level = {}
            compartment = {}
            for key in self.level:
                if self.level[key] > other.level[key]:
                    level[key] = self.level[key]
                else:
                    level[key] = other.level[key]
            for key in self.compartment:
                compartment[key] = self.compartment[key].union(
                    other.compartment[key])
            return RLLabel(level=level, compartment=compartment)

    def parseConstString(self, str):
        re_match = (re.compile(r"CONST\[(.*)\]\[(.*)\]")).match(str)
        if re_match:
            level = re_match.group(1)
            compartment = re_match.group(2)
            level = re.split(",", level)
            for lvl in level:
                l_match = re.compile(r"([a-zA-Z0-9]*):([0-9]*)\(.*\)").match(
                    lvl)
                self.level[l_match.group(1)] = int(l_match.group(2))
            compartment = re.compile(
                r"([a-zA-Z0-9]*:\{[a-zA-Z0-9,]*\})").findall(compartment)
            for c in compartment:
                c_match = re.compile(
                    r"([a-zA-Z0-9]*):\{([a-zA-Z0-9,]*)\}").match(c)
                self.compartment[c_match.group(1)] = set()
                if c_match.group(2) == "":
                    c_set = []
                else:
                    c_set = re.split(",", c_match.group(2))
                for e in c_set:
                    self.compartment[c_match.group(1)].add(e)
        else:
            raise Exception("Bad format!")


def powerset(iterable):
    """powerset([1,2,3]) --> () (1,) (2,) (3,) (1,2) (1,3) (2,3) (1,2,3)"""
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(len(s) + 1))


class Lattice:

    def __init__(self):
        self.sub = []
        self.top = Label("Top")
        self.bot = Label("Bot")
        self.lowest = self.bot
        self.labels = []

    def add_sub(self, low, high):
        self.sub.append((low, high))
        if low not in self.labels:
            self.labels.append(low)
        if high not in self.labels:
            self.labels.append(high)
        if self.lowest == self.bot or self.check_sub(low, self.lowest):
            self.lowest = low

    def check_sub(self, low, high):
        if high.compare(low) >= 0:
            return True
        else:
            return False
        if low == high or low == self.bot or high == self.top:
            return True
        elif low == self.top or high == self.bot:
            return False
        work = [high]
        while bool(work):
            can = work.pop(0)
            for r in self.sub:
                if (r[1] == can) and (r[0] == low):
                    return True
                elif r[1] == can:
                    work.append(r[0])
        return False

    def join(self, l1, l2):
        """Joining two labels"""
        if l1 == l2 or self.check_sub(l1, l2):
            return l2
        elif self.check_sub(l2, l1):
            return l1
        else:
            work = [l1]
            while bool(work):
                can = work.pop(0)
                for r in self.sub:
                    if (r[0] == can) and self.check_sub(l2, r[1]):
                        return r[1]
                    elif r[0] == can:
                        work.append(r[1])
            return self.top

    def __str__(self):
        result = "Model"
        for r in self.sub:
            result = result + "\n\t" + str(r[0]) + " <: " + str(r[1])
        result = result + "\nSize of all sub: {0}".format(len(self.sub))
        return result


class RLLattice(Lattice):
    """RLLattice"""

    def __init__(self, lattice_file=None):
        Lattice.__init__(self)
        self.levels = {}
        self.compartments = {}
        self.constants = set()
        self.read_lattice_file(lattice_file)

    def read_lattice_file(self, file):
        """Parsing lattice.txt file"""
        if file is None:
            self.levels = {"value": ["L", "H"]}
        else:
            with open(file, "r") as f:
                for line in f:
                    line = re.split(" ", line.strip())
                    if line[0] == "C":
                        c = set()
                        for i in range(2, len(line)):
                            c.add(line[i].rstrip())
                        self.compartments[line[1]] = c
                    elif line[0] == "L":
                        lvl = []
                        for i in range(2, len(line)):
                            lvl.append(line[i].rstrip())
                        self.levels[line[1]] = lvl
        labels = set()
        labels.add(RLLabel())
        temp = set()
        for key in self.levels.keys():
            for label in labels:
                for level in range(len(self.levels[key])):
                    new_label = copy.deepcopy(label)
                    new_label.level[key] = level
                    temp.add(new_label)
            labels = temp.copy()
            temp.clear()

        for key in self.compartments.keys():
            ps = []
            for i in powerset(self.compartments[key]):
                ps.append(set(i))
            for label in labels:
                for s in ps:
                    new_label = copy.deepcopy(label)
                    new_label.compartment[key] = s
                    temp.add(new_label)
            labels = temp.copy()
            temp.clear()

        for i in labels:
            for j in labels:
                if j.compare(i) > 0:
                    self.add_sub(i, j)
            self.constants.add(i)

    def join(self, l1, l2):
        return l1.join(l2)
