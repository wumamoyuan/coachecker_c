#!/bin/bash

for k in `seq 4`; do
  s=$((1 + $k))
  ruleNum=$((5000 * $s))
  dir=samples2-1000-$ruleNum

  if [ -z $dir ]; then
    echo "dir is empty"
    exit
  fi

  rm -rf ../logs/exp_ablation/${dir}
  mkdir -p ../logs/exp_ablation/${dir}

  for id in `seq 50`; do
    (time ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../data/$dir/arbac/test${id}.mohawk) >& ../logs/exp_ablation/$dir/output${id}-all.txt
    (time ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../data/$dir/arbac/test${id}.mohawk -no_slicing) >& ../logs/exp_ablation/$dir/output${id}-noslicing.txt
    (time ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../data/$dir/arbac/test${id}.mohawk -no_absref) >& ../logs/exp_ablation/$dir/output${id}-noabsref.txt
    (time ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../data/$dir/arbac/test${id}.mohawk -smc) >& ../logs/exp_ablation/$dir/output${id}-smc.txt
    (time ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../data/$dir/arbac/test${id}.mohawk -bl 1) >& ../logs/exp_ablation/$dir/output${id}-bl1.txt
  done
done
