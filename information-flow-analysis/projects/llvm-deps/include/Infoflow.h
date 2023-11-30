//===- Infoflow.h -----------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a constraint-based interprocedural (2-call site sensitive)
// information flow analysis for a two-level security lattice
// (Untainted--Tainted). While the analysis is context-sensitive, the Infoflow
// pass interface is not.
//
//===----------------------------------------------------------------------===//

#ifndef INFOFLOW_H
#define INFOFLOW_H

#include "CallContext.h"
#include "CallSensitiveAnalysisPass.h"
#include "Constraints/RLConsSoln.h"
#include "Constraints/RLConstraintKit.h"
#include "FlowRecord.h"
#include "InfoflowSignature.h"
#include "PointsToInterface.h"
#include "SignatureLibrary.h"
#include "SourceSinkAnalysis.h"
#include "TaintAnalysisBase.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

#include "json/json.hpp"

#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <tuple>
#include <utility>

using json = nlohmann::json;

#ifndef DEBUG_TYPE_CONSTRAINT
#define DEBUG_TYPE_CONSTRAINT "constraint"
#endif
#define DEBUG_TYPE_CALLINST "callinst"
#define DEBUG_TYPE_METAINFO "meta"
#define DEBUG_TYPE_FLOW "flow"
#define DEBUG_TYPE_DEBUG "debug"
#define DEBUG_TYPE_PROFILE "profile"
#define DEBUG_TYPE_TAINT "taint"
#define DEBUG_TYPE_CONSTANT "constant"

#define IMPLICIT 1
#define HOTSPOT 0

namespace deps {

using namespace llvm;

/// Unit is the interprocedural analysis input type.
/// The analysis uses unit since each function should only
/// be analyzed once per context.
class Unit {
public:
  Unit(){};
  ~Unit(){};
  const Unit &operator=(const Unit &u) { return u; }
  bool operator<=(const Unit &) const { return true; }
  bool operator==(const Unit &) const { return true; }
  bool operator!=(const Unit &) const { return false; }
  const Unit upperBound(const Unit &u) const { return u; }
};

enum class ConfigVariableType { Argument, Variable, Constant };

class ConfigVariable {
public:
  std::string function;
  ConfigVariableType type;
  std::string name;
  int number;
  int index;
  std::string file;
  unsigned int line;
  long value; // field storing value of the constant
  RLLabel label;
  ContextID ctxt;       // field added by the analysis for sink labeling
  Value *val;           // field added by the analysis for sink labeling
  std::string callsite; // field added by the analysis for sink labeling

  ConfigVariable() = default;
  ConfigVariable(std::string fn, ConfigVariableType ty, std::string name,
                 int num, int idx, std::string file, int line, int value,
                 RLLabel label)
      : function(fn), type(ty), name(name), number(num), index(idx), file(file),
        line(line), value(value), label(label) {}
  ConfigVariable(std::string fn, std::string name, int idx)
      : function(fn), name(name), index(idx) {}
  ~ConfigVariable(){};

  bool operator< (const ConfigVariable &var) const {
    if (function < var.function) {
      return true;
    } else if (function == var.function && name < var.name) {
      return true;
    } else if (function == var.function && name == var.name && index < var.index) {
      return true;
    } else {
      return false;
    }
  }
};

class Infoflow;

/// InfoflowSolution wraps a constraint set solution with
/// the information required to extract taint summaries for
/// values and locations.
class InfoflowSolution {
public:
  InfoflowSolution(Infoflow &infoflow, ConsSoln *s, const ConsElem &top,
                   const ConsElem &bot, bool defaultTainted,
                   DenseMap<const Value *, const ConsElem *> &valueMap,
                   DenseMap<const AbstractLoc *,
                            std::map<unsigned, const ConsElem *>> &locMap,
                   DenseMap<const Function *, const ConsElem *> &vargMap)
      :

        infoflow(infoflow), soln(s), topConstant(top), botConstant(bot),
        defaultTainted(defaultTainted), valueMap(valueMap), locMap(locMap),
        vargMap(vargMap) {}
  ~InfoflowSolution();

  /// isTainted - returns true if the security level of the value is High.
  bool isTainted(const Value &);
  bool isTainted(const AbstractLoc &);
  bool isTainted(const ConsElem &);
  DenseSet<const ConsElem *> taintedConsElemFromLoc(const AbstractLoc &);
  DenseSet<const ConsElem *> taintedConsElemFromValue(const Value &);
  void getOriginalLocation(const Value *);
  void allTainted();
  std::set<const Value *> getAllTaintValues();
  std::set<const ConsElem *> getAllWLPConsElem();
  /// isDirectPtrTainted - returns true if the security level of the memory
  /// pointed to is High.
  bool isDirectPtrTainted(const Value &);
  /// isReachPtrTainted - returns true if the security level of memory reachable
  /// from the pointer is High.
  bool isReachPtrTainted(const Value &);
  /// isVargTainted - returns true if the security level of the varargs of
  /// the function is High.
  bool isVargTainted(const Function &);

private:
  InfoflowSolution &operator=(const InfoflowSolution &rhs);

  const Infoflow &infoflow;
  ConsSoln *soln;
  const ConsElem &topConstant;
  const ConsElem &botConstant;
  bool defaultTainted;
  DenseMap<const Value *, const ConsElem *> &valueMap;
  DenseMap<const AbstractLoc *, std::map<unsigned, const ConsElem *>> &locMap;
  DenseMap<const Function *, const ConsElem *> &vargMap;
};

class TaintAnalysis;
class ImplicitFunction;
class VulnerableBranch;
class VulnerableInst;
class ConstraintGen;

/// A constraint-based, context-sensitive, interprocedural information
/// flow analysis pass. Exposes information-flow constraints with a
/// Tainted/Untainted security lattice that can be used with constraint
/// kinds to implement arbitrary security lattices.
class Infoflow
    : public CallSensitiveAnalysisPass<Unit, Unit, 1, CallerContext> {
  friend class InfoflowSolution;
  friend class TaintAnalysis;
  friend class ImplicitFunction;
  friend class VulnerableBranch;
  friend class VulnerableInst;
  friend class ConstraintGen;
  friend class TaintAnalysisBase;
  friend class VulnerableBranchWrapper;

public:
  typedef std::set<const AbstractLoc *> AbsLocSet;
  typedef std::set<const ConsElem *> ConsElemSet;
  typedef std::set<RLConstraint> ConsSet;
  typedef DenseMap<const Value *, const ConsElem *> ValueConsElemMap;
  typedef FlowRecord::value_iterator value_iterator;
  typedef FlowRecord::value_set value_set;
  typedef std::vector<FlowRecord> Flows;
  typedef unsigned long FlowRecordID;
  // pair.first: taintAnalysis, pair.second: whitelist pointer
  typedef DenseMap<const Instruction *, std::pair<Flows, Flows>> InstFlowMap;
  typedef DenseMap<FlowRecordID, ConsSet> FlowConsSetMap;
  typedef DenseMap<const Instruction *, ConsSet> InstConsSetMap; 

  static std::set<const Value *> tainted;
  static std::set<const Value *> whitelistPointers;
  static ConsElemSet solutionSetWLP;
  static bool WLPTR_ROUND;
  InstFlowMap instFlowMap;
  FlowConsSetMap flowConsSetMap;
  static InstConsSetMap instTaintConsSetMap;
  static InstConsSetMap instWLPConsSetMap;
  static int iterationTag;
  
  // 4 for each, implicit or not, sink or not
  // 0: not implicit, not sink
  // 1: implicit, not sink
  // 2: not implicit, sink
  // 3: implicit, sink
  static ConsSet consSetTaint[4];
  static ConsSet consSetWLP[4];

  static char ID;
  bool offset_used;
  json config;
  FlowRecordID currentFlowRecord;

  Infoflow();
  virtual ~Infoflow() {
    // delete kit;
    delete signatureRegistrar;
  }
  const char *getPassName() const { return "Infoflow"; }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    CallSensitiveAnalysisPass<Unit, Unit, 1, CallerContext>::getAnalysisUsage(
        AU);
    AU.addRequired<SourceSinkAnalysis>();
    AU.addRequired<PostDominatorTree>();
    // AU.addRequired<PointsToInterface>();
    AU.setPreservesAll();
  }

  virtual void releaseMemory() {
    // Clear out all the maps
    // valueConstraintMap.clear();
    // vargConstraintMap.clear();
    // summarySinkValueConstraintMap.clear();
    // summarySourceValueConstraintMap.clear();
    // summarySinkVargConstraintMap.clear();
    // summarySourceVargConstraintMap.clear();

    // And free the kit and all its constraints
    // delete kit;
    // kit = NULL;
  }

  bool DropAtSinks() const;

  void getOriginalLocation(const Value *V);
  std::string getOriginalLocationConsElem(const Value *);

  /// registerSignatures - Register an information flow signature
  /// to be used when calling external code.
  virtual void registerSignatures();

  //////////////////////////////////////////////////////////////
  /// Adding taint sources and taint constraints
  ///-----------------------------------------------------------
  /// The following methods allow the user to add constraints
  /// to the default set computed by the information flow
  /// analysis. The first argument specifies the constraint
  /// 'kind' the new constraint should be added to. When
  /// solving for an information flow solution, the user
  /// may specify a set of constraints to include.

  void setLabel(std::string, const Value &, RLLabel, bool, std::string = "");
  /// Adds the constraint "TAINTED <= VALUE" to the given kind
  void setUntainted(std::string, const Value &);
  /// Adds the constraint "VALUE <= UNTAINTED" to the given kind
  void setTainted(std::string, const Value &);
  /// Adds the constraint "TAINTED <= LOC" for all locations
  /// the value could point to to the given kind
  void setDirectPtrUntainted(std::string, const Value &);
  /// Adds the constraint "LOC <= UNTAINTED" for all locations
  /// the value could point to to the given kind
  void setDirectPtrTainted(std::string, const Value &);
  /// Adds the constraint "TAINTED <= LOC" for all locations
  /// reachable through the given pointer to the given kind
  void setReachPtrUntainted(std::string, const Value &);
  /// Adds the constraint "LOC <= UNTAINTED" for all locations
  /// reachable through the given pointer to the given kind
  void setReachPtrTainted(std::string, const Value &);
  /// Adds the constraint "TAINTED <= VARGS OF FUN" to the given kind
  void setVargUntainted(std::string, const Function &);
  /// Adds the constraint "VARGS OF FUN <= UNTAINTED" to the given kind
  void setVargTainted(std::string, const Function &);

  //////////////////////////////////////////////////////////////
  /// Solving sets of contraints
  ///-----------------------------------------------------------
  /// The following methods return solutions to a set of
  /// information flow constraints. Users can request the
  /// greatest or least fixed point of the constraint set.
  /// The list of kinds specifies additional constraints that
  /// should be satisfied in addition to the default constraints
  /// computed by the information flow analysis.

  /// Solve the default information flow constraints combined
  /// with any constraints from the given kinds, and if implicit
  /// is true, the implicit kind. Returns the least fixed point
  /// of the constraint system: unconstrained values and locations
  /// are considered UNTAINTED.
  ///
  /// Once a solution is requested for a given kind, no further
  /// constraints may be added to that kind.
  InfoflowSolution *leastSolution(std::set<std::string> kinds, bool implicit,
                                  bool sinks);

  /// Solve the default information flow constraints combined
  /// with any constraints from the given kinds, and if implicit
  /// is true, the implicit kind. Returns the greatest fixed point
  /// of the constraint system: unconstrained values and locations
  /// are considered TAINTED.
  ///
  /// Once a solution is requested for a given kind, no further
  /// constraints may be added to that kind.
  InfoflowSolution *greatestSolution(std::set<std::string> kinds,
                                     bool implicit);

  void clearSolutions();
  Flows getInstructionFlows(const Instruction &);

  // Solve the given kind using two threads.
  void solveMT(std::string kind, Predicate *pred) {
    assert(kit);
    kit->solveMT(kind, pred);
  }
  std::vector<InfoflowSolution *>
  solveLeastMT(std::vector<std::string> kinds, bool useDefaultSinks,
               const Predicate *pred = &RLConstraintKit::truePredicate());

private:
  virtual void doInitialization();
  virtual void doFinalization();
  virtual const Unit bottomInput() const;
  virtual const Unit bottomOutput() const;
  virtual const Unit runOnContext(const AUnitType unit, const Unit input);

  RLConstraintKit *kit;

  static PointsToInterface *pti;
  SourceSinkAnalysis *sourceSinkAnalysis;
  SignatureRegistrar *signatureRegistrar;

  Flows flowVector;

  FlowRecord currentContextFlowRecord(bool implicit);

  const AbsLocSet &locsForValue(const Value &value) const;
  const AbsLocSet &reachableLocsForValue(const Value &value) const;

  const std::set<const AbstractHandle *> &
  HandlesForValue(const Value &value) const;
  const std::set<const AbstractHandle *> &
  reachableHandlesForValue(const Value &value) const;

  bool offsetForValue(const Value &value, unsigned *Offset);

  const Function *findEnclosingFunc(const Value *V) const;
  const MDLocation *findVar(const Value *V, const Function *F) const;
  const MDLocalVariable *findVarNode(const Value *V, const Function *F) const;

  std::set<ConfigVariable> sourceVariables;
  std::vector<ConfigVariable> sinkVariables;
  std::vector<ConfigVariable> indexedSinkVariables;
  std::vector<ConfigVariable> whitelistVariables;
  static std::set<ConfigVariable> sourceWhitelistPointers;
  static std::set<ConfigVariable> fullyTainted;

  static DenseMap<const AbstractLoc *, std::set<const Value *>>
      invertedLocConstraintMap;
  static DenseMap<const BasicBlock *, std::string> predicateMap;
  static DenseMap<ContextID, ValueConsElemMap> valueConstraintMap;
  static DenseMap<const AbstractLoc *, std::map<unsigned, const ConsElem *>>
      locConstraintMap;
  static DenseMap<ContextID, DenseMap<const Function *, const ConsElem *>>
      vargConstraintMap;

  static ValueConsElemMap summarySinkValueConstraintMap;
  static ValueConsElemMap summarySourceValueConstraintMap;
  static std::map<std::string, DenseMap<const ConstantInt *, const ConsElem *>>
      constantValueConstraintMap;
  static DenseMap<const Value *, std::string> valueStringMap;
  static DenseMap<const Function *, const ConsElem *> summarySinkVargConstraintMap;
  static DenseMap<const Function *, const ConsElem *> summarySourceVargConstraintMap;

  ValueConsElemMap &getOrCreateValueConstraintMap(const ContextID);
  DenseMap<const Function *, const ConsElem *> &
  getOrCreateVargConstraintMap(const ContextID);

  virtual const Unit signatureForExternalCall(const ImmutableCallSite &cs,
                                              const Unit input);

  void constrainFlowRecord(const FlowRecord &);

  void addMemorySourceLocations(const FlowRecord &, ConsElemSet &,
                                ConsElemSet &);
  void addDirectSourceLocations(const FlowRecord &, ConsElemSet &,
                                ConsElemSet &, AbsLocSet &, AbsLocSet &);
  void addDirectValuesToSources(FlowRecord::value_set, ConsElemSet &,
                                AbsLocSet &, bool);
  void addReachSourceLocations(const FlowRecord &, ConsElemSet &, ConsElemSet &,
                               AbsLocSet &, AbsLocSet &);
  void addReachValuesToSources(FlowRecord::value_set, ConsElemSet &,
                               AbsLocSet &, bool);

  void constrainSinkMemoryLocations(const FlowRecord &, const ConsElem &,
                                    const ConsElem &, bool, bool, ConsSet &);
  void constrainDirectSinkLocations(const FlowRecord &, AbsLocSet &,
                                    const ConsElem &, const ConsElem &, bool,
                                    bool, ConsSet &);
  void constrainReachSinkLocations(const FlowRecord &, AbsLocSet &,
                                   const ConsElem &, const ConsElem &, bool,
                                   bool, ConsSet &);
  const std::string kindFromImplicitSink(bool implicit, bool sink) const;

  const std::string getOrCreateStringFromValue(const Value &, bool = true);
  unsigned GEPInstCalculateNumberElements(const GetElementPtrInst *);
  unsigned GEPInstCalculateArrayOffset(const GetElementPtrInst *,
                                       std::set<const AbstractLoc *>);
  unsigned GEPInstCalculateStructOffset(const GetElementPtrInst *,
                                        std::set<const AbstractLoc *>);
  unsigned GEPInstCalculateOffset(const GetElementPtrInst *,
                                  std::set<const AbstractLoc *>);
  unsigned findOffsetFromFieldIndex(StructType *, unsigned,
                                    const AbstractLoc *);
  bool checkGEPOperandsConstant(const GetElementPtrInst *);
  void processGetElementPtrInstSink(const Value *, bool, bool, const ConsElem &,
                                    std::set<const AbstractLoc *>, ConsSet &);
  void processGetElementPtrInstSource(const Value *,
                                      std::set<const ConsElem *> &,
                                      std::set<const AbstractLoc *>, bool);

  ConsElemSet findRelevantConsElem(const AbstractLoc *,
                                   std::map<unsigned, const ConsElem *>,
                                   unsigned, Type *);
  ConsElemSet findRelevantConsElem(const AbstractLoc *,
                                   std::map<unsigned, const ConsElem *>,
                                   unsigned, const Value *);
  ConsElemSet findRelevantConsElem(const AbstractLoc *,
                                   std::map<unsigned, const ConsElem *>,
                                   unsigned, unsigned);
  ConsElemSet findRelevantConsElem(const AbstractLoc *,
                                   std::map<unsigned, const ConsElem *>,
                                   unsigned, unsigned, const Value *, Type *);

  const ConsElem &getOrCreateConsElem(const ContextID, const Value &, ConsSet &);
  void putOrConstrainConsElem(bool imp, bool sink, const ContextID,
                              const Value &, const ConsElem &, ConsSet &);
  const ConsElem &getOrCreateConsElemSummarySource(const Value &);
  void putOrConstrainConsElemSummarySource(std::string, const Value &,
                                           const ConsElem &, std::string = "");
  const ConsElem &getOrCreateConsElemSummarySink(const Value &);
  void putOrConstrainConsElemSummarySink(std::string, const Value &,
                                         const ConsElem &, ConsSet &);
  const ConsElem &getOrCreateConsElem(const Value &, ConsSet &);
  std::map<unsigned, const ConsElem *> getOrCreateConsElem(const AbstractLoc &);
  std::map<unsigned, const ConsElem *>
  getOrCreateConsElemTyped(const AbstractLoc &, unsigned, const Value *v = NULL,
                           bool force = false);
  void createConsElemFromStruct(const AbstractLoc &, StructType *,
                                std::map<unsigned, const ConsElem *> &,
                                unsigned);
  void createConsElemFromArray(const AbstractLoc &, ArrayType *,
                               std::map<unsigned, const ConsElem *> &);

  std::map<unsigned, const ConsElem *>
  getOrCreateConsElemCollapsedStruct(const AbstractLoc &, const StructType *);
  void putOrConstrainConsElem(bool imp, bool sink, const Value &,
                              const ConsElem &, ConsSet &);
  void putOrConstrainConsElem(bool imp, bool sink, const AbstractLoc &,
                              const ConsElem &, ConsSet &);
  void putOrConstrainConsElem(bool imp, bool sink, const AbstractLoc &,
                              const ConsElem &, unsigned offset, const Value *,
                              unsigned, ConsSet &);
  void putOrConstrainConsElemStruct(bool, bool, const AbstractLoc &,
                                    const ConsElem &, unsigned, ConsSet &,
                                    const Value *v = NULL);
  void putOrConstrainConsElemCollapsed(bool, bool, const AbstractLoc &,
                                       const ConsElem &, unsigned,
                                       const StructType *);

  std::tuple<std::string, int, std::string> parseTaintString(std::string line);
  void readConfiguration();
  ConfigVariable parseConfigVariable(json v);

  int matchValueAndParsedString(const Value &, std::string, ConfigVariable);
  void getOrCreateLocationValueMap();
  void removeConstraint(std::string, ConfigVariable);
  void removeConstraintFromIndex(std::string, const AbstractLoc *,
                                 const Value *,
                                 std::map<unsigned, const ConsElem *>, int);
  bool matchImplicitWhitelist(const Instruction &inst);
  void constrainAllConsElem(std::string kind,
                            std::map<unsigned, const ConsElem *>, RLLabel,
                            std::string = "");
  void constrainAllConsElem(std::string kind, std::set<const ConsElem *>,
                            RLLabel, std::string = "");
  void constrainAllConsElem(std::string kind, const Value &,
                            std::set<const ConsElem *>, RLLabel,
                            std::string = "");
  void constrainOffsetFromIndex(std::string, const AbstractLoc *, const Value *,
                                std::map<unsigned, const ConsElem *>, int);

  const ConsElem &getOrCreateVargConsElem(const ContextID, const Function &, ConsSet &);
  void putOrConstrainVargConsElem(bool imp, bool sink, const ContextID,
                                  const Function &, const ConsElem &, ConsSet &);
  const ConsElem &getOrCreateVargConsElemSummarySource(const Function &);
  void putOrConstrainVargConsElemSummarySource(std::string, const Function &,
                                               const ConsElem &);
  const ConsElem &getOrCreateVargConsElemSummarySink(const Function &);
  void putOrConstrainVargConsElemSummarySink(std::string, const Function &,
                                             const ConsElem &, ConsSet &);
  const ConsElem &getOrCreateVargConsElem(const Function &, ConsSet &);
  void putOrConstrainVargConsElem(bool imp, bool sink, const Function &,
                                  const ConsElem &, ConsSet &);

  void generateFunctionConstraints(const Function &);
  void generateBasicBlockConstraints(const BasicBlock &, Flows &);
  void getInstructionFlowsInternal(const Instruction &, bool callees, Flows &,
                                   std::string);

  void operandsAndPCtoValue(const Instruction &, Flows &);

  void constrainMemoryLocation(bool imp, bool sink, const Value &,
                               const ConsElem &, ConsSet &);
  void constrainReachableMemoryLocations(bool imp, bool sink, const Value &,
                                         const ConsElem &, ConsSet &);
  const ConsElem &getOrCreateMemoryConsElem(const Value &);
  const ConsElem &getOrCreateReachableMemoryConsElem(const Value &);

  void constrainConditionalSuccessors(const TerminatorInst &, FlowRecord &rec);

  void constrainAtomicCmpXchgInst(const AtomicCmpXchgInst &, Flows &);
  void constrainAtomicRMWInst(const AtomicRMWInst &, Flows &);
  void constrainBinaryOperator(const BinaryOperator &, Flows &);
  void constrainCallInst(const CallInst &, bool callees, Flows &);
  void constrainCmpInst(const CmpInst &, Flows &);
  void constrainExtractElementInst(const ExtractElementInst &, Flows &);
  void constrainFenceInst(const FenceInst &, Flows &);
  void constrainGetElementPtrInst(const GetElementPtrInst &, Flows &);
  void constrainInsertElementInst(const InsertElementInst &, Flows &);
  void constrainInsertValueInst(const InsertValueInst &, Flows &);
  void constrainLandingPadInst(const LandingPadInst &, Flows &);
  void constrainPHINode(const PHINode &, Flows &);
  void constrainSelectInst(const SelectInst &, Flows &);
  void constrainShuffleVectorInst(const ShuffleVectorInst &, Flows &);
  void constrainStoreInst(const StoreInst &, Flows &);
  void constrainTerminatorInst(const TerminatorInst &, bool callees, Flows &);
  void constrainUnaryInstruction(const UnaryInstruction &, Flows &);
  void constrainAllocaInst(const AllocaInst &, Flows &);
  void constrainCastInst(const CastInst &, Flows &);
  void constrainExtractValueInst(const ExtractValueInst &, Flows &);
  void constrainLoadInst(const LoadInst &, Flows &);
  void constrainVAArgInst(const VAArgInst &, Flows &);
  void constrainBranchInst(const BranchInst &, Flows &);
  void constrainIndirectBrInst(const IndirectBrInst &, Flows &);
  void constrainInvokeInst(const InvokeInst &, bool callees, Flows &);
  void constrainReturnInst(const ReturnInst &, Flows &);
  void constrainResumeInst(const ResumeInst &, Flows &);
  void constrainSwitchInst(const SwitchInst &, Flows &);
  void constraintUnreachableInst(const UnreachableInst &, Flows &);

  void constrainCallSite(const ImmutableCallSite &cs, bool callees, Flows &);
  void constrainCallee(const ContextID calleeContext, const Function &callee,
                       const ImmutableCallSite &cs, Flows &);
  void constrainIntrinsic(const IntrinsicInst &, Flows &);
  void constrainMemcpyOrMove(const IntrinsicInst &intr, Flows &flows);
  void constrainMemset(const IntrinsicInst &intr, Flows &flows);

  void pushToFullyTainted(const Instruction &inst);
  void insertIntoInstFlowMap(const Instruction *inst, Flows &taintFlows, Flows &WLPFlows);
  void insertIntoFlowConsSetMap(const FlowRecord &flow, ConsSet &set);

  AbstractLocSet getPointedToAbstractLocs(const Value *v);
};

StructType *convertValueToStructType(const Value *v);
const Value *getAllocationValue(const GetElementPtrInst *gep);
std::string getCaption(const AbstractHandle *N, const DSGraph *G);
std::string getCaption(const AbstractLoc *N, const DSGraph *G);
const ConsElem *
findConsElemAtOffset(std::map<unsigned, const ConsElem *> elemMap,
                     unsigned offset);
} // namespace deps

#endif /* INFOFLOW_H */
