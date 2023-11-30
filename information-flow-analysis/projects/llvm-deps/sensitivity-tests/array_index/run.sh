#!/usr/bin/env bash
# linking example

if [ "$(uname)" == "Darwin" ]; then
        EXT="dylib"
else
        EXT="so"
fi

CPPFLAGS="-O0 -g -emit-llvm"
LLVMLIBS=
LDFLAGS=
LEVEL="../../../.."

# if your instrumentation code calls into LLVM libraries, then comment out the above and use these instead:
#CPPFLAGS=`llvm-config --cppflags`
#LLVMLIBS=`llvm-config --libs`
#LDFLAGS=`llvm-config --ldflags`

## compile the instrumentation module to bitcode
## clang $CPPFLAGS -O0 -emit-llvm -c sample.cpp -o sample.bc
$LEVEL/Debug+Asserts/bin/clang $INCLUDES $CPPFLAGS -c main.c -o test.bc
# $LEVEL/Debug+Asserts/bin/clang -O0 -g -emit-llvm -S main.cpp
$LEVEL/Debug+Asserts/bin/opt -mem2reg -instnamer test.bc -o test.bc
$LEVEL/Debug+Asserts/bin/llvm-dis test.bc

$LEVEL/Debug+Asserts/bin/opt -mem2reg -instnamer test.bc -o test.bc

## opt -load *.so -infoflow < $BENCHMARKS/welcome/welcome.bc -o welcome.bc
# valgrind --tool=memcheck 
$LEVEL/Debug+Asserts/bin/opt \
  -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
  -vulnerablebranchwrapper  -debug < test.bc 2> tmp.dat > /dev/null

# used in fix-point iteration
# ITER=$(cat tmp.dat | grep -E 'Done after [0-9]* iterations.' | grep -oE '[0-9]*')
# ITER=$((ITER))
# ITERTAG=$(expr $ITER \* 2)
# cat tmp.dat | grep '^'$ITERTAG':.*<:' | sed -nr 's/^[0-9]+:(.*)/\1/p' > constraints.con

cat tmp.dat | grep '^3:.*<:' | sed -nr 's/^[0-9]+:(.*)/\1/p' > constraints.con

## link instrumentation module
#llvm-link welcome.bc sample.bc -o welcome.linked.bc

## compile to native object file
#llc -filetype=obj welcome.linked.bc -o=welcome.o

## generate native executable
#g++ welcome.o $LLVMLIBS $LDFLAGS -o welcome

#./welcome

