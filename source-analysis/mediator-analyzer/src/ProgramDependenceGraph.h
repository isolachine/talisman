#ifndef PROGRAMDEPENDENCEGRAPH_H
#define PROGRAMDEPENDENCEGRAPH_H

#include "ProgramInfoHelper.h"

// LLVM includes
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/DebugInfo.h"

#include <llvm/Support/FileSystem.h>
#include "llvm/Support/raw_ostream.h"


#include <string.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <list>


using namespace llvm;
using namespace std;

typedef enum {
    ntDefault = -1, ntActualIn = 0, ntActualOut = 1, ntFormalIn = 2, ntFormalOut = 3, ntEntry = 4, ntGlobal = 5, ntReturn = 6, ntInst = 8, ntParam= 9
} NodeType;

typedef enum {
    etData = 0, etControl = 1, etParam = 2, etField = 3, etCall = 4
} EdgeType;

struct dot_edge_prop {
    std::string style;
    std::string label;
    std::string color;
    std::string penwidth;

    dot_edge_prop()
    {
        this->style = "";
        this->label = "";
        this->color = "";
        this->penwidth = "";
    }
};

struct dot_node_prop {

    std::string shape;
    std::string color;
    std::string label;

    dot_node_prop()
    {
        this->shape = "";
        this->color = "";
        this->label = "";
    }

};

class GraphEdge
{
public:

    long from, to;
    struct dot_edge_prop edge_properties;

    GraphEdge(long from, long to)
    {
        this->from = from;
        this->to = to;
        this->edge_properties = dot_edge_prop();
    }
    ~GraphEdge();
};

typedef class GraphEdge GraphEdge;

class GraphNode
{

public:

    long id;
    NodeType node_type;

    std::string function_string;
    std::string instruction_string;

    Function* function;
    Instruction* instruction;

    std::string taint_label;
    std::string raw_graph_string;

    std::list<GraphNode*> successor_list;
    std::list<GraphNode*> predeccesor_list;

    struct dot_node_prop node_properties;

    bool matched;

    // Just a bunch of constructors.
    GraphNode(long id, std::string inst_label, NodeType node_type)
    {
        this->id = id;
        this->instruction_string = inst_label;
        this->node_type = node_type;
        this->function_string = "";
        this->function = nullptr;
        this->instruction = nullptr;
        this->taint_label = "";
        this->node_properties = dot_node_prop();
        this->matched = false;
        std::string raw_graph_string = "";
    }

    GraphNode(long id, std::string inst_label, NodeType node_type, Function* f)
    {
        this->id = id;
        this->instruction_string = inst_label;
        this->node_type = node_type;
        this->function = f;
        this->function_string = "";
        this->instruction = nullptr;
        this->taint_label = "";
        this->node_properties = dot_node_prop();
        this->matched = false;
        std::string raw_graph_string = "";
    }

    GraphNode(long id, std::string inst_label, NodeType node_type, Function* f, Instruction* inst)
    {
        this->id = id;
        this->instruction_string = inst_label;
        this->node_type = node_type;
        this->function = f;
        this->instruction = inst;
        this->function_string = "";
        this->taint_label = "";
        this->node_properties = dot_node_prop();
        this->matched = false;
        std::string raw_graph_string = "";
    }

    ~GraphNode();


};

typedef class GraphNode GraphNode;

class PDGraph
{
public:
    std::list<GraphNode*> matched_node_list;
    std::list<GraphNode*> unmatched_node_list;

    std::list<GraphNode*> node_list;
    std::list<GraphEdge*> edge_list;

    std::list<GraphNode*> switch_nodes;

    std::map<long, std::list<long>> edge_map;
    int edge_count;

    std::map<long, GraphNode*> master_node_map;

    PDGraph(std::list<GraphNode*> node_list, std::list<GraphEdge*> edge_list)
    {
        this->node_list = node_list;
        this->edge_list = edge_list;
    }

};

typedef class PDGraph PDGraph;

// Prototypes held under FunctionInfoHelper namespace
namespace ProgramDependenceGraph
{
    PDGraph* reconstruct_graph_from_dot_file(std::string file_path, Module &M);
    std::string remove_prefix_substring(std::string prefix, std::string full_string);
    std::string remove_suffix_substring(std::string suffix, std::string full_string);
    dot_edge_prop parse_edge_properties(std::string edge_properties_string);
    dot_node_prop parse_node_properties(std::string node_properties_string);

    GraphNode* find_node_by_instruction(std::string instruction_string, Function* function, PDGraph* graph);
    void output_function_to_dot(std::string function_name, PDGraph* pdGraph);
    void output_callgraph_to_dot(std::string starting_function, PDGraph* pdGraph);

    bool does_entry_node_point_to_inst(GraphNode* entry_node, std::string inst_string);

    void match_asm_node_to_inst(GraphNode* node);
}

#endif /* PROGRAMDEPENDENCEGRAPH_H */
