//===-- RLConsSoln.h --------------------------------------------*- C++ -*-===//
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

#ifndef RLCONSSOLN_H_
#define RLCONSSOLN_H_

#include "Constraints/ConstraintKit.h"
#include "Constraints/RLConstraints.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace deps {

class RLConstraint;
class RLConstant;
class RLConstraintKit;

class RLConsSoln : public ConsSoln {
public:
  RLConsSoln(RLConstraintKit &kit, const RLConstant &defaultValue,
             std::vector<const RLConstraint *> *constraints);
  virtual const RLConstant &subst(const ConsElem &elem);

protected:
  RLConstraintKit &kit;

  const RLConstant &defaultValue;
  std::vector<const RLConstraint *> *constraints;

  std::deque<const RLConstraint *> queue;
  llvm::DenseSet<const RLConstraint *> queueSet;

  llvm::DenseSet<const ConsVar *> changed;
  bool solved;

  void solve(void);
  void enqueueConstraints(const std::vector<const RLConstraint *> &constraints);
  const RLConstraint &dequeueConstraint(void);

  virtual ~RLConsSoln();
  virtual void releaseMemory() = 0;
  virtual void satisfyConstraint(const RLConstraint &constraint,
                                 const ConsElem &left,
                                 const ConsElem &right) = 0;
};

class RLConsGreatestSoln : public RLConsSoln {
  llvm::DenseMap<const ConsVar *, std::vector<const RLConstraint *>>
      invalidIfDecreased;
  std::vector<const RLConstraint *> &
  getOrCreateInvalidIfDecreasedSet(const ConsVar *var);
  void addInvalidIfDecreased(const ConsVar *var, const RLConstraint *c);

public:
  RLConsGreatestSoln(RLConstraintKit &kit,
                     std::vector<const RLConstraint *> *constraints);

protected:
  virtual void satisfyConstraint(const RLConstraint &constraint,
                                 const ConsElem &left, const ConsElem &right);
  virtual void releaseMemory() { invalidIfDecreased.clear(); }
};

class RLConsLeastSoln : public RLConsSoln {
  llvm::DenseMap<const ConsVar *, std::vector<const RLConstraint *>>
      invalidIfIncreased;
  std::vector<const RLConstraint *> &
  getOrCreateInvalidIfIncreasedSet(const ConsVar *var);
  void addInvalidIfIncreased(const ConsVar *var, const RLConstraint *c);

public:
  RLConsLeastSoln(RLConstraintKit &kit,
                  std::vector<const RLConstraint *> *constraints);

protected:
  virtual void satisfyConstraint(const RLConstraint &constraint,
                                 const ConsElem &left, const ConsElem &right);
  virtual void releaseMemory() { invalidIfIncreased.clear(); }
};

} // namespace deps

#endif /* RLCONSSOLN_H_ */
