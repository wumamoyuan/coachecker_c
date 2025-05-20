#!/bin/bash

# the number of instances in a dataset
num=50
resultFile_a_r=../data/PruningRate-attrNum-ruleNum.csv
resultFile_mc_v=../data/PruningRate-maxCond-domSize.csv
resultFile_aq_v=../data/PruningRate-attrNumInQuery-domSize.csv

# for figure (a)
rm -rf ${resultFile_a_r}

for a in `seq 4`; do
  attrNum=`expr ${a} + 1`
  attrNum=`expr ${attrNum} \* 30`
  echo "attrNum=${attrNum}"
  for k in `seq 10`; do
    ruleNum=`expr ${k} \* 500`
    echo "ruleNum=${ruleNum}"
    dir=data/ACoAC_Datasets/MC5-Q1-V20-A${attrNum}/P${ruleNum}

    for id in `seq ${num}`; do
      ./exp1 --instance-file ../${dir}/test${id}.aabac --result-file ${resultFile_a_r}
    done
  done
done

# for figure (b)
rm -rf ${resultFile_mc_v}

for mc in `seq 4`; do
  maxCond=`expr ${mc} + 2`
  echo "maxCond=${maxCond}"
  for k in `seq 10`; do
    domSize=`expr ${k} \* 10`
    echo "domSize=${domSize}"
    dir=data/ACoAC_Datasets/MC${maxCond}-Q1-A90-R3000/V${domSize}

    for id in `seq ${num}`; do
      ./exp1 --instance-file ../${dir}/test${id}.aabac --result-file ${resultFile_mc_v}
    done
  done
done

# for figure (c)
rm -rf ${resultFile_aq_v}

for Aq in `seq 5`; do
  echo "Aq=${Aq}"
  for k in `seq 10`; do
    domSize=`expr ${k} \* 10`
    echo "domSize=${domSize}"
    dir=data/ACoAC_Datasets/MC5-Q${Aq}-A60-R3000/V${domSize}

    for id in `seq ${num}`; do
      ./exp1 --instance-file ../${dir}/test${id}.aabac --result-file ${resultFile_aq_v}
    done
  done
done

# compute the average GPR and LPR of each dataset
./exp1 --stat a-r --result-file ${resultFile_a_r}
echo ""
./exp1 --stat mc-v --result-file ${resultFile_mc_v}
echo ""
./exp1 --stat aq-v --result-file ${resultFile_aq_v}
echo ""