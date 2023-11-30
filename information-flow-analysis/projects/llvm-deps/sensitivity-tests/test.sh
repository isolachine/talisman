#!/bin/bash

# This script is used to run and verify that all test cases have passed
# If you would like to -
# ADD additional test cases, create a folder with a main.c and an expected.txt
# ALTER a test case, the expected.txt must also be changed.
# REMOVE a test case, delete the folder from the directory

# If a test case is passed you should see the "PASS: ./test_folder_name"
# Otherwise you'll se a "FAIL:" with the foldername and a result.log file will
# be generated in the corresponding folder

HOME_DIR=`pwd`
PASSES=0
FAILS=0

function execute_test() {
    cd $1
    if [ -f test.bc ]; then
        rm test.bc # make sure that the previously compiled code is gone
    else
        pwd
    fi

    if [ "$(uname)" == "Darwin" ]; then
        EXT="dylib"
    else
        EXT="so"
    fi

    CPPFLAGS="-O0 -g -emit-llvm"
    LLVMLIBS=
    LDFLAGS=
    LEVEL="../../../.."
    INCLUDES="-Iinclude"
    MEM2REG=
    if [ "$1" == "./flow_sensitive" ]; then
        MEM2REG="-mem2reg"
    fi
    
    if [ "$1" == "./mem2reg" ]; then
        MEM2REG="-mem2reg"
    fi

    # Compile using clang
    $LEVEL/Debug+Asserts/bin/clang  $INCLUDES $CPPFLAGS -c main.c* -o test.bc 2> /dev/null
    $LEVEL/Debug+Asserts/bin/opt -instnamer test.bc -o test.bc

    # Run the pass
    $LEVEL/Debug+Asserts/bin/opt $MEM2REG -load $LEVEL/projects/poolalloc/Debug+Asserts/lib/LLVMDataStructure.$EXT \
                                  -load ../../Debug+Asserts/lib/Constraints.$EXT  \
                                  -load ../../Debug+Asserts/lib/sourcesinkanalysis.$EXT \
                                  -load ../../Debug+Asserts/lib/pointstointerface.$EXT \
                                  -load ../../Debug+Asserts/lib/Deps.$EXT  \
                                  -load ../../Debug+Asserts/lib/Security.$EXT  \
                                  -vulnerablebranchwrapper -debug < test.bc 2>$2  > /dev/null

    exit_code="$?"

    if [[ "${exit_code}"  -ne 0 ]]; then
        echo "Failed to run analysis" > $3
    else
    #filter lines for only relevant output
    grep -e 'line [0-9]\+' $2 | sort -uV  > $3
    fi
    cd $HOME_DIR
}

for d in $(find ./* -maxdepth 0 -type d);
do
    if execute_test $d tmp filtered; then
        CHECK=$(diff $d/filtered $d/expected.txt | wc -l)
        if [ $CHECK -eq 0 ]
        then
            echo "PASS:" $d
            ((++PASSES))
        else
            echo "FAIL:" $d "... see result.log"
            cp $d/filtered $d/result.log
            ((++FAILS))
        fi
        rm $d/tmp $d/filtered;
    else
        ((++FAILS))
        echo "FAIL:" $d " execute function failed"
    fi
done

((TOTAL=PASSES+FAILS))
PCTPASS=$(printf %.2f $(echo $PASSES/$TOTAL "* 100" | bc -l))
PCTFAIL=$(printf %.2f $(echo $FAILS/$TOTAL "* 100" | bc -l))
echo "PASSING:" $PASSES "[$PCTPASS%]:: FAILING" $FAILS "[$PCTFAIL%]"
