#!/bin/bash

prog=replace
EXT=so
CPPFLAGS="$CPPFLAGS -ansi"
option="-prtBBTrace"


if [ "$(uname)" == "Darwin" ]; then
  EXT=dylib
else
  EXT=so
fi

function printUsage {
  echo "USAGE:"
  echo "      ./$(basename $0) <version>"
  exit
}

if [[ $# -lt 1 ]]; then
  printUsage
elif [[ $1 -lt 1 ]]; then
  printUsage
elif [[ $1 -gt 32 ]]; then
  printUsage
fi

ver=$1

mkdir -p ../source
cp ../versions.alt/versions.orig/v$ver/$prog.c ../source
sed -i '' 's/getline/getline_new/g' ../source/$prog.c

cd ../source

# Start instrumentation

## Check virtual environment
if [ -z $VTENV ]; then
	echo "
Run \`source ./env.sh\` in \`ResearchTests\` to activate the virtual environment"
  exit
fi

clang -g $CPPFLAGS -O0 -emit-llvm -c $prog.c

echo 0 > /tmp/llvm0
opt -load ../../../../Debug+Asserts/lib/Research.$EXT $option $prog.bc -o $prog.gbc &> ../nodes

echo "#define __SOURCE__ \"$prog.c\"" > printLine.cpp
cat ../../../instrumentation/BBInfo/printLine.cpp >> printLine.cpp
clang $CPPFLAGS -O0 -emit-llvm -c printLine.cpp

llvm-link $prog.gbc printLine.bc -o $prog.linked.bc

llc -filetype=obj $prog.linked.bc -o $prog.o

g++ $CPPFLAGS $prog.o -o $prog.c.inst.exe
