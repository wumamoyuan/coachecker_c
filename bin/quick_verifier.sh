#!/bin/bash

for k in `seq 3`; do
  s=$k
  ruleNum=$((1000 * $s))
  dir=samples2-100-$ruleNum

  if [ -z $dir ]; then
    echo "dir is empty"
    exit
  fi

  rm -rf logs/${dir}
  mkdir -p logs/${dir}

  for id in `seq 50`; do
    (time ./verifier.sh -timeout 600 -input /root/nn/vac/$dir/arbac/test${id}.mohawk) >& logs/$dir/output${id}.txt
  done
done
