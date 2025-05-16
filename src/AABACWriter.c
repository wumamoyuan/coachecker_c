#include "AABACIO.h"
#include "AABACUtils.h"

/****************************************************************************************************
 * 功能：将AABAC实例中的用户集合写入指定文件中，格式为：
 *      Users
 *      user user ... user;
 *      其中Users为关键字，独占一行，user为用户集合中的用户，之间用空格分隔，最后以分号结束。
 * 参数：
 *      @fp[in]： 文件指针
 *      @pInst[in]： AABAC实例
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
static int writeUsers(FILE *fp, AABACInstance *pInst) {
    logAABAC(__func__, __LINE__, 0, DEBUG, "[Writing] Writing users\n");
    if (pInst->pVecUserIdxes == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Users is a NULL pointer\n");
        return -1;
    }

    fprintf(fp, USERS);
    // 遍历用户集合，写入文件
    int i = 0, userNum = iVector.Size(pInst->pVecUserIdxes);
    if (userNum == 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The set of users is empty\n");
        return -1;
    }
    fprintf(fp, "\n%s", istrCollection.GetElement(pscUsers, *(int *)iVector.GetElement(pInst->pVecUserIdxes, i++)));
    while (i < userNum) {
        fprintf(fp, " %s", istrCollection.GetElement(pscUsers, *(int *)iVector.GetElement(pInst->pVecUserIdxes, i++)));
    }
    fprintf(fp, ";\n\n");
    return 0;
}

/****************************************************************************************************
 * 功能：将AABAC实例中的属性集合与各属性的默认值写入指定文件中。
 *       属性集合的格式为：
 *       Attributes
 *       boolean: attr attr ... attr
 *       string: attr attr ... attr
 *       int: attr attr ... attr;
 *       其中Attributes为关键字，独占一行，boolean、string、int为属性的数据类型，每种数据类型的属性集合独占一行，
 *       attr为属性集合中的属性，之间用空格分隔，最后以分号结束。
 *
 *       属性的默认值的格式为：
 *       Default Value
 *       attr: defaultValue
 *       attr: defaultValue
 *       ...
 *       attr: defaultValue;
 *       其中Default Value为关键字，独占一行，attr为属性集合中的属性，defaultValue为属性的默认值，
 *       每个属性的默认值独占一行，属性与默认值之间用冒号分隔，最后一组以分号结束。
 *       注意：如果出现以下三种情况，则不写入默认值。
 *       1）属性为布尔类型且默认值为false；
 *       2）属性为字符串类型且默认值为空字符串；
 *       3）属性为整数类型且默认值为0。
 * 参数：
 *      @fp[in]： 文件指针
 *      @pInst[in]： AABAC实例
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
static int writeAttrsAndDefaultValues(FILE *fp, AABACInstance *pInst) {
    if (iHashMap.Size(pInst->pmapAttr2Dom) == 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The set of attributes is empty\n");
        return -1;
    }

    // HashSet<String>
    HashSet *attrs[3];
    int i = 0;
    for (i = 0; i < 3; i++) {
        attrs[i] = iHashSet.Create(sizeof(int), IntHashCode, IntEqual);
    }

    HashNodeIterator *itmapAttr2Dom = iHashMap.NewIterator(pInst->pmapAttr2Dom);
    HashNode *node;
    int *pAttrIdx;
    AttrType attrType;
    while (itmapAttr2Dom->HasNext(itmapAttr2Dom)) {
        node = itmapAttr2Dom->GetNext(itmapAttr2Dom);
        pAttrIdx = (int *)node->key;
        attrType = getAttrTypeByIdx(*pAttrIdx);
        iHashSet.Add(attrs[attrType], pAttrIdx);
    }
    iHashMap.DeleteIterator(itmapAttr2Dom);

    // 写入属性及其数据类型
    fprintf(fp, ATTRIBUTES);
    HashSetIterator *itHashSet;
    char *typename;
    for (i = 0; i < 3; i++) {
        typename = (i == 0) ? "boolean" : (i == 1) ? "string" : "int";
        if (!iHashSet.Size(attrs[i])) {
            continue;
        }
        fprintf(fp, "\n%s:", typename);
        itHashSet = iHashSet.NewIterator(attrs[i]);
        while (itHashSet->HasNext(itHashSet)) {
            fprintf(fp, " %s", istrCollection.GetElement(pscAttrs, *(int *)itHashSet->GetNext(itHashSet)));
        }
        iHashSet.DeleteIterator(itHashSet);
    }
    fprintf(fp, ";\n\n");

    // 写入各属性的默认值
    fprintf(fp, DEFAULT_VALUE);
    int defaultValueIdx;
    int f = 0;
    for (i = 0; i < 3; i++) {
        if (!iHashSet.Size(attrs[i])) {
            continue;
        }
        itHashSet = iHashSet.NewIterator(attrs[i]);
        while (itHashSet->HasNext(itHashSet)) {
            pAttrIdx = (int *)itHashSet->GetNext(itHashSet);
            defaultValueIdx = *(int *)iHashMap.Get(pmapAttr2DefVal, pAttrIdx);
            if (defaultValueIdx) {
                fprintf(fp, "\n%s: %s", istrCollection.GetElement(pscAttrs, *pAttrIdx), getValueByIndex(getAttrTypeByIdx(*pAttrIdx), defaultValueIdx));
                f = 1;
            }
        }
        iHashSet.DeleteIterator(itHashSet);
    }
    if (f) {
        fprintf(fp, ";\n\n");
    } else {
        fprintf(fp, "\n;\n\n");
    }
    for (i = 0; i < 3; i++) {
        iHashSet.Finalize(attrs[i]);
    }
    return 0;
}

/****************************************************************************************************
 * 功能：将AABAC实例中初始状态下的用户属性值集合写入指定文件中，格式为：
 *      UAV
 *      (user, attr, value)
 *      (user, attr, value)
 *      ...
 *      (user, attr, value);
 *      其中UAV为关键字，独占一行，user为用户，attr为属性，value为属性值，
 *      每组用户属性值独占一行，user、attr、value之间用逗号分隔且用小括号()括起来，最后一组以分号结束。
 * 参数：
 *      @fp[in]： 文件指针
 *      @pInst[in]： AABAC实例
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
static int writeUAVs(FILE *fp, AABACInstance *pInst) {
    logAABAC(__func__, __LINE__, 0, DEBUG, "[Writing] Writing UAV\n");
    if (pInst->pTableInitState == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The initial state is a NULL pointer\n");
        return -1;
    }

    fprintf(fp, UAV);
    if (iHashMap.Size(pInst->pTableInitState->pRowMap) == 0) {
        fprintf(fp, "\n;\n\n");
        return 0;
    }
    int f = 0;
    int attrIdx, valueIdx;
    char *user, *attr, *value;
    HashNodeIterator *itUsers = iHashMap.NewIterator(pInst->pTableInitState->pRowMap), *itAttrs;
    HashNode *nodeUsers, *nodeAttrs;
    HashMap *pmapAttrValues;
    while (itUsers->HasNext(itUsers)) {
        nodeUsers = itUsers->GetNext(itUsers);
        user = istrCollection.GetElement(pscUsers, *(int *)nodeUsers->key);

        pmapAttrValues = *(HashMap **)nodeUsers->value;
        itAttrs = iHashMap.NewIterator(pmapAttrValues);
        while (itAttrs->HasNext(itAttrs)) {
            nodeAttrs = itAttrs->GetNext(itAttrs);
            valueIdx = *(int *)nodeAttrs->value;
            if (valueIdx) {
                attrIdx = *(int *)nodeAttrs->key;
                attr = istrCollection.GetElement(pscAttrs, attrIdx);
                value = getValueByIndex(getAttrTypeByIdx(attrIdx), valueIdx);
                fprintf(fp, "\n(%s, %s, %s)", user, attr, value);
                f = 1;
            }
        }
        iHashMap.DeleteIterator(itAttrs);
    }
    iHashMap.DeleteIterator(itUsers);
    if (f) {
        fprintf(fp, ";\n\n");
    } else {
        fprintf(fp, "\n;\n\n");
    }
    return 0;
}

/****************************************************************************************************
 * 功能：将AABAC实例中的规则集合写入指定文件中，格式为：
 *      Rules
 *      rule
 *      rule
 *      ...
 *      rule;
 *      其中Rules为关键字，独占一行，rule为规则集合中的规则，每个规则独占一行，最后一条规则以分号结束。
 * 参数：
 *      @fp[in]： 文件指针
 *      @pInst[in]： AABAC实例
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
static int writeRules(FILE *fp, AABACInstance *pInst) {
    logAABAC(__func__, __LINE__, 0, DEBUG, "[Writing] Writing rules\n");
    if (pVecRules == NULL || pInst->pSetRuleIdxes == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The set of rules is a NULL pointer\n");
        return -1;
    }

    fprintf(fp, RULES);;
    if (iHashSet.Size(pInst->pSetRuleIdxes) == 0) {
        fprintf(fp, "\n;\n\n");
        return 0;
    }

    // 遍历规则集合，写入文件
    int ruleIdx;
    char *rStr;
    Rule *pRule;
    HashSetIterator *itRules = iHashSet.NewIterator(pInst->pSetRuleIdxes);
    while (itRules->HasNext(itRules)) {
        ruleIdx = *(int *)itRules->GetNext(itRules);
        pRule = (Rule *)iVector.GetElement(pVecRules, ruleIdx);
        rStr = RuleToString(&pRule);
        fprintf(fp, "\n%s", rStr);
        free(rStr);
    }
    iHashSet.DeleteIterator(itRules);
    fprintf(fp, ";\n\n");
    return 0;
}

/****************************************************************************************************
 * 功能：将AABAC实例中的查询目标写入指定文件中，格式为：
 *      Spec
 *      (user, attr = value, attr = value, ..., attr = value);
 *      其中Spec为关键字，独占一行，user为待查询的用户，attr为属性，value为属性值。
 * 参数：
 *      @fp[in]： 文件指针
 *      @pInst[in]： AABAC实例
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
static int writeSpec(FILE *fp, AABACInstance *pInst) {
    logAABAC(__func__, __LINE__, 0, DEBUG, "[Writing] Writing spec\n");
    if (pInst->pmapQueryAVs == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The query is a NULL pointer\n");
        return -1;
    }
    if (iHashMap.Size(pInst->pmapQueryAVs) == 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] The set of query attribute-value pairs is empty\n");
        return -1;
    }

    fprintf(fp, SPEC);
    fprintf(fp, "\n(%s", istrCollection.GetElement(pscUsers, pInst->queryUserIdx));

    // 遍历查询属性值集合，写入文件
    HashNodeIterator *itAttrs = iHashMap.NewIterator(pInst->pmapQueryAVs);
    int attrIdx;
    char *attr, *value;
    HashNode *node;
    while (itAttrs->HasNext(itAttrs)) {
        node = itAttrs->GetNext(itAttrs);
        attrIdx = *(int *)node->key;
        attr = istrCollection.GetElement(pscAttrs, attrIdx);
        value = getValueByIndex(getAttrTypeByIdx(attrIdx), *(int *)node->value);
        fprintf(fp, ", %s = %s", attr, value);
    }
    iHashMap.DeleteIterator(itAttrs);
    fprintf(fp, ");\n");
    return 0;
}

int writeAABACInstance(AABACInstance *pInst, char *filename) {
    logAABAC(__func__, __LINE__, 0, DEBUG, "[Writing] writing AABAC instance into file %s\n", filename);
    if (pInst == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Writing failed, the given AABAC instance is a NULL pointer\n");
        return -1;
    }
    if (filename == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Writing failed, the given filename is a NULL pointer\n");
        return -1;
    }
    // 检查文件是否存在，如果存在则删除
    FILE *fp = fopen(filename, "r");
    if (fp != NULL) {
        fclose(fp);
        if (remove(filename) != 0) {
            logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] File %s exists but failed to delete\n", filename);
            return -1;
        }
    }

    // 创建文件，如果文件所在目录不存在，则创建目录
    fp = fopen(filename, "w");
    if (fp == NULL) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to create file %s\n", filename);
        return -1;
    }

    int ret;
    if ((ret = writeUsers(fp, pInst)) != 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to write users\n");
        fclose(fp);
        return ret;
    }
    if ((ret = writeAttrsAndDefaultValues(fp, pInst)) != 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to write attributes and default values\n");
        fclose(fp);
        return ret;
    }
    if ((ret = writeUAVs(fp, pInst)) != 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to write user attribute values\n");
        fclose(fp);
        return ret;
    }
    if ((ret = writeRules(fp, pInst)) != 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to write rules\n");
        fclose(fp);
        return ret;
    }
    if ((ret = writeSpec(fp, pInst)) != 0) {
        logAABAC(__func__, __LINE__, 0, ERROR, "[Writing] Failed to write special attributes\n");
        fclose(fp);
        return ret;
    }
    fclose(fp);
    logAABAC(__func__, __LINE__, 0, INFO, "[Writing] Successfully wrote AABAC instance into file %s\n", filename);
    return 0;
}
