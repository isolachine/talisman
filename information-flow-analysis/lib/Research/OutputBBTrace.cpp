#define DEBUG_TYPE "PrintBBTrace"
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
	struct OutputBBTrace : public ModulePass {
		static char ID;
		OutputBBTrace() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M);
		virtual bool runOnFunction(Function &F, Module &M);
		virtual bool runOnBasicBlock(BasicBlock &BB, Module &M);

		std::map<std::string, Value *> filenames;
		std::map<DebugLoc, int> nodes;

	};
}
char OutputBBTrace::ID = 0;
static RegisterPass<OutputBBTrace> X("prtBBTrace", "Output the execution trace.");

bool OutputBBTrace::runOnModule(Module &M) {
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

bool OutputBBTrace::runOnFunction(Function &F, Module &M) {
	bool retval = false;

	if (F.hasUWTable())
		for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
			retval |= runOnBasicBlock(*BB, M);
		}
	return retval;
}

bool OutputBBTrace::runOnBasicBlock(BasicBlock &BB, Module &M) {
	FunctionType *FTy = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt32Ty(M.getContext())});
	Constant *printTrace = M.getOrInsertFunction("_Z10printTracei", FTy);
	
//	BasicBlock::iterator I = BB.getFirstInsertionPt();

	for (BasicBlock::iterator I = BB.getFirstInsertionPt(), E = BB.end(); I != E; I++) {
		while (isa<PHINode>(I)) I++;

		DebugLoc loc = I->getDebugLoc();
		if (!loc) continue;
		IRBuilder<> builder(I);

		int lEnt = loc->getLine(), cEnt = loc->getColumn();

		E--;
		while (!E->getDebugLoc()) E--;
		int lExt = E->getDebugLoc()->getLine(), cExt = E->getDebugLoc()->getColumn();

		int node;
		if (nodes.find(loc) == nodes.end()) {
			node = count;
			dbgs() << count << " = (" << loc->getFilename().str() << "; (" << lEnt << "," << lExt << "); "  \
				<< "(" << cEnt << "," << cExt << "); " << loc->getScope() <<"; " << loc->getInlinedAt() << ") \n";
			nodes[loc] = count++;
		}
		else
			node = nodes[loc];

		ConstantInt *nodeCI = ConstantInt::get(M.getContext(), APInt(32, uint64_t(node), false));

		builder.CreateCall(printTrace, {nodeCI});
	
		break;
	}

#if 0
	for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; I++) {
		MDLocation *loc = I->getDebugLoc();
		Value *filename;
		if (!loc) continue;
		if (isa<PHINode>(*I)) I++;
		IRBuilder<> builder(I);

		if (filenames.find(loc->getFilename()) == filenames.end()) {
			filename = builder.CreateGlobalStringPtr(loc->getFilename(), ".str");
			filenames[loc->getFilename()] = filename;
		}
		else
			filename = filenames[loc->getFilename()];

		ConstantInt *lineEnt = ConstantInt::get(M.getContext(), APInt(32, uint64_t(loc->getLine()), false));
		ConstantInt *lineExt;

		for (E--; !E->getDebugLoc(); E--);
		IRBuilder<> builderExt(E);
		loc = E->getDebugLoc();
		if (loc) {
			lineExt = ConstantInt::get(M.getContext(), APInt(32, uint64_t(loc->getLine()), false));
		} else {
			lineExt = ConstantInt::get(M.getContext(), APInt(32, int64_t(-1), false));
		}

		builder.CreateCall(printTrace, {filename, lineEnt, lineExt});
		builderExt.CreateCall(printTrace, {filename, lineEnt, lineExt});
		break;
	}
#endif
	return true;
}
