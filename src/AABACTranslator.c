#include "AABACTranslator.h"
#include "AABACUtils.h"
#include <assert.h>
#include <time.h>

static void computeAttrDom(AABACInstance *pInst) {
    HashSetIterator *itSet1 = iHashSet.NewIterator(pInst->pSetRuleIdxes), *itSet2;
    int ruleIdx, *pAttrIdx;
    Rule *pRule;
    HashNodeIterator *itMap;
    HashNode *node;
    HashSet **ppSetValIdxes;
    while (itSet1->HasNext(itSet1)) {
        ruleIdx = *(int *)itSet1->GetNext(itSet1);
        pRule = iVector.GetElement(pVecRules, ruleIdx);
        itMap = iHashMap.NewIterator(pRule->pmapUserCondValue);
        while (itMap->HasNext(itMap)) {
            node = itMap->GetNext(itMap);
            pAttrIdx = (int *)node->key;
            ppSetValIdxes = iHashMap.Get(pInst->pmapAttr2Dom, pAttrIdx);
            assert(ppSetValIdxes != NULL);
            itSet2 = iHashSet.NewIterator(*(HashSet **)node->value);
            while (itSet2->HasNext(itSet2)) {
                iHashSet.Add(*ppSetValIdxes, itSet2->GetNext(itSet2));
            }
            iHashSet.DeleteIterator(itSet2);
        }
        iHashMap.DeleteIterator(itMap);
    }
    iHashMap.DeleteIterator(itSet1);

    itMap = iHashMap.NewIterator(pInst->pmapQueryAVs);
    while (itMap->HasNext(itMap)) {
        node = itMap->GetNext(itMap);
        pAttrIdx = (int *)node->key;
        ppSetValIdxes = iHashMap.Get(pInst->pmapAttr2Dom, pAttrIdx);
        assert(ppSetValIdxes != NULL);
        // if (ppSetValIdxes == NULL) {
        //     pSetValIdxes = iHashSet.Create(sizeof(int), IntHashCode, IntEqual);
        //     ppSetValIdxes = &pSetValIdxes;
        //     iHashMap.Put(pInst->pmapAttr2Dom, pAttrIdx, ppSetValIdxes);
        // }
        iHashSet.Add(*ppSetValIdxes, node->value);
    }
    iHashMap.DeleteIterator(itMap);
}

/**
 * 写入状态变量
 * @param instance[in]: 待翻译的AABAC实例
 * @param fp[in]: 输出文件
 */
void translateVars(AABACInstance *pInst, FILE *fp) {
    fprintf(fp, "VAR\n");

    // 定义变量
    HashSet *pSetVals = iHashSet.Create(sizeof(char *), StringHashCode, StringEqual);

    HashNodeIterator *itMap = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    int *pAttrIdx;
    char *attr, *val;
    AttrType attrType;
    HashNode *node;
    HashSetIterator *itSet;
    int first;
    while (itMap->HasNext(itMap)) {
        node = itMap->GetNext(itMap);
        pAttrIdx = (int *)node->key;
        attr = istrCollection.GetElement(pscAttrs, *pAttrIdx);
        attrType = *(AttrType *)iHashMap.Get(pmapAttr2Type, pAttrIdx);
        fprintf(fp, "%s : {", attr);

        first = 1;
        itSet = iHashSet.NewIterator(*(HashSet **)node->value);
        while (itSet->HasNext(itSet)) {
            val = getValueByIndex(attrType, *(int *)itSet->GetNext(itSet));
            fprintf(fp, "%s%s", first ? "" : ",", val);
            first = 0;
            iHashSet.Add(pSetVals, &val);
        }
        iHashSet.DeleteIterator(itSet);
        fprintf(fp, "};\n");
    }
    iHashMap.DeleteIterator(itMap);

    fprintf(fp, "attr : {");
    itMap = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    first = 1;
    while (itMap->HasNext(itMap)) {
        node = itMap->GetNext(itMap);
        attr = istrCollection.GetElement(pscAttrs, *(int *)node->key);
        fprintf(fp, "%s%s%s", first ? "" : ",", attr, ALIAS_SUFFIX);
        first = 0;
    }
    iHashMap.DeleteIterator(itMap);
    fprintf(fp, "};\n");

    fprintf(fp, "val : {");
    itSet = iHashSet.NewIterator(pSetVals);
    first = 1;
    while (itSet->HasNext(itSet)) {
        fprintf(fp, "%s%s", first ? "" : ",", *(char **)itSet->GetNext(itSet));
        first = 0;
    }
    iHashSet.DeleteIterator(itSet);
    fprintf(fp, "};\n\n");
}

/**
 * 写入初始状态
 * @param instance[in]: 待翻译的AABAC实例
 * @param fp[in]: 输出文件
 */
void translateInitState(AABACInstance *pInst, FILE *fp) {
    fprintf(fp, "ASSIGN\n");

    HashMap *avsOfUser = iHashBasedTable.GetRow(pInst->pTableInitState, &pInst->queryUserIdx);
    int *pAttrIdx, valIdx;
    AttrType attrType;
    HashNode *node;
    HashNodeIterator *itMap = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    while (itMap->HasNext(itMap)) {
        node = itMap->GetNext(itMap);
        if (iHashSet.Size(*(HashSet **)node->value) <= 1) {
            continue;
        }
        pAttrIdx = (int *)node->key;
        valIdx = *(int *)iHashMap.Get(avsOfUser, pAttrIdx);
        attrType = *(AttrType *)iHashMap.Get(pmapAttr2Type, pAttrIdx);
        fprintf(fp, "init(%s) := %s;\n", istrCollection.GetElement(pscAttrs, *pAttrIdx), getValueByIndex(attrType, valIdx));
    }
    iHashMap.DeleteIterator(itMap);

    fprintf(fp, "\n");
}

static void translateCanSetRules(AABACInstance *pInst, FILE *fp) {
    int *pTargetAttrIdx, condAttrIdx;
    char *targetAttr, *targetVal, *condAttr;
    AttrType attrType, condAttrType;
    Rule *pRule;
    char *ruleStr;
    int isEffectiveRule, first, isAtLeastOneEffectiveRule;
    HashSet *pSetAttrDom, *pSetEffectiveValues;
    HashSetIterator *itSetRules, *itSetAtomConds, *itSetEffectiveValues;
    AtomCondition *pAtomCond;
    HashMap *pMapValToRules, *pMapAdminCondValue;
    HashNode *node;
    HashNodeIterator *itMapAttr2Dom = iHashMap.NewIterator(pInst->pmapAttr2Dom), *itMapValToRules, *itMapCondValue;
    while (itMapAttr2Dom->HasNext(itMapAttr2Dom)) {
        node = itMapAttr2Dom->GetNext(itMapAttr2Dom);
        pTargetAttrIdx = (int *)node->key;
        pSetAttrDom = *(HashSet **)node->value;
        if (iHashSet.Size(pSetAttrDom) <= 1) {
            continue;
        }
        targetAttr = istrCollection.GetElement(pscAttrs, *pTargetAttrIdx);

        pMapValToRules = iHashBasedTable.GetRow(pInst->pTableTargetAV2Rule, pTargetAttrIdx);
        if (pMapValToRules == NULL) {
            // 如果没有以attr为目标属性的规则，那么该属性的值将永远不会变化
            // 即next(attr) := attr
            fprintf(fp, "next(%s) := %s;\n\n", targetAttr, targetAttr);
            continue;
        }

        attrType = *(AttrType *)iHashMap.Get(pmapAttr2Type, pTargetAttrIdx);
        isAtLeastOneEffectiveRule = 0;

        // 遍历规则，列出next(attr[i])的所有可能变化
        itMapValToRules = iHashMap.NewIterator(pMapValToRules);
        while (itMapValToRules->HasNext(itMapValToRules)) {
            node = itMapValToRules->GetNext(itMapValToRules);
            itSetRules = iHashSet.NewIterator(*(HashSet **)node->value);

            while (itSetRules->HasNext(itSetRules)) {
                pRule = (Rule *)iVector.GetElement(pVecRules, *(int *)itSetRules->GetNext(itSetRules));

                // 检查该规则是否有效
                isEffectiveRule = 1;
                pMapAdminCondValue = iHashMap.Create(sizeof(int), sizeof(HashSet *), IntHashCode, IntEqual);
                iHashMap.SetDestructValue(pMapAdminCondValue, iHashSet.DestructPointer);

                itSetAtomConds = iHashSet.NewIterator(pRule->adminCond);
                while (itSetAtomConds->HasNext(itSetAtomConds)) {
                    pAtomCond = (AtomCondition *)itSetAtomConds->GetNext(itSetAtomConds);
                    condAttrIdx = pAtomCond->attribute;

                    // 在condAttr的值域中寻找所有满足条件adminAtomCond的值effectiveValues
                    pSetAttrDom = iHashMap.Get(pInst->pmapAttr2Dom, &condAttrIdx);
                    pSetEffectiveValues = iHashSet.Create(sizeof(int), IntHashCode, IntEqual);
                    iAtomCondition.FindEffectiveValues(pAtomCond, pSetAttrDom, pSetEffectiveValues);

                    // 如果effectiveValues为空集，则设置标志位isEffectiveRule = false，然后break
                    if (iHashSet.Size(pSetEffectiveValues) == 0) {
                        isEffectiveRule = 0;
                        iHashSet.Finalize(pSetEffectiveValues);
                        break;
                    }

                    // 如果effectiveValues等于condAttr的值域，则不用添加后续条件，直接continue就行
                    if (iHashSet.Equal(&pSetAttrDom, &pSetEffectiveValues)) {
                        iHashSet.Finalize(pSetEffectiveValues);
                        continue;
                    }
                    iHashMap.Put(pMapAdminCondValue, &condAttrIdx, &pSetEffectiveValues);
                }
                // 如果isEffectiveRule = 0，那么continue，跳过该规则
                if (!isEffectiveRule) {
                    iHashMap.Finalize(pMapAdminCondValue);
                    continue;
                }

                // 规则有效，需写入文件
                // 如果还没有写入过next(attr) := case的语句，则写入
                if (!isAtLeastOneEffectiveRule) {
                    fprintf(fp, "next(%s) :=\ncase\n", targetAttr);
                    isAtLeastOneEffectiveRule = 1;
                }

                // 按该规则变化须满足以下几个条件
                // 1.管理属性为attr，即attr = attrAlias
                // 2.管理值为规则的目标值，即val = targetVal
                // 3.被管理为i，即user = i
                // 4.管理员与被管理者分别满足adminCondition与userCondition
                targetVal = getValueByIndex(attrType, pRule->targetValueIdx);
                ruleStr = RuleToString(&pRule);
                fprintf(fp, "-- %s\nattr=%s%s & val=%s", ruleStr, targetAttr, ALIAS_SUFFIX, targetVal);
                free(ruleStr);

                HashMap *condValues[2] = {pMapAdminCondValue, pRule->pmapUserCondValue};
                int i;
                for (i = 0; i < 2; i++) {
                    itMapCondValue = iHashMap.NewIterator(condValues[i]);
                    while (itMapCondValue->HasNext(itMapCondValue)) {
                        node = itMapCondValue->GetNext(itMapCondValue);
                        condAttrIdx = *(int *)node->key;
                        condAttr = istrCollection.GetElement(pscAttrs, condAttrIdx);
                        condAttrType = *(AttrType *)iHashMap.Get(pmapAttr2Type, &condAttrIdx);
                        pSetEffectiveValues = *(HashSet **)node->value;
                        itSetEffectiveValues = iHashSet.NewIterator(pSetEffectiveValues);
                        if (iHashSet.Size(pSetEffectiveValues) == 1) {
                            while (itSetEffectiveValues->HasNext(itSetEffectiveValues)) {
                                fprintf(fp, " & %s=%s", condAttr, getValueByIndex(condAttrType, *(int *)itSetEffectiveValues->GetNext(itSetEffectiveValues)));
                            }
                        } else {
                            first = 1;
                            while (itSetEffectiveValues->HasNext(itSetEffectiveValues)) {
                                fprintf(fp, first ? " & (%s=%s" : " | %s=%s", condAttr, getValueByIndex(condAttrType, *(int *)itSetEffectiveValues->GetNext(itSetEffectiveValues)));
                                first = 0;
                            }
                            fprintf(fp, ")");
                        }
                        iHashSet.DeleteIterator(itSetEffectiveValues);
                    }
                    iHashMap.DeleteIterator(itMapCondValue);
                }
                fprintf(fp, " : %s;\n", targetVal);

                iHashMap.Finalize(pMapAdminCondValue);
            }
            iHashSet.DeleteIterator(itSetRules);
        }
        iHashMap.DeleteIterator(itMapValToRules);

        if (!isAtLeastOneEffectiveRule) {
            fprintf(fp, "next(%s) := %s;\n\n", targetAttr, targetAttr);
            continue;
        }

        fprintf(fp, "-- default\nTRUE : %s;\nesac;\n\n", targetAttr);
    }
    iHashMap.DeleteIterator(itMapAttr2Dom);
}

/**
 * 写入查询目标
 * @param instance[in]: 待翻译的AABAC实例
 * @param fp[in]: 输出文件
 */
static void translateQuery(AABACInstance *pInst, FILE *fp) {
    fprintf(fp, "LTLSPEC\n");

    int *pAttrIdx, first = 1;
    char *attr, *val;
    AttrType attrType;
    HashNode *node;
    HashNodeIterator *itMap = iHashMap.NewIterator(pInst->pmapQueryAVs);
    while (itMap->HasNext(itMap)) {
        node = itMap->GetNext(itMap);
        pAttrIdx = (int *)node->key;
        attr = istrCollection.GetElement(pscAttrs, *pAttrIdx);
        attrType = *(AttrType *)iHashMap.Get(pmapAttr2Type, pAttrIdx);
        val = getValueByIndex(attrType, *(int *)node->value);
        fprintf(fp, first ? "G (%s!=%s" : " | %s!=%s", attr, val);
        first = 0;
    }
    fprintf(fp, ")");
}

int translate(AABACInstance *instance, char *nusmvFilePath, int sliced) {
    logAABAC(__func__, __LINE__, 0, INFO, "[begin] translating aabac instance into nusmv file %s\n", nusmvFilePath);
    clock_t startTranslating = clock();

    FILE *fp = fopen(nusmvFilePath, "w");
    if (fp == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "Failed to open file: %s\n", nusmvFilePath);
        return -1;
    }

    if (!sliced) {
        computeAttrDom(instance);
    }

    fprintf(fp, "-- This NuSMV specification was automatically generated by aabac policy verifier\n\n");
    fprintf(fp, "MODULE main\n\n");

    translateVars(instance, fp);
    translateInitState(instance, fp);
    translateCanSetRules(instance, fp);
    translateQuery(instance, fp);

    fclose(fp);

    double timeSpent = (double)(clock() - startTranslating) / CLOCKS_PER_SEC * 1000;
    logAABAC(__func__, __LINE__, 0, INFO, "[end] translating aabac instance into nusmv file, cost => %.2fms\n", timeSpent);
    return 0;
}