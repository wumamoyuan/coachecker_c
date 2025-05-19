#!/bin/bash

for i in `seq 5`; do
  echo "queryNum=${i}"
  for k in `seq 10`; do
    path=samples/ex3C/cond5query${i}Attr30Rule1500/suite${k}
    java Analyzer logs/${path} ex3C_result.csv
  done
done
