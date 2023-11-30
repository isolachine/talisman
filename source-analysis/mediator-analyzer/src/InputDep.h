#ifndef __INPUTDEP_H__
#define __INPUTDEP_H__


#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/CFG.h"


#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/FileSystem.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/User.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Pass.h"

#include "ProgramDependenceGraph.h"

#include <set>
#include <string>
#include <map>
#include <fstream>
#include <utility>
#include <list>



using namespace llvm;
using namespace std;


// Class to store all taint input values
class TaintSource {
public:

    std::string functionName;
    std::string variable;
    std::string label;

	std::string fieldName;
	int fieldIndex;

    Instruction* instruction;

	TaintSource() {
		this->functionName = "";
		this->variable = "";
		this->label = "";

		this->fieldName = "";
		this->fieldIndex = -1;

		this->instruction = nullptr;
	}

	void Print() {
	    errs() << "Function Name: "  << functionName << "\n";
	    errs() << "Variable: " << variable << "\n";
	    errs() << "Label: " << label << "\n";
	}
};

class ConstantInfo {
public:
    std::string function_name;
    std::string constant_type;
    std::string constant_value;
    std::string file ;
    std::string line;
    std::string col;

    std::string def_file_name = "";
    std::string def_line = "";
    std::string def_col = "";

    ConstantInfo(std::string function_name, std::string constant_type, std::string constant_value, std::string file, std::string line, std::string col) {
        this->function_name = function_name;
        this->constant_type = constant_type;
        this->constant_value = constant_value;
        this->file = file;
        this->line = line;
        this->col = col;
    }
};

//Class to store config file.
class ConfigInfo {
public:

	// Configuration flags
	bool print_input_taint;

	bool generate_call_strings;
	bool call_string_context_sensitive;
	bool output_call_strings_to_dot;
	bool output_call_strings_to_file;
	bool output_call_strings_to_stdout;

    std::string application_name;
    std::string module_name;

    std::list<std::string> subject_filters;

    std::string pp_file_string;
    std::string ast_file_string;

	std::string root_function_string;
	Function* root_function;

	std::string sink_function_string;
	Function* sink_function;

	int call_depth = 10;

	std::list<TaintSource*> user_taint_inputs;

    std::list<std::string> authorization_functions;
    std::list<std::string> hook_functions;
    std::list<ConstantInfo*> constant_list;

    std::map<string, string> source_label_map;

    ConfigInfo() {
		print_input_taint = false;
        generate_call_strings = false;
        call_string_context_sensitive = false;
        output_call_strings_to_file = true;

        application_name = "";
        module_name = "";
	};
};


class InputDep : public ModulePass {
	private:


        std::set<std::string> mediatorFunctions;
        std::set<std::string> sinkFunctions;


        ConfigInfo config;

		bool runOnModule(Module &M);


	public:
		static char ID; // Pass identification, replacement for typeid.
		InputDep();

		void getAnalysisUsage(AnalysisUsage &AU) const;
        std::set<std::string> getMediators();
        ConfigInfo getConfigInfo();

        StoreInst* find_store_for_var_in_function(Function* function, std::string var_name);
        Instruction* find_first_instruction_using_arg(Function* function, std::string arg_name);



        void ReadConfigFile(Module &M);



};

#endif
