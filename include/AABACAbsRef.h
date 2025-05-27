#ifndef AABAC_ABS_REF_H
#define AABAC_ABS_REF_H

#include "AABACInstance.h"

typedef struct AbsRef {
    // The current round of abstraction refinement
    int round;

    // The set of rules selected by the forward rule-selection strategy
    HashSet *pSetF;
    HashMap *pMapReachableAVs;
    HashMap *pMapReachableAVsInc;

    // The set of rules selected by the backward rule-selection strategy
    HashSet *pSetB;
    HashMap *pMapUsefulAVs;
    HashMap *pMapUsefulAVsInc;

    // The AABAC instance before abstraction refinement
    AABACInstance *pOriInst;

    // The global rule list before abstraction refinement
    Vector *pOriVecRules;
} AbsRef;

/**
 * Allocate necessary memory for the abstraction refinement.
 * 
 * @param pOriInst[in]: The AABAC instance before abstraction refinement
 * @return The AbsRef instance
 */
AbsRef *createAbsRef(AABACInstance *pOriInst);

/**
 * Generate an abstract sub-policy and get a smaller AABAC instance.
 * 
 * @param pAbsRef[in]: The AbsRef instance
 * @return A smaller AABAC instance containing the abstracted sub-policy
 */
AABACInstance *abstract(AbsRef *pAbsRef);

/**
 * Refine the AABAC instance by adding more rules to the sub-policy.
 * If the instance cannot be refined, return NULL.
 * 
 * @param pAbsRef[in]: The AbsRef instance
 * @return The refined AABAC instance
 */
AABACInstance *refine(AbsRef *pAbsRef);

#endif //AABAC_ABS_REF_H