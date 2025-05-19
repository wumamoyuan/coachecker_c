#!/bin/bash

if [ $# != 1 ]; then
  echo "please input the benchmark name!";
  exit;
fi

benchmark=$1
#id=$2
#loop=$3


for i in `seq 9`; do
  ./verifier.sh -input ../samples/testcases/${benchmark}/test0${i}.aabac
done
./verifier.sh -input ../samples/testcases/${benchmark}/test10.aabac
