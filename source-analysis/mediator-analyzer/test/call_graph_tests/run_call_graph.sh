#!/bin/bash
# Run Demo

opt -load /home/computer/PDG/Backdoor-Project-LLVM9.0/build/Backdoor/libBackdoor.so -call-graph -configFile=call_test_1_config.txt call_test_1.bc > call_test_1.ll
