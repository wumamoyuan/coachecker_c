#!/bin/bash

num=50
attrNum=1000

for i in `seq 4`; do
  j=`expr ${i} + 1`
  attrNum=`expr ${j} \* 3`
  echo "attrNum=${attrNum}"
  for k in `seq 10`; do
    path=samples/ex3A/cond5query1value10Attr${attrNum}/suite${k}

    rm -rf logs/${path}
    mkdir -p logs/${path}

    for id in `seq ${num}`; do
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac) >& logs/${path}/output${id}.txt
    done
  done
done
