#!/usr/bin/env bash
# linking example

if [ "$(uname)" == "Darwin" ]; then
        EXT="dylib"
else
        EXT="so"
fi

CPPFLAGS="-O0 -g -emit-llvm"
INCLUDES="-isystem include"
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
# $LEVEL/Debug+Asserts/bin/clang $INCLUDES $CPPFLAGS -c bn_exp.c -o bn_exp.bc
# $LEVEL/Debug+Asserts/bin/clang $INCLUDES $CPPFLAGS -c bn_lib.c -o bn_lib.bc
# $LEVEL/Debug+Asserts/bin/llvm-link bn_exp.bc bn_lib.bc -o test.bc
# $LEVEL/Debug+Asserts/bin/clang -O0 -g -emit-llvm -S main.c

# $LEVEL/Debug+Asserts/bin/opt -mem2reg -S main.ll -o main2.ll

# rm $1
# make $1
$LEVEL/Debug+Asserts/bin/llvm-dis $1

cp whitelist.txt whitelist_tmp.txt
## opt -load *.so -infoflow < $BENCHMARKS/welcome/welcome.bc -o welcome.bc
$LEVEL/Debug+Asserts/bin/opt -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Constraints.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/sourcesinkanalysis.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/pointstointerface.$EXT \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Deps.$EXT  \
  -load $LEVEL/projects/llvm-deps/Debug+Asserts/lib/Security.$EXT  \
  -vulnerablebranchwrapper -debug < $1 > /dev/null 2>tmp.dat

cp tmp.dat tmp-cur.dat

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