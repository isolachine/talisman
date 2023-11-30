#define DEBUG_TYPE "PrintBBLine"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <map>

using namespace llvm;

/*
 * debuginfo needs -g while compiling
 * clang -g ...
 */
namespace {
	struct PrintBBLine : public ModulePass {
		static char ID;
		PrintBBLine() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M);
		virtual bool runOnFunction(Function &F, Module &M);
		virtual bool runOnBasicBlock(BasicBlock &BB, Module &M);

		std::map<std::string, Value *> filenames;

	};
}
char PrintBBLine::ID = 0;
static RegisterPass<PrintBBLine> X("prtLnNum", "Print Line Number For Each BasicBlock Starts and Ends");

bool PrintBBLine::runOnModule(Module &M) {
	bool retval = false;

	for (Module::iterator F = M.begin(), E = M.end(); F != E; F++) {
		retval |= runOnFunction(*F, M);
	}
	return retval;
}

bool PrintBBLine::runOnFunction(Function &F, Module &M) {
	bool retval = false;

	if (F.hasUWTable())
		for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
			retval |= runOnBasicBlock(*BB, M);
		}
	return retval;
}

bool PrintBBLine::runOnBasicBlock(BasicBlock &BB, Module &M) {
	FunctionType *FTy = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt32Ty(M.getContext())});
	Constant *enter = M.getOrInsertFunction("_Z12printBBEntryiPc", FTy);
	Constant *exit = M.getOrInsertFunction("_Z11printBBExitiPc", FTy);

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

		builder.CreateCall(enter, {lineEnt, filename});

		for (E--; !E->getDebugLoc(); E--);
		IRBuilder<> builderExt(E);
		loc = E->getDebugLoc();
		if (loc) {
			ConstantInt *lineExt = ConstantInt::get(M.getContext(), APInt(32, uint64_t(loc->getLine()), false));
			builderExt.CreateCall(exit, {lineExt, filename});
		}
		break;
	}

	return true;
}
