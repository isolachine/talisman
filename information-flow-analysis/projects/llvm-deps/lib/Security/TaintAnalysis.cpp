//===- SourceSinkAnalysis.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// A taint analysis based InfoFlow analysis.
//
//===----------------------------------------------------------------------===//

#include "TaintAnalysis.h"
#include "Infoflow.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>

// For __cxa_demangle (demangling c++ function names)
// Requires libstdc++
#include <cxxabi.h>

namespace deps {

static RegisterPass<TaintAnalysis> X("taintanalysis",
                                     "A simple taint analysis");

char TaintAnalysis::ID;

bool TaintAnalysis::runOnModule(Module &M) {
  ifa = &getAnalysis<Infoflow>();
  if (!ifa) {
    errs() << "No instance\n";
    return false;
  }
  parser.setInfoflow(ifa);

  for (auto whitelist : ifa->whitelistVariables) {
    ifa->removeConstraint("default", whitelist);
  }

  std::set<std::string> kinds;
  kinds.insert("test");

  errs() << "Least solution with explicit constraints\n";
  InfoflowSolution *soln = ifa->leastSolution(kinds, false, true);
  soln->allTainted();

  errs() << "Least solution with implicit contraints\n";
  soln = ifa->leastSolution(kinds, true, true);
  soln->allTainted();

  kinds.clear();
  kinds.insert("white");
  errs() << "Whitelist with constraints\n";
  soln = ifa->leastSolution(kinds, false, true);
  soln->allTainted();
  return false;
}

} // namespace deps
