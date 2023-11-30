#include "ProgramInfoHelper.h"

#define DEBUG 0

using namespace std;

namespace ProgramInfoHelper
{

	/*
		Gets a list of arguments for a specified function.

		Each argument is saved in a struct function_argument, which
		holds a string representing the argument and the store instruction where it is initially stored.

		Parameters: llvm::Function
		Return: A list of function_argument structs. One for each argument.
	*/
	std::list<function_argument> getFunctionArguments(Function* function)
    {

		std::list<function_argument> function_arguments_list;

		for (auto psArg = function->arg_begin(); psArg != function->arg_end(); psArg++)
		{

            int found_store = 0;

			for (auto psUse = psArg->user_begin(); psUse!= psArg->user_end(); psUse++)
			{
				Value* argument_value = psUse->getOperand(0);

                std::string argument_string;
                raw_string_ostream rso(argument_string);
                argument_value->print(rso);



				if ((argument_value) && (argument_string != ""))
                {
					for (auto f_block = function->begin(); f_block != function->end(); f_block++)
					{
						for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++)
						{
                            if (found_store == 0)
                            {
                                if (StoreInst* store_inst = dyn_cast<StoreInst>(f_inst))
                                {
                                    std::string store_inst_string;
                                    raw_string_ostream rso(store_inst_string);
                                    store_inst->print(rso);

                                    if (store_inst_string.find(argument_string) != std::string::npos) {
                                        //errs() << "Store: " << store_inst_string << "\n";

                                        found_store = 1;
                                        function_argument temp_arg = function_argument(argument_string, store_inst_string, store_inst);

                                        std::string source_line_info;
                                        const llvm::DebugLoc &debugInfo = store_inst->getDebugLoc();

                                        if (debugInfo) {
                                            std::string directory = debugInfo->getDirectory().str();
                                            std::string filePath = debugInfo->getFilename().str();
                                            int line = debugInfo->getLine();
                                            int column = debugInfo->getColumn();

                                            source_line_info = directory + "/" + filePath + ":" + to_string(line) + ":" + to_string(column);

                                        }

                                        // We will want to return a list of function_argument stucts
                                        function_arguments_list.push_back(temp_arg);
                                    }
                                } // END IF STORE INSTRUCTION
                            }
						} // END FOR F_INST =
					} // END FOR F_BLOCK =
		  		}
				else
				{
					errs() << "Value is null or argument name is empty.\n";
				} // END IF-ELSE CHECK IF NULL INSTRUCTION
			}
		}

		return function_arguments_list;
	}

	/*
		Gets a list of return instructions for a specified function.

		Each return is saved in a struct function_return, which
		holds an llvm::Instruction related to the return instruction
	    in addition to a string representation of the instruction.

		Parameters: llvm::Function
		Return: A list of function_return structs. One for each return.
	*/


	std::list<function_return> getFunctionReturns(Function* function)
	{
		std::list<function_return> ret_inst_list;

	  	Value* returnValue;
		Value* loadValue;
	  
	  	// Iterate through each basic block
	  	for (Function::iterator BB = function->begin(), endBB = function->end(); BB!= endBB; ++BB)
	  	{
			// Iterate through each instruction
			for (BasicBlock::iterator I = BB->begin(), endI = BB->end(); I != endI; ++I)
			{
		 	 	// Check if it is a return instruction
		  		if (ReturnInst *return_inst = dyn_cast<ReturnInst>(&*I))
		  		{
                    std::string return_inst_string;
                    raw_string_ostream rso(return_inst_string);
                    return_inst->print(rso);

                    function_return temp_return = function_return(return_inst_string, return_inst);
                    ret_inst_list.push_back(temp_return);
		  		}
			}
	  	}
	  return ret_inst_list;
	}


    std::string getFunctionStart(Function* function, std::string app_name) {
        std::string starting_location;
        std::string ending_location;

        bool found_start = false;
        for (Function::iterator BB = function->begin(), endBB = function->end(); BB!= endBB; ++BB) {
            // Iterate through each instruction
            for (BasicBlock::iterator I = BB->begin(), endI = BB->end(); I != endI; ++I) {

                const llvm::DebugLoc &debugInfo = I->getDebugLoc();

                if (debugInfo) {
                    std::string directory = debugInfo->getDirectory().str();
                    std::string filePath = debugInfo->getFilename().str();
                    int line = debugInfo->getLine();
                    int column = debugInfo->getColumn();

                    std::string source_line_info = directory + "/" + filePath + ":" + std::to_string(line) + ":" + std::to_string(column);

                    return directory.substr(directory.find(app_name), directory.size() - 1) + "/" + filePath + ":" + to_string(line);
                }
            }
        }
        return "";
    }


    /**
     * For a given function identify the source code line number range that the function encompasses
     * Identify function argument stores as a starting line number
     * Identify all return instructions and leverage the highest line number as the last line
     * Return a pair of debug locations representing the starting and ending locations
     * @param function
     * @return
     */
    std::pair<std::string, std::string> getSourceRange(Function* function) {

        std::string starting_location;
        std::string ending_location;

        bool found_start = false;
        for (Function::iterator BB = function->begin(), endBB = function->end(); BB!= endBB; ++BB) {
            // Iterate through each instruction
            for (BasicBlock::iterator I = BB->begin(), endI = BB->end(); I != endI; ++I) {

                const llvm::DebugLoc &debugInfo = I->getDebugLoc();

                if (debugInfo) {
                    std::string directory = debugInfo->getDirectory().str();
                    std::string filePath = debugInfo->getFilename().str();
                    int line = debugInfo->getLine();
                    int column = debugInfo->getColumn();

                    std::string source_line_info = directory + "/" + filePath + ":" + std::to_string(line) + ":" + std::to_string(column);
                    starting_location = source_line_info;
                    found_start = true;
                    break;

                }
            }

            if (found_start) {
                break;
            }
        }

        Value* returnValue;

        std::list<ReturnInst*> return_list;

        // Iterate through each basic block
        for (Function::iterator BB = function->begin(), endBB = function->end(); BB!= endBB; ++BB) {
            // Iterate through each instruction
            for (BasicBlock::iterator I = BB->begin(), endI = BB->end(); I != endI; ++I) {

                // Check if it is a return instruction
                if (ReturnInst *returnInst = dyn_cast<ReturnInst>(&*I)) {

                    const llvm::DebugLoc &debugInfo = returnInst->getDebugLoc();

                    if (debugInfo) {
                        std::string directory = debugInfo->getDirectory().str();
                        std::string filePath = debugInfo->getFilename().str();
                        int line = debugInfo->getLine();
                        int column = debugInfo->getColumn();

                        std::string source_line_info = directory + "/" + filePath + ":" + std::to_string(line) + ":" + std::to_string(column);
                        ending_location = source_line_info;

                    }
                }
            }
        }

        std::pair<std::string, std::string> source_range;
        source_range = std::make_pair(starting_location, ending_location);

        return source_range;
    }

    /**
     * Naive iterate through all functions, through all basic blocks, through all instructions.
     * If the instruction label matches the provided isntruction_string the corresponding
     * instruction will be returned.
     *
     * @param instruction_string string representing bitcode instruction
     * @param M program module for this analysis (Needed to get list of functions err function iterator)
     * @return corresponding Instruction if a match is found, nullptr otherwise.
     */
    Instruction* find_instruction_from_string(std::string instruction_string, Module &M)
    {
        Instruction* null_inst = nullptr;

        for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it)
        {
            if (!func_it->isDeclaration())
            {
                for (Function::iterator block_it = func_it->begin(); block_it != func_it->end(); ++block_it)
                {
                    for (BasicBlock::iterator inst = block_it->begin(); inst != block_it->end(); ++inst) {
                        std::string inst_string;
                        raw_string_ostream rso(inst_string);
                        inst->print(rso);

                        if (inst_string == instruction_string)
                        {
                            Instruction *temp_inst = &(*inst);

                            return temp_inst;
                        }
                    }
                }
            }
        }
        return null_inst;
    }

    std::vector<Instruction*> get_store_for_function_return(Module &M, Function &F)
    {
        std::vector<Instruction*> inst_list;

        for (Module::iterator mod = M.begin(), mod_e = M.end(); mod != mod_e; ++mod)
        {
            if(!mod->isDeclaration())
            {
                for (Function::iterator func = mod->begin(), func_e = mod->end(); func != func_e; ++func)
                {
                    for (BasicBlock::iterator block = func->begin(), block_e = func->end(); block != block_e; ++block)
                    {
                        if (isa<CallInst>(block))
                        {
                            std::string temp_inst;
                            llvm::raw_string_ostream rso(temp_inst);
                            (&*block)->print(rso);

                            if (temp_inst.find(F.getName().str()) != std::string::npos)
                            {

                                Instruction *temp = &(*block);
                                int x = block->getNumOperands();

                                // Query for the next instruction
                                ++block;

                                // Check if it is a store instruction
                                if (isa<StoreInst>(block))
                                {
                                    inst_list.push_back(temp);
                                }
                                errs() << "\n";
                            }
                        }
                    }
                }
            }
        }
        return inst_list;

    }



    /*
     * Given a function and call instruction, find the arguments for the specified call instruction
     * within the functions list of instructions.
     *
     * 1. Parse the call instruction to find the LLVM variable names for arguments
     *      - Notable difficulty is that call instructions may be bitcast
     *      Normal: %call2 = call i32 @avc_has_perm(i32 %4,
     *                                              i32 %6,
     *                                              i16 zeroext 1,
     *                                              i32 %7,
     *                                              %struct.common_audit_data* null), !dbg !95
     *      Bitcast: %25 = call i32 bitcast(
     *               i32 (i32, i32, i16, i32, %struct.common_audit_data*)* @avc_has_perm to
     *               i32 (i32, i32, i16, i32, %struct.common_audit_data.506*)*)
     *               (i32 %19, i32 %22, i16 zeroext 5, i32 %23, %struct.common_audit_data.506* %24)  // Arguments
     *               #13, !dbg !56653
     *
     * 2. Identify the load instructions corresponding to the LLVM variables.
     * 3. Populate call argument structs to hold necessary information for later taint analysis
     *
     *
     *
     */
    std::list<call_argument> parse_call_instruction(Function* caller, Instruction* call_inst, std::string module_name) {

        std::list<call_argument> call_arguments;

        // Get LLVM instruction string for call instruction
        std::string call_inst_string;
        llvm::raw_string_ostream rso(call_inst_string);
        call_inst->print(rso);

        //std::string call_inst_string = "%25 = call i32 bitcast (i32 (i32, i32, i16, i32, %struct.common_audit_data*)* @avc_has_perm to i32 (i32, i32, i16, i32, %struct.common_audit_data.506*)*)(i32 %19, i32 %22, i16 zeroext 5, i32 %23, %struct.common_audit_data.506* %24) #13, !dbg !56653";
        //std::string call_inst_string = "%call2 = call i32 @avc_has_perm(i32 %4, i32 %6, i16 zeroext 1, i32 %7, %struct.common_audit_data* null), !dbg !95";

        // Handle Tomoyo Call Instructions

        // call void bitcast (void (%struct.tomoyo_request_info*, i1 (%struct.tomoyo_request_info*, %struct.tomoyo_acl_info*)*)* @tomoyo_check_acl to void (%struct.tomoyo_request_info.5871*, i1 (%struct.tomoyo_request_info.5871*, %struct.tomoyo_acl_info*)*)*)(%struct.tomoyo_request_info.5871* %r, i1 (%struct.tomoyo_request_info.5871*, %struct.tomoyo_acl_info*)* @tomoyo_check_inet_acl) #12, !dbg !124306
        // Tomoyo's bitcast calls have an additional casting and hence an additional set of '(' & ')'
        if (strcmp(module_name.c_str(), "tomoyo") == 0) {

            if (call_inst_string.find("bitcast ") != std::string::npos) {

                std::string first_paren = call_inst_string.substr(call_inst_string.find(')') + 1, call_inst_string.size() - 1);
                std::string second_paren = first_paren.substr(first_paren.find(')') + 1, first_paren.size() - 1);
                std::string third_paren = second_paren.substr(second_paren.find(')') + 1, second_paren.size() - 1);
                std::string fourth_paren = third_paren.substr(third_paren.find(')') + 1, third_paren.size() - 1);
                std::string fifth_paren = fourth_paren.substr(fourth_paren.find(')') + 1, fourth_paren.size() - 1);

                std::string argument_string = fifth_paren.substr(fifth_paren.find('(') + 1, fifth_paren.find(')') - 1);

                std::string first_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string first_argument = first_argument_string.substr(first_argument_string.find(" ") + 1, first_argument_string.size() -1);

                // Cut second_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string second_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string second_argument = second_argument_string.substr(second_argument_string.find(" ") + 1, second_argument_string.size() -1);



                Instruction* request_load = find_load_inst_for_var_string(caller, first_argument);


                // Create call argument struct for subject
                std::string request_load_inst_string;
                raw_string_ostream rso(request_load_inst_string);
                if (request_load) {
                    request_load->print(rso);
                } else {
                    request_load_inst_string = "";
                }


                call_argument request_argument = call_argument(first_argument, request_load_inst_string, request_load);

                call_arguments.push_back(request_argument);

            }
            else {
            }

          // Handle AppArmor Call instructions
          // %call1 = call i32 @aa_path_perm(i8* %4, %struct.aa_label* %5, %struct.path* %6, i32 0, i32 %7, %struct.path_cond* %8) #10, !dbg !166121
        } else if (strcmp(module_name.c_str(), "apparmor") == 0) {

            if (call_inst_string.find("bitcast ") != std::string::npos) {
            }
            else {
                std::string argument_string = call_inst_string.substr(call_inst_string.find('(') + 1, call_inst_string.size() - 1);

                std::string first_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string first_argument = first_argument_string.substr(first_argument_string.find(" ") + 1, first_argument_string.size() -1);

                // Cut first_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string second_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string second_argument = second_argument_string.substr(second_argument_string.find(" ") + 1, second_argument_string.size() -1);

                // Cut second_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string third_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string third_argument = third_argument_string.substr(third_argument_string.find(" ") + 1, third_argument_string.size() -1);

                // Cut third_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fourth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fourth_argument = fourth_argument_string.substr(fourth_argument_string.find(" ") + 1, fourth_argument_string.size() -1);

                // Cut fourth_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fifth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fifth_argument = fifth_argument_string.substr(fifth_argument_string.find(" ") + 1, fifth_argument_string.size() -1);

                // Cut fourth_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string sixth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string sixth_argument = sixth_argument_string.substr(sixth_argument_string.find(" ") + 1, sixth_argument_string.size() -1);

                Instruction* operation_load = find_load_inst_for_var_string(caller, first_argument);
                Instruction* profile_load = find_load_inst_for_var_string(caller, second_argument);
                Instruction* path_load = find_load_inst_for_var_string(caller, third_argument);
                Instruction* flag_load = find_load_inst_for_var_string(caller, fourth_argument);
                Instruction* request_load = find_load_inst_for_var_string(caller, fifth_argument);
                Instruction* cond_load = find_load_inst_for_var_string(caller, sixth_argument);

                // Create call argument struct for operation
                std::string operation_load_inst_string;
                raw_string_ostream rso(operation_load_inst_string);
                if (operation_load) {
                    operation_load->print(rso);
                } else {
                    operation_load_inst_string = "";
                }

                call_argument operation_argument = call_argument(first_argument, operation_load_inst_string, operation_load);

                // Create call argument struct for profile
                std::string profile_load_inst_string;
                raw_string_ostream rso1(profile_load_inst_string);
                if (profile_load) {
                    profile_load->print(rso1);
                } else {
                    profile_load_inst_string = "";
                }

                call_argument profile_argument = call_argument(second_argument, profile_load_inst_string, profile_load);

                // Create call argument struct for path
                std::string path_load_inst_string;
                raw_string_ostream rso2(path_load_inst_string);
                if (path_load) {
                    path_load->print(rso2);
                } else {
                    path_load_inst_string = "";
                }

                call_argument path_argument = call_argument(third_argument, path_load_inst_string, path_load);


                // Create call argument struct for operation
                std::string flag_load_inst_string;
                raw_string_ostream rso3(flag_load_inst_string);
                if (flag_load) {
                    flag_load->print(rso3);
                } else {
                    flag_load_inst_string = "";
                }

                call_argument flag_argument = call_argument(fourth_argument, flag_load_inst_string, flag_load);

                // Create call argument struct for operation
                std::string request_load_inst_string;
                raw_string_ostream rso4(request_load_inst_string);
                if (request_load) {
                    request_load->print(rso4);
                } else {
                    request_load_inst_string = "";
                }

                call_argument request_argument = call_argument(fifth_argument, request_load_inst_string, request_load);

                // Create call argument struct for operation
                std::string cond_load_inst_string;
                raw_string_ostream rso5(cond_load_inst_string);
                if (cond_load) {
                    cond_load->print(rso5);
                } else {
                    cond_load_inst_string = "";
                }

                call_argument cond_argument = call_argument(sixth_argument, cond_load_inst_string, cond_load);

                call_arguments.push_back(operation_argument);
                call_arguments.push_back(profile_argument);
                call_arguments.push_back(path_argument);
                call_arguments.push_back(flag_argument);
                call_arguments.push_back(request_argument);
                call_arguments.push_back(cond_argument);


            }

            // Handle SELinux Call Instructions
        } else {
            // If the instruction is a bitcast we will need to parse the instruction differently
            if (call_inst_string.find("bitcast ") != std::string::npos) {


                std::string first_paren = call_inst_string.substr(call_inst_string.find(')') + 1, call_inst_string.size() - 1);
                std::string second_paren = first_paren.substr(first_paren.find(')') + 1, first_paren.size() - 1);
                std::string third_paren = second_paren.substr(second_paren.find(')') + 1, second_paren.size() - 1);

                std::string argument_string = third_paren.substr(third_paren.find('(') + 1, third_paren.find(')') - 1);

                std::string first_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string first_argument = first_argument_string.substr(first_argument_string.find(" ") + 1, first_argument_string.size() -1);

                // Cut first_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string second_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string second_argument = second_argument_string.substr(second_argument_string.find(" ") + 1, second_argument_string.size() -1);

                // Cut second_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string third_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string third_argument = third_argument_string.substr(third_argument_string.find(" ") + 1, third_argument_string.size() -1);

                // Cut third_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fourth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fourth_argument = fourth_argument_string.substr(fourth_argument_string.find(" ") + 1, fourth_argument_string.size() -1);

                // Cut fourth_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fifth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fifth_argument = fifth_argument_string.substr(fifth_argument_string.find(" ") + 1, fifth_argument_string.size() -1);

                Instruction* subject_load = find_load_inst_for_var_string(caller, first_argument);
                Instruction* object_load = find_load_inst_for_var_string(caller, second_argument);
                Instruction* operation_load = find_load_inst_for_var_string(caller, fourth_argument);

                // Create call argument struct for subject
                std::string subject_load_inst_string;
                raw_string_ostream rso(subject_load_inst_string);
                subject_load->print(rso);


                call_argument subject_argument = call_argument(first_argument, subject_load_inst_string, subject_load);



                // Create call argument struct for object
                std::string object_load_inst_string;
                raw_string_ostream rso2(object_load_inst_string);
                if (object_load) {
                    object_load->print(rso2);
                } else {
                    object_load_inst_string = "";
                }


                call_argument object_argument = call_argument(second_argument, object_load_inst_string, object_load);
                if (second_argument.find('%') == std::string::npos) {
                    object_argument.is_constant = true;
                }


                // Create call argument struct for object
                std::string operation_load_inst_string;
                raw_string_ostream rso3(operation_load_inst_string);
                if (operation_load) {
                    operation_load->print(rso3);
                } else {
                    operation_load_inst_string = "";
                }


                if (fourth_argument.find('%') == std::string::npos) {
                    operation_load_inst_string = "";
                    operation_load = NULL;
                }


                call_argument operation_argument = call_argument(fourth_argument, operation_load_inst_string, operation_load);

                if (fourth_argument.find('%') == std::string::npos) {
                    operation_argument.is_constant = true;
                }

                call_arguments.push_back(subject_argument);
                call_arguments.push_back(object_argument);
                call_arguments.push_back(operation_argument);

                if (DEBUG) {
                    errs() << "\t" << first_paren<< "\n";
                    errs() << "\t" << second_paren << "\n";
                    errs() << "\t" << third_paren  << "\n";

                    errs() << "\t" << first_argument  << "\n";
                    errs() << "\t" << second_argument << "\n";
                    errs() << "\t" << third_argument  << "\n";
                    errs() << "\t" << fourth_argument << "\n";
                    errs() << "\t" << fifth_argument_string  << "\n";

                    errs() << argument_string  << "\n";
                }

            } else {
                std::string argument_string = call_inst_string.substr(call_inst_string.find('(') + 1, call_inst_string.size() - 1);

                std::string first_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string first_argument = first_argument_string.substr(first_argument_string.find(" ") + 1, first_argument_string.size() -1);

                // Cut first_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string second_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string second_argument = second_argument_string.substr(second_argument_string.find(" ") + 1, second_argument_string.size() -1);

                // Cut second_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string third_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string third_argument = third_argument_string.substr(third_argument_string.find(" ") + 1, third_argument_string.size() -1);

                // Cut third_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fourth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fourth_argument = fourth_argument_string.substr(fourth_argument_string.find(" ") + 1, fourth_argument_string.size() -1);

                // Cut fourth_argument
                argument_string = argument_string.substr(argument_string.find(',') + 2, argument_string.size() - 1);

                std::string fifth_argument_string = argument_string.substr(0, argument_string.find(','));
                std::string fifth_argument = fifth_argument_string.substr(fifth_argument_string.find(" ") + 1, fifth_argument_string.size() -1);

                Instruction* subject_load = find_load_inst_for_var_string(caller, first_argument);
                Instruction* object_load = find_load_inst_for_var_string(caller, second_argument);
                Instruction* operation_load = find_load_inst_for_var_string(caller, fourth_argument);

                // Create call argument struct for subject
                std::string subject_load_inst_string;
                raw_string_ostream rso(subject_load_inst_string);
                subject_load->print(rso);

                call_argument subject_argument = call_argument(first_argument, subject_load_inst_string, subject_load);

                // Create call argument struct for object
                std::string object_load_inst_string;
                raw_string_ostream rso2(object_load_inst_string);

                if (object_load) {
                    object_load->print(rso2);
                } else {
                    object_load_inst_string = "";
                }
                call_argument object_argument = call_argument(second_argument, object_load_inst_string, object_load);

                // Create call argument struct for object
                std::string operation_load_inst_string;
                raw_string_ostream rso3(operation_load_inst_string);
                if (operation_load) {
                    operation_load->print(rso3);
                } else {
                    operation_load_inst_string = "";
                }

                call_argument operation_argument = call_argument(fourth_argument, operation_load_inst_string, operation_load);
                if (fourth_argument.find('%') == std::string::npos) {
                    operation_argument.is_constant = true;
                }

                call_arguments.push_back(subject_argument);
                call_arguments.push_back(object_argument);
                call_arguments.push_back(operation_argument);

                if (DEBUG) {
                    errs() << "Arguments: ";
                    errs() << argument_string  << "\n";
                    errs() << "\t" << first_argument<< "\n";
                    errs() << "\t" << second_argument << "\n";
                    errs() << "\t" << third_argument  << "\n";
                    errs() << "\t" << fourth_argument << "\n";
                    errs() << "\t" << fifth_argument_string  << "\n";

                    errs() << argument_string  << "\n";
                }
            }
        }
        return call_arguments;
    }


    Instruction* find_load_inst_for_var_string(Function* function, std::string llvm_var) {

        Instruction *ret_inst = NULL;

        if (llvm_var.find('%') == std::string::npos) {
            return ret_inst;
        }

        for (auto f_block = function->begin(); f_block != function->end(); f_block++) {
            for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++) {
                if (LoadInst *load_inst = dyn_cast<LoadInst>(f_inst)) {
                    std::string load_inst_string;
                    raw_string_ostream rso(load_inst_string);
                    load_inst->print(rso);

                    std::string load_var_string = load_inst_string.substr(load_inst_string.find('%'), load_inst_string.size() -1);
                    std::string load_var = load_var_string.substr(0, load_var_string.find(' '));

                    if (load_var == llvm_var) {
                        return load_inst;
                    }

                } else {

                    if (Instruction *inst = dyn_cast<Instruction>(f_inst)) {
                        std::string inst_string;
                        raw_string_ostream rso(inst_string);
                        inst->print(rso);
                        if (inst_string.find('%') != std::string::npos) {
                            std::string var_string = inst_string.substr(inst_string.find('%'), inst_string.size() - 1);
                            std::string var = var_string.substr(0, var_string.find(' '));
                            if (var == llvm_var) {
                                ret_inst = inst;
                            }
                        }
                    }
                }
            }
        }

        return ret_inst;
    }



    // This test iterates through all instructions and print out debugging information.
    // This test was primary used to debug LLVM debug info to determine if any instructions are missing debugging information
    void test_debugging_symbols(Module &M)
    {
        for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it) {
            if (!func_it->isDeclaration()) {
                Function *temp_func = &(*func_it);
                for (Function::iterator block_it = func_it->begin(); block_it != func_it->end(); ++block_it) {
                    for (BasicBlock::iterator inst = block_it->begin(); inst != block_it->end(); ++inst) {


                        Instruction *temp_inst = &(*inst);

                        if (temp_inst) {
                            const llvm::DebugLoc &debugInfo = temp_inst->getDebugLoc();
                            if (debugInfo) {
                                std::string directory = debugInfo->getDirectory().str();
                                std::string filePath = debugInfo->getFilename().str();
                                int line = debugInfo->getLine();
                                int column = debugInfo->getColumn();

                                std::string source_line_info = directory + "/" + filePath + ":" + std::to_string(line) + ":" + std::to_string(column);

                                errs() << "Function: " << temp_func->getName().str() << " : ";
                                inst->print(errs());
                                errs() << " : " << source_line_info << "\n";

                            }
                            else {
                                inst->print(errs());
                                errs() << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
}