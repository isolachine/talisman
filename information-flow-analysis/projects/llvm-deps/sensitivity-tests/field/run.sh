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



cd ../../../poolalloc/
pwd
make
cd -

cd ../../
pwd
make
cd -

# if your instrumentation code calls into LLVM libraries, then comment out the above and use these instead:
#CPPFLAGS=`llvm-config --cppflags`
#LLVMLIBS=`llvm-config --libs`
#LDFLAGS=`llvm-config --ldflags`

## compile the instrumentation module to bitcode
## clang $CPPFLAGS -O0 -emit-llvm -c sample.cpp -o sample.bc
$LEVEL/Debug+Asserts/bin/clang  $INCLUDES $CPPFLAGS -c main.c -o main.bc
$LEVEL/Debug+Asserts/bin/clang -O0 -g -emit-llvm -S main.c

$LEVEL/Debug+Asserts/bin/opt -mem2reg -instnamer main.bc -o main.bc
$LEVEL/Debug+Asserts/bin/llvm-dis main.bc

## opt -load *.so -infoflow < $BENCHMARKS/welcome/welcome.bc -o welcome.bc
$LEVEL/Debug+Asserts/bin/opt -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
  -vulnerablebranchwrapper -debug < main.bc > /dev/null 2>tmp.dat

# $LEVEL/Debug+Asserts/bin/opt -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
#   -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
#   -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
#   -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
#   -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
#   -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
#   -vulnerablebranchwrapper -debug < main2.ll > /dev/null 2>flow-sen.dat

## link instrumentation module
#llvm-link welcome.bc sample.bc -o welcome.linked.bc

## compile to native object file
#llc -filetype=obj welcome.linked.bc -o=welcome.o

## generate native executable
#g++ welcome.o $LLVMLIBS $LDFLAGS -o welcome

#./welcome


CONS_FILENAME=$( echo 'constraints.con' | tr '/' '-')
cat tmp.dat | grep '<:' > $CONS_FILENAME