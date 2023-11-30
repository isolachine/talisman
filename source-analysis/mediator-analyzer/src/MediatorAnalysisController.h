#ifndef BACKDOOR_MEDIATORANALYSISCONTROLLER_H
#define BACKDOOR_MEDIATORANALYSISCONTROLLER_H

// llvm includes
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"

// Tool includes
#include "InputDep.h"
#include "TaintEngine.h"
#include "CallGraphHelper.h"
#include "ProgramInfoHelper.h"

// PDG includes
#include <PDGEnums.hh>

// std:: includes
#include <set>
#include <string>
#include <map>
#include <fstream>
#include <utility>
#include <list>


struct hook_analysis_metadata {

    // Function we are analyzing
    Function* hook_function;

    // Call graph starting from the hook function
    std::map<std::string, CallNode*> local_call_graph;
    std::map<std::string, CallNode*> program_call_graph;

    // List of calls to authorization functions
    std::list<Instruction*> authorization_call_sites;

    // Map of authorization call site to a list of call arguments
    std::map<std::string, std::list<call_argument>> auth_callsite_arg_map;

    // Hashmap for reverse taints
    std::map<std::string, std::list<pdg::Node*>> argument_taint_mapping;

    // List of hook argument locations
    std::list<function_argument> function_arguments;

    hook_analysis_metadata (){
        this->hook_function = NULL;


    };

};

namespace MediatorAnalysisController
{
    void mediatorAnalysis(Module &M, pdg::ProgramGraph *program_dependence_graph, std::list<std::string> hooks, ConfigInfo* configInfo, CallGraphHelper call_helper);
    void analyzeHook(Module &M, pdg::ProgramGraph *program_dependence_graph, Function* function, ConfigInfo* configInfo, CallGraphHelper call_helper);
    void print_hook_analysis_data(hook_analysis_metadata hook_data);
    void print_macros_for_function(std::map<std::string, int> found_hash, std::string hook_function, std::pair<std::string, std::string> source_range, ConfigInfo* configInfo);
    void print_sources_to_file(std::string file_name, std::list<taint_source> tainted_sources);
    bool in_range(std::string target, std::string start, std::string end);
}

#endif //BACKDOOR_MEDIATORANALYSISCONTROLLER_H
