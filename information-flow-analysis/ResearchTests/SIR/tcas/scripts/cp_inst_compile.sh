#!/bin/bash

source=tcas.c
EXT=so
option="-prtBBTrace"

if [ "$(uname)" == "Darwin" ]; then
  EXT=dylib
else
  EXT=so
fi

function printUsage {
  echo "USAGE:"
  echo "      ./cp_and_compile.sh <version>"
  echo "      <version> varies from 1 to 41"
  exit
}

if [[ $# -lt 1 ]]; then
  printUsage
elif [[ $1 -lt 1 ]]; then
  printUsage
elif [[ $1 -gt 41 ]]; then
  printUsage
fi

ver=$1

cp ../versions.alt/versions.orig/v$ver/$source ../source

cd ../source

# Start instrumentation

## Check virtual environment
if [ -z $VTENV ]; then
	echo "
Run \`source ./env.sh\` in \`ResearchTests\` to activate the virtual environment"
  exit
fi

clang -g $CPPFLAGS -O0 -emit-llvm -c $source

echo 0 > /tmp/llvm0
opt -load ../../../../Debug+Asserts/lib/Research.$EXT $option tcas.bc -o tcas.c.bc &> ../nodes

echo "#define __SOURCE__ \"$source\"" > printLine.cpp
cat ../../../instrumentation/BBInfo/printLine.cpp >> printLine.cpp
clang $CPPFLAGS -O0 -emit-llvm -c printLine.cpp

llvm-link tcas.c.bc printLine.bc -o tcas.linked.bc

llc -filetype=obj tcas.linked.bc -o tcas.o

g++ $CPPFLAGS tcas.o -o tcas.c.inst.exe
