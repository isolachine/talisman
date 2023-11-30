//===------- PointsToInterface.h Simple points-to analysis interface ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a simple interface for querying points-to information.
//
//===----------------------------------------------------------------------===//


#ifndef POINTSTOINTERFACE_H_
#define POINTSTOINTERFACE_H_

#include "dsa/DataStructure.h"
#include "dsa/DSNodeEquivs.h"

#include "llvm/Pass.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/EquivalenceClasses.h"

#include <set>
#include <map>

using namespace llvm;

namespace deps {

//
// An abstract memory location that represents the target of a pointer.
//
typedef DSNode AbstractLoc;
typedef DSNodeHandle AbstractHandle;

typedef std::set<const AbstractLoc *> AbstractLocSet;
typedef std::set<const AbstractHandle *> AbstractHandleSet;

//
// This pass provides an interface to a points-to analysis (currently DSA) that
// returns sets of abstracts locations for pointer expressions.
//
class PointsToInterface : public ModulePass {
private:
  static const AbstractLocSet EmptySet;
  //static const AbstractHandleSet EmptyHandleSet;
  

  std::map<const DSNode *, AbstractLocSet> ClassForLeader;
  std::map<const DSNode *, AbstractLocSet> ReachablesForLeader;
  DenseMap<const Value*, const DSNode*> LeaderForValue;

  std::map<const DSNode *, AbstractHandleSet> HandlesForLeader;
  std::map<const DSNode *, AbstractHandleSet> ReachableHandlesForLeader;

  EquivalenceClasses<const DSNode *> MergedLeaders;

  const EquivalenceClasses<const DSNode *> *Classes;
  DSNodeEquivs *EquivsAnalysis;

  void mergeAllIncomplete();
  const DSNode *getMergedLeaderForValue(const Value *V, unsigned* offset=NULL);
  void findReachableAbstractLocSetForNode(AbstractLocSet &S, const DSNode *Nd);

public:
  static char ID;

  PointsToInterface() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<DSNodeEquivs>();
    AU.setPreservesAll();
  }

  //
  // For a given value in the module, returns a pointer to a set representing
  // the set of abstract memory locations that the value can point to.
  //
  const AbstractLocSet *getAbstractLocSetForValue(const Value *V);
  const AbstractHandleSet *getAbstractHandleSetForValue(const Value *V);
  bool getOffsetForValue(const Value * V, unsigned * Offset);

  //
  // For a given value in the module, returns the set of all abstract memory
  // locations reachable from that value.
  //
  const AbstractLocSet *getReachableAbstractLocSetForValue(const Value *V);
  const AbstractHandleSet *getReachableAbstractHandleSetForValue(const Value *V);

  //
  // For a given node, see if there are links to other nodes
  // Returns true if there are other nodes which the argument node links to
  // False if the node is of size 0 or does not link to a node.
  //
  bool nodeHasLinks(const DSNode * node);

  //
  // Returns the Link Map from the exposed edge_iterators from DSNode
  //
  DSNode::LinkMapTy getLinks(const DSNode* node);

  //
  // Traverses type map returning the type which represents the widest data type
  // within the set
  //
  Type * findWidestType(const DSNode & node, SuperSet<Type*>::setPtr start);

};

}

#endif
