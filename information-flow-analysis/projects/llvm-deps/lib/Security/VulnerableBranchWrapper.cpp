//===- VulnerableBranchWrapper.cpp
//---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// An analysis for identifying vulnerable branches.
//
//===----------------------------------------------------------------------===//
#include <chrono>
#include <ctime>

#include "Infoflow.h"
#include "VulnerableBranchWrapper.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

// For __cxa_demangle (demangling c++ function names)
// Requires libstdc++
#include <cxxabi.h>

namespace deps {

static RegisterPass<VulnerableBranchWrapper>
    X("vulnerablebranchwrapper", "wrapping vulnerable branch pass");

char VulnerableBranchWrapper::ID;

bool VulnerableBranchWrapper::runOnModule(Module &M) {
  pti = &getAnalysis<PointsToInterface>();
  errs() << "\n---------Phase 0: Constraint Set Generation---------\n";
  legacy::PassManager *passManager = new legacy::PassManager();
  vba = new VulnerableBranch();
  Infoflow::iterationTag++;
  Infoflow::WLPTR_ROUND = false;
  Infoflow::pti = pti;
  passManager->add(vba);
  passManager->run(M);
  errs() << vba->ifa->currentFlowRecord << " flow records processed\n";
  delete vba;

  // Phase 1
  errs() << "\n---------Phase 1: Whitelist pointer propagation---------\n";
  passManager = new legacy::PassManager();
  vba = new VulnerableBranch();
  Infoflow::iterationTag++;
  Infoflow::WLPTR_ROUND = true;
  vba->ifa->pti = pti;
  passManager->add(vba);
  passManager->run(M);
  errs() << vba->ifa->currentFlowRecord << " flow records processed\n";
  delete vba;

  // Phase 2
  errs() << "\n---------Phase 2: Taint analysis---------\n";
  passManager = new legacy::PassManager();
  vba = new VulnerableBranch();
  Infoflow::iterationTag++;
  Infoflow::WLPTR_ROUND = false;
  vba->ifa->pti = pti;
  passManager->add(vba);
  passManager->run(M);
  errs() << vba->ifa->currentFlowRecord << " flow records processed\n";

  errs() << "\n---- Tainted Values BEGIN ----\n";
  for (auto i : Infoflow::tainted) {
    i->dump();
  }
  errs() << "---- Tainted Values END ----\n\n";

  errs() << "\n---- Whitelisted Pointers BEGIN ----\n";
  for (auto i : Infoflow::whitelistPointers) {
    i->dump();
  }
  errs() << "---- Whitelisted Pointers END ----\n\n";

  // Variables to gather branch statistics
  unsigned long number_branches = 0;
  unsigned long tainted_branches = 0;
  unsigned long number_conditional = 0;
  // iterating over all branches
  errs() << "#--------------Results------------------\n";
  for (Module::const_iterator F = M.begin(), FEnd = M.end(); F != FEnd; ++F) {
    for (const_inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I)
      if (const BranchInst *bi = dyn_cast<BranchInst>(&*I)) {
        const MDLocation *loc = bi->getDebugLoc();
        number_branches++;
        if (bi->isConditional())
          number_conditional++;
        if (bi->isConditional() && loc) {
          const Value *v = bi->getCondition();
          if (Infoflow::tainted.find(v) != Infoflow::tainted.end()) {
            tainted_branches++;
            errs() << loc->getFilename() << " line "
                   << std::to_string(loc->getLine()) << "\n";
            // errs() << loc->getFilename() << " line " <<
            // std::to_string(loc->getLine()) << ":";
            // v->dump(); errs() << "\n";
          }
        }
      }
  }

  errs() << "#--------------Array Indices------------------\n";
  for (auto &F : M.functions()) {
    for (auto &I : inst_range(F)) {
      const User *user = nullptr;
      const Value *value = nullptr;
      if (const LoadInst *load = dyn_cast<LoadInst>(&I)) {
        if (const ConstantExpr *ce =
                dyn_cast<ConstantExpr>(load->getPointerOperand()))
          user = ce;
        value = load->getPointerOperand();
      } else if (const StoreInst *store = dyn_cast<StoreInst>(&I)) {
        if (const ConstantExpr *ce =
                dyn_cast<ConstantExpr>(store->getPointerOperand()))
          user = ce;
        value = store->getPointerOperand();
      }
      // } else if (const GetElementPtrInst *gep =
      //                dyn_cast<GetElementPtrInst>(&I)) {
      //   user = gep;
      // }

      if ((user || value) && matchNonPointerWhitelistAndTainted(value, user,
                                                     Infoflow::tainted, I)) {
        const MDLocation *loc = I.getDebugLoc();
        if (user)
          user->dump();
        else
          value->dump();
        errs() << loc->getFilename() << " at " << std::to_string(loc->getLine())
               << "\n";
      }
    }
  }

  // errs() << "Done after " << 1 << " iterations.\n";
  // Dump statistics
  errs() << "#--------------Statistics----------------\n";
  errs() << ":: Tainted Branches: " << tainted_branches << "\n";
  errs() << ":: Branch Instructions: " << number_branches << "\n";
  errs() << ":: Conditional Branches: " << number_conditional << "\n";
  if (number_branches > 0) {
    double tainted_percentage =
        tainted_branches * 1.0 / number_branches * 100.0;
    errs() << ":: Vulnerable Branches: "
           << format("%2.2f%% [%d/%d]\n", tainted_branches, number_branches,
                     tainted_percentage);
  }

  return false;
}

bool VulnerableBranchWrapper::matchNonPointerWhitelistAndTainted(
    const Value *value, const User *user, std::set<const Value *> &tainted,
    const Instruction &I) {
  // const BasicBlock *bc = I.getParent();
  // const Function *func = bc->getParent();
  // for (auto &op : user->operands()) {
  //   bool isWhitelisted = false;
  //   for (auto ptr : Infoflow::whitelistPointers) {
  //     if (op.get()->getType()->isPtrOrPtrVectorTy() &&
  //         op.get()->getName() == ptr.name &&
  //         (func->getName() == ptr.function ||
  //         (ptr.function == "" && isa<GlobalVariable>(op.get())))) {
  //       isWhitelisted = true;
  //     }
  //   }
  //   if (!isWhitelisted && tainted.find(op) != tainted.end()) {
  //     return true;
  //   }
  // }
  // return false;
  if (user)
    for (auto &op : user->operands()) {
      if (Infoflow::tainted.find(op) != Infoflow::tainted.end()) {
        return true;
      }
    }
  if (value)
    if (Infoflow::tainted.find(value) != Infoflow::tainted.end()) {
      return true;
    }
  return false;
}

} // namespace deps