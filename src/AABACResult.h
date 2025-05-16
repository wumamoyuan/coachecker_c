#ifndef AABAC_RESULT_H
#define AABAC_RESULT_H

#define AABAC_RESULT_ERROR -1
#define AABAC_RESULT_TIMEOUT -2
#define AABAC_RESULT_UNKNOWN -3
#define AABAC_RESULT_REACHABLE 0
#define AABAC_RESULT_UNREACHABLE 1

#include "AABACInstance.h"
#include "ccl/containers.h"

typedef struct _AdminstrativeAction {
    int adminIdx;
    int userIdx;
    char *attr;
    char *val;
} AdminstrativeAction;

typedef struct _AABACResult {
    int code;
    Vector *pVecActions;
    Vector *pVecRules;
} AABACResult;

void printResult(AABACResult result, int showRules);

#endif
