#ifndef __TAINTFLOW_H__
#define __TAINTFLOW_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DebugInfo.h"

#include "InputDep.h"
#include "ProgramDependenceGraph.h"
#include "TaintEngine.h"
#include "MediatorAnalysisController.h"
#include "CallGraphHelper.h"

// PDG Headers
#include "ProgramDependencyGraph.hh"
#include "SharedFieldsAnalysis.hh"

#include<set>




using namespace llvm;

class AnalysisController : public ModulePass {
	private:

        std::set<Value*> inputDepValues;
        std::set<std::string> mediatorFunctions;
		bool runOnModule(Module &M);


	public:
		static char ID;
		void getAnalysisUsage(AnalysisUsage &AU) const;

        AnalysisController();

};

#endif
