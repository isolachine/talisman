//===-- SignatureLibrary.cpp -----------------------------------*- C++ -*-===//
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

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "deps"

#include "SignatureLibrary.h"

namespace deps {

bool TaintByConfig::accept(const ContextID ctxt,
                           const ImmutableCallSite cs) const {
  return true;
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
TaintByConfig::process(const ContextID ctxt, const ImmutableCallSite cs) const {
  DEBUG(errs() << "Using taint reachable signature for: "
               << *cs.getInstruction() << "\n");

  std::vector<FlowRecord> taintFlows;
  std::vector<FlowRecord> WLPFlows;

  SignatureFlowDirection directionMode = config.at("direction");
  SignatureFlowPointer pointerMode = config.at("pointer");
  json customFlows;
  for (json f : config.at("custom")) {
    std::string name = f.at("name");
    const Function *F = cs.getCalledFunction();
    if (F && F->getName() == name) {
      directionMode = f.at("mode");
      if (directionMode == SFD_Custom)
        customFlows = f.at("flows");
      break;
    }
  }

  switch (directionMode) {
  case SFD_NoFlow:
    break;

  case SFD_Custom:
    for (json flowJSON : customFlows) {
      int idx = 0;
      int argIdx = flowJSON.at("arg");
      int destIdx = flowJSON.at("dest");
      assert(static_cast<int>(cs.arg_size()) > argIdx &&
             static_cast<int>(cs.arg_size()) > destIdx);
      for (ImmutableCallSite::arg_iterator arg = cs.arg_begin();
           arg != cs.arg_end(); ++arg, ++idx) {
        if (idx == argIdx) {
          FlowRecord exp(false, ctxt, ctxt);
          FlowRecord imp(true, ctxt, ctxt);

          imp.addSourceValue(*cs->getParent());
          imp.addSourceValue(*cs.getCalledValue());

          exp.addSourceValue(**arg);
          if ((*arg)->getType()->isPointerTy()) {
            if (pointerMode == SFP_DirectPointer) {
              exp.addSourceDirectPtr(**arg);
            } else {
              exp.addSourceReachablePtr(**arg);
            }
            imp.addSourceValue(**arg);
          }

          if (destIdx == -1) {
            if (!cs->getType()->isVoidTy()) {
              imp.addSinkValue(*cs.getInstruction());
              exp.addSinkValue(*cs.getInstruction());
              if (cs->getType()->isPointerTy()) {
                if (pointerMode == SFP_DirectPointer) {
                  imp.addSinkDirectPtr(*cs.getInstruction());
                  exp.addSinkDirectPtr(*cs.getInstruction());
                } else {
                  exp.addSinkReachablePtr(*cs.getInstruction());
                  imp.addSinkReachablePtr(*cs.getInstruction());
                }
              }
            }
          } else {
            int otherIdx = 0;
            for (ImmutableCallSite::arg_iterator other = cs.arg_begin();
                 other != cs.arg_end(); ++other, ++otherIdx) {
              if (otherIdx == destIdx) {
                // TODO: Verify:
                // should not taint directly on values
                // but do need make flows transitive,
                // e.g., arg1 -> arg2 -> ret
                // Same for the counterpart in the default case
                if ((*other)->getType()->isPointerTy()) {
                  exp.addSinkValue(**other);
                  if (pointerMode == SFP_DirectPointer) {
                    exp.addSinkDirectPtr(**other);
                    imp.addSinkDirectPtr(**other);
                  } else {
                    exp.addSinkReachablePtr(**other);
                    imp.addSinkReachablePtr(**other);
                  }
                }
                break;
              }
            }
          }
          taintFlows.push_back(imp);
          taintFlows.push_back(exp);
          break;
        }
      }
    }
    break;

  default:
    for (ImmutableCallSite::arg_iterator arg = cs.arg_begin();
         arg != cs.arg_end(); ++arg) {

      FlowRecord exp(false, ctxt, ctxt);
      FlowRecord imp(true, ctxt, ctxt);

      // implicit from the pc of the call site and the function pointer
      imp.addSourceValue(*cs->getParent());
      imp.addSourceValue(*cs.getCalledValue());

      // every argument's value is a source
      exp.addSourceValue(**arg);
      // if argument is pointer, everything it POINTS TO is source
      if ((*arg)->getType()->isPointerTy()) {
        if (pointerMode == SFP_DirectPointer) {
          exp.addSourceDirectPtr(**arg);
        } else {
          exp.addSourceReachablePtr(**arg);
        }
        imp.addSourceValue(**arg);
      }

      // Sources and sinks of the args
      // MODE: >= SFD_ArgToRetAndOther
      if (directionMode >= SFD_ArgToRetAndOther) {
        for (ImmutableCallSite::arg_iterator other = cs.arg_begin();
             other != cs.arg_end(); ++other) {
          // if argument is pointer, everything it POINTS TO is sink
          if ((other != arg || directionMode == SFD_ArgToAllReachable) &&
              !isa<Constant>(other->get())) {
            if ((*other)->getType()->isPointerTy()) {
              if (pointerMode == SFP_DirectPointer) {
                exp.addSinkDirectPtr(**other);
                imp.addSinkDirectPtr(**other);
              } else {
                exp.addSinkReachablePtr(**other);
                imp.addSinkReachablePtr(**other);
              }
              // whitelist pointer exclusive flows
              if ((*arg)->getType()->isPointerTy()) {
                FlowRecord WLPFlow = FlowRecord(false, ctxt, ctxt);
                WLPFlow.addSourceValue(**arg);
                if (pointerMode == SFP_DirectPointer) {
                  WLPFlow.addSourceDirectPtr(**arg);
                } else {
                  WLPFlow.addSourceReachablePtr(**arg);
                }
                WLPFlow.addSinkValue(**other);
                WLPFlows.push_back(WLPFlow);
              }
            }
          }
        }
      }
      taintFlows.push_back(exp);
      WLPFlows.push_back(exp);

      // if the function has a return value it is a sink
      // MODE: SFD_ArgToRet
      if (!cs->getType()->isVoidTy()) {
        FlowRecord exp = FlowRecord(false, ctxt, ctxt);
        // add source again
        exp.addSourceValue(**arg);
        if ((*arg)->getType()->isPointerTy()) {
          if (pointerMode == SFP_DirectPointer) {
            exp.addSourceDirectPtr(**arg);
          } else {
            exp.addSourceReachablePtr(**arg);
          }
        }
        imp.addSinkValue(*cs.getInstruction());
        exp.addSinkValue(*cs.getInstruction());
        // TODO: VERIFY THIS WITH malloc()
        // malloc had this but we don't
        if (cs->getType()->isPointerTy()) {
          // TODO: Also using pointer mode at this location?
          imp.addSinkDirectPtr(*cs.getInstruction());
          exp.addSinkDirectPtr(*cs.getInstruction());
        }
        taintFlows.push_back(exp);
      } 
      taintFlows.push_back(imp);
    }
    break;
  }

  std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> flowPair;
  flowPair.first = taintFlows;
  flowPair.second = WLPFlows;
  return flowPair;
}

bool TaintReachable::accept(const ContextID ctxt,
                            const ImmutableCallSite cs) const {
  return true;
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
TaintReachable::process(const ContextID ctxt,
                        const ImmutableCallSite cs) const {
  DEBUG(errs() << "Using taint reachable signature for: "
               << *cs.getInstruction() << "\n");

  std::vector<FlowRecord> flows;

  for (ImmutableCallSite::arg_iterator arg = cs.arg_begin();
       arg != cs.arg_end(); ++arg) {

    FlowRecord exp(false, ctxt, ctxt);
    FlowRecord imp(true, ctxt, ctxt);

    // implicit from the pc of the call site and the function pointer
    imp.addSourceValue(*cs->getParent());
    imp.addSourceValue(*cs.getCalledValue());

    // every argument's value is a source
    exp.addSourceValue(**arg);
    // if the argument is a pointer, everything it reaches is a source
    if ((*arg)->getType()->isPointerTy()) {
      exp.addSourceReachablePtr(**arg);
      imp.addSourceValue(**arg);
    }

    // Sources and sinks of the args
    for (ImmutableCallSite::arg_iterator other = cs.arg_begin();
         other != cs.arg_end(); ++other) {
      // if the argument is a pointer, everything it reaches is a sink
      if (other != arg && (*other)->getType()->isPointerTy()) {
        exp.addSinkReachablePtr(**other);
        imp.addSinkReachablePtr(**other);
      }
    }

    // if the function has a return value it is a sink
    if (!cs->getType()->isVoidTy()) {
      imp.addSinkValue(*cs.getInstruction());
      exp.addSinkValue(*cs.getInstruction());
      // TODO: VERIFY THIS WITH malloc()
      // malloc had this but we don't
      if (cs->getType()->isPointerTy()) {
        imp.addSinkDirectPtr(*cs.getInstruction());
        exp.addSinkDirectPtr(*cs.getInstruction());
      }
    }

    flows.push_back(imp);
    flows.push_back(exp);
  }
  std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> flowPair;
  flowPair.first = flows;
  flowPair.second = std::vector<FlowRecord>();
  return flowPair;
}

bool ArgsToRet::accept(const ContextID ctxt, const ImmutableCallSite cs) const {
  return true;
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
ArgsToRet::process(const ContextID ctxt,
                                           const ImmutableCallSite cs) const {
  DEBUG(errs() << "Using ArgsToRet reachable signature for: "
               << *cs.getInstruction() << "\n");

  std::vector<FlowRecord> flows;

  if (!cs->getType()->isVoidTy()) {
    FlowRecord exp(false, ctxt, ctxt);

    // Sources and sinks of the args
    for (ImmutableCallSite::arg_iterator arg = cs.arg_begin(),
                                         end = cs.arg_end();
         arg != end; ++arg) {
      // every argument's value is a source
      exp.addSourceValue(**arg);
    }

    // if the function has a return value it is a sink
    exp.addSinkValue(*cs.getInstruction());

    flows.push_back(exp);
  }

  std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> flowPair;
  flowPair.first = flows;
  flowPair.second = std::vector<FlowRecord>();
  return flowPair;
}

bool NoFlows::accept(const ContextID ctxt, const ImmutableCallSite cs) const {
  return true;
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
NoFlows::process(const ContextID ctxt,
                                         const ImmutableCallSite cs) const {
  DEBUG(errs() << "Using no flows signature...\n");
  return std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>();
}

bool OverflowChecks::accept(const ContextID ctxt,
                            const ImmutableCallSite cs) const {
  const Function *F = cs.getCalledFunction();
  return F && F->getName().startswith("____jf_check");
}

std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>>
OverflowChecks::process(const ContextID ctxt,
                        const ImmutableCallSite cs) const {
  DEBUG(errs() << "Using OverflowChecks signature...\n");

  FlowRecord exp(false, ctxt, ctxt);
  FlowRecord imp(true, ctxt, ctxt);

  imp.addSourceValue(*cs->getParent());

  // Add all argument values as sources
  for (ImmutableCallSite::arg_iterator arg = cs.arg_begin(), end = cs.arg_end();
       arg != end; ++arg)
    exp.addSourceValue(**arg);
  assert(!cs->getType()->isVoidTy() && "Found 'void' overflow check?");

  // And the return value as a sink
  exp.addSinkValue(*cs.getInstruction());
  imp.addSinkValue(*cs.getInstruction());

  std::vector<FlowRecord> flows;
  flows.push_back(imp);
  flows.push_back(exp);
  std::pair<std::vector<FlowRecord>, std::vector<FlowRecord>> flowPair;
  flowPair.first = flows;
  flowPair.second = std::vector<FlowRecord>();
  return flowPair;
}

} // namespace deps

#endif
