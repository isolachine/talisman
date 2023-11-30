#ifndef CALLGRAPHHELPER_H
#define CALLGRAPHHELPER_H

// LLvm headers
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"

#include <set>
#include <string>
#include <map>
#include <fstream>
#include <utility>
#include <cstring>
#include <list>

#include "InputDep.h"

using namespace std;
using namespace llvm;

class CallNode;

class CallSiteInst
{
public:

    int id;

    Function* caller_function;
    std::string caller_function_string;
    CallNode* caller;

    Function* callee_function;
    std::string callee_function_string;
    CallNode* callee;

    Instruction* call_inst;
    std::string call_inst_string;


    std::string filename;
    std::string directory_path;
    std::string line_number;

    CallSiteInst(Function* caller, Function* callee, Instruction* call_inst)
    {
        this->caller_function = caller;
        this->callee_function = callee;
        this->call_inst = call_inst;
    }

    CallSiteInst(Function* callee)
    {
        this->callee_function = callee;
    }
    ~CallSiteInst();
};

class CallNode
{
public:

    long id;
    std::string function_string;

    Function* function;

    std::list<CallNode*> predeccesor_list;
    std::list<CallNode*> successor_list;
    int num_references;

    std::list<CallSiteInst*> call_sites;

    // Just a bunch of constructors.
    CallNode(long id, Function* function)
    {
        this->id = id;
        this->function = function;
        this->function_string = function->getName().str();
        this->num_references = 0;
    }
    ~CallNode();

};

class CallGraphHelper {
private:

    //bool runOnModule(Module &M);
    CallGraph *CG;
    ConfigInfo* config;
    Module* M;

    std::map<std::string, CallNode *> call_graph_map;
    std::map<std::string, int> call_strings_function_map;

public:

    CallGraphHelper(ConfigInfo* config_info, CallGraph* call_graph, Module* module){
        CG = call_graph;
        config = config_info;
        M = module;

    }


    void initialize_call_map();
    std::map<std::string, CallNode *> get_call_map();
    std::map<std::string, int> get_call_strings_function_map();

    int how_many_calls_for_func_found(CallNode* caller_node, std::string function_name);
    Instruction* find_nth_call_in_function(Function* caller, Function* callee, int num_funcs_before);
    bool is_auth_func(std::string current, std::list<std::string> auth_functions);

    std::list<Instruction*> find_call_sites_for_function(Function* caller_function, std::list<std::string> callee_list);

    void generate_call_strings(std::string starting_function, std::string ending_function, int call_depth, bool output_to_dot, bool output_to_stdin);
    void recursive_call_strings_sink_function(std::list<std::list<CallSiteInst*>> *path_set, std::list<CallSiteInst*> *curr_path, CallNode* sink_node, int call_depth);

    std::map<std::string, CallNode*> generate_function_call_graph(Function* starting_function, std::map<std::string, CallNode*> call_map, std::list<std::string> terminating_functions);
    void populate_function_map(CallNode* root_node, std::map<std::string, CallNode*> call_map);
    void call_strings_to_file(std::list<std::list<CallSiteInst*>> path_set, std::map<std::string, CallNode*> call_map, std::string filename);
    void call_string_path_to_dot(std::list<CallSiteInst*> path_set, std::map<std::string, CallNode*> call_map, CallNode* sink_node, std::ofstream *stream);
    void call_strings_to_dot(std::list<std::list<CallSiteInst*>> path_set, std::map<std::string, CallNode*> call_map, CallNode* sink_node, std::string filename);

    void recursively_add_callsites_to_map(std::map<std::string, CallSiteInst*> *callsite_map, std::map<std::string, CallNode*> *visited_map, std::list<CallSiteInst*> path, CallSiteInst* next_site, CallSiteInst* callsite);

    void toDot(CallNode* RootNode, std::map<std::string, CallNode*> call_map, std::string fileName, std::string module_name);
    void toDot(CallNode* RootNode, std::map<std::string, CallNode*> call_map, raw_ostream *stream, std::string module_name);


    static char ID;

};



#endif //CALLGRAPHHELPER_H