#!/bin/bash

num=50
attrNum=20

for i in `seq 1`; do
  j=`expr ${i} + 1`
  attrNum=`expr ${j} \* 3`
  echo "attrNum=${attrNum}"
  for k in `seq 1`; do
    path=samples/ex10/cond1to5query1Attr20Rule1000/suite${k}

    rm -rf logs/${path}
    mkdir -p logs/${path}

    for id in `seq ${num}`; do
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac)>& logs/${path}/output${id}-all.txt
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac -no_slicing)>& logs/${path}/output${id}-noslicing.txt
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac -no_absref)>& logs/${path}/output${id}-noabsref.txt
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac -smc)>& logs/${path}/output${id}-smc.txt
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac -bl 1)>& logs/${path}/output${id}-bl1.txt
    done
  done
done
