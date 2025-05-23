#!/bin/bash

# check whether the coachecker executable exists
if [ ! -f ./coachecker ]; then
  echo "coachecker is not found in the current directory"
  exit
fi

nuXmv_dir=../nuXmv

# check whether the nuXmv directory exists
if [ ! -d ${nuXmv_dir} ]; then
  echo "nuXmv directory ${nuXmv_dir} does not exist"
  exit
fi

# check whether the nuXmv executable exists
if [ ! -f ${nuXmv_dir}/bin/nuXmv ]; then
  echo "nuXmv is not found in ${nuXmv_dir}/bin"
  exit
fi

dataset_dir=../data/URA_Datasets

# check whether the dataset directory exists
if [ ! -d ${dataset_dir} ]; then
  echo "dataset directory ${dataset_dir} does not exist"
  exit
fi

log_dir=../logs/exp_ablation

# if log directory does not exist, create the log directory
if [ ! -d ${log_dir} ]; then
  mkdir -p ${log_dir}
fi

for i in `seq 3`; do
  min_cond=$(($i - 1))
  for k in `seq 4`; do
    s=$((1 + $k))
    ruleNum=$((5000 * $s))
    subdir=MinC${min_cond}-P$ruleNum

    if [ -z ${dataset_dir}/$subdir ]; then
      echo "dir ${dataset_dir}/$subdir is empty"
      exit
    fi

    rm -rf ${log_dir}/${subdir}
    mkdir -p ${log_dir}/${subdir}

    for id in `seq 50`; do
      echo "analyzing ${dataset_dir}/$subdir/test${id}.mohawk"
      (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/) >& ${log_dir}/${subdir}/output${id}-all.txt
      (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -no_slicing) >& ${log_dir}/${subdir}/output${id}-noslicing.txt
      (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -no_absref) >& ${log_dir}/${subdir}/output${id}-noabsref.txt
      (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -smc) >& ${log_dir}/${subdir}/output${id}-smc.txt
    done
  done
done

subdir=D-real

if [ -z ${dataset_dir}/$subdir ]; then
  echo "dir ${dataset_dir}/$subdir is empty"
  exit
fi

rm -rf ${log_dir}/${subdir}
mkdir -p ${log_dir}/${subdir}

for id in `seq 50`; do
  echo "analyzing ${dataset_dir}/$subdir/test${id}.aabac"
  (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/) >& ${log_dir}/${subdir}/output${id}-all.txt
  (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -no_slicing) >& ${log_dir}/${subdir}/output${id}-noslicing.txt
  (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -no_absref) >& ${log_dir}/${subdir}/output${id}-noabsref.txt
  (time ./coachecker -t 600 -m ${nuXmv_dir}/bin/nuXmv -i ${dataset_dir}/$subdir/test${id}.mohawk -l ${log_dir}/ -smc) >& ${log_dir}/${subdir}/output${id}-smc.txt
done
