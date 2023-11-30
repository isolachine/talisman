#!/usr/bin/env bash
# linking example

if [ "$(uname)" == "Darwin" ]; then
        EXT="dylib"
else
        EXT="so"
fi

FUNC=$2
LEVEL="../../../.."
PROJECT_LEVEL="../.."

$LEVEL/Debug+Asserts/bin/opt -mem2reg -instnamer $1 -o $1
$LEVEL/Debug+Asserts/bin/llvm-dis $1

TIME=$(date +%s)

$LEVEL/Debug+Asserts/bin/opt -stats -time-passes \
  -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
  -constraint-generation < $1 2> tmp.dat > /dev/null

TIME=$(echo "$(date +%s) - $TIME" | bc)
printf "Execution time: %d seconds\n" $TIME

CONS_FILENAME=$( echo 'constraints-'$FUNC'.con' | tr '/' '-')
python3 $PROJECT_LEVEL/processing_tools/constraint_file.py tmp.dat

FILENAME=$( echo 'results_with_source-'$FUNC'.txt' | tr '/' '-')
export PATH="$PATH:../../processing_tools"

FUNC=$( echo 'tmp-'$FUNC'.dat' | tr '/' '-')

echo Output log: ./$FUNC
mv tmp.dat $FUNC
