//===-- SignatureLibrary.h --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a library of information flow signatures that can be
// registered with the Infoflow signature registrar.
//
//===----------------------------------------------------------------------===//

#ifndef SIGNATURELIBRARY_H_
#define SIGNATURELIBRARY_H_

#include "FlowRecord.h"
#include "InfoflowSignature.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace deps {

using namespace llvm;

enum SignatureFlowDirection {
  SFD_NoFlow,
  SFD_ArgToRet,
  SFD_ArgToRetAndOther,
  SFD_ArgToAllReachable,
  SFD_Custom = -1
};

enum SignatureFlowPointer { SFP_DirectPointer, SFP_ReachablePointer };

class TaintByConfig : public Signature {
public:
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_TaintByConfig; }
  static inline bool classof(const TaintByConfig *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_TaintByConfig;
  }
  json config;
};

/// Description: A conservative signature that taints all reachable sinks with
///   all reachable sources (ignoring that callees may be memory-unsafe).
/// Accepts: all call sites.
/// Flows:
///   - Implicit flow from pc and function pointer at the call site to all sinks
///   - All arguments are sources
///   - The reachable objects from all pointer arguments are sources
///   - The reachable objects from all pointer arguments are sinks
///   - The return value (if applicable) is a sink
class TaintReachable : public Signature {
public:
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_TaintReachable; }
  static inline bool classof(const TaintReachable *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_TaintReachable;
  }
};

/// Description: A dummy signature that assumes no information flows happen
///   as a result of the call. (Not very useful.)
/// Accepts: all call sites.
/// Flows:
///   - None
class NoFlows : public Signature {
public:
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_NoFlows; }
  static inline bool classof(const TaintReachable *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_NoFlows;
  }
};

class ArgsToRet : public Signature {
public:
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_ArgsToRet; }
  static inline bool classof(const TaintReachable *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_ArgsToRet;
  }
};

// StdLib - Signature generation for StdLib calls
struct CallSummary;
class StdLib : public Signature {
  std::vector<const CallSummary *> Calls;
  void initCalls();
  bool findEntry(const ImmutableCallSite cs, const CallSummary *&S) const;

public:
  StdLib() : Signature() { initCalls(); }
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_StdLib; }
  static inline bool classof(const TaintReachable *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_StdLib;
  }
};

// OverflowChecks
// Description: Signatures for the "____jf_check" family of overflow checks
// Accepts: All callees starting with "____jf_check"
// Flows: From all arguments to return value, no direct/reachable pointers.
class OverflowChecks : public Signature {
public:
  virtual bool accept(const ContextID ctxt, const ImmutableCallSite cs) const;
  virtual std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> process(const ContextID ctxt,
                                          const ImmutableCallSite cs) const;

  /// Support for llvm-style RTTI (isa<>, dyn_cast<>, etc.)
  virtual SignatureType type() const { return ST_OverflowChecks; }
  static inline bool classof(const TaintReachable *sig) { return true; }
  static inline bool classof(const Signature *sig) {
    return sig->type() == ST_OverflowChecks;
  }
};

} // namespace deps

#endif /* SIGNATURELIBRARY_H_ */
