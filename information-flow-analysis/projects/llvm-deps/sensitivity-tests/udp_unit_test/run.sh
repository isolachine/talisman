#!/usr/bin/env bash
# linking example

if [ "$(uname)" == "Darwin" ]; then
        EXT="dylib"
else
        EXT="so"
fi

LEVEL="../../../.."
PROJECT_LEVEL="../.."

cd $PROJECT_LEVEL
pwd
make
cd -

$LEVEL/Debug+Asserts/bin/clang -O0 -g -emit-llvm -o $1 -c main.c
$LEVEL/Debug+Asserts/bin/opt -instnamer -mem2reg $1 -o $1
$LEVEL/Debug+Asserts/bin/llvm-dis $1 

TIME=$(date +%s)

$LEVEL/Debug+Asserts/bin/opt -stats -time-passes \
  -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
  -implicit-function -debug < $1 2> tmp.dat > /dev/null

TIME=$(echo "$(date +%s) - $TIME" | bc)
printf "Execution time: %d seconds\n" $TIME

python3 constraint_file.py tmp.dat
python3 constraint_graph.py call-stack call-stack.dot -graph 1
rm call-stack
