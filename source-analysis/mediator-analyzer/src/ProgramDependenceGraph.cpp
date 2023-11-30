#include "ProgramDependenceGraph.h"

#define DEBUG 0

namespace ProgramDependenceGraph
{

    PDGraph* reconstruct_graph_from_dot_file(std::string file_path, Module &M)
    {

        // DOT File Meta-Data Identifiers
        std::string dot_title = "digraph ";
        std::string dot_graph_traits = "graph [";

        // DOT File Handler
        std::ifstream file;
        file.open(file_path);

        // String for each line to be read in and analyzed
        std::string line;

        // Consume the title of the graph
        std::getline(file, line);
        std::getline(file, line);

        // Consume the graph traits
        std::getline(file, line);

        // Create hashmaps to store nodes and edges read from DOT
        std::map<long, GraphNode*> dot_node_map;
        std::map<long, std::list<long>> dot_edge_map;

        // Instruction map for mapping instructions
        //std::map<std::string, std::map<GraphNode*, int>> instruction_map;
        std::map<std::string, GraphNode*> instruction_map;

        // These are for iterating
        std::list<GraphNode*> node_list;
        std::list<GraphEdge*> edge_list;

        std::list<GraphNode*> switch_nodes;

        errs() << "Reading in dot graph file (generating nodes and edges)\n";

        int read_edge_count = 0;
        int read_node_count = 0;

        // For the rest of the file, each line is either a node or edge
        while (std::getline(file, line))
        {

            // This is an edge
            if (line.find("->") != std::string::npos)
            {
                read_edge_count++;
                // Separate out the first node id and remove the Node0x prefix
                std::string first_node_id_string_raw = remove_suffix_substring(" -> ", line);
                std::string first_node_id_string = remove_prefix_substring("x", first_node_id_string_raw);

                // Separate the " -> " from the second node id string. In many cases edge properties are appended,
                // so extra needs to be performed.
                std::string second_node_id_string_raw = remove_prefix_substring(" -> ", line);
                std::string second_node_id_string_raw_1 = remove_suffix_substring(";", second_node_id_string_raw);
                std::string second_node_id_string_raw_2 = remove_prefix_substring("x", second_node_id_string_raw_1);

                // Grab the appended properties if they exist.
                if (second_node_id_string_raw_2.find("[") != std::string::npos)
                {
                    std::string edge_properties_string_raw =
                            second_node_id_string_raw_2.substr(
                                    second_node_id_string_raw_2.find('['),
                                    second_node_id_string_raw_2.size() - 1);
                    std::string second_node_id_string = remove_suffix_substring("[", second_node_id_string_raw_2);

                    //errs() << "Edge: " << first_node_id_string << " -> " << second_node_id_string << "\n";
                    long first_node_long = std::stol(first_node_id_string, nullptr, 16);
                    long second_node_long = std::stol(second_node_id_string, nullptr, 16);

                    GraphEdge* temp_edge = new GraphEdge(first_node_long, second_node_long);

                    temp_edge->edge_properties = parse_edge_properties(edge_properties_string_raw);

                    dot_edge_map[first_node_long].push_back(second_node_long);
                    edge_list.push_back(temp_edge);
                }
                // Edges without properties are handled here
                else
                {
                    //errs() << "Edge: " << first_node_id_string << " -> " << second_node_id_string_raw_2 << "\n";
                    long first_node_long = std::stol(first_node_id_string, nullptr, 16);
                    long second_node_long = std::stol(second_node_id_string_raw_2, nullptr, 16);

                    GraphEdge* temp_edge = new GraphEdge(first_node_long, second_node_long);

                    dot_edge_map[first_node_long].push_back(second_node_long);
                    edge_list.push_back(temp_edge);
                }


            }
            // This is a node
            else
            {
                read_node_count++;

                // Grab the node id value
                std::string node_id_string_raw = line.substr(0, line.find(" "));
                std::string node_id_string = remove_prefix_substring("x", node_id_string_raw);

                // There are a variety of node types
                // This is an initial check for ENTRY nodes. Typically the first node of a function
                if (line.find("ENTRY") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_2.substr(0, instruction_string_raw_2.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    std::string raw_entry = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);
                    //errs() << "Raw Entry:" << raw_entry << "\n";
                    //errs() << "Function: " << instruction_string << "\n";

                    Function* function = M.getFunction(instruction_string);
                    GraphNode* temp_node;

                    if (function)
                    {
                        temp_node = new GraphNode(l_hex, instruction_string, ntEntry, function);
                        temp_node->function_string = instruction_string;
                    }
                    else
                    {
                        temp_node = new GraphNode(l_hex, instruction_string, ntEntry);
                    }

                    temp_node->raw_graph_string = raw_entry;

                    dot_node_map[l_hex] = temp_node;

                }
                else if (line.find("ACTUAL_IN") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);
                    GraphNode* temp_node = new GraphNode(l_hex, instruction_string, ntActualIn);
                    dot_node_map[l_hex] = temp_node;
                }
                else if (line.find("ACTUAL_OUT") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    GraphNode* temp_node = new GraphNode(l_hex, instruction_string, ntActualOut);
                    dot_node_map[l_hex] = temp_node;
                }
                else if (line.find("FORMAL_IN") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);
                    GraphNode* temp_node = new GraphNode(l_hex, instruction_string, ntFormalIn);
                    dot_node_map[l_hex] = temp_node;
                }
                else if (line.find("FORMAL_OUT") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);
                    GraphNode* temp_node = new GraphNode(l_hex, instruction_string, ntFormalOut);
                    dot_node_map[l_hex] = temp_node;
                }
                else if (line.find("GLOBAL_VALUE") != std::string::npos)
                {

                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    //errs() << "Global: " << l_hex << " : " << instruction_string << "\n";
                }
                else if (line.find("PARAMETER_FIELD") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    //errs() << "Parameter Field: " << l_hex << " : " << instruction_string << "\n";
                }
                else if (line.find("POINTER_RW") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    //errs() << "Pointer: " << l_hex << " : " << instruction_string << "\n";

                }
                else if (line.find("STRUCT_FIELD") != std::string::npos)
                {
                    std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                    std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                    std::string instruction_string_raw_2 = remove_prefix_substring(" ", instruction_string_raw_1);
                    std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                    long l_hex = std::stol(node_id_string, nullptr, 16);

                    //errs() << "Struct: " << l_hex << " : " << instruction_string << "\n";
                }

                // Everything else is actual IR instruction node. These are the important ones.
                else
                {


                    if (!node_id_string.empty())
                    {


                        std::string instruction_string_raw = line.substr(line.find(" ") + 1, line.size() - 1);
                        std::string instruction_string_raw_1 = remove_prefix_substring("{", instruction_string_raw);
                        std::string instruction_string = instruction_string_raw_1.substr(0, instruction_string_raw_1.find(";") - 3);

                        //errs() << instruction_string << "\n";

                        long l_hex = std::stol(node_id_string, nullptr, 16);

                        GraphNode* temp_node = new GraphNode(l_hex, instruction_string, ntInst);
                        temp_node->node_properties = parse_node_properties(instruction_string_raw);
                        temp_node->instruction_string = temp_node->node_properties.label;

                        // FIXME FORMAL and ACTUAL are blue, so these need to be updated as they appear to be Parameter Tree information.
                        if (temp_node->node_properties.color.find("blue") != std::string::npos)
                        {
                            temp_node->node_type = ntParam;
                        }

                        if (instruction_string_raw.find("switch") != std::string::npos)
                        {
                            switch_nodes.push_back(temp_node);
                        }

                        dot_node_map[l_hex] = temp_node;

                        if (!instruction_map[temp_node->instruction_string])
                        {
                            instruction_map[temp_node->instruction_string] = temp_node;
                        }
                    }
                    else
                    {
                        // Do nothing, but note this will be hit at least once, otherwise the dot file may be corrupted.
                    }
                }
            }
        }


        errs() << "Connecting edges defined in dot-graph file\n";

        int edge_count = 0;

        for (std::map<long, GraphNode*>::iterator it = dot_node_map.begin(); it != dot_node_map.end(); it++)
        {
            long node_id = it->first;
            GraphNode* temp_node = it->second;
            std::list<long> forward_edges = dot_edge_map[node_id];
            edge_count += forward_edges.size();

            for (long node_id : forward_edges)
            {

                temp_node->successor_list.push_back(dot_node_map[node_id]);
                // Set a backward edge
                //errs() << "seg fault 2.2";

                if (dot_node_map[node_id]) {
                    dot_node_map[node_id]->predeccesor_list.push_back(temp_node);
                }
                else {
                    break;
                }

                //errs() << "seg fault 2.3";
            }

            node_list.push_back(temp_node);
        }

        // This is total edge count.
        errs() << "Total Nodes: " << dot_node_map.size() << "\n";
        errs() << "Total Edges: " << edge_count << "\n";

        /*
         * At this point the .dot file has been read in
         * - Nodes and edges have been gathered separately
         * - Edges have been assigned to nodes
         */

        std::map<long, GraphNode *>::iterator it = dot_node_map.begin();
        std::map<long, GraphNode *>::iterator it_end = dot_node_map.end();

        std::map<std::string, int> visited_entry_map;
        std::map<std::string, GraphNode*> entry_node_map;

        // Create a hashmap of entryNodes for easy lookup
        for (it = dot_node_map.begin(); it != it_end; it++) {
            if (it->second) {
                if (it->second->node_type == ntEntry) {
                    if (!visited_entry_map[it->second->function_string])
                    {
                        visited_entry_map[it->second->function_string] = 1;
                        entry_node_map[it->second->function_string] = it->second;
                    }
                    else {
                        errs() << "Multiple Entry Node for " << it->second->function_string << "\n";
                    }
                }
            }
        }

        errs() << "Set entry nodes for: " << entry_node_map.size() << " entry points\n";

        int match_count = 0;
        int unmatch_count = 0;

        int inst_count = 0;

        std::map<Instruction*, int> unmatched_instructions;

        for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it) {
            if (!func_it->isDeclaration()) {
                Function *temp_func = &(*func_it);
                for (Function::iterator block_it = func_it->begin(); block_it != func_it->end(); ++block_it) {
                    for (BasicBlock::iterator inst = block_it->begin(); inst != block_it->end(); ++inst) {
                        std::string inst_string;
                        raw_string_ostream rso(inst_string);
                        inst->print(rso);
                        inst_count++;

                        Instruction *temp_inst = &(*inst);

                        // At this point inst_string contains the string of the current instruction
                        // temp_func->getName().str() contains the string for the function

                        GraphNode* entryNode = entry_node_map[temp_func->getName().str()];
                        if (entryNode) {
                            // Search successors of entry node
                            int count = 0;
                            GraphNode* known_node = NULL;
                            for (auto temp_node : entryNode->successor_list) {
                                if (inst_string == temp_node->instruction_string) {
                                    count++;
                                    known_node = temp_node;
                                }
                            }
                            if (count == 1) {
                                known_node->instruction = temp_inst;
                                known_node->function_string = temp_inst->getFunction()->getName().str();
                                known_node->function = temp_inst->getFunction();
                                known_node->matched = true;
                                match_count++;
                            }
                            else {
                                unmatched_instructions[temp_inst] = 1;
                                unmatch_count++;
                            }
                        }

                        if ((match_count % 1000) == 0)
                        {
                            errs() << "Matched: " << match_count << "/" << inst_count << "\n";
                        }
                    }
                }
            }
        }

        errs() << "Matched Nodes: " << match_count << "\n";
        errs() << "UnMatched Nodes: " << unmatch_count << "\n";


        // This does nothing useful.

        if (unmatched_instructions.size() > 0) {
            errs() << "Iterating over unmatched instructions to attempt to fix\n";
            std::map<Instruction*, int>::iterator inst_it = unmatched_instructions.begin();
            std::map<Instruction*, int>::iterator inst_it_end = unmatched_instructions.end();

            for (inst_it; inst_it != inst_it_end; inst_it++)
            {
                Instruction* inst = inst_it->first;
                Function* inst_function = inst->getFunction();

                std::string inst_string;
                raw_string_ostream rso(inst_string);
                inst->print(rso);

            }
        }

        errs() << "Matched Nodes: " << match_count << "\n";
        errs() << "UnMatched Nodes: " << unmatch_count << "\n";



        // Finally create the pdg and return it
        PDGraph *pdg = new PDGraph(node_list, edge_list);
        pdg->edge_map = dot_edge_map;
        pdg->edge_count = edge_count;

        return pdg;
    }

    dot_edge_prop parse_edge_properties(std::string edge_properties_string)
    {
        struct dot_edge_prop edge_properties = dot_edge_prop();

        std::string edge_prop_raw2 = remove_prefix_substring("[", edge_properties_string);
        std::string edge_prop_raw3 = remove_suffix_substring("]", edge_prop_raw2);

        size_t position;
        std::string property;

        // This should print out each prop
        while ((position = edge_prop_raw3.find(",")) != std::string::npos) {
            property = edge_prop_raw3.substr(0, position);
            property.erase(std::remove_if(property.begin(), property.end(), ::isspace), property.end());

            // Dot needs " marks to differentiate between string and reserved words
            //property.erase(std::remove(property.begin(), property.end(), '\"'), property.end());

            // Now separate property into two parts and switch over the left hand side.
            std::string property_type = property.substr(0, property.find('='));
            std::string property_label = property.substr(property.find('=') + 1, property.length());

            if (property_type.find("style") != std::string::npos)
            {
                edge_properties.style = property_label;
            }
            else if (property_type.find("label") != std::string::npos)
            {
                edge_properties.label = property_label;
            }
            else if (property_type.find("color") != std::string::npos)
            {
                edge_properties.color = property_label;
            }
            else if (property_type.find("penwidth") != std::string::npos)
            {
                edge_properties.penwidth = property_label;
            }
            else
            {
                errs() << "warning: dot property type not supported: " << property_type << "=" << property_label << "\n";
            }

            edge_prop_raw3.erase(0, position + 1);
        }

        return edge_properties;
    }

    dot_node_prop parse_node_properties(std::string node_properties_string)
    {
        struct dot_node_prop node_properties = dot_node_prop();

        std::string node_prop_raw2 = remove_prefix_substring("[", node_properties_string);
        std::string node_prop_raw3 = remove_suffix_substring("]", node_prop_raw2);

        size_t position_equal;
        size_t position_comma;

        while ((position_equal = node_prop_raw3.find("=")) != std::string::npos)
        {

            std::string property_type = node_prop_raw3.substr(0, position_equal);

            node_prop_raw3.erase(0, position_equal + 1);

            if (property_type.find("shape") != std::string::npos)
            {
                position_comma = node_prop_raw3.find(",");
                std::string property_label = node_prop_raw3.substr(0, position_comma);

                node_properties.shape = property_label;
                node_prop_raw3.erase(0, position_comma + 1);
                //errs() << property_type << " " << property_label << "\n";
            }
            else if (property_type.find("color") != std::string::npos)
            {
                position_comma = node_prop_raw3.find(",");
                std::string property_label = node_prop_raw3.substr(0, position_comma);

                node_properties.color = property_label;
                node_prop_raw3.erase(0, position_comma + 1);
                //errs() << property_type << " " << property_label << "\n";
            }
            else if (property_type.find("label") != std::string::npos)
            {
                std::string label_raw = remove_prefix_substring("{", node_prop_raw3);
                std::string property_label = remove_suffix_substring("}", label_raw);

                node_properties.label = property_label;
                //errs() << property_type << " " << property_label << "\n";
            }
        }

        return node_properties;

    }

    /*
     * A helper function.
     *
     * Pass two strings, a prefix delimiter and a string.
     * The function will return the "full_string" with every character from start to "prefix string" removed.
     */
    std::string remove_prefix_substring(std::string prefix, std::string full_string)
    {
        return full_string.substr(full_string.find(prefix) + 1, full_string.size()-1);
    }

    /*
    * A helper function.
    *
    * Pass two strings, a suffix delimiter and a string.
    * The function will return the "full_string" with every character from "suffix string" to end removed.
    */
    std::string remove_suffix_substring(std::string suffix, std::string full_string)
    {
        size_t last_suffix_pos = full_string.find_last_of(suffix);
        return full_string.substr(0, last_suffix_pos);
    }

    /*
     *
     *
     */
    void match_asm_node_to_inst(GraphNode* node)
    {
        std::map<std::string, int> functions;

        for (auto pred_it : node->predeccesor_list)
        {
            functions[pred_it->function->getName().str()];
        }

        for (auto succ_it : node->successor_list)
        {
            functions[succ_it->function->getName().str()];
        }

        errs() << "Functions: " << functions.size() << "\n";
    }


    GraphNode* find_node_by_instruction(std::string instruction_string, Function* function, PDGraph* graph)
    {

        GraphNode* result = NULL;

        for (auto temp_node : graph->node_list)
        {
            if (temp_node) {
                NodeType node_nt = temp_node->node_type;

                if (temp_node->instruction != NULL && node_nt == ntInst && temp_node->function != NULL)
                {

                    std::string temp_function_name = temp_node->function->getName().str();
                    std::string inst_string = temp_node->instruction_string;

                    // Grab source function name
                    std::string source_function_name = function->getName().str();

                    if (strcmp(source_function_name.c_str(), temp_function_name.c_str()) == 0)
                    {

                        // Grab instruction string and function from graph node
                        if (strcmp(instruction_string.c_str(), inst_string.c_str()) == 0)
                        {
                            result = temp_node;
                            return result;
                        }
                    }
                }
                    // Otherwise check for entry node
                else if (node_nt == ntEntry) {
                    //errs() << "Comparing: " << instruction_string.c_str() << " : " << temp_node->raw_graph_string << "\n";
                    if (strcmp(temp_node->raw_graph_string.c_str(), instruction_string.c_str()) == 0)
                    {
                        result = temp_node;
                        return result;
                    }
                }
            }
        }

        return result;
    }



    /**
     * Attempts to print out a function's program dependence graph.
     *
     * Expects a function name as a std::string and searches the PDG for the corresponding
     * Entry Node for the function. If it exists, all child nodes of the entry node will be selected
     * as the functions PDG.
     *
     * This set of nodes will be converted into dot readable graph format.
     *
     * This function will automatically create a file using the naming scheme "<function_name>_PDG.dot"
     * @param function_name
     */
    void output_function_to_dot(std::string function_name, PDGraph* pdGraph) {

        // Find function in PDG
        GraphNode* function_entry_node;
        std::list<GraphNode*> function_node_list;


        for (auto temp_node: pdGraph->node_list) {
            if (temp_node->function_string == function_name) {
                if (temp_node->node_type == ntInst)
                {
                    if (strstr(temp_node->instruction_string.c_str(), "llvm.dbg.declare") == NULL) {
                        function_node_list.push_back(temp_node);
                    }
                } else {
                    if ((temp_node->node_type != ntFormalIn) && (temp_node->node_type != ntFormalOut)) {
                        function_node_list.push_back(temp_node);
                    }
                }

            }
            if (temp_node->node_type == ntEntry) {
                if (temp_node->function_string == function_name) {
                    function_entry_node = temp_node;
                }
            }
        }

        errs() << "Printing PDG for: " << function_name << " with " << function_node_list.size() << " nodes.\n";

        if (function_entry_node) {

            // Create and open appropriate output file
            sys::fs::OpenFlags Flags = sys::fs::F_Append;
            std::error_code errcode;

            std::string output_file_name = function_name + "_PDG.dot";

            raw_fd_ostream File(output_file_name, errcode, Flags);
            raw_ostream *stream = &File;

            errs() << "Opening: " << output_file_name << " for PDG printing.\n";

            if (errcode.value() != 0) {
                errs() << "Error opening file " << output_file_name
                       << " for writing! Error Info: " <<  " \n";
                return;
            }

            // Hash map to keep track of added nodes added.
            std::map<int, int> added_map;

            // Write dot preamble
            (*stream) << "digraph \"PDG for \'" << function_name << "\'  \"{\n";
            (*stream) << "label=\"PDG for \'" << function_name << "\' \";\n";

            // Write function entry node as root node
            string node_label = function_entry_node->instruction_string;
            node_label.erase (std::remove(node_label.begin(), node_label.end(), '"'), node_label.end());

            (*stream) << "Node_" << function_entry_node->id << "[shape=box"  << ",style=solid" << ",label=\"" << function_entry_node->function_string << "\"]\n";

            added_map[function_entry_node->id] = 1;

            for (auto temp_node:function_node_list) {
                string node_label = temp_node->instruction_string;
                node_label.erase (std::remove(node_label.begin(), node_label.end(), '"'), node_label.end());



                if (temp_node->instruction) {
                    const llvm::DebugLoc &debugInfo = temp_node->instruction->getDebugLoc();
                    if (debugInfo) {
                        std::string directory = debugInfo->getDirectory().str();
                        std::string filePath = debugInfo->getFilename().str();
                        int line = debugInfo->getLine();
                        int column = debugInfo->getColumn();

                        std::string source_line_info =
                                directory + "/" + filePath + ":" + to_string(line) + ":" + to_string(column);
                        (*stream) << "Node_" << temp_node->id << "[shape=record" << ",style=solid" << ",label=\""
                                  << temp_node->instruction_string << "\n" << source_line_info << "\"]\n";

                        errs() << "Debug Info: " << source_line_info << "\n";
                        added_map[temp_node->id] = 1;
                    }
                    else {
                        errs() << "DebugInfo Null: " << temp_node->instruction_string << "\n";
                        (*stream) << "Node_" << temp_node->id << "[shape=record"  << ",style=solid" << ",label=\"" << temp_node->instruction_string << "\"]\n";
                        added_map[temp_node->id] = 1;
                    }
                }
                else {
                    errs() << "Instruction Null: " << temp_node->instruction_string << "\n";
                    (*stream) << "Node_" << temp_node->id << "[shape=record"  << ",style=solid" << ",label=\"" << temp_node->instruction_string << "\"]\n";
                    added_map[temp_node->id] = 1;
                }
            }

            errs() << "Added map size: " << added_map.size() << "\n";

            // Write all relevant edges, by checking that both nodes are within the list of children.
            for (auto temp_node:function_node_list) {
                for (auto child_node:temp_node->successor_list) {
                    if ((added_map[temp_node->id] == 1) && (added_map[child_node->id] == 1)) {
                        (*stream) << "\"" << "Node_" << temp_node->id << "\"";

                        (*stream) << "->";

                        //Destination
                        (*stream) << "\"" << "Node_" << child_node->id  << "\"";
                        (*stream) << "\n";

                    }
                }
            }

            (*stream) << "}\n\n";
        }
        return;
    }

    bool does_entry_node_point_to_inst(GraphNode* entry_node, std::string inst_string) {
        for (auto temp_node: entry_node->successor_list) {
            if (temp_node->instruction_string == inst_string) {
                return true;
            }
        }
        return false;
    }
}
