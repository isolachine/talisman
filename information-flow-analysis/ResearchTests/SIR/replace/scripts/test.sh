#!/bin/bash

prog=replace

function printUsage {
  echo "USAGE:"
  echo "      $0 <version> [py script]"
  echo "      <version> varies from 1 to 32"
  echo "      <py script> for calculating error nodes"
  exit
}

if [[ $# -lt 1 ]]; then
  printUsage
elif [[ $# -lt 2 ]]; then
  py=../../../computeErrorNodes.py
elif [[ $1 -lt 1 ]]; then
  printUsage
elif [[ $1 -gt 41 ]]; then
  printUsage
else
  py=$2
fi

mkdir -p ../newoutputs ../orgoutputs ../t_all ../n_all ../outputs

# run the tests on original code
## skip for efficiency
if [ ! -e "../orgoutputs/t5542" ]; then
  # copy the original source code and compile
  mkdir -p ../source
  rm -f ../source/*
  cp ../source.alt/source.orig/* ../source

  (
    cd ../source;
    gcc -ansi $prog.c -o $prog.exe
  ) &> /dev/null

  echo Running ./runall.sh
  (./runall.sh) &> /dev/null
  mkdir -p ../orgoutputs
  rm -f ../orgoutputs/*
  mv -f ../outputs/* ../orgoutputs
fi

# cp intrument and compile version $1
echo "Running rm ../traces/*"
mkdir -p ../traces
rm -f ../traces/*

if [[ ( -e "../n_all/$1.nd" ) && ( -e "../t_all/$1.tr" ) ]]; then
  cp ../n_all/$1.nd ../nodes
  cp ../t_all/$1.tr ../trace 
else
  echo Running ./cp_inst_compile.sh $1
  (./cp_inst_compile.sh $1) &> /dev/null
  cp ../nodes ../n_all/$1.nd

  echo Running ./gettraces.sh
  (./gettraces.sh) &> /dev/null

  echo Running ./collect_traces.sh
  ./collect_traces.sh
  cp ../trace ../t_all/$1.tr
fi

# diff the source files to see where has been changed
echo "diff <original $prog.c> <new $prog.c>"
diff ../source.alt/source.orig/$prog.c ../versions.alt/versions.orig/v$1/$prog.c

if [[ ($# -gt 1) && ( -f "$2" ) ]]; then
    # Generate the output
    echo
    echo Generate the output
    python $py ../nodes ../trace ../errorInfo $1
fi
