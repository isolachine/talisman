//===-- constraints/RLConstraintKit.h - RL Lattice Solver -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a concrete constraint solver for solving constraints
// over the multi-level lattice.
//
//===----------------------------------------------------------------------===//

#ifndef RLCONSTRAINTKIT_H_
#define RLCONSTRAINTKIT_H_

#include "Constraints/ConstraintKit.h"
#include "Constraints/PredicatedConstraints.h"
#include "Constraints/RLConstraint.h"
// #include "Constraints/RLConstraints.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Value.h"

#include <map>
#include <set>
#include <vector>

namespace deps {

class RLConstant;
class RLConsVar;
class RLJoin;
class PartialSolution;

/// Singleton, concrete implementation of ConstraintKit for creating and
/// solving constraints over a two level lattice.
class RLConstraintKit : public ConstraintKit {
public:
  static RLConstraintKit* get() {
    if (nullptr == singleton) singleton = new RLConstraintKit;
    return singleton;
  }

  RLConstraintKit();
  ~RLConstraintKit();
  /// Get a reference to the constant "low" element of the lattice
  const ConsElem &botConstant() const;
  /// Get a reference to the constant "high" element of the lattice
  const ConsElem &topConstant() const;

  const ConsElem &constant(RLLevel l, RLCompartment c) const;
  const ConsElem &constant(RLLabel) const;

  /// Create a new constraint variable
  virtual const ConsVar &newVar(const std::string description,
                                const std::string metainfo = "");
  /// Create a new constraint element by taking the upper bound of two
  /// existing elements
  virtual const ConsElem &upperBound(const ConsElem &e1, const ConsElem &e2);
  /// Create a new constraint element by taking the upper bound of two
  /// existing elements. Arguments and return type may be null.
  virtual const ConsElem *upperBound(const ConsElem *e1, const ConsElem *e2);
  /// Create a new constraint element by taking the upper bound of the
  /// given set of elements.
  virtual const ConsElem &upperBound(std::set<const ConsElem *> elems);

  /// Add the constraint lhs <= rhs to the set "kind"
  virtual std::vector<RLConstraint> addConstraint(const std::string kind, const ConsElem &lhs,
                             const ConsElem &rhs,
                             const Predicate &pred = truePredicate());
  std::vector<RLConstraint> addConstraint(const std::string kind, const ConsElem &lhs,
                     const ConsElem &rhs, std::string info,
                     const Predicate &pred = truePredicate());
  std::vector<RLConstraint> addConstraint(const std::string kind, const ConsElem &lhs,
                     const ConsElem &rhs, const llvm::Value &value,
                     const Predicate &pred = truePredicate());
  virtual void removeConstraintRHS(const std::string kind, const ConsElem &rhs,
                                   const Predicate &pred = truePredicate());

  /// Find the lfp of the constraints in the "kinds" sets
  /// Unconstrained variables will be "Low" (caller delete)
  virtual ConsSoln *leastSolution(const std::set<std::string> kinds,
                                  const Predicate &pred = truePredicate());
  /// Find the gfp of the constraints in the "kinds" sets
  /// Unconstrained variables will be "High" (caller delete)
  virtual ConsSoln *greatestSolution(const std::set<std::string> kinds,
                                     const Predicate &pred = truePredicate());
  void clearSolutions(const Predicate &pred = truePredicate());
  /// return the vars and joins
  std::vector<const RLConsVar *> getVars() { return vars; }
  std::set<RLJoin> &getJoins() { return joins; }

  // Compute both least and greatest solutions simultaneously
  // for the given kind.
  void solveMT(std::string kind, Predicate *pred);
  // Solve the given kinds in parallel (per thread limit)
  std::vector<PartialSolution *> solveLeastMT(std::vector<std::string> kinds,
                                              bool useDefaultSinks,
                                              const Predicate *pred);
  std::vector<RLConstraint> &
  getOrCreateConstraintSet(const std::string kind,
                           const Predicate &pred = truePredicate());

  void copyConstraint(Predicate *src, Predicate *dest);

  void partitionPredicateSet(std::vector<Predicate *> &P);

  static const Predicate &truePredicate();

  void setConstraints(std::set<RLConstraint> consSet,
                      const std::string kind,
                      const Predicate &pred = truePredicate());

private:
  static RLConstraintKit *singleton;

  // "defult" "default-sinks" "implicit" "implicit-sinks"
  // llvm::StringMap<std::vector<RLConstraint>> constraints;
  std::map<const Predicate *, llvm::StringMap<std::vector<RLConstraint>>>
      constraints;
  std::map<const Predicate *, std::set<std::string>> lockedConstraintKinds;
  // std::map<Predicate *, std::set<std::string>> lockedConstraintKinds;

  std::vector<const RLConsVar *> vars;
  std::set<RLJoin> joins;

  // Cached solutions for each kind
  // llvm::StringMap<PartialSolution *> leastSolutions;
  // llvm::StringMap<PartialSolution *> greatestSolutions;
  std::map<const Predicate *, llvm::StringMap<PartialSolution *>>
      leastSolutions;
  std::map<const Predicate *, llvm::StringMap<PartialSolution *>>
      greatestSolutions;

  void freeUnneededConstraints(std::string kind,
                               const Predicate &pred = truePredicate());
};

} // namespace deps

#endif /* RLCONSTRAINTKIT_H_ */
