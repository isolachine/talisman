//===-- RLConsSoln.cpp ------------------------------------------*- C++ -*-===//
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

#include "Constraints/RLConsSoln.h"
#include "Constraints/RLConstraint.h"
#include "Constraints/RLConstraintKit.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

namespace deps {

RLConsSoln::RLConsSoln(RLConstraintKit &kit, const RLConstant &defaultValue,
                       std::vector<const RLConstraint *> *constraints)
    : kit(kit), defaultValue(defaultValue), constraints(constraints),
      solved(false) {}

const RLConstant &RLConsSoln::subst(const ConsElem &elem) {
  solve();
  if (const RLConsVar *var = llvm::dyn_cast<RLConsVar>(&elem)) {
    const RLConstant &other = (defaultValue == RLConstant::bot())
                                  ? RLConstant::top()
                                  : RLConstant::bot();
    return changed.count(var) ? other : defaultValue;
  } else if (const RLConstant *var = llvm::dyn_cast<RLConstant>(&elem)) {
    return *var;
  } else if (const RLJoin *join = llvm::dyn_cast<RLJoin>(&elem)) {
    const std::set<const ConsElem *> &elements = join->elements();
    const RLConstant *substVal = &defaultValue;
    for (std::set<const ConsElem *>::iterator elem = elements.begin(),
                                              end = elements.end();
         elem != end; ++elem) {
      substVal = &(substVal->join(subst(**elem)));
    }
    return *substVal;
  } else {
    assert("Unknown security policy");
  }
  llvm_unreachable("Unknown security policy");
}

void RLConsSoln::enqueueConstraints(
    const std::vector<const RLConstraint *> &constraints) {
  for (std::vector<const RLConstraint *>::const_iterator
           elem = constraints.begin(),
           end = constraints.end();
       elem != end; ++elem) {
    if (queueSet.insert(*elem).second)
      queue.push_back(*elem);
  }
}

const RLConstraint &RLConsSoln::dequeueConstraint() {
  const RLConstraint &front = *(queue.front());
  queue.pop_front();
  queueSet.erase(&front);
  return front;
}

void RLConsSoln::solve(void) {
  if (solved)
    return;

  llvm::errs() << "Solving ";
  llvm::errs() << constraints->size();
  llvm::errs() << " constraints\n";
  solved = true;
  // Add all of the constraint to the queue
  enqueueConstraints(*constraints);

  unsigned int number = 0;
  while (!queue.empty()) {
    number++;
    const RLConstraint &c = dequeueConstraint();
    const ConsElem *left = c.lhs();
    const ConsElem *right = c.rhs();
    if (subst(*left).leq(subst(*right))) {
      // The constraint is already satisfied!
    } else {
      // Need to satisfy the constraint
      satisfyConstraint(c, *left, *right);
    }
  }
  llvm::errs() << "Solved after ";
  llvm::errs() << number;
  llvm::errs() << " iterations\n";

  // Free contrainsts and other unneeded datastructures
  delete constraints;
  constraints = NULL;
  releaseMemory();
}

RLConsSoln::~RLConsSoln() { delete constraints; }

RLConsLeastSoln::RLConsLeastSoln(RLConstraintKit &kit,
                                 std::vector<const RLConstraint *> *constraints)
    : RLConsSoln(kit, RLConstant::bot(), constraints) {
  std::set<const ConsVar *> leftVariables;
  for (std::vector<const RLConstraint *>::iterator cons = constraints->begin(),
                                                   end = constraints->end();
       cons != end; ++cons) {
    leftVariables.clear();
    (*cons)->lhs()->variables(leftVariables);

    for (std::set<const ConsVar *>::iterator var = leftVariables.begin(),
                                             end = leftVariables.end();
         var != end; ++var) {
      addInvalidIfIncreased(*var, *cons);
    }
  }
}

void RLConsLeastSoln::satisfyConstraint(const RLConstraint &constraint,
                                        const ConsElem &left,
                                        const ConsElem &right) {
  std::set<const ConsVar *> vars;
  right.variables(vars);
  const RLConstant &L = subst(left);
  for (std::set<const ConsVar *>::iterator var = vars.begin(), end = vars.end();
       var != end; ++var) {
    const RLConstant &R = subst(**var);
    if (!L.leq(R)) {
      changed.insert(*var);
      // add every constraint that may now be invalidated by changing var
      enqueueConstraints(getOrCreateInvalidIfIncreasedSet(*var));
    }
  }
}

std::vector<const RLConstraint *> &
RLConsLeastSoln::getOrCreateInvalidIfIncreasedSet(const ConsVar *var) {
  return invalidIfIncreased[var];
}

void RLConsLeastSoln::addInvalidIfIncreased(const ConsVar *var,
                                            const RLConstraint *c) {
  std::vector<const RLConstraint *> &varSet =
      getOrCreateInvalidIfIncreasedSet(var);
  varSet.push_back(c);
}

RLConsGreatestSoln::RLConsGreatestSoln(
    RLConstraintKit &kit, std::vector<const RLConstraint *> *constraints)
    : RLConsSoln(kit, RLConstant::top(), constraints) {
  std::set<const ConsVar *> rightVariables;
  for (std::vector<const RLConstraint *>::iterator cons = constraints->begin(),
                                                   end = constraints->end();
       cons != end; ++cons) {
    rightVariables.clear();
    (*cons)->rhs()->variables(rightVariables);

    for (std::set<const ConsVar *>::iterator var = rightVariables.begin(),
                                             end = rightVariables.end();
         var != end; ++var) {
      addInvalidIfDecreased(*var, *cons);
    }
  }
}

void RLConsGreatestSoln::satisfyConstraint(const RLConstraint &constraint,
                                           const ConsElem &left,
                                           const ConsElem &right) {
  std::set<const ConsVar *> vars;
  left.variables(vars);
  const RLConstant &R = subst(right);
  for (std::set<const ConsVar *>::iterator var = vars.begin(), end = vars.end();
       var != end; ++var) {
    const RLConstant &L = subst(**var);
    if (L.leq(R)) {
      // nothing to do, the variable is already low enough
    } else if (R.leq(L)) {
      // lower the variable var
      changed.insert(*var);
      // add every constraint that may now be invalidated by changing var
      enqueueConstraints(getOrCreateInvalidIfDecreasedSet(*var));
    } else {
      assert(false && "Meets not supported yet... not sure what to do...");
    }
  }
}

std::vector<const RLConstraint *> &
RLConsGreatestSoln::getOrCreateInvalidIfDecreasedSet(const ConsVar *var) {
  return invalidIfDecreased[var];
}

void RLConsGreatestSoln::addInvalidIfDecreased(const ConsVar *var,
                                               const RLConstraint *c) {
  std::vector<const RLConstraint *> &varSet =
      getOrCreateInvalidIfDecreasedSet(var);
  varSet.push_back(c);
}
} // namespace deps
