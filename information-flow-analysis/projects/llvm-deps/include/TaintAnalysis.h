//===------- SourceSinkAnalysis.h -- Identify taint sources and sinks ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a pass for taint analysis.
//
//===----------------------------------------------------------------------===//

#ifndef TAINTANALYSIS_H_
#define TAINTANALYSIS_H_

#include "Infoflow.h"
#include "TaintAnalysisBase.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Pass.h"

#include <set>
#include <string>

using namespace llvm;

namespace deps {

class TaintAnalysis : public ModulePass {
public:
  static char ID;

  TaintAnalysis() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<Infoflow>();
    AU.setPreservesAll();
  }

private:
  Infoflow *ifa;
  TaintAnalysisBase parser;
};

} // namespace deps

#endif
