#include "MediatorAnalysisController.h"

#define DEBUG 0

namespace MediatorAnalysisController
{
    /**
     *  Mediator Analysis Controller will direct the tesks for the mediator analysis.
     *
     *  Takes in the program module, program dependence graph, a list of hooks, and a list of input specifications
     *
     **/

    void mediatorAnalysis(Module &M, pdg::ProgramGraph *program_dependence_graph, std::list<std::string> hooks, ConfigInfo* configInfo, CallGraphHelper call_helper)
    {


        // Initialize program dependence graph
        pdg::ProgramGraph *pd_graph = program_dependence_graph;

        for (auto med_it = hooks.begin(); med_it != hooks.end(); med_it++)
        {
            if (!(M.getFunction(*med_it)))
            {
                continue;
            }

            Function* mediator = M.getFunction(*med_it);
            analyzeHook(M, program_dependence_graph, mediator, configInfo, call_helper);
        }
        return;
    }


    void analyzeHook(Module &M, pdg::ProgramGraph *program_dependence_graph, Function* function_a, ConfigInfo* configInfo, CallGraphHelper call_helper)
    {

        errs() << "-------------------------------------------------------------------------------------------\n";
        errs() << "                      Hook Analysis for: " << function_a->getName() << "                       \n";
        errs() << "-------------------------------------------------------------------------------------------\n";

        // Test printing debugging information
        for (auto f_block = function_a->begin(); f_block != function_a->end(); f_block++)
        {
            for (auto f_inst = f_block->begin(); f_inst != f_block->end(); f_inst++)
            {
                if (Instruction* inst = dyn_cast<Instruction>(f_inst))
                {
                    std::string inst_string;
                    raw_string_ostream rso(inst_string);
                    inst->print(rso);

                    std::string source_line_info;
                    const llvm::DebugLoc &debugInfo = inst->getDebugLoc();

                    if (debugInfo) {
                        std::string directory = debugInfo->getDirectory().str();
                        std::string filePath = debugInfo->getFilename().str();
                        int line = debugInfo->getLine();
                        int column = debugInfo->getColumn();

                        source_line_info = directory + "/" + filePath + ":" + to_string(line) + ":" + to_string(column);


                        //errs() << "Instruction: " << inst_string << " At: " << source_line_info << "\n";
                    }


                } // END IF STORE INSTRUCTION
            } // END FOR F_INST =
        } // END FOR F_BLOCK =





        // Create data structure to organize analysis data for the hook function being analyzed
        hook_analysis_metadata hook_analysis_data = hook_analysis_metadata();
        hook_analysis_data.hook_function = function_a;

        // Lookup hook function arguments
        std::list<function_argument> function_args = ProgramInfoHelper::getFunctionArguments(function_a);

        /*
        errs() << "Found: " << function_args.size() << " function arguments\n";
        for (auto temp_arg:function_args) {
            errs() << "\tArgument: " << temp_arg.arg_string << " " << temp_arg.store_inst_string << "\n";
        }
        */


        hook_analysis_data.function_arguments = function_args;

        // Generate call graph for hook function. We derive the rest of the functions that should be incorporated
        // into the analysis from this call graph.
        std::map<std::string, CallNode *> call_map = call_helper.get_call_map();
        std::map<std::string, CallNode*> function_call_map = call_helper.generate_function_call_graph(function_a, call_map, configInfo->authorization_functions);
        //errs() << "Call Graph for " << function_a->getName() << " contains " << function_call_map.size() << " of " << call_map.size() << " total functions.\n";

        // Store local call graph
        hook_analysis_data.local_call_graph = function_call_map;
        hook_analysis_data.program_call_graph = call_map;


        if (DEBUG) {
            // Print out the call graph
            std::map<std::string, CallNode*>::iterator temp_node_it = function_call_map.begin();
            for (; temp_node_it != function_call_map.end(); temp_node_it++)
            {
                errs() << temp_node_it->second->function_string << " : " << temp_node_it->second->successor_list.size() << "\n";
                for (auto temp_node = temp_node_it->second->successor_list.begin(); temp_node != temp_node_it->second->successor_list.end(); temp_node++)
                {
                    errs() << "\t" << (*temp_node)->function_string << "\n";
                }
            }
        }

        // Iterate through call graph and find all functions that directly call the authorization functions
        // We will want to iterate through these functions to find all callsites to the authorization function
        // as these will be the starting locations for our taint analysis.
        std::list<std::string> callers_for_callee;
        std::map<std::string, CallNode*>::iterator node_it = function_call_map.begin();
        for (; node_it != function_call_map.end(); node_it++)
        {
            for (auto temp_node = node_it->second->successor_list.begin(); temp_node != node_it->second->successor_list.end(); temp_node++)
            {
                for (auto temp_auth_function: configInfo->authorization_functions) {
                    if ((*temp_node)->function_string == temp_auth_function)
                        callers_for_callee.push_back(node_it->second->function_string);
                }
            }
        }

        if (DEBUG) {
            errs() << "The following functions call an authorization function\n";
            for(auto temp_string:callers_for_callee) {
                errs() << temp_string << "\n";
            }
        }


        // Iterate through functions that call authorization functions and find all callsites
        // For each callsite we recover the call arguments that are loaded onto the stack
        std::list<Instruction*> authorization_function_callsites;
        std::list<std::list<pdg::Node*>> reverse_taints;

        for (auto temp_string:callers_for_callee) {
            Function *caller = M.getFunction(temp_string);
            if (caller) {
                authorization_function_callsites = call_helper.find_call_sites_for_function(caller, configInfo->authorization_functions);
                for (auto temp_callsite: authorization_function_callsites) {
                    std::string call_inst_string;
                    raw_string_ostream rso(call_inst_string);
                    temp_callsite->print(rso);

                    // Parse call instruction for arguments
                    std::list<call_argument> call_args = ProgramInfoHelper::parse_call_instruction(caller, temp_callsite, configInfo->module_name);


                    // Iterate through the call arguments
                    // 1. Find call argument load instruction in PDG using Value* lookup
                    // 2. Compute a reverse taint of identified argument
                    for (auto temp_arg:call_args) {

                        pdg::Node *arg_node = program_dependence_graph->getNode(*temp_arg.load_inst);

                        std::set<pdg::EdgeType> edge_types = {pdg::EdgeType::PARAMETER_IN, pdg::EdgeType::PARAMETER_IN_REV, pdg::EdgeType::DATA_RAW_REV};

                        std::list<pdg::Node *> reverse_taint_slice;
                        reverse_taint_slice = TaintEngine::generate_reverse_taint_slice(*arg_node, hook_analysis_data.local_call_graph, edge_types, configInfo->authorization_functions);
                        hook_analysis_data.argument_taint_mapping[temp_arg.arg_string] = reverse_taint_slice;
                        reverse_taints.push_back(reverse_taint_slice);

                    }
                    // Store the call arguments for each call site
                    hook_analysis_data.auth_callsite_arg_map[call_inst_string] = call_args;
                }
            }
        }

        hook_analysis_data.authorization_call_sites = authorization_function_callsites;


        std::list<pdg::Node*> merged_taint = TaintEngine::computeUnion(reverse_taints);
        std::list<taint_source> tainted_sources = TaintEngine::find_data_sources_on_taint(M, configInfo, program_dependence_graph, function_call_map, function_a, hook_analysis_data.function_arguments, merged_taint, configInfo->constant_list);


        std::string file_name = "Results/" + hook_analysis_data.hook_function->getName().str() + ".md";
        print_sources_to_file(file_name, tainted_sources);

        return;
    }


    /**
     * Prints out analysis metadata including:
     * 1. Hook function name
     * 2. Call graph information
     * 3. Authorization function callsites and arguments
     * 4. Taint analsysis for callsites
     * 5. Data sources identified
     * @param hook_data Hook analysis data structure to be printed
     */
    void print_hook_analysis_data(hook_analysis_metadata hook_data) {

        errs() << "Hook Function: " << hook_data.hook_function->getName() << " ";
        errs() << "w/ call graph containing " << hook_data.local_call_graph.size() << " of " << hook_data.program_call_graph.size() << " total functions.\n";
        errs() << "# Auth Calls: " << hook_data.authorization_call_sites.size() << "\n";
        for (auto temp_callsite:hook_data.authorization_call_sites) {
            std::string call_inst_string;
            raw_string_ostream rso(call_inst_string);
            temp_callsite->print(rso);

            errs() << "\tFunction: " << temp_callsite->getFunction()->getName() << " Call: " << call_inst_string << "\n";

            for (auto temp_arg:hook_data.auth_callsite_arg_map[call_inst_string]) {
                if (temp_arg.is_constant) {
                    errs() << "\t\tConstant: " << temp_arg.arg_string << "\n";
                } else {
                    errs() << "\t\tArgument: " << temp_arg.arg_string << " Instruction: " << temp_arg.load_inst_string << "\n";
                    errs() << "\t\t\tReverse taint size: " << hook_data.argument_taint_mapping[temp_arg.arg_string].size() << "\n";
                }
            }
        }
    }

    void print_sources_to_file(std::string file_name, std::list<taint_source> tainted_sources) {

            // Create and open appropriate output file
            ofstream output_file;
            output_file.open(file_name);

            // -'s 29, 18, 33, 27

            output_file << "| Source (name [type])          | Field (index [id]) | Source Location                                  | Label at Source                  |\n";
            output_file << "| ----------------------------- | ------------------ | ------------------------------------------------ | -------------------------------- |\n";

            for (auto temp_source:tainted_sources) {
                output_file << "| " << temp_source.var_name;

                for (int i = temp_source.var_name.size(); i < 29; i++) {
                    output_file << " ";
                }

                output_file << " | " << "                  " << " | " << temp_source.trimmed_filepath;

                for (int i = temp_source.trimmed_filepath.size(); i < 48; i++) {
                    output_file << " ";
                }

                std::string label = "{" + temp_source.purpose + ", " + temp_source.value + ", " + temp_source.location + "}";

                output_file << " | " << label;

                for (int i = label.size(); i < 32; i++) {
                    output_file << " ";
                }

                output_file << " |\n";
            }

            output_file.close();
            return;
    }




}
