//===-- RLConstraints.cpp ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// TODO Description
//
//===----------------------------------------------------------------------===//

#include "Constraints/RLConstraints.h"
#include "llvm/Support/Casting.h"
#include <regex>

namespace deps {

CompartmentMap RLConstant::RLCompartmentMap;
LevelMap RLConstant::RLLevelMap;
RLConstantMap RLConstant::constants;
RLLabel RLConstant::TopLabel;
RLLabel RLConstant::BotLabel;
bool RLConstant::latticeLock = false;

RLConstant::RLConstant(RLLevel l, RLCompartment c) : level(l), compartment(c) {}

const RLConstant &RLConstant::bot() { return RLConstant::constant(BotLabel); }

const RLConstant &RLConstant::top() { return RLConstant::constant(TopLabel); }

const RLConstant &RLConstant::constant(RLLevel l, RLCompartment c) {
  RLLabel pair = RLLabel(l, c);
  if (RLConstant::constants.find(pair) == RLConstant::constants.end()) {
    RLConstant::constants[pair] = new RLConstant(l, c);
  }
  return *RLConstant::constants[pair];
}

const RLConstant &RLConstant::constant(RLLabel label) {
  return constant(label.first, label.second);
}

bool RLConstant::leq(const ConsElem &elem) const {
  const RLConstant *other;
  if ((other = llvm::dyn_cast<RLConstant>(&elem))) {
    for (auto l : level) {
      if (level.at(l.first) > other->level.at(l.first))
        return false;
    }
    for (auto c : compartment) {
      std::set<std::string> diff;
      set_difference(c.second.begin(), c.second.end(),
                     other->compartment.at(c.first).begin(),
                     other->compartment.at(c.first).end(),
                     inserter(diff, diff.end()));
      if (diff.size() > 0)
        return false;
    }
    return true;
  } else {
    return false;
  }
}

const RLConstant &RLConstant::join(const RLConstant &other) const {
  return constant(upperBoundLabel(other));
}

const RLLabel RLConstant::upperBoundLabel(const RLConstant &other) const {
  return upperBoundLabel(make_pair(level, compartment),
                         make_pair(other.level, other.compartment));
}

const RLLabel RLConstant::lowerBoundLabel(const RLConstant &other) const {
  return lowerBoundLabel(make_pair(level, compartment),
                         make_pair(other.level, other.compartment));
}

const RLLabel RLConstant::upperBoundLabel(RLLabel label, RLLabel other) {
  RLLabel lub;
  for (auto l : label.first) {
    std::string key = l.first;
    lub.first[key] = label.first[key] > other.first[key] ? label.first[key]
                                                         : other.first[key];
  }
  for (auto c : label.second) {
    std::set<std::string> setUnion;
    std::set<std::string> otherC = other.second.at(c.first);
    set_union(c.second.begin(), c.second.end(), otherC.begin(), otherC.end(),
              inserter(setUnion, setUnion.end()));
    lub.second[c.first] = setUnion;
  }
  return lub;
}

const RLLabel RLConstant::lowerBoundLabel(RLLabel label, RLLabel other) {
  RLLabel glb;
  for (auto l : label.first) {
    std::string key = l.first;
    glb.first[key] = label.first[key] < other.first[key] ? label.first[key]
                                                         : other.first[key];
  }
  for (auto c : label.second) {
    std::set<std::string> setIntersection;
    std::set<std::string> otherC = other.second.at(c.first);
    set_intersection(c.second.begin(), c.second.end(), otherC.begin(),
                     otherC.end(),
                     inserter(setIntersection, setIntersection.end()));
    glb.second[c.first] = setIntersection;
  }
  return glb;
}

const RLLabel RLConstant::label() const {
  return make_pair(level, compartment);
}

void RLConstant::dump(llvm::raw_ostream &o) const {
  assert(isLatticeLocked() && "Lattice should be created and locked already!");
  o << "CONST[";
  for (auto l : level) {
    o << l.first << ":" << l.second << "("
      << RLConstant::RLLevelMap[l.first][l.second] << ")"
      << ((level.rbegin()->first != l.first) ? "," : "");
  }
  o << "][";
  for (auto c : compartment) {
    o << c.first << ":{";
    for (auto s : c.second) {
      o << s << ((*c.second.rbegin() != s) ? "," : "");
    }
    o << "}" << ((compartment.rbegin()->first != c.first) ? "," : "");
  }
  o << "]";
}

void RLConstant::dump_lattice(llvm::raw_ostream &o) {
  assert(isLatticeLocked() && "Lattice should be created and locked already!");
  o << "Levels: [\n";
  for (auto d : RLConstant::RLLevelMap) {
    o << "\t" << d.first << ": { ";
    for (auto s : d.second) {
      o << s;
      if (s != *d.second.rbegin())
        o << " -> ";
    }
    o << " }\n";
  }
  o << "]\n";

  o << "Compartments: [\n";
  for (auto d : RLConstant::RLCompartmentMap) {
    o << "\t" << d.first << ": { ";
    for (auto s : d.second) {
      o << s << (s != *d.second.rbegin() ? ", " : " ");
    }
    o << "}\n";
  }
  o << "]\n";
}

RLLabel RLConstant::parseLabelString(std::string line) {
  RLLabel ret;
  std::regex group("\\[(.*)\\]\\[(.*)\\]");
  std::regex comma(",");
  std::regex colon(":");
  std::regex compartmentSet("([a-zA-Z0-9]*:\\{[a-zA-Z0-9,]*\\})");
  std::regex setElem("([a-zA-Z0-9]+)");

  std::smatch match;
  std::regex_match(line, match, group);

  llvm::errs() << "REGEX:: \n";
  for (auto s : match) {
    llvm::errs() << s.str() << "\n";
  }

  std::string level = match[1].str();
  llvm::errs() << level << "\n";
  std::vector<std::string> levels(
      std::sregex_token_iterator(level.begin(), level.end(), comma, -1),
      std::sregex_token_iterator());
  for (auto &s : levels) {
    llvm::errs() << s << '\n';
    std::vector<std::string> thisLevel(
        std::sregex_token_iterator(s.begin(), s.end(), colon, -1),
        std::sregex_token_iterator());
    auto v = RLConstant::RLLevelMap.at(thisLevel[0]);
    auto it = find(v.begin(), v.end(), thisLevel[1]);
    assert(it != v.end());
    int index = it - v.begin();
    ret.first.insert(std::make_pair(thisLevel[0], index));
  }

  // TODO: assert for name matchs in the map
  std::string compartment = match[2].str();
  llvm::errs() << compartment << "\n";
  std::vector<std::string> compartments(
      std::sregex_token_iterator(compartment.begin(), compartment.end(),
                                 compartmentSet, 1),
      std::sregex_token_iterator());
  for (auto &s : compartments) {
    llvm::errs() << s << "\n";
    std::vector<std::string> thisSet(
        std::sregex_token_iterator(s.begin(), s.end(), colon, -1),
        std::sregex_token_iterator());
    std::set<std::string> set;
    llvm::errs() << thisSet[1] << "\n";
    std::vector<std::string> elems(
        std::sregex_token_iterator(thisSet[1].begin(), thisSet[1].end(),
                                   setElem, 1),
        std::sregex_token_iterator());
    for (auto &e : elems)
      set.insert(e);
    ret.second[thisSet[0]] = set;
  }
  return ret;
}

bool RLConstant::operator==(const ConsElem &elem) const {
  if (const RLConstant *other = llvm::dyn_cast<const RLConstant>(&elem)) {
    return (this == other);
  } else {
    return false;
  }
}

RLConsVar::RLConsVar(const std::string description, const std::string metainfo)
    : desc(description), meta(metainfo) {
  std::regex re("[ =@\\\\[\\];\n\"\\{\\}~]+");
  replaced = std::regex_replace(desc, re, "_");
  replaced.append(std::regex_replace(meta, re, "_"));
  size_t delimCount = std::count(replaced.begin(), replaced.end(), '|');
  while (delimCount < 2) {
    delimCount++;
    replaced.append("|");
  }
}

bool RLConsVar::leq(const ConsElem &elem) const { return false; }

bool RLConsVar::operator==(const ConsElem &elem) const {
  if (const RLConsVar *other = llvm::dyn_cast<const RLConsVar>(&elem)) {
    return this == other;
  } else {
    return false;
  }
}

void RLConsVar::dump(llvm::raw_ostream &o) const { o << replaced; }

RLJoin::RLJoin(std::set<const ConsElem *> elements) : elems(elements) {}

bool RLJoin::leq(const ConsElem &other) const {
  for (std::set<const ConsElem *>::iterator elem = elems.begin(),
                                            end = elems.end();
       elem != end; ++elem) {
    if (!(*elem)->leq(other)) {
      return false;
    }
  }
  return true;
}

void RLJoin::variables(std::set<const ConsVar *> &vars) const {
  for (std::set<const ConsElem *>::iterator elem = elems.begin(),
                                            end = elems.end();
       elem != end; ++elem) {
    (*elem)->variables(vars);
  }
}

const RLJoin *RLJoin::create(const ConsElem &e1, const ConsElem &e2) {
  std::set<const ConsElem *> elements;

  if (const RLJoin *j1 = llvm::dyn_cast<RLJoin>(&e1)) {
    const std::set<const ConsElem *> &j1elements = j1->elements();
    elements.insert(j1elements.begin(), j1elements.end());
  } else {
    elements.insert(&e1);
  }

  if (const RLJoin *j2 = llvm::dyn_cast<RLJoin>(&e2)) {
    const std::set<const ConsElem *> &j2elements = j2->elements();
    elements.insert(j2elements.begin(), j2elements.end());
  } else {
    elements.insert(&e2);
  }

  return new RLJoin(elements);
}

void RLJoin::dump(llvm::raw_ostream &o) const {
  o << "Join (";
  for (std::set<const ConsElem *>::iterator elem = elems.begin(),
                                            end = elems.end();
       elem != end; ++elem) {
    (*elem)->dump(o);
  }
  o << ")";
}

bool RLJoin::operator==(const ConsElem &elem) const {
  if (const RLJoin *other = llvm::dyn_cast<const RLJoin>(&elem)) {
    return this == other;
  } else {
    return false;
  }
}

} // namespace deps
