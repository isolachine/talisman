//===-- Test.cpp ------------------------------------------------*- C++ -*-===//
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

#include "Constraints/PredicatedConstraints.h"
#include "Constraints/RLConstraintKit.h"
#include "llvm/Support/Casting.h"
#include <iostream>
#include <regex>

using namespace deps;

void parseLine(std::string line) {
  llvm::errs() << line << "\n";
  // Move any extra whitespace to end
  std::string::iterator new_end =
      unique(line.begin(), line.end(),
             [](const char &x, const char &y) { return x == y and x == ' '; });

  // Remove the extra space
  line.erase(new_end, line.end());
  line = std::regex_replace(line, std::regex("^ +| +$"), "");

  // Split up line
  std::vector<std::string> splits;
  char delimiter = ' ';

  size_t i = 0;
  size_t pos = line.find(delimiter);

  while (pos != std::string::npos) {
    splits.push_back(line.substr(i, pos - i));
    i = pos + 1;
    pos = line.find(delimiter, i);
  }
  splits.push_back(line.substr(i, std::min(pos, line.size()) - i + 1));
  assert(splits.size() > 2 &&
         "Incompatible input. Check lattice.txt for format issues");
  if (splits[0] == "C") {
    std::set<std::string> compartments;
    for (uint i = 2; i < splits.size(); i++) {
      compartments.insert(splits[i]);
    }
    RLConstant::RLCompartmentMap.insert(
        std::pair<std::string, std::set<std::string>>(splits[1], compartments));
  } else if (splits[0] == "L") {
    std::vector<std::string> levels;
    for (uint i = 2; i < splits.size(); i++) {
      levels.push_back(splits[i]);
    }
    RLConstant::RLLevelMap.insert(
        std::pair<std::string, std::vector<std::string>>(splits[1], levels));
  }
}

void test(void) {

  // std::vector<const Predicate *> PSet;

  // // Test 0
  // PSet.push_back(new Predicate(-1, 5, 'x', 10, 20, 'y'));
  // PSet.push_back(new Predicate(2, 6, 'x', 15, 17, 'y'));
  // PSet.push_back(new Predicate(4, 12, 'x', 20, 30, 'z'));

  // // // // Test 1
  // // PSet.push_back(new Predicate(-1, 5, 'x'));
  // // PSet.push_back(new Predicate(2, 7, 'y'));

  // // // Test 2
  // // PSet.push_back(new Predicate(-1, 2, 'x'));
  // // PSet.push_back(new Predicate(5, 2, 'x'));
  // // PSet.push_back(new Predicate(1, 6, 'x'));

  // // // Test 3
  // // PSet.push_back(new Predicate(-1, 6, 'x'));
  // // PSet.push_back(new Predicate(4, 8, 'x'));
  // // PSet.push_back(new Predicate(0, 5, 'y'));
  // // PSet.push_back(new Predicate(-3, 7, 'y'));

  // // // // Test 4
  // // PSet.push_back(new Predicate(P_NEGINF, P_INF, 'x'));
  // // PSet.push_back(new Predicate(P_NEGINF, 5, 'x'));
  // // PSet.push_back(new Predicate(-2, P_INF, 'x'));

  // // Testing addinequality function
  // // PSet[0]->addinequality(10,20,'z');
  // // PSet[1]->addinequality(15,17, 'z');
  // // PSet[1]->addinequality(1,3, 'y');

  RLConstraintKit kit;

  // const ConsElem &a = kit.newVar("a");
  // const ConsElem &b = kit.newVar("b");
  // const ConsElem &c = kit.newVar("c");

  parseLine("C purpose subject object operation");
  parseLine("C other a b c");
  parseLine("L lh L M H ");
  parseLine("L source kernal mediator");

  llvm::errs() << "CompartmentSet: [\n";
  for (auto d : RLConstant::RLCompartmentMap) {
    llvm::errs() << "\t" << d.first << ": { ";
    RLConstant::BotLabel.second[d.first] = std::set<std::string>();
    RLConstant::TopLabel.second[d.first] = d.second;
    for (auto s : d.second) {
      llvm::errs() << s << " ";
    }
    llvm::errs() << "}\n";
  }
  llvm::errs() << "]\n";

  llvm::errs() << "Levels: [\n";
  for (auto d : RLConstant::RLLevelMap) {
    llvm::errs() << "\t" << d.first << ": { ";
    RLConstant::BotLabel.first[d.first] = 0;
    RLConstant::TopLabel.first[d.first] = d.second.size() - 1;
    for (auto s : d.second) {
      llvm::errs() << s;
      if (s.c_str() != d.second.back().c_str())
        llvm::errs() << " -> ";
    }
    llvm::errs() << " }\n";
  }
  llvm::errs() << "]\n";

  RLConstant::lockLattice();

  RLConstant::bot().dump(llvm::errs());
  llvm::errs() << "\n";
  RLConstant::top().dump(llvm::errs());
  llvm::errs() << "\n";

  RLConstant c1 = RLConstant::constant(
      std::make_pair(RLLevel({{"lh", 1}, {"source", 0}}),
                     RLCompartment({{"purpose", {"subject", "operation"}},
                                    {"other", {"a"}}})));
  c1.dump(llvm::errs());
  llvm::errs() << "\n";
  RLConstant c2 = RLConstant::constant(std::make_pair(
      RLLevel({{"lh", 0}, {"source", 1}}),
      RLCompartment({{"purpose", {"subject"}}, {"other", {"a"}}})));
  c2.dump(llvm::errs());
  llvm::errs() << "\n";
  RLConstant c3 = RLConstant::constant(std::make_pair(
      RLLevel({{"lh", 2}, {"source", 0}}),
      RLCompartment({{"purpose", {"object"}}, {"other", {"b"}}})));
  c3.dump(llvm::errs());
  llvm::errs() << "\n";

  // llvm::errs() << "\n" << kit.botConstant().leq(kit.topConstant()) << "\n";
  // llvm::errs() << "\n" << kit.botConstant().leq(kit.botConstant()) << "\n";
  // llvm::errs() << "\n" << kit.topConstant().leq(kit.botConstant()) << "\n";
  // llvm::errs() << "\n" << kit.topConstant().leq(kit.topConstant()) << "\n";
  llvm::errs() << "\n" << kit.topConstant().leq(c1) << "\n";
  llvm::errs() << "\n" << kit.botConstant().leq(c1) << "\n";
  llvm::errs() << "\n" << c1.leq(kit.topConstant()) << "\n";
  llvm::errs() << "\n" << c1.leq(kit.botConstant()) << "\n";
  llvm::errs() << "\n" << c1.leq(c2) << "\n";
  llvm::errs() << "\n" << c2.leq(c1) << "\n";
  llvm::errs() << "\n" << c1.leq(c3) << "\n";
  llvm::errs() << "\n" << c3.leq(c1) << "\n";

  c3.join(c1).dump(llvm::errs() << "\n");
  c1.join(c3).dump(llvm::errs() << "\n");
  c2.join(c3).dump(llvm::errs() << "\n");
  c3.join(c2).dump(llvm::errs() << "\n");
  c1.join(c2).dump(llvm::errs() << "\n");
  c2.join(c1).dump(llvm::errs() << "\n");

  kit.constant(c1.lowerBoundLabel(c2)).dump(llvm::errs() << "\n");
  kit.constant(c1.lowerBoundLabel(c3)).dump(llvm::errs() << "\n");
  kit.constant(c2.lowerBoundLabel(c3)).dump(llvm::errs() << "\n");

  // // Testing kit for least solution
  // if (PSet.size() > 0) {
  //   kit.addConstraint("least", kit.botConstant(), a, *PSet[0]);
  //   kit.addConstraint("least", a, b, *PSet[0]);
  // }
  // if (PSet.size() > 1) {
  //   kit.addConstraint("least", kit.constant(RLLevel::MID, cS4), b, *PSet[1]);
  // }
  // if (PSet.size() > 2) {
  //   kit.addConstraint("least", kit.constant(RLLevel::MID, cS2), b, *PSet[2]);
  //   kit.addConstraint("least", b, c, *PSet[2]);
  // }

  // // Testing kit for greatest solution
  // if (PSet.size() > 0) {
  //   kit.addConstraint("greatest", a, kit.constant(RLLevel::LOW, cS3),
  //   *PSet[0]); kit.addConstraint("greatest", a, b, *PSet[0]);
  //   kit.addConstraint("greatest", b, kit.constant(RLLevel::MID, cS1),
  //   *PSet[0]);
  // }
  // if (PSet.size() > 1) {
  //   kit.addConstraint("greatest", c, kit.constant(RLLevel::HIGH, cS1),
  //   *PSet[1]); kit.addConstraint("greatest", c, a, *PSet[1]);
  // }
  // if (PSet.size() > 2) {
  // }

  // std::set<std::string> kinds{"least"};
  // std::set<std::string> greatest{"greatest"};
  // kit.partitionPredicateSet(PSet);

  // llvm::errs() << "\n===== Least solution ====="
  //              << "\n";

  // for (auto P : PSet) {
  //   P->dump();
  //   llvm::errs() << "\n";
  //   ConsSoln *leastSoln = kit.leastSolution(kinds, P);
  //   for (auto elem : elemVec) {
  //     llvm::errs() << "\t";
  //     elem->dump(llvm::errs());
  //     llvm::errs() << ": ";
  //     leastSoln->subst(*elem).dump(llvm::errs());
  //     llvm::errs() << "\n";
  //   }
  //   delete leastSoln;
  // }

  // llvm::errs() << "\n===== Greatest solution ====="
  //              << "\n";

  // for (auto P : PSet) {
  //   P->dump();
  //   llvm::errs() << "\n";
  //   ConsSoln *greatestSoln = kit.greatestSolution(greatest, P);
  //   for (auto elem : elemVec) {
  //     llvm::errs() << "\t";
  //     elem->dump(llvm::errs());
  //     llvm::errs() << ": ";
  //     greatestSoln->subst(*elem).dump(llvm::errs());
  //     llvm::errs() << "\n";
  //   }
  //   delete greatestSoln;
  // }
}

int main(void) {
  test();
  return 0;
}
