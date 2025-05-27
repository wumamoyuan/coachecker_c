#ifndef AABAC_RESULT_H
#define AABAC_RESULT_H

#define AABAC_RESULT_ERROR -1
#define AABAC_RESULT_TIMEOUT -2
#define AABAC_RESULT_UNKNOWN -3
#define AABAC_RESULT_REACHABLE 0
#define AABAC_RESULT_UNREACHABLE 1

#include "AABACInstance.h"
#include "ccl/containers.h"

/* An administrative action, consisting of an administrator, a user, an attribute, and a value. */
typedef struct _AdminstrativeAction {
    // The index of the administrator who performs the action
    int adminIdx;
    // The index of the user whose attribute is modified by the action
    int userIdx;
    // The attribute that is modified
    char *attr;
    // The value that is assigned to the attribute
    char *val;
} AdminstrativeAction;

typedef struct _AABACResult {
    // The result code, 0: reachable, 1: unreachable, -1: error, -2: timeout, -3: unknown
    int code;
    // The sequence of administrative actions that lead to the reachability of the target state
    Vector *pVecActions;
    // The sequence of rules that are used to authorize the sequence of administrative actions
    Vector *pVecRules;
} AABACResult;

/**
 * Print the AABAC analysis result.
 * 
 * @param result[in]: The result of the AABAC analysis
 * @param showRules[in]: Whether to show the rules
 */
void printResult(AABACResult result, int showRules);

#endif
