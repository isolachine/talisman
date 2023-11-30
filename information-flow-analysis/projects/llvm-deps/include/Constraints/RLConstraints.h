//===-- constraints/RLConstraints - RL Constraint Classes -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares concrete classes of constraint elements for use with
// the RLConstraintKit.
//
//===----------------------------------------------------------------------===//

#ifndef RLCONSTRAINTS_H_
#define RLCONSTRAINTS_H_

#include "Constraints/ConstraintKit.h"
#include <map>
#include <string>

namespace deps {

class RLConstant;

typedef std::map<std::string, std::vector<std::string>> LevelMap;
typedef std::map<std::string, std::set<std::string>> CompartmentMap;
typedef std::map<std::string, int> RLLevel;
typedef CompartmentMap RLCompartment;
typedef std::pair<RLLevel, RLCompartment> RLLabel;
typedef std::map<RLLabel, RLConstant *> RLConstantMap;

/// Singleton constants in the lattice
class RLConstant : public ConsElem {
public:
  /// Get a reference to the constant (do not delete!)
  static const RLConstant &bot();
  static const RLConstant &top();

  static const RLConstant &constant(RLLevel l, RLCompartment c);
  static const RLConstant &constant(RLLabel label);

  /// Compare with another constraint element.
  /// False if element is not an RLConstant.
  virtual bool leq(const ConsElem &elem) const;

  /// Returns the empty set of constraint variables.
  virtual void variables(std::set<const ConsVar *> &) const {}

  /// Returns the least upper bound of two members of the lattice
  virtual const RLConstant &join(const RLConstant &other) const;

  virtual const RLLabel upperBoundLabel(const RLConstant &other) const;
  virtual const RLLabel lowerBoundLabel(const RLConstant &other) const;

  static const RLLabel upperBoundLabel(RLLabel label, RLLabel other);
  static const RLLabel lowerBoundLabel(RLLabel label, RLLabel other);

  virtual const RLLabel label() const;

  virtual bool operator==(const ConsElem &c) const;

  virtual void dump(llvm::raw_ostream &o) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual DepsType type() const { return DT_RLConstant; }
  static inline bool classof(const RLConstant *) { return true; }
  static inline bool classof(const ConsElem *elem) {
    return elem->type() == DT_RLConstant;
  }

  static void lockLattice() { latticeLock = true; }
  static bool isLatticeLocked() { return latticeLock; }
  static void dump_lattice(llvm::raw_ostream &o);
  static RLLabel parseLabelString(std::string);

  static CompartmentMap RLCompartmentMap;
  static LevelMap RLLevelMap;
  static RLLabel TopLabel;
  static RLLabel BotLabel;

protected:
  RLConstant(RLLevel l, RLCompartment c);
  const RLLevel level;
  const RLCompartment compartment;
  static RLConstantMap constants;

private:
  static bool latticeLock;
};

/// Concrete implementation of constraint variables for use with
/// RLConstraintKit.
class RLConsVar : public ConsVar {
public:
  /// Create a new variable with description
  RLConsVar(const std::string desc, const std::string meta);
  /// Compare two elements for constraint satisfaction
  virtual bool leq(const ConsElem &elem) const;
  /// Returns the singleton set containing this variable
  virtual void variables(std::set<const ConsVar *> &set) const {
    set.insert(this);
  }
  virtual bool operator==(const ConsElem &c) const;
  virtual void dump(llvm::raw_ostream &o) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual DepsType type() const { return DT_RLConsVar; }
  static inline bool classof(const RLConsVar *) { return true; }
  static inline bool classof(const ConsVar *var) {
    return var->type() == DT_RLConsVar;
  }
  static inline bool classof(const ConsElem *elem) {
    return elem->type() == DT_RLConsVar;
  }

  const std::string getDesc() const { return desc; }
  const std::string getMeta() const { return meta; }

private:
  RLConsVar(const RLConsVar &);
  RLConsVar &operator=(const RLConsVar &);
  const std::string desc;
  const std::string meta;
  std::string replaced;
};

/// Constraint element representing the join of L-H lattice elements.
class RLJoin : public ConsElem {
public:
  /// Returns true if all of the elements of the join are leq(elem)
  virtual bool leq(const ConsElem &elem) const;
  /// Returns set of sub-elements that are constraint variables
  virtual void variables(std::set<const ConsVar *> &set) const;
  /// Create a new constraint element by joining two existing constraints
  /// (caller delete)
  static const RLJoin *create(const ConsElem &e1, const ConsElem &e2);
  /// Returns the set of elements joined by this element
  const std::set<const ConsElem *> &elements() const { return elems; }
  virtual bool operator==(const ConsElem &c) const;
  bool operator<(const RLJoin &c) const {
    if (elems.size() != c.elems.size())
      return elems.size() < c.elems.size();
    return elems < c.elems;
  }
  virtual void dump(llvm::raw_ostream &o) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual DepsType type() const { return DT_RLJoin; }
  static inline bool classof(const RLJoin *) { return true; }
  static inline bool classof(const ConsElem *elem) {
    return elem->type() == DT_RLJoin;
  }
  RLJoin(std::set<const ConsElem *> elems);

private:
  const std::set<const ConsElem *> elems;
};

} // namespace deps

#endif /* RLCONSTRAINTS_H_ */
