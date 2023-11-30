#!/bin/bash

rm -f ../trace ../failed_traces ../succeeded_traces

for f in ../newoutputs/*; do
	bname=$(basename $f)
	diff ../newoutputs/$bname ../orgoutputs/$bname &> /tmp/out
	ret=$?
	echo $ret >> ../trace
	cat ../traces/$((${bname:1} - 1)).tr >> ../trace
	echo >> ../trace

	if [[ $ret -ne 0 ]]; then
		(cat ../traces/$((${bname:1} - 1)).tr; echo) >> ../failed_traces
		# if grep -v ",132," ../traces/$((${bname:1} - 1)).tr; then
		# 	echo "../traces/$((${bname:1} - 1)).tr doesn't contain 132"
		# fi
	else
		(cat ../traces/$((${bname:1} - 1)).tr; echo) >> ../succeeded_traces
	fi
done
