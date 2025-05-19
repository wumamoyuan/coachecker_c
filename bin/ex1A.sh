#!/bin/bash

resultFile=../data/GPR_1.csv
num=50

rm -rf ${resultFile}

for a in `seq 4`; do
  attrNum=`expr ${a} + 1`
  attrNum=`expr ${attrNum} \* 30`
  echo "attrNum=${attrNum}"
  for k in `seq 10`; do
    ruleNum=`expr ${k} \* 500`
    echo "ruleNum=${ruleNum}"
    dir=data/ACoAC_Datasets/MC5-Q1-V20-A60_150/MC5-Q1-V20-A${attrNum}/P${ruleNum}

    for id in `seq ${num}`; do
      ./exp1 --mode gpr --input ../${dir}/test${id}.aabac --output ${resultFile}
    done
  done
done

./exp1 --mode stat-gpr-a-r --stat-file ${resultFile}