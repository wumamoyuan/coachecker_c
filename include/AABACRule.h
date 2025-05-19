#ifndef _AABACRule_H
#define _AABACRule_H

#include "ccl/containers.h"
#include "hashSet.h"

typedef enum {
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN_OR_EQUAL
} comparisonOperator;

typedef enum {
    BOOLEAN,
    STRING,
    INTEGER,
} AttrType;

typedef struct {
    int attribute;
    int value;
    comparisonOperator op;
} AtomCondition;

typedef struct _Rule {
    HashSet *adminCond;
    HashSet *userCond;
    int targetAttrIdx;
    int targetValueIdx;
    HashMap *pmapUserCondValue;
} Rule;

typedef struct _AtomConditionInterface {
    int (*Evaluate)(AtomCondition *atomCond, int valIdx);
    void (*FindEffectiveValues)(AtomCondition *atomdCond, HashSet *domain, HashSet *effectiveValues);
    unsigned int (*HashCode)(void *pAtomCond);
    int (*Equal)(void *pAtomCond1, void *pAtomCond2);
} AtomConditionInterface;

extern AtomConditionInterface iAtomCondition;

typedef struct _RuleInterface {
    Rule *(*Create)(HashSet *adminCond, HashSet *userCond, int targetAttrIdx, int targetValueIdx);
    int (*DiscreteCond)(Rule *r, HashMap *reachableAVs);
    int (*CanBeManaged)(Rule *r, HashMap *userState);
    int (*IsEffective)(Rule *r, HashMap *reachableAVs);
    unsigned int (*HashCode)(void *pRule);
    int (*Equal)(void *pRule1, void *pRule2);
} RuleInterface;

extern RuleInterface iRule;

#endif // _AABACRule_H