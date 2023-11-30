#!/bin/bash
# Run Demo

opt -load /home/computer/PDG/Backdoor-Project-LLVM9.0/build/Backdoor/libBackdoor.so -call-graph -configFile=call_test_multiple_calls_multiple_paths_config_2.txt call_test_multiple_calls_multiple_paths.bc > call_test_multiple_calls_multiple_paths.ll
