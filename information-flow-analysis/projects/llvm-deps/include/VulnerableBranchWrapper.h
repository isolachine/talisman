//===------- VulnerableBranchWrapper.h -- Identify taint sources and sinks
//----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a pass for wrapping vulnerable branch pass.
//
//===----------------------------------------------------------------------===//

#include "TaintAnalysisBase.h"
#include "VulnerableBranch.h"
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

class VulnerableBranchWrapper : public ModulePass {
public:
  static char ID;

  VulnerableBranchWrapper() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    // AU.addRequired<VulnerableBranch>();
    AU.addRequired<PointsToInterface>();
    AU.setPreservesAll();
  }

private:
  VulnerableBranch *vba;
  PointsToInterface *pti;
  bool matchNonPointerWhitelistAndTainted(const Value *, const User *,
                                          std::set<const Value *> &,
                                          const Instruction &);
};

} // namespace deps