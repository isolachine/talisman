#define DEBUG_TYPE "PrintBBLine"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include <map>
#include <fstream>

using namespace llvm;

/*
 * debuginfo needs -g while compiling
 * clang -g ...
 */

static int count = 0;

namespace {
	struct OutputLnTrace : public ModulePass {
		static char ID;
		OutputLnTrace() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M);
		virtual bool runOnFunction(Function &F, Module &M);
		virtual bool runOnBasicBlock(BasicBlock &BB, Module &M);

		std::map<std::string, Value *> filenames;
		std::map<DebugLoc, int> nodes;
		// std::map<MDLocation *, int> nodes;
	};
}
char OutputLnTrace::ID = 0;
static RegisterPass<OutputLnTrace> X("prtLnTrace", "Output the execution trace (line by line).");

bool OutputLnTrace::runOnModule(Module &M) {
	std::fstream fs;
  fs.open ("/tmp/llvm0", std::fstream::in);
	fs >> count;
	fs.close();

	bool retval = false;

	for (Module::iterator F = M.begin(), E = M.end(); F != E; F++) {
		retval |= runOnFunction(*F, M);
	}

	fs.open("/tmp/llvm0", std::fstream::out);
	fs << count;
	fs.close();
	return retval;
}

bool OutputLnTrace::runOnFunction(Function &F, Module &M) {
	bool retval = false;

	if (F.hasUWTable())
		for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
			retval |= runOnBasicBlock(*BB, M);
		}
	return retval;
}

bool OutputLnTrace::runOnBasicBlock(BasicBlock &BB, Module &M) {
	FunctionType *FTy = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt32Ty(M.getContext())});
	Constant *printTrace = M.getOrInsertFunction("_Z10printTracei", FTy);

	for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; I++) {
		while (isa<PHINode>(I)) I++;
		DebugLoc loc = I->getDebugLoc();
		if (!loc) continue;
		IRBuilder<> builder(I);

		int node;
		if (nodes.find(loc) == nodes.end()) {
			node = count;
			dbgs() << count << " = (" << loc->getFilename().str() << "; " << loc->getLine() << "; "  << loc->getColumn() << "; " << loc->getScope() <<"; " << loc->getInlinedAt() << ") \n";
			nodes[loc] = count++;
		}
		else
			node = nodes[loc];


		ConstantInt *nodeCI = ConstantInt::get(M.getContext(), APInt(32, uint64_t(node), false));

		builder.CreateCall(printTrace, {nodeCI});
	}

	return true;
}
