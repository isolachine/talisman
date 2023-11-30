#!/bin/bash

start=1
end=10
function printUsage {
    echo "Usage: $0 [py script] [start] [end]"
    echo "      [py script]	calculates error nodes"
    echo "      [start]		start index of test cases, default=$start"
    echo "      [end]		end index of test cases, default=$end"
}

if [[ $# -lt 1 ]]; then
	printUsage
	echo "Now ignoring the analysis"
else 
	py=$1
	echo "Using Product to calculate likelihood	$(basename $1)
Test	FailsAt	Rank" > /tmp/log

	if [[ $# == 2 ]]; then
		start=$2
	elif [[ $# == 3 ]]; then
		start=$2
		end=$(($3 + 1))
	fi
fi

for (( i = $start; i <= $end; i++ )); do
	echo
	echo Start Running Test $i
	./test.sh $i $py
done

echo "All Tests are Finished"
