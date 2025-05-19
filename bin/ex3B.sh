#!/bin/bash

for i in `seq 4`; do
  condNum=`expr ${i} + 2`
  echo "condNum=${condNum}"
  for k in `seq 10`; do
    path=samples/ex3B/cond${condNum}query1Attr9Rule300/suite${k}

    rm -rf logs/${path}
    mkdir -p logs/${path}

    for id in `seq 50`; do
      (time ./verifier.sh -no_precheck -timeout 600 -input ../${path}/test${id}.aabac) >& logs/${path}/output${id}.txt
    done
  done
done
