//===- PointsToInterface.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of the basic points-to interface.
//
//===----------------------------------------------------------------------===//

#include "PointsToInterface.h"

#include "dsa/DSGraphTraits.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/DepthFirstIterator.h"

namespace deps {

static RegisterPass<deps::PointsToInterface>
X("pointstointerface", "Basic points-to interface");

char PointsToInterface::ID;

const AbstractLocSet PointsToInterface::EmptySet;

//
// To preserve soundness, we need to go over the computed equivalence classes
// and merge together those which contain incomplete DSNodes. We don't merge
// the classes explicitly, but only merge the sets of of leader nodes of the
// classes.
//
void PointsToInterface::mergeAllIncomplete() {
  // This node is a representative of an equivalence class which contains an
  // incomplete node.
  const DSNode *IncompleteRepresentative = 0;

  // Iterate over all nodes in the classes, and find and merge the class
  // leaders for those nodes which are incomplete.
  EquivalenceClasses<const DSNode *>::iterator EqIt = Classes->begin();
  EquivalenceClasses<const DSNode *>::iterator EqItEnd = Classes->end();
  for (; EqIt != EqItEnd; ++EqIt) {
    // Insert all leaders into the merged leader equivalence classes.
    if (EqIt->isLeader())
      MergedLeaders.insert(EqIt->getData());

    // Get the DSNode and check if the node is incomplete.
    //
    // DZ: It doesn't seem like a good idea to merge all external nodes. I
    // understand the worry about unknown/incomplete nodes, but merging
    // external nodes does not make too much sense. So I have disabled the
    // merging for external nodes 
    const DSNode *N = EqIt->getData();
    if (!N->isIncompleteNode() && /* !N->isExternalNode() &&*/ !N->isUnknownNode())
      continue;

    // Get the leader of this node's class.
    const DSNode *Leader = Classes->getLeaderValue(N);
    if (IncompleteRepresentative == 0)
      IncompleteRepresentative = Leader;

    // Merge the leader with the class that contains other leaders with
    // incomplete class members.
    MergedLeaders.unionSets(Leader, IncompleteRepresentative);
  }
}

bool PointsToInterface::runOnModule(Module &M) {
  EquivsAnalysis = &getAnalysis<DSNodeEquivs>();
  Classes = &EquivsAnalysis->getEquivalenceClasses();
  mergeAllIncomplete();

  // Does not modify module.
  return false;
}

//
// Given a value in the program, returns a pointer to a set of abstract
// locations that the value points to.
//
const AbstractLocSet *
PointsToInterface::getAbstractLocSetForValue(const Value *V) {
  const DSNode *MergedLeader = getMergedLeaderForValue(V);

  // If the class for the value doesn't exist, return the empty set.
  if (MergedLeader == 0)
    return &EmptySet;

  // Find or build the set for the merged class leader.
  if (ClassForLeader.find(MergedLeader) == ClassForLeader.end())
    ClassForLeader[MergedLeader].insert(MergedLeader);

  return &ClassForLeader[MergedLeader];
}

const AbstractHandleSet *
PointsToInterface::getAbstractHandleSetForValue(const Value *V) {
  // get the offset of the handle that pointes to the DSNode
  unsigned Offset = 0;
  const DSNode *MergedLeader = getMergedLeaderForValue(V, &Offset);

  // Commented out because MergedLeader is likely in EC
  // if(*Offset < MergedLeader->getSize() && MergedLeader->hasLink(*Offset))
  // HandlesForLeader[MergedLeader].insert(&MergedLeader->getLink(*Offset));
  const AbstractLocSet *Result = getAbstractLocSetForValue(V);

  //errs() << "Offset: " << *Offset << "\n";

  AbstractLocSet::iterator nodeIt = Result->begin();
  AbstractLocSet::iterator nodeEnd = Result->end();
  Offset = 0;
  for (; nodeIt != nodeEnd; ++nodeIt) {
    const DSNode *Node = *nodeIt;
    //(*nodeIt)->dump();
    if(Offset < Node->getSize() && Node->hasLink(Offset)) {
      HandlesForLeader[MergedLeader].insert(&Node->getLink(Offset));
    }
  }

    /*
  EquivalenceClasses<const DSNode *>::member_iterator
    ClassesIt = Classes->member_begin(Classes->findValue(MergedLeader)),
    ClassesItEnd = Classes->member_end();


  // Loop through the merged class nodes and if they have a link at the specfied
  // offset add that DSHandle to the set
  for (; ClassesIt != ClassesItEnd; ++ClassesIt) {
    const DSNode *Node = *ClassesIt;
    if(*Offset < Node->getSize())
      errs() << "Node->Haslink:"  << Node->hasLink(*Offset) << "\n";

    if(*Offset < Node->getSize() && Node->hasLink(*Offset)) {
      HandlesForLeader[MergedLeader].insert(&Node->getLink(*Offset));
    }
  }

    */
  //errs() << "Length of set: " << HandlesForLeader[MergedLeader].size() << "\n";

  return &HandlesForLeader[MergedLeader];
}

//
// Given a value in the program, returns a pointer to a set of abstract
// locations that are reachable from the value.
//
const AbstractLocSet *
PointsToInterface::getReachableAbstractLocSetForValue(const Value *V) {
  const DSNode *MergedLeader = getMergedLeaderForValue(V);

  // If the class for the value doesn't exist, return the empty set.
  if (MergedLeader == 0)
    return &EmptySet;

  // Check if the reachable set has been computed.
  if (ReachablesForLeader.find(MergedLeader) != ReachablesForLeader.end())
    return &ReachablesForLeader[MergedLeader];

  // Otherwise, for each element in each equivalence class merged in with
  // MergedLeader, we need to compute its reachable set.
  AbstractLocSet ReachableSet;

  EquivalenceClasses<const DSNode *>::member_iterator
    MIt = MergedLeaders.member_begin(MergedLeaders.findValue(MergedLeader)),
    MItEnd = MergedLeaders.member_end();

  for (; MIt != MItEnd; ++MIt) {
    const DSNode *Leader = *MIt;

    EquivalenceClasses<const DSNode *>::member_iterator
      ClassesIt = Classes->member_begin(Classes->findValue(Leader)),
      ClassesItEnd = Classes->member_end();

    for (; ClassesIt != ClassesItEnd; ++ClassesIt) {
      const DSNode *Node = *ClassesIt;
      findReachableAbstractLocSetForNode(ReachableSet, Node);
    }
  }

  // ReachableSet now contains all DSNodes reachable from V. We cut this down
  // to the set of merged equivalence class leaders of these nodes.
  AbstractLocSet &Result = ReachablesForLeader[MergedLeader];
  AbstractLocSet::iterator ReachableIt = ReachableSet.begin();
  AbstractLocSet::iterator ReachableEnd = ReachableSet.end();
  for (; ReachableIt != ReachableEnd; ++ReachableIt) {
    const DSNode *Node = *ReachableIt;
    const DSNode *ClassLeader = Classes->getLeaderValue(Node);
    const DSNode *MergedLeader = MergedLeaders.getLeaderValue(ClassLeader);
    Result.insert(MergedLeader);
  }

  return &Result;
}

//
// Converts the Reachable set of AbstractLocs to a set of AbstractHandles
//
const AbstractHandleSet *
PointsToInterface::getReachableAbstractHandleSetForValue(const Value *V) {
  unsigned Offset = 0;
  const DSNode *MergedLeader = getMergedLeaderForValue(V, &Offset);

  // If the class for the value doesn't exist, return the empty set.
  //if (MergedLeader == 0)
  //return &EmptyHandleSet;

  if (ReachableHandlesForLeader.find(MergedLeader) != ReachableHandlesForLeader.end()){
    return &ReachableHandlesForLeader[MergedLeader];
  }

  const AbstractLocSet* reachableNodes = getReachableAbstractLocSetForValue(V);

  AbstractLocSet::iterator reachIt = reachableNodes->begin();
  AbstractLocSet::iterator reachEnd = reachableNodes->end();
  for(; reachIt != reachEnd; ++reachIt){
    DSNode::const_edge_iterator edgeIt = (*reachIt)->edge_begin();
    DSNode::const_edge_iterator edgeEnd = (*reachIt)->edge_end();
    for(; edgeIt != edgeEnd; ++edgeIt){
      const DSNodeHandle * link = &(*edgeIt).second;
      if (link != NULL) {
        ReachableHandlesForLeader[MergedLeader].insert(link);
      }
    }
  }

  return &HandlesForLeader[MergedLeader];
  ///return &ReachableHandlesForLeader[MergedLeader];
}

//
// Return the DSNode that represents the leader of the given value's DSNode
// equivalence class after the classes have been merged to account for
// incomplete nodes. Returns null if the value's DSNode doesn't exist.
//
const DSNode *
PointsToInterface::getMergedLeaderForValue(const Value *V, unsigned* offset) {
  const DSNode *Node;

  // print out information from value.
  // raw_string_ostream* valueStream;
  // std::string valueString;
  // valueStream = new raw_string_ostream(valueString);
  // *valueStream << *V;
  // errs() <<"Value from getMergedLeader: " <<  valueStream->str() << "\n";

  if (LeaderForValue.count(V))
    return LeaderForValue[V];

  // Get the node for V and return null if it doesn't exist.
  Node = EquivsAnalysis->getMemberForValue(V, offset);
  if (Node == 0)
    return LeaderForValue[V] = 0;

  // Search for the equivalence class of Node.
  assert(Classes->findValue(Node) != Classes->end() && "Class not found!");
  const DSNode *NodeLeader = Classes->getLeaderValue(Node);
  const DSNode *MergedLeader = MergedLeaders.getLeaderValue(NodeLeader);

  return LeaderForValue[V] = MergedLeader;
}


//
// Return the offset of the value in the DSNode if possible.
// getMErgedLeaderFromValue can return NULL. If no DSNode is retrieved do not
// use the value stored in the offset
//
bool
PointsToInterface::getOffsetForValue(const Value * V, unsigned *Offset) {
  const DSNode * N = getMergedLeaderForValue(V, Offset);
  if (N != NULL)
    return true;
  return false;
}

//
// Add to the given set all nodes reachable from the given DSNode that are not
// in the set already.
//
void
PointsToInterface::findReachableAbstractLocSetForNode(AbstractLocSet &Set,
                                                      const DSNode *Node) {
  df_ext_iterator<const DSNode *> DFIt = df_ext_begin(Node, Set);
  df_ext_iterator<const DSNode *> DFEnd = df_ext_end(Node, Set);
  for (; DFIt != DFEnd; ++DFIt)
    ;
}

//
// Calculates if the number of links from the node is greater than 0
// by copying the map from exposed iterators.
bool
PointsToInterface::nodeHasLinks(const DSNode *node){
  if (node == NULL)
    return false;

  DSNode::LinkMapTy links{node->edge_begin(), node->edge_end()};
  if(links.size() > 0)
    return true;

  return false;
}


//
// Returns the edge map in a way that can be used in range-based for-loops
DSNode::LinkMapTy
PointsToInterface::getLinks(const DSNode* node){
  if (node == NULL){
    return DSNode::LinkMapTy();
  }
  DSNode::LinkMapTy links{node->edge_begin(), node->edge_end()};
  return links;
}


Type *
PointsToInterface::findWidestType(const DSNode & node, SuperSet<Type*>::setPtr types){
  const DataLayout &TD = node.getParentGraph()->getDataLayout();
  Type * n = NULL;
  unsigned maxWidth = 0;
  for(auto it = types->begin(); it != types->end(); ++it){
    unsigned size = TD.getTypeStoreSize(*it);
    if (size > maxWidth){
      n = *it;
      maxWidth = size;
    }
  }
  return n;
}

}
