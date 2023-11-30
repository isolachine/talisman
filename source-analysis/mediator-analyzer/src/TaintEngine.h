#ifndef BACKDOOR_TAINTENGINE_H
#define BACKDOOR_TAINTENGINE_H

// Local includes
#include "CallGraphHelper.h"

// PDG includes
#include "ProgramDependencyGraph.hh"
#include "Graph.hh"

// std:: includes
#include <set>


// Struct to hold information regarding taint sources
struct taint_source
{
    // Node in PDG
    pdg::Node *instruction_node;

    // Instruction w/ printable string variant
    Value *instruction;
    std::string instruction_string;

    // Is it a constant value?
    bool is_constant;
    int constant_value;

    // Is it a data structure?
    bool is_data_struct;
    std::string data_type;
    std::string var_name;

    std::string trimmed_filepath;

    // Debugging information (i.e., line number information)
    std::string source_line_info;


    // Labeling
    std::string location;
    std::string value;
    std::string purpose;


    taint_source(Value *instruction, std::string instruction_string, bool is_constant, pdg::Node* pdg_node, std::string source_line_info)
    {
        this->instruction_node = pdg_node;

        this->instruction = instruction;
        this->instruction_string = instruction_string;

        this->is_constant = is_constant;

        this->source_line_info = source_line_info;
    }
};


namespace TaintEngine
{
    // To be used with KSplit PDG nodes and edges
    std::list<pdg::Node *> generate_forward_taint_slice(pdg::Node &src, std::map<std::string, CallNode*> function_call_map, std::set<pdg::EdgeType> &exclude_edge_types);
    std::list<pdg::Node *> generate_reverse_taint_slice(pdg::Node &dst, std::map<std::string, CallNode*> function_call_map, std::set<pdg::EdgeType> &exclude_edge_types, std::list<std::string> auth_functions);

    std::list<taint_source> find_data_sources_on_taint(Module &M, ConfigInfo* configInfo, pdg::ProgramGraph *program_dependence_graph, std::map<std::string, CallNode*> function_call_map, Function* function_a, std::list<function_argument> arguments, std::list<pdg::Node*> reverse_taint_slice, std::list<ConstantInfo*> configured_constants);

    std::list<taint_source> find_subjects_in_node_list(Module &M, ConfigInfo* configInfo, std::map<std::string, CallNode*> function_call_map, std::list<pdg::Node*> node_list, std::list<std::string> subject_filters, std::string app_name);
    std::list<taint_source> check_arguments_on_taint(Module &M, ConfigInfo* configInfo, pdg::ProgramGraph *program_dependence_graph, std::map<std::string, CallNode*> function_call_map, Function* function_a, std::list<function_argument> arguments, std::list<pdg::Node*> node_list);
    std::list<taint_source> find_nodes_with_no_in_edges(std::list<pdg::Node*> node_list);
    std::list<taint_source> find_macros_on_taint(Module &M, ConfigInfo* configInfo, std::map<std::string, CallNode*> function_call_map, std::list<pdg::Node*> reverse_taint_slice);

    std::list<pdg::Node*> computeUnion(std::list<std::list<pdg::Node*>> taint_slices);
    bool is_auth_function(std::list<std::string> function_names, std::string current_function);
    void find_macros_for_function(std::map<std::string, int> found_hash, Function* function, std::pair<std::string, std::string> source_range, ConfigInfo* configInfo);
    bool in_range(std::string target, std::string start, std::string end);


}

#endif //BACKDOOR_TAINTENGINE_H
