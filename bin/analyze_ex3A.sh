#!/bin/bash

for i in `seq 4`; do
  j=`expr ${i} + 1`
  attrNum=`expr ${j} \* 3`
  echo "attrNum=${attrNum}"
  for k in `seq 10`; do
    path=samples/ex3A/cond5query1value10Attr${attrNum}/suite${k}
    java Analyzer logs/${path} ex3A_result.csv
  done
done
