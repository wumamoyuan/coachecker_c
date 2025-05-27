#ifndef AABAC_BOUND_CALCULATOR_H
#define AABAC_BOUND_CALCULATOR_H

#include "AABACInstance.h"
#include "BigInteger.h"

/**
 * Bound estimation for an AABAC instance. Used to invoke bounded model checking.
 * 
 * @param pInst[in]: The AABAC instance
 * @param tightLevel[in]: The tight level of the bound
 * @return The bound of the AABAC instance
 */
BigInteger computeBound(AABACInstance *pInst, int tightLevel);

#endif // AABAC_BOUND_CALCULATOR_H
