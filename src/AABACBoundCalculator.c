#include "AABACBoundCalculator.h"
#include "AABACUtils.h"

static BigInteger compute1(AABACInstance *pInst) {
    BigInteger bound = iBigInteger.createFromInt(1);
    BigInteger tmp;
    HashNodeIterator *it = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    char *attr;
    int domSize;
    while (it->HasNext(it)) {
        HashNode *pNode = it->GetNext(it);
        attr = istrCollection.GetElement(pscAttrs, *(int *)pNode->key);
        if (strcmp(attr, "Admin") == 0) {
            continue;
        }
        domSize = iHashSet.Size(*(HashSet **)pNode->value);
        tmp = iBigInteger.multiplyByInt(bound, domSize);
        iBigInteger.finalize(bound);
        bound = tmp;
    }
    iHashMap.DeleteIterator(it);
    return bound;
}

/**
 * attr'是目标属性，attr''是其它属性
 * |Dom(attr''1)|*...*|Dom(attr''m)|*(|Dom(attr'1)|*...*|Dom(attr'n)| - 1)
 * @param pInst[in]: 待翻译的AABAC实例
 * @return
 */
static BigInteger compute2(AABACInstance *pInst) {
    BigInteger boundPart1 = iBigInteger.createFromInt(1);
    BigInteger boundPart2 = iBigInteger.createFromInt(1);
    BigInteger tmp;
    int domSize;
    char *attr;
    HashNode *pNode;
    HashNodeIterator *it = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    while (it->HasNext(it)) {
        pNode = it->GetNext(it);
        attr = istrCollection.GetElement(pscAttrs, *(int *)pNode->key);
        if (strcmp(attr, "Admin") == 0) {
            continue;
        }
        domSize = iHashSet.Size(*(HashSet **)pNode->value);
        if (!iHashMap.ContainsKey(pInst->pmapQueryAVs, pNode->key)) {
            tmp = iBigInteger.multiplyByInt(boundPart1, domSize);
            iBigInteger.finalize(boundPart1);
            boundPart1 = tmp;
        } else {
            tmp = iBigInteger.multiplyByInt(boundPart2, domSize);
            iBigInteger.finalize(boundPart2);
            boundPart2 = tmp;
        }
    }
    iHashMap.DeleteIterator(it);
    
    tmp = iBigInteger.subtractByInt(boundPart2, 1);
    BigInteger bound = iBigInteger.multiply(boundPart1, tmp);
    iBigInteger.finalize(boundPart1);
    iBigInteger.finalize(boundPart2);
    iBigInteger.finalize(tmp);
    return bound;
}

/**
 * 使用二分查找，将给定整数插入降序排列的数组中，并使数组保持降序
 * @param array[in]: 待插入的数组
 * @param len[in]: 数组长度
 * @param n[in]: 待插入的整数
 */
void insertDesc(int *array, int len, int n) {
    int left = 0, right = len - 1, mid;
    while (left <= right) {
        mid = (left + right) / 2;
        if (array[mid] > n) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    int i;
    for (i = len; i > left; i--) {
        array[i] = array[i - 1];
    }
    array[left] = n;
}

/**
 * 设A_1是一个属性集合，其中a属于A_1当前仅当不存在任何以(a,InitUAV(u_t, a))为目标属性值的规则
 * 设A是所有属性构成的集合，并且A_2=A\A_1
 * 再设A_1中的属性按取值个数降序排列为a_11,a_12,...,a_1m，
 * 设A_2中的属性为a_21,a_22,...,a_2n
 * 那么上界不超过|Dom(a_21)|*|Dom(a_22)|*...|Dom(a_2n)|*P
 * 其中P=1+(|Dom(a_11)|-1)+(|Dom(a_11)|-1)*(|Dom(a_12)|-1)+...+(|Dom(a_11)|-1)*...*(|Dom(a_1m)|-1)
 *
 * @param pInst[in]: 待翻译的AABAC实例
 * @return
 */
static BigInteger compute3(AABACInstance *pInst) {
    HashMap *pMapInitAVs = iHashBasedTable.GetRow(pInst->pTableInitState, &pInst->queryUserIdx);
    int attrNum = iHashMap.Size(pInst->pmapAttr2Dom);
    int attrs1[attrNum], attrs4[attrNum], attrs1MinusQueryAttrs[attrNum], attrs4MinusQueryAttrs[attrNum];
    int attrs1Len = 0, attrs4Len = 0, attrs1MinusQueryAttrsLen = 0, attrs4MinusQueryAttrsLen = 0;

    int *pAttrIdx, *pInitValIdx, *pQueryValIdx, domSize;
    char *attr;
    HashNode *node;
    HashNodeIterator *it = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    while (it->HasNext(it)) {
        node = it->GetNext(it);
        pAttrIdx = (int *)node->key;
        attr = istrCollection.GetElement(pscAttrs, *pAttrIdx);
        if (strcmp(attr, "Admin") == 0) {
            continue;
        }
        domSize = iHashSet.Size(*(HashSet **)node->value);
        pInitValIdx = (int *)iHashMap.Get(pMapInitAVs, pAttrIdx);
        pQueryValIdx = (int *)iHashMap.Get(pInst->pmapQueryAVs, pAttrIdx);
        if (iHashBasedTable.Get(pInst->pTableTargetAV2Rule, pAttrIdx, pInitValIdx) == NULL) {
            // attr is non restorable，即初始值一旦被修改，则无法恢复
            if (pQueryValIdx == NULL) {
                // attr不是被查询属性，it is not already-satisfied
                attrs4MinusQueryAttrs[attrs4MinusQueryAttrsLen++] = domSize;
                insertDesc(attrs4, attrs4Len++, domSize);
            } else if (*pQueryValIdx != *pInitValIdx) {
                // attr是被查询属性，但初始值不等于被查询值，it is not already-satisfied
                insertDesc(attrs4, attrs4Len++, domSize);
            }
        } else {
            if (pQueryValIdx == NULL) {
                attrs1MinusQueryAttrs[attrs1MinusQueryAttrsLen++] = domSize;
            }
            // attr is restorable，即初始值被修改后可以恢复
            attrs1[attrs1Len++] = domSize;
        }
    }
    iHashMap.DeleteIterator(it);
    // 计算P=1+(|Dom(a_11)|-1)+(|Dom(a_11)|-1)*(|Dom(a_12)|-1)+...+(|Dom(a_11)|-1)*...*(|Dom(a_1m)|-1)
    BigInteger boundPart1 = iBigInteger.createFromInt(1);
    BigInteger product = iBigInteger.createFromInt(1);
    BigInteger tmp;
    int i;
    for (i = 0; i < attrs4Len; i++) {
        tmp = iBigInteger.multiplyByInt(product, attrs4[i] - 1);
        iBigInteger.finalize(product);
        product = tmp;

        tmp = iBigInteger.add(boundPart1, product);
        iBigInteger.finalize(boundPart1);
        boundPart1 = tmp;
    }

    BigInteger boundPart2 = iBigInteger.createFromInt(1);
    for (i = 0; i < attrs1Len; i++) {
        tmp = iBigInteger.multiplyByInt(boundPart2, attrs1[i]);
        iBigInteger.finalize(boundPart2);
        boundPart2 = tmp;
    }

    BigInteger boundMinus = iBigInteger.createFromInt(1);
    for (i = 0; i < attrs1MinusQueryAttrsLen; i++) {
        tmp = iBigInteger.multiplyByInt(boundMinus, attrs1MinusQueryAttrs[i]);
        iBigInteger.finalize(boundMinus);
        boundMinus = tmp;
    }
    for (i = 0; i < attrs4MinusQueryAttrsLen; i++) {
        tmp = iBigInteger.multiplyByInt(boundMinus, attrs4MinusQueryAttrs[i] - 1);
        iBigInteger.finalize(boundMinus);
        boundMinus = tmp;
    }

    tmp = iBigInteger.multiply(boundPart1, boundPart2);
    BigInteger bound = iBigInteger.subtract(tmp, boundMinus);
    iBigInteger.finalize(boundPart1);
    iBigInteger.finalize(boundPart2);
    iBigInteger.finalize(boundMinus);
    iBigInteger.finalize(tmp);

    return bound;
}

static BigInteger compute4(AABACInstance *pInst) {
    return ZERO;
}

/****************************************************************************************************
 * 功能：为有界模型检测计算AABAC实例的界
 * 参数：
 *      @pInst[in]: 待翻译的AABAC实例
 *      @boundLevel[in]: 界的紧度，共4级，1为最松，4为最紧
 * 返回值：
 *      无
 ****************************************************************************************************/
BigInteger computeBound(AABACInstance *pInst, int boundLevel) {
    switch (boundLevel) {
    case 1:
        return compute1(pInst);
    case 2:
        return compute2(pInst);
    case 3:
        return compute3(pInst);
    case 4:
        return compute4(pInst);
    default:
        return ZERO;
    }
}