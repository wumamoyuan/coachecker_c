#!/bin/bash

for i in `seq 4`; do
  condNum=`expr ${i} + 2`
  echo "condNum=${condNum}"
  for k in `seq 10`; do
    path=samples/ex3B/cond${condNum}query1Attr9Rule300/suite${k}
    java Analyzer logs/${path} ex3B_result.csv
  done
done
