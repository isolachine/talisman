//===- SourceSinkAnalysis.cpp ---------------------------------------------===//
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

#include "VulnerableInst.h"
#include "Infoflow.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

// For __cxa_demangle (demangling c++ function names)
// Requires libstdc++
#include <cxxabi.h>

namespace deps {

static RegisterPass<VulnerableInst>
    X("vulnerable-inst", "An analysis for identifying vulnerable instructions");

char VulnerableInst::ID;

bool VulnerableInst::runOnModule(Module &M) {
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

  std::set<std::string> kinds;
  kinds.insert("source-sink");

  InfoflowSolution *soln = ifa->leastSolution(kinds, false, true);
  std::set<const Value *> tainted = soln->getAllTaintValues();

  // Create constraints for Derivation Solver
  for (Module::const_iterator F = M.begin(), FEnd = M.end(); F != FEnd; ++F) {
    for (const_inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E;
         ++I) {
      const Instruction *inst = &*I;
      const MDLocation *loc = inst->getDebugLoc();
      if (loc) {
        const Value *v = dyn_cast<Value>(inst);
        for (auto ctxt : ifa->valueConstraintMap) {
          DenseMap<const Value *, const ConsElem *> valueConsMap = ctxt.second;
          DenseMap<const Value *, const ConsElem *>::iterator vIter =
              valueConsMap.find(v);
          if (vIter != valueConsMap.end()) {
            const ConsElem *elem = vIter->second;
            const ConsElem &low = RLConstant::bot();
            errs() << "\n";
            RLConstraint c(elem, &low, &Predicate::TruePred(), false, false,
                           "  ;  [ConsDebugTag-*]   conditional branch");
            ifa->kit->getOrCreateConstraintSet("source-sink").push_back(c);
          }
        }
      }
    }
  }

  errs() << "\n---- Tainted Values BEGIN ----\n";
  for (auto i : tainted) {
    i->dump();
  }
  errs() << "---- Tainted Values END ----\n\n";

  errs() << "\n---- Constraints BEGIN ----\n";
  kinds.insert({"default", "default-sink"});
  for (auto kind : kinds) {
    errs() << kind << ":\n";
    for (auto cons : ifa->kit->getOrCreateConstraintSet(kind))
      cons.dump();
  }
  errs() << "---- Constraints END ----\n\n";

  /**
     std::set<const Value*> vul;
     std::set_intersection(tainted.begin(), tainted.end(), untrusted.begin(),
     untrusted.end(), std::inserter(vul, vul.end())); for(std::set<const
     Value*>::iterator it=vul.begin(); it != vul.end(); it++) {
     soln->getOriginalLocation(*it);
     errs() << "\n";
     }*/

  // Variables to gather branch statistics
  unsigned long number_branches = 0;
  unsigned long tainted_branches = 0;
  // iterating over all branches
  errs() << "#--------------Results------------------\n";
  for (Module::const_iterator F = M.begin(), FEnd = M.end(); F != FEnd; ++F) {
    for (const_inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E;
         ++I) {
      const Instruction *inst = &*I;
      const MDLocation *loc = inst->getDebugLoc();
      number_branches++;
      if (loc) {
        const Value *v = dyn_cast<Value>(inst);
        if (tainted.find(v) != tainted.end()) {
          tainted_branches++;
          errs() << loc->getFilename() << " line "
                 << std::to_string(loc->getLine()) << "\n";
        }
      }
    }
  }

  errs() << "#--------------Array Indices------------------\n";
  for (Module::const_iterator F = M.begin(), FEnd = M.end(); F != FEnd; ++F) {
    for (const_inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E;
         ++I) {
      if (const GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(&*I)) {
        if (ArrayType *a = dyn_cast<ArrayType>(gep->getSourceElementType())) {
          const MDLocation *loc = gep->getDebugLoc();
          // get the index value and lookup in the tainted set
          const Value *v = gep->getOperand(2);
          if (!isa<Constant>(v) && tainted.find(v) != tainted.end()) {
            errs() << loc->getFilename() << " at "
                   << std::to_string(loc->getLine()) << "\n";
            gep->dump();
            gep->getOperand(2)->dump();
          }
        }
      }
    }
  }

  // Dump statistics
  errs() << "#--------------Statistics----------------\n";
  errs() << ":: Tainted Instructions: " << tainted_branches << "\n";
  errs() << ":: Total Instructions: " << number_branches << "\n";
  if (number_branches > 0) {
    double tainted_percentage =
        tainted_branches * 1.0 / number_branches * 100.0;
    errs() << ":: Vulnerable Branches: "
           << format("%2.2f%% [%d/%d]\n", tainted_branches, number_branches,
                     tainted_percentage);
  }

  return false;
}

} // namespace deps
