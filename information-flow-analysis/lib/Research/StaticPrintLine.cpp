#define DEBUG_TYPE "StaticPrintLine"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

namespace {
	struct StaticPrintBBLine : public ModulePass {
		static char ID;
		StaticPrintBBLine() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M);
		virtual bool runOnFunction(Function &F, Module &M);
		virtual bool runOnBasicBlock(BasicBlock &BB, Module &M);

	};
}
char StaticPrintBBLine::ID = 0;
static RegisterPass<StaticPrintBBLine> X("sprtLnNum", "Print Line Number For Each BasicBlock Starts and Ends");

bool StaticPrintBBLine::runOnModule(Module &M) {
	bool retval = false;

	for (Module::iterator F = M.begin(), E = M.end(); F != E; F++) {
		retval |= runOnFunction(*F, M);
	}
	return retval;
}

bool StaticPrintBBLine::runOnFunction(Function &F, Module &M) {
	bool retval = false;
	errs() << F.hasUWTable() << "\n";
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
		retval |= runOnBasicBlock(*BB, M);
	}
	return retval;
}

bool StaticPrintBBLine::runOnBasicBlock(BasicBlock &BB, Module &M) {
	for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; I++) {
		if (isa<PHINode>(*I)) continue;
		if (MDLocation *loc = I->getDebugLoc()) {
			loc->dump();
			errs() << loc->getFilename() << "\n";
			if (loc = (--E)->getDebugLoc())
				loc->dump();
			break;
		}
	}
	return false;
}
