#!/bin/bash

for i in `seq 5`; do
  #j=`expr ${i} + 1`
  #attrNum=`expr ${j} \* 3`
  echo "queryNum=${i}"
  for k in `seq 10`; do
    path=samples/ex3C/cond5query${i}Attr30Rule1500/suite${k}

    rm -rf logs/${path}
    mkdir -p logs/${path}

    for id in `seq 50`; do
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac) >& logs/${path}/output${id}.txt
    done
  done
done
