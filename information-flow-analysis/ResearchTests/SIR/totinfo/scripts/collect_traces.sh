#!/bin/bash

rm -f ../trace ../failed_traces ../succeeded_traces

for f in ../newoutputs/*; do
	bname=$(basename $f)
	diff ../newoutputs/$bname ../orgoutputs/$bname &> /tmp/out
	ret=$?
	echo $ret >> ../trace
	cat ../traces/$((${bname:1} - 1)).tr >> ../trace
	echo >> ../trace
if false; then
	if [[ $ret -ne 0 ]]; then
		(cat ../traces/$((${bname:1} - 1)).tr; echo) >> ../failed_traces
	else
		(cat ../traces/$((${bname:1} - 1)).tr; echo) >> ../succeeded_traces
	fi
fi
done
