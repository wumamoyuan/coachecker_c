#!/bin/bash

if [ $# != 3 ]; then
	echo "please input the benchmark type, size and loop num!";
	exit;
fi

type=$1
size=$2;
loop=$3


for i in `seq ${loop}`; do
	./mymohawk.sh -nusmv ./NuSMV -input ../samples/casestudy/${type}/dresdner_${size}_branches.aabac
done

