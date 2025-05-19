#!/bin/bash

resultFile=../data/GPR_3.csv
num=50

rm -rf ${resultFile}

for Aq in `seq 5`; do
  echo "Aq=${Aq}"
  for k in `seq 10`; do
    domSize=`expr ${k} \* 10`
    echo "domSize=${domSize}"
    dir=data/ACoAC_Datasets/MC5-Q1_5-A60-R3000/MC5-Q${Aq}-A60-R3000/V${domSize}

    for id in `seq ${num}`; do
      ./exp1 --mode gpr --input ../${dir}/test${id}.aabac --output ${resultFile}
    done
  done
done

./exp1 --mode stat-gpr-aq-v --stat-file ${resultFile}