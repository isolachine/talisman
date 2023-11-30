#include "AnalysisController.h"


AnalysisController::AnalysisController() :
    ModulePass(ID) {

}

bool AnalysisController::runOnModule(Module &M) {

    // Old test to check debugging symbols.
    //ProgramInfoHelper::test_debugging_symbols(M);


    /***********************************************/
    /**      Gather configuration information     **/
    /***********************************************/


    // Lookup config parser analysis
    InputDep &ID = getAnalysis<InputDep> ();

    // Lookup configuration
    ConfigInfo config = ID.getConfigInfo();




    /***********************************************/
    /**     Generate Program Dependence Graph     **/
    /***********************************************/
    pdg::ProgramGraph *g = getAnalysis<pdg::ProgramDependencyGraph>().getPDG();

    // The numEdges function doesnt appear accurate.
    errs() << "Size: " << g->numNode() << " \n";

    /*
     * This block of code was used to test retrieving nodes based on LLVM instruction (a.k.a, LLVM::Value*)
     * For each function, for each basic block, for each instruction attempt to find the node in the PDG.
     *  - Check if the instruction is present in the PDG
     *  - Identifies which instructions if any are not captured by PDG
     *
     *  - some getelementptr instructions do not appear to be placed in the PDG (Debugging related?)
     *
     */
    int total_inst = 0;
    int total_matched = 0;
    int total_unmatched = 0;

    /*
    for (Module::iterator func_it = M.begin(); func_it != M.end(); ++func_it)
    {
        for (Function::iterator block_it = func_it->begin(); block_it != func_it->end(); ++block_it) {
            for (BasicBlock::iterator inst = block_it->begin(); inst != block_it->end(); ++inst) {
                std::string inst_string;
                raw_string_ostream rso(inst_string);
                inst->print(rso);

                Value* src = &(*inst);
                pdg::Node* src_node = g->getNode(*src);

                total_inst++;

                if (src_node)
                {
                    total_matched++;
                    errs() << "Found Node " << " " << total_matched << "/" << total_inst << ": " << inst_string.c_str() << "\n";
                }
                else {
                    total_unmatched++;
                    errs() << "Missing " << " " << total_unmatched << "/" << total_inst << ": " << inst_string.c_str() << "\n";
                }

            }
        }

        errs() << "Function: " << func_it->getName() << " with: " << func_it->getInstructionCount() << " instructions.\n";
    }
    */



    //Function *F;
    //std::set<std::string> _driver_access_fields;

    //for (auto &F : M) {
    //    if (F.getName() == "aa_path_perm") {
    //        pdg::SharedFieldsAnalysis::computeAccessedFieldsInFunc(F, _driver_access_fields);
    //    }
    //}


    /***********************************************/
    /**           Generate Call Graph             **/
    /***********************************************/
    CallGraph* CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    CallGraphHelper call_helper(&config, CG, &M);
    call_helper.initialize_call_map();


    /***********************************************/
    /**   Parse and identify constants & hooks    **/
    /***********************************************/
    std::list<std::string> hook_functions = config.hook_functions;

    //for (auto temp_hook:hook_functions) {
    //    errs() << "Hook function: " << temp_hook << "\n";
    //}


    /***********************************************/
    /**        Perform Mediator Analysis         **/
    /***********************************************/

    MediatorAnalysisController::mediatorAnalysis(M, g, hook_functions, &config, call_helper);

    return false;
}

void AnalysisController::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<InputDep>();
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<pdg::ProgramDependencyGraph>();
    AU.addRequired<pdg::SharedFieldsAnalysis>();
}

char AnalysisController::ID = 0;
static RegisterPass<AnalysisController> P("TaintPDG", "Tainted Flow Analysis for Source Code");
