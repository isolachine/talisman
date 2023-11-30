#!/bin/bash
# Run Demo

opt -load /home/computer/PDG/Backdoor-Project-LLVM9.0/build/Backdoor/libBackdoor.so -call-graph -configFile=call_test_multiple_calls_single_path_config.txt call_test_multiple_calls_single_path.bc > call_test_multiple_calls_single_path.ll
