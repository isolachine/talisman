#include "TaintEngine.h"

namespace TaintEngine
{

    std::list<pdg::Node *> generate_forward_taint_slice(pdg::Node &src, std::map<std::string, int> function_call_map, std::set<pdg::EdgeType> &exclude_edge_types)
    {

        std::list<pdg::Node *> tainted_nodes;
        std::set<pdg::Node *> visited;
        std::stack<pdg::Node *> nodeQueue;
        nodeQueue.push(&src);

        while (!nodeQueue.empty())
        {
            auto currentNode = nodeQueue.top();
            nodeQueue.pop();
            if (currentNode == nullptr)
                continue;
            if (visited.find(currentNode) != visited.end())
                continue;
            visited.insert(currentNode);

            if ((!currentNode->getFunc()) || (!function_call_map[currentNode->getFunc()->getName()]))
                continue;

            for (auto out_edge : currentNode->getOutEdgeSet())
            {
                // exclude path
                if (exclude_edge_types.find(out_edge->getEdgeType()) != exclude_edge_types.end())
                    continue;
                if(out_edge->getDstNode()) {
                    tainted_nodes.push_back(out_edge->getDstNode());
                    nodeQueue.push(out_edge->getDstNode());
                }
                else{
                    errs() << "NULL DST Node\n";
                }
            }
        }
        return tainted_nodes;
    }

    std::list<pdg::Node *> generate_reverse_taint_slice(pdg::Node &dst, std::map<std::string, CallNode*> function_call_map, std::set<pdg::EdgeType> &exclude_edge_types, std::list<std::string> auth_functions)
    {

        std::list<pdg::Node *> tainted_nodes;
        std::set<pdg::Node *> visited;
        std::stack<pdg::Node *> nodeQueue;
        nodeQueue.push(&dst);

        while (!nodeQueue.empty())
        {
            auto currentNode = nodeQueue.top();
            nodeQueue.pop();
            if (currentNode == nullptr)
                continue;
            if (visited.find(currentNode) != visited.end())
                continue;
            visited.insert(currentNode);

            if ((!currentNode->getFunc()) || (!function_call_map[currentNode->getFunc()->getName()]))
                continue;

            for (auto in_edge : currentNode->getInEdgeSet())
            {
                // exclude path
                if (exclude_edge_types.find(in_edge->getEdgeType()) != exclude_edge_types.end())
                    continue;
                if(in_edge->getSrcNode()) {
                    // Only add nodes from function call map BUT dont add sink function
                    if (function_call_map[in_edge->getSrcNode()->getFunc()->getName()])
                    {
                        if (!is_auth_function(auth_functions, in_edge->getSrcNode()->getFunc()->getName()))
                        {
                            tainted_nodes.push_back(in_edge->getSrcNode());
                            nodeQueue.push(in_edge->getSrcNode());
                        }
                    }
                }
                else{
                    errs() << "NULL SRC Node\n";
                }
            }
        }
        return tainted_nodes;
    }

    bool is_auth_function(std::list<std::string> function_names, std::string current_function) {
        for (auto function_string:function_names) {
            if (std::strcmp(function_string.c_str(), current_function.c_str()) == 0) {
                return true;
            }
        }
        return false;
    }


    std::list<pdg::Node*> computeUnion(std::list<std::list<pdg::Node*>> taint_slices) {

        std::map<Value *, int> unioned_node_map;
        std::list<pdg::Node*> unioned_taint;

        for (auto temp_list:taint_slices) {
            for (auto temp_node: temp_list) {
                Value *temp_value = temp_node->getValue();
                if (temp_value) {

                    if (unioned_node_map[temp_value] == 0) {
                        unioned_node_map[temp_value] = 1;

                        unioned_taint.push_back(temp_node);
                    }
                }
            }
        }

        return unioned_taint;
    }


    std::list<taint_source> find_data_sources_on_taint(Module &M, ConfigInfo* configInfo, pdg::ProgramGraph *program_dependence_graph, std::map<std::string, CallNode*> function_call_map, Function* function_a, std::list<function_argument> arguments, std::list<pdg::Node*> reverse_taint_slice, std::list<ConstantInfo*> configured_constants) {
        std::list<taint_source> all_sources;
        std::map<std::string, int> added_map;

        errs() << "Checking taint for sources...\n";

        std::list<taint_source> subjects = find_subjects_in_node_list(M, configInfo, function_call_map, reverse_taint_slice, configInfo->subject_filters, configInfo->application_name);
        std::list<taint_source> argument_nodes = check_arguments_on_taint(M, configInfo, program_dependence_graph, function_call_map, function_a, arguments, reverse_taint_slice);
        std::list<taint_source> terminating_nodes = find_nodes_with_no_in_edges(reverse_taint_slice);
        std::list<taint_source> constants_from_macros = find_macros_on_taint(M, configInfo, function_call_map, reverse_taint_slice);

        errs() << "Identified the following sources:\n";
        errs() << "\tsubjects:" << subjects.size() << "\n";
        errs() << "\targuments:" << argument_nodes.size() << "\n";
        errs() << "\tterminating nodes:" << terminating_nodes.size() << "\n";
        errs() << "\tmacros:" << constants_from_macros.size() << "\n";


        for (auto temp_source : subjects) {
            all_sources.push_back(temp_source);
        }

        for (auto temp_source: argument_nodes) {
            all_sources.push_back(temp_source);
        }

        for (auto temp_source: terminating_nodes) {
            all_sources.push_back(temp_source);
        }

        for (auto temp_source: constants_from_macros) {
            all_sources.push_back(temp_source);
        }

        return all_sources;
    }

    std::list<taint_source> find_macros_on_taint(Module &M, ConfigInfo* configInfo, std::map<std::string, CallNode*> function_call_map, std::list<pdg::Node*> reverse_taint_slice)
    {
        // Iterate through each function in the call graph and gather the source line number range for the function
        // using debugging information.

        std::list<taint_source> constant_macros;

        std::map<std::string, CallNode*>::iterator temp_node_it = function_call_map.begin();
        for (; temp_node_it != function_call_map.end(); temp_node_it++) {

            std::pair<std::string, std::string> source_range = ProgramInfoHelper::getSourceRange(
                    M.getFunction(temp_node_it->second->function_string));


            std::string start = source_range.first;
            std::string end = source_range.second;

            if ((start == "") || (end == "")) {
                //errs() << "Invalid source range...skipping function: " << temp_node_it->second->function_string << "\n";
            } else {
                std::string app_name = configInfo->application_name;

                std::string source_range_filepath = start.substr(0, start.find(':'));
                std::string source_range_filename = source_range_filepath.substr(
                        source_range_filepath.find(app_name) + (app_name.size() + 1), source_range_filepath.size() - 1);

                if (!(source_range_filename == "")) {
                    // Get info for start of range
                    std::string source_line_range_start = start.substr(start.find(':') + 1, start.size() - 1);
                    std::string source_line_number_start = source_line_range_start.substr(0,
                                                                                          source_line_range_start.find(
                                                                                                  ':'));
                    std::string source_column_number_start = source_line_range_start.substr(
                            source_line_range_start.find(':') + 1, source_line_range_start.size() - 1);

                    // Get info for end of range
                    std::string source_line_range_end = end.substr(end.find(':') + 1, end.size() - 1);
                    std::string source_line_number_end = source_line_range_end.substr(0,
                                                                                      source_line_range_end.find(':'));
                    std::string source_column_number_end = source_line_range_end.substr(
                            source_line_range_end.find(':') + 1, source_line_range_end.size() - 1);

                    std::string current_macro = "";


                    std::ifstream ppFile(configInfo->pp_file_string, std::ifstream::in);
                    std::string line;

                    if (!ppFile) {
                        errs() << "Could not open the PP file \n";
                    }

                    while (!ppFile.eof()) {

                        getline(ppFile, line);

                        // Next check that there is an "=". If not, the configuration line should be ignored.
                        if (line.find("macro:") != std::string::npos) {

                            current_macro = line.substr(line.find(':') + 1, line.size() - 1);

                        }
                        if ((line.find("range:") != std::string::npos) && (strcmp(current_macro.c_str(), "current") != 0)) {
                            std::string remove_range = line.substr(line.find(':') + 1, line.size() - 1);
                            std::string file_path = remove_range.substr(0, remove_range.find(':'));
                            std::string file_name_2 = file_path.substr(file_path.find(app_name) + (app_name.size() + 1),file_path.size() - 1);

                            if (strcmp(source_range_filename.c_str(), file_name_2.c_str()) == 0) {
                                std::string line_range = remove_range.substr(remove_range.find(':') + 1,remove_range.size() - 1);
                                std::string line_number = line_range.substr(0, line_range.find(':'));
                                std::string column_number = line_range.substr(line_range.find(':') + 1,line.size() - 1);


                                if (TaintEngine::in_range(line_number, source_line_number_start,
                                                          source_line_number_end)) {

                                    bool found = false;
                                    for (auto temp_taint_node: reverse_taint_slice) {

                                        Value *tainted_value = temp_taint_node->getValue();
                                        if (isa<Instruction>(tainted_value)) {
                                            Instruction *tainted_instruction = dyn_cast<Instruction>(tainted_value);

                                            std::string tainted_inst_string;
                                            raw_string_ostream rso(tainted_inst_string);
                                            tainted_instruction->print(rso);


                                            if (tainted_instruction) {
                                                const llvm::DebugLoc &debugInfo = tainted_instruction->getDebugLoc();
                                                if (debugInfo) {
                                                    std::string directory = debugInfo->getDirectory().str();
                                                    std::string filePath = debugInfo->getFilename().str();
                                                    int line = debugInfo->getLine();
                                                    int column = debugInfo->getColumn();

                                                    std::string source_line_info =
                                                            directory + "/" + filePath + ":" + std::to_string(line) +
                                                            ":" + std::to_string(column);


                                                    if (stoi(line_number) == line) {
                                                        found = true;

                                                        taint_source temp_source = taint_source(tainted_value, tainted_inst_string, true, temp_taint_node, source_line_info);
                                                        temp_source.constant_value = 0;
                                                        temp_source.data_type = "";
                                                        temp_source.var_name = current_macro;
                                                        temp_source.location = "external";
                                                        temp_source.value = "static";
                                                        temp_source.purpose = "operation";
                                                        temp_source.trimmed_filepath = directory.substr(directory.find(app_name), directory.size() - 1) + "/" + filePath + ":" + to_string(line);

                                                        constant_macros.push_back(temp_source);
                                                        break;
                                                    }
                                                } else {
                                                }
                                            }
                                        }
                                    }

                                    // Handles switch statements
                                    if (!found) {
                                        taint_source temp_source = taint_source(NULL, "", true, NULL, "");
                                        temp_source.constant_value = 0;
                                        temp_source.data_type = "";
                                        temp_source.var_name = current_macro;
                                        temp_source.location = "external";
                                        temp_source.value = "static";
                                        temp_source.purpose = "operation";

                                        temp_source.trimmed_filepath = file_path.substr(file_path.find(app_name), file_path.size() - 1) + ":" + line_number;

                                        constant_macros.push_back(temp_source);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        // Iterate through enums from constant analysis
        // Identify calls that relate to subject filters
        // For each node in the taint
        for (auto temp_node:reverse_taint_slice) {

            // Fetch value and test
            Value *temp_value = temp_node->getValue();
            if (temp_value) {
                if (Instruction *temp_inst = dyn_cast<Instruction>(temp_value)) {
                    std::string inst_string;
                    raw_string_ostream rso(inst_string);
                    temp_inst->print(rso);
                    if (temp_inst->getNumOperands() > 0) {
                        for (int i = 0; i < temp_inst->getNumOperands(); i++) {
                            Value *temp_val = temp_inst->getOperand(i);
                            if (ConstantInt *CI = dyn_cast<ConstantInt>(temp_val)) {

                                if (CI->getBitWidth() <= 64) {

                                    const llvm::DebugLoc &debugInfo = temp_inst->getDebugLoc();

                                    if (debugInfo) {
                                        std::string directory = debugInfo->getDirectory().str();
                                        std::string filePath = debugInfo->getFilename().str();
                                        int line = debugInfo->getLine();
                                        int column = debugInfo->getColumn();

                                        std::string source_line_info =
                                                directory + "/" + filePath + ":" + to_string(line) + ":" +
                                                to_string(column);
                                        std::string source_file_line = filePath + ":" + std::to_string(line);
                                        int constIntValue = CI->getSExtValue();


                                        // Iterate through constants from constant analysis
                                        std::string current_enum = "";

                                        std::ifstream astFile(configInfo->ast_file_string, std::ifstream::in);
                                        std::string ast_line;

                                        if (!astFile) {
                                            errs() << "Could not open the PP file \n";
                                        }

                                        while (!astFile.eof()) {

                                            getline(astFile, ast_line);

                                            if (ast_line != "") {
                                                std::string app_name = configInfo->application_name;

                                                std::string enum_name = ast_line.substr(0, ast_line.find(':'));
                                                std::string enum_name_rest = ast_line.substr(ast_line.find(':') + 1, ast_line.size() - 1);

                                                std::string enum_filler = enum_name_rest.substr(0, enum_name_rest.find(':'));
                                                std::string enum_filler_rest = enum_name_rest.substr(enum_name_rest.find(':') + 1, enum_name_rest.size() - 1);

                                                std::string enum_value = enum_filler_rest.substr(0, enum_filler_rest.find(':'));
                                                std::string enum_value_rest = enum_filler_rest.substr(enum_filler_rest.find(':') + 1, enum_filler_rest.size() - 1);


                                                std::string enum_use_full_path = enum_value_rest.substr(0, enum_value_rest.find(':'));
                                                std::string enum_use_full_path_rest = enum_value_rest.substr(enum_value_rest.find(':') + 1, enum_value_rest.size() - 1);

                                                std::string enum_use_line = enum_use_full_path_rest.substr(0, enum_use_full_path_rest.find(':'));
                                                std::string enum_use_line_rest = enum_use_full_path_rest.substr(enum_use_full_path_rest.find(':') + 1, enum_use_full_path_rest.size() - 1);

                                                std::string enum_use_column = enum_use_line_rest.substr(0, enum_use_line_rest.find(' '));
                                                std::string enum_use_column_rest = enum_use_line_rest.substr(enum_use_line_rest.find(' ') + 1, enum_use_line_rest.size() - 1);

                                                std::string trimmed_path = enum_use_full_path.substr(enum_use_full_path.find(app_name) + app_name.size(), enum_use_full_path.size() - 1);
                                                std::string use_file_line = trimmed_path + ':' + enum_use_line;

                                                /*
                                                errs() << "AST Line: " << ast_line << "\n";
                                                errs() << "\t" << enum_name << "\n";
                                                errs() << "\t" << enum_filler << "\n";
                                                errs() << "\t" << enum_value << "\n";
                                                errs() << "\t" << enum_use_full_path << "\n";
                                                errs() << "\t" << enum_use_line << "\n";
                                                errs() << "\t" << enum_use_column << "\n";
                                                errs() << "\t" << use_file_line << "\n";
                                                */

                                                if (strcmp(use_file_line.c_str(), source_file_line.c_str()) == 0) {
                                                    std::string enum_def_full_file_path = enum_use_column_rest.substr(enum_use_column_rest.find(' ') + 1, enum_use_column_rest.find(':') - 3);
                                                    std::string enum_def_full_file_path_rest = enum_use_column_rest.substr(enum_use_column_rest.find(':') + 1, enum_use_column_rest.size() - 1);

                                                    std::string enum_def_line = enum_def_full_file_path_rest.substr(0, enum_def_full_file_path_rest.find(':'));
                                                    std::string enum_def_line_rest = enum_def_full_file_path_rest.substr(enum_def_full_file_path_rest.find(':') + 1, enum_def_full_file_path_rest.size() - 1);

                                                    std::string enum_def_col = enum_def_line_rest.substr(0, enum_def_line_rest.find(' '));

                                                    std::string source_line_info = enum_def_full_file_path + ":" + enum_def_line + ":" + enum_def_col;


                                                    std::string trimmed_def_path = enum_def_full_file_path.substr(enum_def_full_file_path.find(app_name) + app_name.size(), enum_def_full_file_path.size() - 1);
                                                    std::string def_file_line = trimmed_def_path + ':' + enum_def_line;

                                                    /*
                                                    errs() << "\t" << enum_def_full_file_path << "\n";
                                                    errs() << "\t" << enum_def_line << "\n";
                                                    errs() << "\t" << enum_def_col << "\n";
                                                    */

                                                    taint_source temp_source = taint_source(temp_inst, inst_string, true, temp_node, source_line_info);
                                                    temp_source.constant_value = 0;
                                                    temp_source.data_type = "";
                                                    temp_source.var_name = enum_name;
                                                    temp_source.location = "external";
                                                    temp_source.value = "static";
                                                    temp_source.purpose = "operation";

                                                    temp_source.trimmed_filepath = trimmed_def_path;

                                                    constant_macros.push_back(temp_source);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return constant_macros;
    }

    std::list<taint_source> find_subjects_in_node_list(Module &M, ConfigInfo* configInfo, std::map<std::string, CallNode*> function_call_map, std::list<pdg::Node*> node_list, std::list<std::string> subject_filters, std::string app_name) {


        std::list<taint_source> subject_sources;

        // First iterate through macros to identify macro uses in range
        std::map<std::string, int> added_instructions;
        std::map<std::string, CallNode *>::iterator temp_node_it = function_call_map.begin();
        for (; temp_node_it != function_call_map.end(); temp_node_it++) {


            Function* temp_func = M.getFunction(temp_node_it->second->function_string);
            std::pair<std::string, std::string> source_range = ProgramInfoHelper::getSourceRange(
                    temp_func);

            std::string start = source_range.first;
            std::string end = source_range.second;

            if ((start == "") || (end == "")) {
                //errs() << "Invalid source range...skipping function: " << temp_node_it->second->function_string << "\n";
            } else {
                std::string app_name = configInfo->application_name;

                std::string source_range_filepath = start.substr(0, start.find(':'));
                std::string source_range_filename = source_range_filepath.substr(source_range_filepath.find(app_name) + (app_name.size() + 1), source_range_filepath.size() - 1);



                if (source_range_filename.find("./") != std::string::npos) {
                    source_range_filename = source_range_filename.substr(2, source_range_filename.size() - 1);
                }

                if (!(source_range_filename == "")) {
                    // Get info for start of range
                    std::string source_line_range_start = start.substr(start.find(':') + 1, start.size() - 1);
                    std::string source_line_number_start = source_line_range_start.substr(0,
                                                                                          source_line_range_start.find(
                                                                                                  ':'));
                    std::string source_column_number_start = source_line_range_start.substr(
                            source_line_range_start.find(':') + 1, source_line_range_start.size() - 1);

                    // Get info for end of range
                    std::string source_line_range_end = end.substr(end.find(':') + 1, end.size() - 1);
                    std::string source_line_number_end = source_line_range_end.substr(0,
                                                                                      source_line_range_end.find(':'));
                    std::string source_column_number_end = source_line_range_end.substr(
                            source_line_range_end.find(':') + 1, source_line_range_end.size() - 1);


                    std::string current_macro = "";


                    std::ifstream ppFile(configInfo->pp_file_string, std::ifstream::in);
                    std::string line;

                    if (!ppFile) {
                        errs() << "Could not open the PP file \n";
                    }

                    while (!ppFile.eof()) {

                        getline(ppFile, line);


                        // Next check that there is an "=". If not, the configuration line should be ignored.
                        if (line.find("macro:") != std::string::npos) {

                            current_macro = line.substr(line.find(':') + 1, line.size() - 1);
                        }

                        if (line.find("range:") != std::string::npos) {

                            std::string remove_range = line.substr(line.find(':') + 1, line.size() - 1);
                            std::string file_path = remove_range.substr(0, remove_range.find(':'));
                            std::string file_name_2 = file_path.substr(file_path.find(app_name) + (app_name.size() + 1),
                                                                       file_path.size() - 1);

                            if (strcmp(source_range_filename.c_str(), file_name_2.c_str()) == 0) {
                                std::string line_range = remove_range.substr(remove_range.find(':') + 1,
                                                                             remove_range.size() - 1);
                                std::string line_number = line_range.substr(0, line_range.find(':'));
                                std::string column_number = line_range.substr(line_range.find(':') + 1,
                                                                              line.size() - 1);

                                std::string file2_line_number = file_name_2 + ":" + line_number;

                                if (TaintEngine::in_range(line_number, source_line_number_start,
                                                          source_line_number_end)) {

                                    if (current_macro.find("current") != std::string::npos) {
                                        if (added_instructions[file2_line_number] == 0) {

                                            taint_source temp_source = taint_source(NULL, "", false, NULL, "");
                                            temp_source.constant_value = 0;
                                            temp_source.data_type = "";
                                            temp_source.var_name = current_macro;
                                            temp_source.location = "external";
                                            temp_source.value = "dynamic";
                                            temp_source.purpose = "subject";

                                            temp_source.trimmed_filepath = file_path.substr(file_path.find(app_name), file_path.size() - 1) + ":" + line_number;

                                            subject_sources.push_back(temp_source);
                                            added_instructions[file2_line_number] = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Identify calls that relate to subject filters
        // For each node in the taint
        for (auto temp_node:node_list) {

            bool found = false;
            // Fetch value and test
            Value *temp_value = temp_node->getValue();
            if (temp_value) {
                // Test if value is a non-null instruction
                if (CallInst *call_inst = dyn_cast<CallInst>(temp_value)) {
                    std::string inst_string;
                    raw_string_ostream rso(inst_string);
                    call_inst->print(rso);


                    std::string callee_part = inst_string.substr(inst_string.find('@') + 1, inst_string.size() - 1);
                    std::string callee = callee_part.substr(0, callee_part.find('('));


                    if (call_inst) {
                        const llvm::DebugLoc &debugInfo = call_inst->getDebugLoc();
                        if (debugInfo) {
                            std::string directory = debugInfo->getDirectory().str();
                            std::string filePath = debugInfo->getFilename().str();
                            int line = debugInfo->getLine();
                            int column = debugInfo->getColumn();

                            std::string source_line_info =
                                    directory + "/" + filePath + ":" + std::to_string(line) +
                                    ":" + std::to_string(column);
                            std::string source_file_line = filePath + ":" + std::to_string(line);

                            for (auto filter_string : subject_filters) {
                                if (strcmp(callee.c_str(), filter_string.c_str()) == 0) {
                                    if (!added_instructions[source_file_line]) {

                                        taint_source temp_source = taint_source(call_inst, inst_string, false, temp_node, source_line_info);
                                        temp_source.constant_value = 0;
                                        temp_source.data_type = "";
                                        temp_source.var_name = filter_string;
                                        temp_source.location = "external";
                                        temp_source.value = "dynamic";
                                        temp_source.purpose = "subject";

                                        temp_source.trimmed_filepath = source_file_line;
                                        subject_sources.push_back(temp_source);
                                        added_instructions[source_file_line] = 1;
                                    }
                                }
                            }
                        }
                    } // Special case for subject structs
                } else if (Instruction *temp_inst = dyn_cast<Instruction>(temp_value)) {

                    std::string inst_string;
                    raw_string_ostream rso(inst_string);
                    temp_inst->print(rso);

                    if (inst_string.find("alloca") != std::string::npos) {

                        for (auto subject_filter: subject_filters) {
                            if (inst_string.find(subject_filter) != std::string::npos) {
                                const llvm::DebugLoc &debugInfo = temp_inst->getDebugLoc();
                                if (debugInfo) {
                                    std::string directory = debugInfo->getDirectory().str();
                                    std::string filePath = debugInfo->getFilename().str();
                                    int line = debugInfo->getLine();
                                    int column = debugInfo->getColumn();

                                    std::string source_line_info =
                                            directory + "/" + filePath + ":" + std::to_string(line) +
                                            ":" + std::to_string(column);
                                    std::string source_file_line = filePath + ":" + std::to_string(line);

                                    if (!added_instructions[source_file_line]) {

                                        taint_source temp_source = taint_source(temp_inst, inst_string, false, temp_node, source_line_info);
                                        temp_source.constant_value = 0;
                                        temp_source.data_type = "";
                                        temp_source.var_name = subject_filter;
                                        temp_source.location = "external";
                                        temp_source.value = "dynamic";
                                        temp_source.purpose = "subject";

                                        temp_source.trimmed_filepath = source_file_line;
                                        subject_sources.push_back(temp_source);
                                        added_instructions[source_file_line] = 1;
                                    } // find use with debug info
                                } else {
                                    for (auto temp_use = temp_inst->user_begin(); temp_use!= temp_inst->user_end(); temp_use++)
                                    {

                                        for(auto temp_use : temp_use->users()){  // U is of type User*
                                            if (!found) {
                                                if (auto temp_inst = dyn_cast<Instruction>(temp_use)){
                                                    const llvm::DebugLoc &debugInfo = temp_inst->getDebugLoc();
                                                    if (debugInfo) {
                                                        std::string directory = debugInfo->getDirectory().str();
                                                        std::string filePath = debugInfo->getFilename().str();
                                                        int line = debugInfo->getLine();
                                                        int column = debugInfo->getColumn();

                                                        std::string source_line_info =
                                                                directory + "/" + filePath + ":" + std::to_string(line) +
                                                                ":" + std::to_string(column);
                                                        std::string source_file_line =
                                                                filePath + ":" + std::to_string(line);

                                                        if (!added_instructions[inst_string]) {

                                                            taint_source temp_source = taint_source(temp_inst, inst_string,
                                                                                                    false, temp_node,
                                                                                                    source_line_info);
                                                            temp_source.constant_value = 0;
                                                            temp_source.data_type = "";
                                                            temp_source.var_name = subject_filter;
                                                            temp_source.location = "external";
                                                            temp_source.value = "dynamic";
                                                            temp_source.purpose = "subject";

                                                            temp_source.trimmed_filepath = directory.substr(directory.find(configInfo->application_name), directory.size() - 1) + "/" + filePath + ":" + to_string(line);;
                                                            subject_sources.push_back(temp_source);
                                                            added_instructions[source_file_line] = 1;
                                                            found=true;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        return subject_sources;
    }



    std::list<taint_source> find_nodes_with_no_in_edges(std::list<pdg::Node*> node_list) {

        std::list<taint_source> no_in_edge_list;
        std::map<std::string, int> added_map;

        for (auto temp_node:node_list) {
            Value *temp_value = temp_node->getValue();
            if (temp_value) {
                if (temp_node->getInEdgeSet().size() == 0) {
                    if (Instruction *inst = dyn_cast<Instruction>(temp_value)) {
                        std::string inst_string;
                        raw_string_ostream rso(inst_string);
                        inst->print(rso);

                        if (added_map[inst_string] == 0) {
                            std::string source_line_info;
                            const llvm::DebugLoc &debugInfo = inst->getDebugLoc();

                            if (debugInfo) {
                                std::string directory = debugInfo->getDirectory().str();
                                std::string filePath = debugInfo->getFilename().str();
                                int line = debugInfo->getLine();
                                int column = debugInfo->getColumn();

                                source_line_info = directory + "/" + filePath + ":" + to_string(line) + ":" + to_string(column);

                                added_map[inst_string] = 1;

                                taint_source temp_source = taint_source(temp_value, inst_string, false, temp_node, source_line_info);


                            } else {
                                added_map[inst_string] = 1;

                                taint_source temp_source = taint_source(temp_value, inst_string, false, temp_node, source_line_info);
                                no_in_edge_list.push_back(temp_source);
                            }
                        }
                    }
                }
            }
        }
        return no_in_edge_list;
    }

    std::list<taint_source> check_arguments_on_taint(Module &M, ConfigInfo* configInfo, pdg::ProgramGraph *program_dependence_graph, std::map<std::string, CallNode*> function_call_map, Function* function_a, std::list<function_argument> arguments, std::list<pdg::Node*> node_list) {

        std::list<taint_source> tainted_arguments;
        std::map<std::string, int> added_map;

        // For each node in the taint
        for (auto temp_node:node_list) {

            // Fetch value and test
            Value *temp_value = temp_node->getValue();
            if (temp_value) {
                // Test if value is a non-null instruction
                if (Instruction *inst = dyn_cast<Instruction>(temp_value)) {
                    std::string inst_string;
                    raw_string_ostream rso(inst_string);
                    inst->print(rso);

                    if (added_map[inst_string] == 0) {

                        // For each argument, test if the tainted node matches the original store instruction
                        // of a hook argument
                        for (auto temp_arg : arguments) {
                            if (std::strcmp(temp_arg.store_inst_string.c_str(), inst_string.c_str()) == 0) {

                                // If data structure, get data type and var name
                                // store %struct.super_block* %sb, %struct.super_block** %sb.addr, align 8
                                std::string data_type;
                                std::string var_name;
                                bool is_data_struct;

                                if (inst_string.find("struct") != std::string::npos) {

                                    is_data_struct = true;

                                    std::string next_1 = inst_string.substr(inst_string.find(' ') + 1, inst_string.size() - 1);
                                    std::string next_2 = next_1.substr(next_1.find(' ') + 1, next_1.size() - 1);
                                    std::string next_3 = next_2.substr(next_2.find(' ') + 1, next_2.size() - 1);
                                    std::string next_4 = next_3.substr(next_3.find(' ') + 1, next_3.size() - 1);


                                    data_type = next_3.substr(next_3.find('.') + 1, next_3.find('*') - 8);
                                    var_name = next_4.substr(next_4.find('%') + 1, next_4.find(',') - 1);

                                } else {
                                    std::string next_1 = inst_string.substr(inst_string.find(' ') + 1, inst_string.size() - 1);
                                    std::string next_2 = next_1.substr(next_1.find(' ') + 1, next_1.size() - 1);
                                    std::string next_3 = next_2.substr(next_2.find(' ') + 1, next_2.size() - 1);
                                    std::string next_4 = next_3.substr(next_3.find(' ') + 1, next_3.size() - 1);


                                    data_type = next_3.substr(next_3.find('.') + 1, next_3.find('*') - 8);
                                    var_name = next_4.substr(next_4.find('%') + 1, next_4.find(',') - 1);
                                }


                                std::string source_line_info;

                                const llvm::DebugLoc &debugInfo = temp_arg.store_inst->getDebugLoc();

                                if (debugInfo) {
                                    std::string directory = debugInfo->getDirectory().str();
                                    std::string filePath = debugInfo->getFilename().str();
                                    int line = debugInfo->getLine();
                                    int column = debugInfo->getColumn();

                                    std::string source_line_info = directory + "/" + filePath + ":" + to_string(line) + ":" + to_string(column);

                                    added_map[inst_string] = 1;

                                    taint_source temp_source = taint_source(temp_value, inst_string, false, temp_node, source_line_info);
                                    if (is_data_struct) {
                                        temp_source.is_data_struct = true;
                                        temp_source.purpose = "object";
                                    } else {
                                        temp_source.is_data_struct = false;
                                        temp_source.purpose = "operation";
                                    }
                                    temp_source.data_type = data_type;
                                    temp_source.var_name = var_name;
                                    temp_source.location = "input";
                                    temp_source.value = "dynamic";
                                    temp_source.trimmed_filepath = directory.substr(directory.find(configInfo->application_name), directory.size() - 1) + "/" + filePath + ":" + to_string(line);

                                    tainted_arguments.push_back(temp_source);

                                } else {

                                    added_map[inst_string] = 1;

                                    taint_source temp_source = taint_source(temp_value, inst_string, false, temp_node, source_line_info);
                                    if (is_data_struct) {
                                        temp_source.is_data_struct = true;
                                        temp_source.purpose = "object";
                                    } else {
                                        temp_source.is_data_struct = false;
                                        temp_source.purpose = "operation";
                                    }
                                    temp_source.data_type = data_type;
                                    temp_source.var_name = var_name;
                                    temp_source.location = "input";
                                    temp_source.value = "dynamic";


                                    temp_source.trimmed_filepath =  ProgramInfoHelper::getFunctionStart(temp_node->getFunc(), configInfo->application_name);


                                    tainted_arguments.push_back(temp_source);

                                }
                            }
                        }
                    }
                }
            }
        }

        return tainted_arguments;
    }

    bool in_range(std::string target, std::string start, std::string end) {

        int target_int = atoi(target.c_str());
        int start_int = atoi(start.c_str());
        int end_int = atoi(end.c_str());

        if ((target_int >= start_int) && (target_int <= end_int)){
            return true;
        }
        return false;

    }
}