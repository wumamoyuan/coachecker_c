#ifndef AABAC_ABS_REF_H
#define AABAC_ABS_REF_H

#include "AABACInstance.h"

typedef struct AbsRef {
    int round;

    HashSet *pSetF;
    HashMap *pMapReachableAVs;
    HashMap *pMapReachableAVsInc;

    HashSet *pSetB;
    HashMap *pMapUsefulAVs;
    HashMap *pMapUsefulAVsInc;

    AABACInstance *pOriInst;
    Vector *pOriVecRules;
} AbsRef;

AbsRef *createAbsRef(AABACInstance *pOriInst);

AABACInstance *abstract(AbsRef *pAbsRef);

AABACInstance *refine(AbsRef *pAbsRef);

void deleteAbsRef(AbsRef *pAbsRef);

#endif //AABAC_ABS_REF_H