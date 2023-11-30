//===------- ConstraintGen.h -- Identify taint sources and sinks ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a pass for identifying vulnerable branches.
//
//===----------------------------------------------------------------------===//

#ifndef IMPLICITFUNCTION_H_
#define IMPLICITFUNCTION_H_

#include "Infoflow.h"
#include "TaintAnalysisBase.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Pass.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

namespace deps {

class ImplicitFunction : public ModulePass {
public:
  static char ID;

  ImplicitFunction() : ModulePass(ID) {}

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
