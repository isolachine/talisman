#!/bin/bash
# Run Demo

opt -load /home/computer/PDG/Backdoor-Project-LLVM9.0/build/Backdoor/libBackdoor.so -call-graph -configFile=call_test_recursion_config.txt call_test_recursion.bc > call_test_recusion.ll
