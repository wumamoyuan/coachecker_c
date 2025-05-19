#!/bin/bash

resultFile=../data/GPR_2.csv
num=50

rm -rf ${resultFile}

for mc in `seq 4`; do
  maxCond=`expr ${mc} + 2`
  echo "maxCond=${maxCond}"
  for k in `seq 10`; do
    domSize=`expr ${k} \* 10`
    echo "domSize=${domSize}"
    dir=data/ACoAC_Datasets/MC3_6-Q1-A90-R3000/MC${maxCond}-Q1-A90-R3000/V${domSize}

    for id in `seq ${num}`; do
      ./exp1 --mode gpr --input ../${dir}/test${id}.aabac --output ${resultFile}
    done
  done
done

./exp1 --mode stat-gpr-mc-v --stat-file ${resultFile}