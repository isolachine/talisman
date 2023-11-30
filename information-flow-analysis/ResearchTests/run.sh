#!/bin/bash

# Enable running this script without the virtual environment.
if [ -z $VTENV ]; then
	echo "
Run \`source ./env.sh\` to activate the virtual environment
Run \`source ./autocmp.sh\` to activate autocompletion for ./run.sh"
	source ./env.sh
fi

printUsage() {
	echo
	echo "Usage: ./run.sh \"<options>\" <TEST> (<ARGUMENTLIST>)"
	echo "Usable options: "
	echo " Static: "
	echo "	-sprtLnNum		Statically print the filename and debuginfo for each BasicBlock entrance"
	echo " Dynamic: "
	echo "	-prtLnNum		Dynamically print the filename and debuginfo for each BasicBlock entrance"
	echo "Usable TESTs: "
	echo "	welcome 			Print Welcome message and exit"
	echo "	gcd <arg1> <arg2>	Calculate the greatest common divisor of <arg1> and <arg2> "
	echo "	compression 		Compress a file"
	echo "	recursive (<arg1>)	Test with linkage of two .c files"
	echo
}


if [ $# -gt 1 ]; then

	tst=$2
	opt=$1

	if [[ ! $OPTFLAGS =~ "$opt" ]]; then
		echo "$opt is not an available flag"
		printUsage
		exit
	fi
	if [[ ! $TESTS =~ "$tst" ]]; then
		echo "$tst is not an available test"
		printUsage
		exit
	fi

	make -C $LLVMSRCLIB/Research

	rm -rf tmp trace
	mkdir tmp; mkdir trace

	if [ "$(uname)" == "Darwin" ]; then
		EXT="dylib"
		CPPFLAGS="-g -stdlib=libstdc++"
		LDFLAGS="-stdlib=libstdc++"
	else
		EXT="so"
		CPPFLAGS="-g"
		LDFLAGS=
	fi

	echo "Running Test: $tst with opt options $opt"
	cd $TMP
	make clean

	for f in $BENCHMARKS/$tst/*.c*; do
		clang $CPPFLAGS -O0 -emit-llvm -c $f #-o $(basename ${f%.c*}).bc
	done
	ret=$?
	if [ $ret -ne 0 ]; then
		echo "clang failed with ret=$ret"
		exit
	fi

	echo 0 > /tmp/llvm0

	for f in *.bc; do
		llvm-dis $f
		opt -load $LLVMLIB/Research.$EXT $opt $f -o ${f%.bc}.g.bc >> ../trace/nodes 2>&1
		ret=$?
		if [ $ret -ne 0 ]; then
			echo "opt failed when processing \"$f\", ret=$ret"
			exit
		fi
	done

	if [[ ! $opt =~ "-s" ]]; then ## -s means static analysis
		echo "#define __SOURCE__ \"$tst.c\"" > printLine.cpp
		cat $BBInfo/printLine.cpp >> printLine.cpp
		clang $CPPFLAGS -O0 -emit-llvm -c printLine.cpp -o printLine.bc

		## link instrumentation module
		llvm-link *.g.bc printLine.bc -o "$tst".linked.bc
		ret=$?
		if [ $ret -ne 0 ]; then
			echo "llvm-link failed ret=$ret"
			exit
		fi
		for f in *.bc; do
			llvm-dis $f
		done

		## compile to native object file
		llc -filetype=obj "$tst".linked.bc -o "$tst".o
		ret=$?
		if [ $ret -ne 0 ]; then
			echo "llc failed ret=$ret"
			exit
		fi

		## generate native executable
		# g++ -std=c++11 "$tst".o $LLVMLIBS $LDFLAGS -o "$tst".x
		g++ "$tst".o $LLVMLIBS $LDFLAGS -o "$tst".x
		ret=$?
		if [ $ret -ne 0 ]; then
			echo "g++ failed ret=$ret"
			exit
		fi

		echo "Running ./$tst"
		for f in ../tests/$tst/*; do
			# ./"$tst".x ${f##*/} 2>trace
			./"$tst".x ${f##*/}
			printf "\n$?\n" | cat - /tmp/trace/$tst.c.tr >> ../trace/traces
			rm /tmp/trace/$tst.c.tr
		done
	else
		echo "Analysis only"
	fi

	cd ..
else
	printUsage
fi
