#!/bin/bash

path=samples/real/param2
rm -rf logs/${path}
mkdir -p logs/${path}

for id in `seq 50`; do
   (time ./verifier.sh -timeout 600 -input ../${path}/aabac/test${id}.aabac) >& logs/${path}/output${id}.txt
done
