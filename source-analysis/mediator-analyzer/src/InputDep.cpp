

#include "InputDep.h"


/*
 * Configuration options
 *
 */

#define DEBUG 0

// Constant string definitions for config values
#define TRUE "true"

// Constant string definitions for config keys
#define PRINT_INPUT_TAINT "print_input_taint"


// Constant string definitions for taint source, mediator inputs.
#define TAINT_SOURCE "taint_source"
#define TAINT_TYPE_FUNCTION "function"
#define TAINT_TYPE_VARIABLE "variable"
#define TAINT_TYPE_RETURN "return"

// Specify the application name (or root level directory where source code is located)
#define APPLICATION_NAME "application_name"
#define MODULE_NAME "module_name"

// Specify the authorization function where the reject or grant decision is computed.
#define AUTHORIZATION_FUNCTION "authorization_function"

// Define all high level hook functions for the reference monitor API
#define HOOK_FUNCTION "hook_function"

// Define subject filter
#define SUBJECT_FILTER "subject_filter"

#define DATA_SOURCE_LABEL "data_source_label"

// Config Definitions for call graph generation
#define ROOT_FUNCTION "root_function"
#define SINK_FUNCTION "sink_function"
#define CALL_DEPTH "call_depth"
#define GENERATE_CALL_STRINGS "generate_call_strings"
#define CALL_STRING_SENSITIVE "call_strings_context_sensitive"
#define OUTPUT_CALL_STRING "output_call_strings"

// Tags for constant meta data fed from Clang-AST analysis
#define CONSTANT "constant"

// Constant Input files from pre-processing analysis.
#define PP_FILE "pp_file_path"
#define AST_FILE "ast_file_path"

cl::opt<std::string> ConfigFile("configFile", cl::desc("<Config file for taint analysis tool>"), cl::init("-"));





void InputDep::ReadConfigFile(Module &M){


    std::ifstream srcFile (ConfigFile.c_str(), std::ifstream::in);
    std::string line;

	
	// If a config file was not found, but one was specified on the command line
	// report an error.
    if(!srcFile)
    {
        errs() << "Could not open the Config file \n";
    }
    else
    {
        // Initial check if line is empty. If not, proceed.
        while(!srcFile.eof())
        {

            getline(srcFile, line);

            // Next check that there is an "=". If not, the configuration line should be ignored.
            if (line.find('=') != std::string::npos) {
                // Get rid of spaces.


                //line.erase( std::remove_if( line.begin(), line.end(), ::isspace ), line.end() );

                std::string key = line.substr(0, line.find('='));
                std::string value = line.substr(line.find('=') + 1, line.size()-1);

                //errs() << "Key: " << key << "\n" << "Value: " << value << "\n";

                if ((!key.empty()) && (!value.empty())) {
                    const char *key_c = key.c_str();
                    const char *value_c = value.c_str();

                    if (strcmp(key_c, APPLICATION_NAME) == 0)
                    {
                        config.application_name = value_c;
                    }

                    if (strcmp(key_c, MODULE_NAME) == 0)
                    {
                        config.module_name = value_c;
                    }

                    if (strcmp(key_c, SUBJECT_FILTER) == 0)
                    {
                        config.subject_filters.push_back(value_c);
                    }

                    // Handle printing forward taint bool
                    if (strcmp(key_c, PRINT_INPUT_TAINT) == 0)
                    {
                        config.print_input_taint = strcmp(value_c, TRUE) == 0;
                    }

                    else if (strcmp(key_c, ROOT_FUNCTION) == 0) {
                        config.root_function_string = value_c;

                        for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it) {
                            if (!func_it->isDeclaration()) {
                                Function *temp_func = &(*func_it);
                                if (temp_func->getName() == value_c) {
                                    config.root_function = temp_func;
                                }
                            }
                        }
                    }

                    else if(strcmp(key_c, SINK_FUNCTION) == 0) {
                        config.sink_function_string = value_c;

                        for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it) {
                            if (!func_it->isDeclaration()) {
                                Function *temp_func = &(*func_it);
                                if (temp_func->getName() == value_c) {
                                    config.sink_function = temp_func;
                                }
                            }
                        }
                    }

                    else if (strcmp(key_c, CALL_DEPTH) == 0) {
                        config.call_depth = stoi(value_c, 0, 10);
                    }

                    else if (strcmp(key_c, GENERATE_CALL_STRINGS) == 0) {
                        config.generate_call_strings = strcmp(value_c, TRUE) == 0;
                    }
                    else if (strcmp(key_c, CALL_STRING_SENSITIVE) == 0) {
                        config.call_string_context_sensitive = strcmp(value_c, TRUE) == 0;
                    }
                    else if (strcmp(key_c, OUTPUT_CALL_STRING) == 0) {
                        if (strcmp(value_c, "dot") == 0)
                        {
                            config.output_call_strings_to_dot = true;
                        }
                        else if(strcmp(key_c, "stdout") == 0)
                        {
                            config.output_call_strings_to_stdout = true;
                        }
                    }

                    else if (strcmp(key_c, PP_FILE) == 0) {
                        config.pp_file_string = value_c;
                    }

                    else if (strcmp(key_c, AST_FILE) == 0) {
                        config.ast_file_string = value_c;
                    }

                    else if (strcmp(key_c, CONSTANT) == 0) {

                        std::string sub_str_1 = value.substr(value.find(':') + 1, value.size() - 1);
                        std::string sub_str_2 = sub_str_1.substr(sub_str_1.find(':') + 1, sub_str_1.size() - 1);
                        std::string sub_str_3 = sub_str_2.substr(sub_str_2.find(':') + 1, sub_str_2.size() - 1);
                        std::string sub_str_4 = sub_str_3.substr(sub_str_3.find(':') + 1, sub_str_3.size() - 1);
                        std::string sub_str_5 = sub_str_4.substr(sub_str_4.find(':') + 1, sub_str_4.size() - 1);
                        std::string sub_str_6 = sub_str_5.substr(sub_str_5.find(' ') + 1, sub_str_5.size() - 1);

                        std::string function_name = value.substr(0, value.find(':'));
                        std::string constant_type = sub_str_1.substr(0, sub_str_1.find(':'));
                        std::string constant_value = sub_str_2.substr(0, sub_str_2.find(':'));
                        std::string file_name = sub_str_3.substr(0, sub_str_3.find(':'));
                        std::string line = sub_str_4.substr(0, sub_str_4.find(':'));
                        std::string col = sub_str_5.substr(0, sub_str_5.find(' '));

                        ConstantInfo *constant_input = new ConstantInfo(function_name, constant_type, constant_value,
                                                                        file_name, line, col);

                        if (DEBUG) {
                        errs() << "Constant: \n"
                               << "\t" << function_name << "\n"
                               << "\t" << constant_type << "\n"
                               << "\t" << constant_value << "\n"
                               << "\t" << file_name << "\n"
                               << "\t" << line << "\n"
                               << "\t" << col << "\n";
                        }

                        // Definition is also defined
                        if (sub_str_6.size() > 3)
                        {
                            std::string sub_str_7 = sub_str_6.substr(sub_str_6.find(' ') + 1, sub_str_6.size() - 1);
                            std::string sub_str_8 = sub_str_7.substr(sub_str_7.find(':') + 1, sub_str_7.size() - 1);
                            std::string sub_str_9 = sub_str_8.substr(sub_str_8.find(':') + 1, sub_str_8.size() - 1);


                            std::string arrow = sub_str_6.substr(0, sub_str_6.find(' '));
                            std::string def_file_name = sub_str_7.substr(0, sub_str_7.find(':'));
                            std::string def_line = sub_str_8.substr(0, sub_str_8.find(':'));
                            std::string def_col = sub_str_9.substr(0, sub_str_9.size() - 1);

                            constant_input->def_file_name = def_file_name;
                            constant_input->def_line = def_line;
                            constant_input->def_col = def_col;

                            if (DEBUG) {
                                errs() << "\t Definition: \n"
                                       << "\t" << def_file_name << "\n"
                                       << "\t" << def_line << "\n"
                                       << "\t" << def_col << "\n";
                            }
                        }

                        config.constant_list.push_back(constant_input);

                    }

                    // Handle new taint source
                    else if (strcmp(key_c, TAINT_SOURCE) == 0) {

                        std::string taint_type = value.substr(0, value.find(':'));
                        std::string taint_source = value.substr(value.find(':') + 1, value.size() - 1);

                        if (taint_type == TAINT_TYPE_VARIABLE) {

                            std::string temp_function_name = taint_source.substr(0, taint_source.find(':'));
                            std::string rest = taint_source.substr(taint_source.find(':') + 1, taint_source.size() - 1);
                            std::string temp_variable = rest.substr(0, rest.find(':'));
                            std::string temp_label = rest.substr(rest.find(':') + 1, rest.size() - 1);

                            Function *temp_function = M.getFunction(temp_function_name);

                            for (auto f_block = temp_function->begin(); f_block != temp_function->end(); f_block++)
                            {
                                for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++)
                                {

                                    if (StoreInst* store_inst = dyn_cast<StoreInst>(f_inst))
                                    {

                                        if(temp_variable == f_inst->getOperand(1)->getName())
                                        {

                                            TaintSource *temp_source = new TaintSource();
                                            temp_source->functionName = temp_function->getName().str();
                                            temp_source->variable = temp_variable;
                                            temp_source->label = temp_label;
                                            temp_source->instruction = store_inst;

                                            config.user_taint_inputs.push_back(temp_source);
                                        }
                                    } // END IF STORE INSTRUCTION
                                } // END FOR F_INST =
                            } // END FOR F_BLOCK =
                        } // END TAINT_TYPE VARIABLE

                        else if (taint_type == TAINT_TYPE_FUNCTION)
                        {

                            std::string function_name = taint_source.substr(0, taint_source.find(':'));

                            Function *temp_function = M.getFunction(function_name);

                            std::string temp_label = taint_source.substr(taint_source.find(':') + 1, taint_source.size());

                            // Make sure the function was found.
                            if (temp_function != NULL)
                            {
                                for (auto psArg = temp_function->arg_begin(); psArg != temp_function->arg_end(); psArg++)
                                {
                                    std::string argument_name = psArg->getName().str();

                                    errs() << "Searching for: " << argument_name << "\n";

                                    Instruction *temp_inst = find_first_instruction_using_arg(temp_function, argument_name);

                                    if (temp_inst != NULL) {

                                        TaintSource *temp_source = new TaintSource();
                                        temp_source->functionName = temp_function->getName().str();
                                        temp_source->variable = argument_name;
                                        temp_source->label = temp_label;
                                        temp_source->instruction = temp_inst;

                                        config.user_taint_inputs.push_back(temp_source);

                                    } // If no instruction is found, inst is null
                                    else
                                    {
                                        errs() << "Unable to identify instruction for argument. Double check .ll file\n";
                                    }
                                }


                            } // END IF-FUNCTION
                        } // END TAINT_TYPE_FUNCTION
                        else if (taint_type == TAINT_TYPE_RETURN)
                        {
                            std::string callee_name = taint_source.substr(0, taint_source.find(':'));
                            std::string rest = taint_source.substr(taint_source.find(':') + 1, taint_source.size() - 1);
                            std::string caller_name = rest.substr(0, rest.find(':'));
                            std::string temp_label = rest.substr(rest.find(':') + 1, rest.size() - 1);

                            Function *temp_caller = M.getFunction(caller_name);

                            std::string curr_lhs;

                            for (Function::iterator block_it = temp_caller->begin(); block_it != temp_caller->end(); ++block_it) {
                                for (BasicBlock::iterator inst = block_it->begin(); inst != block_it->end(); ++inst) {

                                    if (isa<CallInst>(inst)) {
                                        std::string temp_inst;
                                        llvm::raw_string_ostream rso(temp_inst);
                                        (&*inst)->print(rso);

                                        if (temp_inst.find(callee_name) != std::string::npos) {

                                            Instruction *temp_call = &(*inst);
                                            curr_lhs = temp_call->getName().str();

                                            // Grab next instruction, which will use result
                                            ++inst;

                                            Instruction *temp = &(*inst);

                                            TaintSource *temp_source = new TaintSource();
                                            temp_source->functionName = temp->getFunction()->getName().str();
                                            temp_source->variable = temp->getName().str();
                                            temp_source->label = temp_label;
                                            temp_source->instruction = temp;

                                            config.user_taint_inputs.push_back(temp_source);
                                        }
                                    }
                                }
                            }
                        }
                        else {
                            errs() << "Taint type not specified.\n";
                        }
                    }

                        // Handle mediator function inputs
                    else if (strcmp(key_c, AUTHORIZATION_FUNCTION) == 0) {
                        config.authorization_functions.push_back(value_c);
                    }
                    else if (strcmp(key_c, HOOK_FUNCTION) == 0) {
                        config.hook_functions.push_back(value_c);
                    }

                    else if (strcmp(key_c, DATA_SOURCE_LABEL) == 0) {


                        std::string instruction_string = value.substr(0, value.find(':'));
                        std::string source_label = value.substr(value.find(':') + 1, value.size() - 1);

                        config.source_label_map[instruction_string] = source_label;
                    }


                } // End if key && value empty
            } // End if delimiter in line
            else {
                continue;
            }
        } // End while line !empty
    } // End if srcfile
} // End ReadConfig()

Instruction* InputDep::find_first_instruction_using_arg(Function* function, std::string arg_name)
{


    std::string var_name = arg_name;

    if (var_name != "") {

        for (auto f_block = function->begin(); f_block != function->end(); f_block++) {
            for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++) {

                Instruction* temp_inst = &(*f_inst);

                for (auto user_it = f_inst->op_begin(); user_it != f_inst->op_end(); user_it++) {
                    Value *temp_value = user_it->get();

                    if (var_name == temp_value->getName()) {
                        return temp_inst;
                    }
                }
            }
        }
    }   // End check for var name
    else {
        errs() << "Argument name is empty.\n";
    }

    return NULL;
}





StoreInst* InputDep::find_store_for_var_in_function(Function* function, std::string var_name)
{

    for (auto f_block = function->begin(); f_block != function->end(); f_block++)
    {
        for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++)
        {
            if (StoreInst* store_inst = dyn_cast<StoreInst>(f_inst))
            {
                errs () << "Var Name: " << var_name << " : Store Name: " << store_inst->getOperand(1)->getName() << "\n";

                if(var_name == store_inst->getOperand(0)->getName())
                {
                    errs() << store_inst << "\n";
                    return store_inst;
                }
            } // END IF STORE INSTRUCTION
        } // END FOR F_INST =
    } // END FOR F_BLOCK =

    return NULL;

}



/**
 * This pass handles inputs to the analysis tool.
 *
 * @param M
 * @return
 */
bool InputDep::runOnModule(Module &M) {

    // Read the configuration file and set appropriate settings
    ReadConfigFile(M);

    return false;
}

InputDep::InputDep() :
    ModulePass(ID) {
}

void InputDep::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
}

std::set<std::string> InputDep::getMediators() {
    return mediatorFunctions;
}

ConfigInfo InputDep::getConfigInfo()
{
    return config;
}

char InputDep::ID = 0;



static RegisterPass<InputDep> X("inputDep",
                                "Input Dependency Pass: looks for values that are input-dependant");
