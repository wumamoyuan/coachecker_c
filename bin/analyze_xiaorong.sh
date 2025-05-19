#!/bin/bash

for p in `seq 3`; do
  q=`expr ${p} - 1`
  for i in `seq 4`; do
    j=`expr ${i} + 1`
    rnum=`expr ${j} \* 5000`;
    path=xiaorong/samples${q}-1000-${rnum}
    java Analyzer logs/${path}-all xiaorong-all.csv
    java Analyzer logs/${path}-noslicing xiaorong-noslicing.csv
    java Analyzer logs/${path}-noabsref xiaorong-noabsref.csv
    java Analyzer logs/${path}-smc xiaorong-smc.csv
    java Analyzer logs/${path}-bl1 xiaorong-bl1.csv
  done
done
