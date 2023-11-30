//===-- RLConstraintKit.cpp -------------------------------------*- C++ -*-===//
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

#define DEBUG_TYPE "deps"

#include "Constraints/RLConstraintKit.h"
#include "Constraints/PartialSolution.h"
#include "Constraints/RLConsSoln.h"
#include "Constraints/RLConstraints.h"
#include <iostream>
#include <chrono>
#include <ctime>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/Casting.h"

namespace deps {

STATISTIC(explicitRLConstraints, "Number of explicit flow constraints");
STATISTIC(implicitRLConstraints, "Number of implicit flow constraints");

typedef std::map<const Predicate *, llvm::StringMap<PartialSolution *>>
    PartialSolutionMap;

RLConstraintKit *RLConstraintKit::singleton = nullptr;

RLConstraintKit::RLConstraintKit() {}

RLConstraintKit::~RLConstraintKit() {
  // Delete all associated RLConsVars
  for (std::vector<const RLConsVar *>::iterator var = vars.begin(),
                                                end = vars.end();
       var != end; ++var) {
    delete (*var);
  }

  for (PartialSolutionMap::iterator I = leastSolutions.begin();
       I != leastSolutions.end(); ++I) {
    for (llvm::StringMap<PartialSolution *>::iterator psI = I->second.begin();
         psI != I->second.end(); psI++) {
      delete psI->second;
    }
  }
  for (PartialSolutionMap::iterator I = greatestSolutions.begin();
       I != greatestSolutions.end(); ++I) {
    for (llvm::StringMap<PartialSolution *>::iterator psI = I->second.begin();
         psI != I->second.end(); psI++) {
      delete psI->second;
    }
  }
}

const ConsVar &RLConstraintKit::newVar(const std::string description,
                                       const std::string metainfo) {
  RLConsVar *var = new RLConsVar(description, metainfo);
  vars.push_back(var);
  return *var;
}

const ConsElem &RLConstraintKit::botConstant() const {
  return RLConstant::bot();
}

const ConsElem &RLConstraintKit::topConstant() const {
  return RLConstant::top();
}

const ConsElem &RLConstraintKit::constant(RLLevel l, RLCompartment c) const {
  return RLConstant::constant(l, c);
}

const ConsElem &RLConstraintKit::constant(RLLabel label) const {
  return RLConstant::constant(label);
}

const Predicate &RLConstraintKit::truePredicate() {
  return Predicate::TruePred();
}

const ConsElem &RLConstraintKit::upperBound(const ConsElem &e1,
                                            const ConsElem &e2) {
  const RLJoin *join = RLJoin::create(e1, e2);
  std::set<RLJoin>::iterator J = joins.insert(*join).first;
  delete join;
  return *J;
}

const ConsElem *RLConstraintKit::upperBound(const ConsElem *e1,
                                            const ConsElem *e2) {
  if (e1 == NULL)
    return e2;
  if (e2 == NULL)
    return e1;

  const RLJoin *join = RLJoin::create(*e1, *e2);
  std::set<RLJoin>::iterator J = joins.insert(*join).first;
  delete join;
  return &*J;
}

const ConsElem &RLConstraintKit::upperBound(std::set<const ConsElem *> elems) {
  const RLJoin join = RLJoin(elems);
  std::set<RLJoin>::iterator J = joins.insert(join).first;
  return *J;
}

std::vector<RLConstraint> &
RLConstraintKit::getOrCreateConstraintSet(const std::string kind,
                                          const Predicate &pred) {
  // return constraints.lookup(kind).getValue();
  // return constraints.insert(constraints.Create(kind))->getValue();
  // if (constraints.find(pred) != constraints.end()) {
  //   return constraints[pred][kind];
  // }
  return constraints[&pred][kind];
}

std::vector<RLConstraint>
RLConstraintKit::addConstraint(const std::string kind, const ConsElem &lhs,
                                    const ConsElem &rhs, std::string info,
                                    const Predicate &pred) {
  if (lockedConstraintKinds[&pred].find(kind) !=
      lockedConstraintKinds[&pred].end()) {
    assert(false && "Have already started solving this kind and cannot add "
                    "more constraints.");
  }

  bool implicit = false;
  bool sink = false;
  if (kind == "default") {
    explicitRLConstraints++;
  }
  if (kind == "implicit") {
    implicitRLConstraints++;
    implicit = true;
  }
  if (kind == "default-sink") {
    explicitRLConstraints++;
    sink = true;
  }
  if (kind == "implicit-sink") {
    implicitRLConstraints++;
    implicit = true;
    sink = true;
  }

  std::vector<RLConstraint> &set = getOrCreateConstraintSet(kind, pred);
  std::vector<RLConstraint> retSet;

  assert(!llvm::isa<RLJoin>(&rhs) && "We shouldn't have joins on rhs!");

  if (const RLJoin *left = llvm::dyn_cast<RLJoin>(&lhs)) {
    std::set<const ConsElem *> elems = left->elements();
    for (auto elem = elems.begin(); elem != elems.end(); ++elem) {
      const RLConstraint c(**elem, rhs, pred, implicit, sink, info);
      set.push_back(c);
      retSet.push_back(c);
    }
  } else {
    RLConstraint c(lhs, rhs, pred, implicit, sink, info);
    set.push_back(c);
    retSet.push_back(c);
  }
  return retSet;
}

std::vector<RLConstraint>
RLConstraintKit::addConstraint(const std::string kind, const ConsElem &lhs,
                                    const ConsElem &rhs,
                                    const llvm::Value &value,
                                    const Predicate &pred) {
  return addConstraint(kind, lhs, rhs, "", pred);
}

std::vector<RLConstraint>
RLConstraintKit::addConstraint(const std::string kind, const ConsElem &lhs,
                                    const ConsElem &rhs,
                                    const Predicate &pred) {
  return addConstraint(kind, lhs, rhs, "", pred);
}

void RLConstraintKit::removeConstraintRHS(const std::string kind,
                                          const ConsElem &rhs,
                                          const Predicate &pred) {
  if (kind == "default")
    explicitRLConstraints--;
  if (kind == "implicit")
    implicitRLConstraints--;

  std::vector<RLConstraint> &set = getOrCreateConstraintSet(kind, pred);

  assert(!llvm::isa<RLJoin>(&rhs) && "We shouldn't have joins on rhs!");

  llvm::errs() << "Size of vector " << set.size() << "\n";
  llvm::errs() << "Constraint element to find ";
  rhs.dump(llvm::errs());
  llvm::errs() << "\n";

  for (auto vIt = set.begin(); vIt != set.end(); vIt++) {
    const ConsElem &constraintRight = *(*vIt).rhs();
    if (&rhs == &constraintRight) {
      llvm::errs() << "Constraint erased: ";
      (*vIt).dump(" => ");
      set.erase(vIt--);
    }
  }
}

ConsSoln *RLConstraintKit::leastSolution(const std::set<std::string> kinds,
                                         const Predicate &pred) {
  PartialSolution *PS = NULL;
  using namespace std::chrono;
  auto start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  for (auto kind : kinds) {
    if (!leastSolutions[&pred].count(kind)) {
      lockedConstraintKinds[&pred].insert(kind);
      leastSolutions[&pred][kind] =
          new PartialSolution(getOrCreateConstraintSet(kind, pred), false);
      // freeUnneededConstraints(kind, pred);
    }
    PartialSolution *P = leastSolutions[&pred][kind];

    if (!PS)
      PS = new PartialSolution(*P);
    else
      PS->mergeIn(*P);
  }
  auto end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  unsigned long long elapsed_seconds = end-start;
  std::cerr << "Actual solving time:" << elapsed_seconds << "\n";

  assert(PS && "No kinds given?");
  return PS;
}

ConsSoln *RLConstraintKit::greatestSolution(const std::set<std::string> kinds,
                                            const Predicate &pred) {
  PartialSolution *PS = NULL;
  for (std::set<std::string>::iterator kind = kinds.begin(), end = kinds.end();
       kind != end; ++kind) {
    if (!greatestSolutions[&pred].count(*kind)) {
      lockedConstraintKinds[&pred].insert(*kind);
      greatestSolutions[&pred][*kind] =
          new PartialSolution(getOrCreateConstraintSet(*kind, pred), true);
      freeUnneededConstraints(*kind, pred);
    }
    PartialSolution *P = greatestSolutions[&pred][*kind];

    if (!PS)
      PS = new PartialSolution(*P);
    else
      PS->mergeIn(*P);
  }
  assert(PS && "No kinds given?");
  return PS;
}

void RLConstraintKit::clearSolutions(const Predicate &pred) {
  lockedConstraintKinds[&pred].clear();
  leastSolutions[&pred].clear();
}

void RLConstraintKit::freeUnneededConstraints(std::string kind,
                                              const Predicate &pred) {
  // If we have the two kinds of PartialSolutions already generated
  // for this kind, then we no longer need the original constraints
  if (lockedConstraintKinds[&pred].count(kind) &&
      leastSolutions[&pred].count(kind) &&
      greatestSolutions[&pred].count(kind)) {
    // Clear out the constraints for this kind!
    getOrCreateConstraintSet(kind, pred).clear();
  }
}

void RLConstraintKit::copyConstraint(Predicate *src, Predicate *dest) {
  for (llvm::StringMap<std::vector<RLConstraint>>::iterator i =
           constraints[src].begin();
       i != constraints[src].end(); ++i) {
    std::vector<RLConstraint> &tempvector =
        getOrCreateConstraintSet(i->first(), *dest);
    std::vector<RLConstraint> &tempvector2 =
        getOrCreateConstraintSet(i->first(), *src);
    for (auto j : tempvector2) {
      RLConstraint temp(j);
      tempvector.push_back(temp);
    }
  }
  return;
}

void RLConstraintKit::partitionPredicateSet(std::vector<Predicate *> &P) {
  for (long unsigned i = 0; i < P.size(); i++) {
    long unsigned const size = P.size(); /* Increases efficiency, rather
     than performing below mentioned loop with range j < P.size(), as P's
     size changes inside the loop */
    for (long unsigned j = i + 1; j < size; j++) {
      int flag;
      if (P[i]->isOverlapping(P[j], &flag)) {
        std::vector<Predicate *> *newPredsArrayvector =
            P[i]->partitionPredicatePair(P[j], flag);
        std::vector<Predicate *> newPredsvector;
        switch (flag) {
        case 1: // P[j] is contained in P[i]
          newPredsvector = newPredsArrayvector[0];
          for (auto newPred : newPredsvector) {
            copyConstraint(P[i], newPred);
            P.push_back(newPred);
          }
          copyConstraint(P[i], P[j]);
          break;
        case -1: // P[i] is contained in P[j]
          newPredsvector = newPredsArrayvector[1];
          for (auto newPred : newPredsvector) {
            copyConstraint(P[j], newPred);
            P.push_back(newPred);
          }
          copyConstraint(P[j], P[i]);
          break;
        case 0: // P[i] and P[j] intersect, but are not subsets of each other
          newPredsvector = newPredsArrayvector[0];
          for (auto newPred : newPredsvector) {
            copyConstraint(P[i], newPred);
            P.push_back(newPred);
          }
          newPredsvector = newPredsArrayvector[1];
          for (auto newPred : newPredsvector) {
            copyConstraint(P[j], newPred);
            P.push_back(newPred);
          }
          newPredsvector = newPredsArrayvector[2];
          for (auto newPred : newPredsvector) {
            copyConstraint(P[i], newPred);
            copyConstraint(P[j], newPred);
            P.push_back(newPred);
          }
          break;
        default:
          break;
        }
      }
    }
  }

  // Remove the empty predicates (D. Figure out how to free empty predicates)
  for (long unsigned k = 0; k < P.size(); k++) {
    if (P[k]->isEmpty()) {
      P.erase(P.begin() + k);
      k--;
    }
  }

  // Sort the predicated constraints
  std::sort(P.begin(), P.end(), Predicate::predcompare);
}

void RLConstraintKit::setConstraints(std::set<RLConstraint> consSet,
                    const std::string kind,
                    const Predicate &pred) {
  constraints[&pred][kind].clear();
  std::vector<RLConstraint> &consVec = getOrCreateConstraintSet(kind, pred);
  consVec.insert(consVec.end(), consSet.begin(), consSet.end());
}

} // namespace deps
