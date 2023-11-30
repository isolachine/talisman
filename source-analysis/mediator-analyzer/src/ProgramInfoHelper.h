
#ifndef PROGRAMINFOHELPER_H
#define PROGRAMINFOHELPER_H

// LLVM includes
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfo.h"


// Tool includes
#include "Logger.h"
#include "Graph.hh"
#include "InputDep.h"

// C++ Includes
#include <list>
#include <string>
#include <utility>


// Struct to hold information regarding function arguments
struct function_argument
{
    std::string arg_string;

    Instruction *store_inst;
    std::string store_inst_string;


    function_argument(std::string arg_string, std::string store_inst_string, Instruction *store_inst)
    {
        this->arg_string = arg_string;

        this->store_inst = store_inst;
        this->store_inst_string = store_inst_string;
    }
};


// Struct to hold information regarding return instruction of function
struct function_return
{
	Instruction* ret_inst;
    std::string return_inst_string;


    function_return(std::string return_inst_string, Instruction *ret_inst)
    {
        this->ret_inst = ret_inst;
        this->return_inst_string = return_inst_string;
    }

};

// Struct to hold information regarding function arguments
struct call_argument
{
    std::string arg_string;

    Value *load_inst;
    std::string load_inst_string;

    bool is_constant;
    bool has_load_inst;


    call_argument(std::string arg_string, std::string load_inst_string, Instruction *load_inst)
    {
        this->arg_string = arg_string;

        this->load_inst = load_inst;
        this->load_inst_string = load_inst_string;

        is_constant = false;
        has_load_inst = true;
    }
};

// Struct to hold metadata for a relevant global variable
struct function_global
{

    Instruction* glob_inst;
    std::string global_inst_string;

    pdg::Node *global_pdg_node;

    function_global()
    {
        glob_inst = NULL;
        global_pdg_node = NULL;

        global_inst_string = "";
    }

};

// Prototypes held under FunctionInfoHelper namespace
namespace ProgramInfoHelper
{

    std::list<function_argument> getFunctionArguments(Function* function);
	std::list<function_return> getFunctionReturns(Function* function);

    std::pair<std::string, std::string> getSourceRange(Function* function);
    std::list<call_argument> parse_call_instruction(Function* caller, Instruction* call_inst, std::string module_name);
    Instruction* find_load_inst_for_var_string(Function* function, std::string llvm_var);
    std::string getFunctionStart(Function* function, std::string app_name);

}

#endif /* PROGRAMINFOHELPER_H */
