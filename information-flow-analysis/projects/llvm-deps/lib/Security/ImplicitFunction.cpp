//===- ConstraintGen.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// A pass to generate information flow constraints.
//
//===----------------------------------------------------------------------===//

#include "ImplicitFunction.h"
#include "Infoflow.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>

// For __cxa_demangle (demangling c++ function names)
// Requires libstdc++
#include <cxxabi.h>

namespace deps {

static RegisterPass<ImplicitFunction>
    X("implicit-function",
      "A pass to detect function calls under sensitive branches.");

char ImplicitFunction::ID;

bool ImplicitFunction::runOnModule(Module &M) {
  int64_t i, s, p, l;
  auto start = std::chrono::steady_clock::now();
  ifa = &getAnalysis<Infoflow>();
  parser.setInfoflow(ifa);
  if (!ifa) {
    errs() << "No instance\n";
    return false;
  }

  parser.labelValue("source-sink", ifa->sourceVariables, true);

  for (auto whitelist : ifa->whitelistVariables) {
    ifa->removeConstraint("default", whitelist);
  }

  auto end = std::chrono::steady_clock::now();
  errs() << "INFOFLOW: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()
         << " ms\n";
  i = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  start = end;

  std::set<std::string> kinds{"source-sink", "default", "default-sink",
                              "implicit"};
  // Printing constraints
  errs() << "\n---- Constraints BEGIN ----\n";
  for (auto kind : kinds) {
    errs() << kind << ":\n";
    for (auto cons : ifa->kit->getOrCreateConstraintSet(kind))
      cons.dump();
  }
  errs() << "---- Constraints END ----\n\n";

  end = std::chrono::steady_clock::now();
  errs() << "PRINTING: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()
         << " ms\n";
  p = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  start = end;

  InfoflowSolution *soln = ifa->leastSolution(kinds, false, true);

  end = std::chrono::steady_clock::now();
  errs() << "SOLUTION: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()
         << " ms\n";

  s = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  start = end;

  std::set<const Value *> tainted = soln->getAllTaintValues();

  // A hacky fix to add globals to the tainted set.
  for (auto v : ifa->summarySourceValueConstraintMap) {
    auto val = v.first;
    if (!isa<BasicBlock>(val)) {
      auto locs = ifa->locsForValue(*val);
      for (auto loc : locs) {
        if (soln->isTainted(*loc)) {
          tainted.insert(val);
        }
      }
    }
  }

  DEBUG_WITH_TYPE(
      DEBUG_TYPE_TAINT, errs() << "\n---- Tainted Values BEGIN ----\n";
      for (auto i
           : tainted) {
        if (auto bb = dyn_cast<BasicBlock>(i)) {
          // continue;
          if (bb->hasName())
            errs() << "[BB] " << bb->getParent()->getName() << "|"
                   << bb->getName() << "\n";
          else
            errs() << "[BB] " << bb->getParent()->getName() << "|" << i << "\n";
          // Example of value printing filtering (by type).
          // } else if (auto global = dyn_cast<GlobalVariable>(i)) {
          //   errs() << "GLOBAL: " << global->getName() << "\n";
        } else {
          errs() << ifa->getOrCreateStringFromValue(*i) << "\n";
          auto locs = ifa->locsForValue(*i);
          for (auto e : soln->taintedConsElemFromValue(*i)) {
            e->dump(errs() << "  | CE" << e << ": ");
            errs() << "\n";
          }
          for (auto loc : locs) {
            for (auto e : soln->taintedConsElemFromLoc(*loc)) {
              e->dump(errs() << "  | CE" << e << ": ");
              errs() << "\n";
            }
          }
        }
      };
      errs() << "---- Tainted Values END ----\n\n";);

  errs() << "#------------------Results------------------#\n";
  std::set<const Function *> criticalFuncs;
  for (Module::const_iterator F = M.begin(); F != M.end(); ++F) {
    // for (auto &bb : *F) {
    // const Value *v = dyn_cast<Value>(&bb);
    // if (tainted.find(v) == tainted.end()) {
    criticalFuncs.insert(F);
    // }
    // }

    if (criticalFuncs.find(F) != criticalFuncs.end()) {
      for (auto &bb : *F) {
        const Value *v = dyn_cast<Value>(&bb);
        std::string tag = "[UB]|";
        if (auto term = bb.getTerminator()) {
          if (auto br = dyn_cast<const BranchInst>(term))
            if (br->isConditional() &&
                tainted.find(br->getCondition()) != tainted.end())
              tag = "[TB]|";
          if (auto sw = dyn_cast<const SwitchInst>(term))
            if (tainted.find(sw->getCondition()) != tainted.end())
              tag = "[TB]|";
        }

        errs() << tag;
        // errs() << (tainted.find(v) != tainted.end() ? "[TB]|" : "[UB]|");

        errs() << bb.getParent()->getName() << "|"
               << (bb.hasName() ? bb.getName().str() : std::to_string(i));
        for (auto succ : successors(&bb)) {
          errs() << "|" << succ->getName();
        }
        errs() << "\n";
      }
    }

    for (const_inst_iterator I = inst_begin(*F); I != inst_end(*F); ++I) {
      if (const CallInst *callInst = dyn_cast<CallInst>(&*I)) {
        if (isa<DbgInfoIntrinsic>(callInst))
          continue;

        auto parent = callInst->getParent();
        if (tainted.find(parent) != tainted.end()) {
          if (const MDLocation *loc = callInst->getDebugLoc()) {
            const Function *callee = callInst->getCalledFunction();
            if (const ConstantExpr *CExpr =
                    dyn_cast<ConstantExpr>(callInst->getCalledValue())) {
              if (CExpr->getOpcode() == Instruction::BitCast &&
                  isa<Function>(CExpr->getOperand(0)))
                callee = cast<Function>(CExpr->getOperand(0));
            }
            if (callee) {
              bool print = true;
              if (!callee->isDeclaration()) {
                auto &BB = *(callee->begin());
                auto V = dyn_cast<Value>(&BB);
                print = (tainted.find(V) != tainted.end());
              }
              if (print) {
                errs() << "[SCG]|";
                errs() << parent->getParent()->getName() << "|"
                       << loc->getFilename() << ", L"
                       << std::to_string(loc->getLine()) << "|"
                       << callee->getName();
                for (auto cf : criticalFuncs) {
                  if (cf == parent->getParent()) {
                    errs() << "|" << parent->getName();
                  }
                }
                errs() << "\n";

                // CFG traversal starts here.
                // errs() << "  [BR] " << parent->getName() << "\n";
                // std::deque<const BasicBlock *> worklist{parent};
                // std::set<const BasicBlock *> visited;

                // while (!worklist.empty()) {
                //   auto bb = worklist.back();
                //   worklist.pop_back();
                //   visited.insert(bb);

                //   for (auto pred : predecessors(bb)) {
                //     if (const BranchInst *inst =
                //             dyn_cast<BranchInst>(&pred->back())) {
                //       if (inst->isConditional()) {
                //         if (tainted.find(inst->getCondition()) !=
                //             tainted.end()) {
                //           worklist.push_front(pred);
                //           errs() << "  [BR] " << pred->getName() << "\n";
                //         }
                //       } else if (tainted.find(pred) != tainted.end()) {
                //         worklist.push_front(pred);
                //         errs() << "  [BR] " << pred->getName() << "\n";
                //       }
                //     }
                //   }
                // }
                // CFG traversal ends here.
              }
            }
          }
        }
      }
    }
  }
  errs() << "\n\n";

  end = std::chrono::steady_clock::now();
  errs() << "LOOKUP: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()
         << " ms\n";

  l = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  start = end;

  errs() << "INFOFLOW: " << i << "\n";
  errs() << "SOLUTION: " << s << "\n";
  errs() << "PRINTING: " << p << "\n";
  errs() << "LOOKUP: " << l << "\n";

  return false;
} // namespace deps

} // namespace deps
