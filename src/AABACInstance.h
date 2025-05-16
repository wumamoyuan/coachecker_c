#ifndef _AABACData_H
#define _AABACData_H

#include "ccl/containers.h"
#include "ccl/ccl_internal.h"
#include "hashBasedTable.h"
#include "AABACRule.h"

typedef struct _AABACInstance
{
    Vector *pVecUserIdxes;
    
    HashMap *pmapAttr2Dom;
    HashBasedTable *pTableInitState;
    
    HashSet *pSetRuleIdxes;

    HashBasedTable *pTableTargetAV2Rule;
    HashBasedTable *pTablePrecond2Rule;
    
    int queryUserIdx;
    HashMap *pmapQueryAVs;
} AABACInstance;

//用户名列表
extern strCollection *pscUsers;
//用户名到索引的映射
extern Dictionary *pdictUser2Index;

//属性名列表
extern strCollection *pscAttrs;
//属性名到索引的映射
extern Dictionary *pdictAttr2Index;

//值列表
extern strCollection *pscValues;
//值到索引的映射
extern Dictionary *pdictValue2Index;

//属性索引到属性类型的映射
extern HashMap *pmapAttr2Type;
//属性索引到属性默认值的映射
extern HashMap *pmapAttr2DefVal;
    
extern Vector *pVecRules;

AABACInstance *createAABACInstance();

/****************************************************************************************************
 * 功能：获取用户在用户表中的索引
 * 参数：
 *      @user[in]: 用户名
 * 返回值：
 *      用户索引，若用户不存在，则返回-1
 ***************************************************************************************************/
int getUserIndex(char *user);

/****************************************************************************************************
 * 功能：获取属性在属性表中的索引
 * 参数：
 *      @attr[in]: 属性名
 * 返回值：
 *      属性索引，若属性不存在，则返回-1
 ***************************************************************************************************/
int getAttrIndex(char *attr);

/****************************************************************************************************
 * 功能：根据属性类型和属性值获取值索引
 *      具体而言，如果属性类型为布尔类型，value必须为"true"或"false"，对应的值索引为1或0；
 *      如果属性类型为整数类型，value必须为整数，对应的值索引就是value本身的整数值；
 *      如果属性类型为字符串类型，则对应的值索引为value在字符串表中的位置。
 * 参数：
 *      @attrType[in]: 属性类型
 *      @value[in]: 值
 *      @pValueIdx[out]: 值索引
 * 返回值：
 *      出现以下三种情况之一返回-1，否则返回0：
 *      1）attrType为布尔类型时，value不为"true"或"false"
 *      2）attrType为整数类型时，value为非整数
 *      3）attrType为字符串类型时，value在字符串表中不存在
 ***************************************************************************************************/
int getValueIndex(AttrType attrType, char *value, int *pValueIdx);

/****************************************************************************************************
 * 功能：根据属性名获取属性类型
 * 参数：
 *      @pInst[in]： AABAC实例
 *      @attr[in]: 属性名
 * 返回值：
 *      属性类型
 ***************************************************************************************************/
AttrType getAttrType(char *attr);

/****************************************************************************************************
 * 功能：根据属性索引获取属性类型
 * 参数：
 *      @pInst[in]： AABAC实例
 *      @attrIdx[in]: 属性索引
 * 返回值：
 *      属性类型
 ***************************************************************************************************/
AttrType getAttrTypeByIdx(int attrIdx);

/****************************************************************************************************
 * 功能：根据值索引获取值
 * 参数：
 *      @pInst[in]： AABAC实例
 *      @attrType[in]: 属性类型
 *      @valueIdx[in]: 值索引
 * 返回值：
 *      值
 ***************************************************************************************************/
char *getValueByIndex(AttrType attrType, int valueIdx);

char* getAttributeDefaultValue(char *attr);

int getInitValue(AABACInstance *pInst, int userIdx, int attrIdx);

int addUAV(AABACInstance *pInst, char *user, char *attr, char *value);

int addUAVByIdx(AABACInstance *pInst, int userIdx, int attrIdx, int valueIdx);

int addRule(AABACInstance *pInst, int ruleIdx);

void init(AABACInstance *pInst);

void setGlobalInst(AABACInstance *pInst);

char *RuleToString(void *ppRule);

#endif // _AABACData_H