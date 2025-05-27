#include "BigDecimal.h"
#include <stdio.h>

#define INFLATED INT64_MIN
#define MAX_COMPACT_DIGITS 18

#define LONG_TEN_POWERS_TABLE_SIZE 19

static const long LONG_TEN_POWERS_TABLE[] = {
    1,                   // 0 / 10^0
    10,                  // 1 / 10^1
    100,                 // 2 / 10^2
    1000,                // 3 / 10^3
    10000,               // 4 / 10^4
    100000,              // 5 / 10^5
    1000000,             // 6 / 10^6
    10000000,            // 7 / 10^7
    100000000,           // 8 / 10^8
    1000000000,          // 9 / 10^9
    10000000000L,        // 10 / 10^10
    100000000000L,       // 11 / 10^11
    1000000000000L,      // 12 / 10^12
    10000000000000L,     // 13 / 10^13
    100000000000000L,    // 14 / 10^14
    1000000000000000L,   // 15 / 10^15
    10000000000000000L,  // 16 / 10^16
    100000000000000000L, // 17 / 10^17
    1000000000000000000L // 18 / 10^18
};

static long compactValFor(BigInteger b) {
    uint32_t m[] = b.mag;
    int len = b.magLen;
    if (len == 0)
        return 0;
    uint32_t d = m[0];
    if (len > 2 || (len == 2 && (int32_t)d < 0))
        return INFLATED;

    long u = (len == 2) ? (m[1] + ((long)d << 32)) : (long)d;
    return (b.signum < 0) ? -u : u;
}

static BigDecimal createFromBigInteger(BigInteger val) {
    return (BigDecimal){
        .scale = 0,
        .intCompact = compactValFor(val),
        .intVal = val,
        .precision = 0};
}

static int checkScale(BigInteger intVal, long val) {
    int asInt = (int)val;
    if (asInt != val) {
        asInt = val > INT32_MAX ? INT32_MAX : INT32_MIN;
        if (intVal.signum != 0) {
            printf(asInt > 0 ? "Underflow" : "Overflow");
            exit(1);
        }
    }
    return asInt;
}

static BigInteger bigTenToThe(int n) {
    if (n < 0)
        return ZERO;

    if (n < BIG_TEN_POWERS_TABLE_MAX) {
        BigInteger[] pows = BIG_TEN_POWERS_TABLE;
        if (n < pows.length)
            return pows[n];
        else
            return expandBigIntegerTenPowers(n);
    }

    return BigInteger.TEN.pow(n);
}

static BigInteger bigMultiplyPowerTen(BigInteger value, int n) {
    if (n <= 0)
        return value;
    BigInteger ret;
    if (n < LONG_TEN_POWERS_TABLE_SIZE) {
        // todo: 需要实现multiplyByLong
        ret = iBigInteger.multiplyByLong(value, LONG_TEN_POWERS_TABLE[n]);
    } else {
        ret = iBigInteger.multiply(value, bigTenToThe(n));
    }
    return ret;
}

static BigDecimal divideAndRound(BigInteger bdividend, BigInteger bdivisor, int scale, int roundingMode, int preferredScale) {
    BigDecimal ret;
    return ret;
}

static BigDecimal divideBIByBI(BigInteger dividend, int dividendScale, BigInteger divisor, int divisorScale, int scale, int roundingMode) {
    if (checkScale(dividend, (long)scale + divisorScale) > dividendScale) {
        int newScale = scale + divisorScale;
        int raise = newScale - dividendScale;
        BigInteger scaledDividend = bigMultiplyPowerTen(dividend, raise);
        return divideAndRound(scaledDividend, divisor, scale, roundingMode, scale);
    } else {
        int newScale = checkScale(divisor, (long)dividendScale - scale);
        int raise = newScale - divisorScale;
        BigInteger scaledDivisor = bigMultiplyPowerTen(divisor, raise);
        return divideAndRound(dividend, scaledDivisor, scale, roundingMode, scale);
    }
}

/**
 * 将两个BigDecimal相除
 * @param dividend 被除数
 * @param divisor 除数
 * @return 相除后的大整数z
 */
static BigDecimal divide(BigDecimal dividend, BigDecimal divisor, int scale, RoundingMode roundingMode) {
    if (roundingMode < ROUND_UP || roundingMode > ROUND_UNNECESSARY) {
        printf("Invalid rounding mode: %d\n", roundingMode);
        exit(1);
    }
    if (roundingMode != ROUND_HALF_UP) {
        printf("rounding mode %d is not supported yet\n", roundingMode);
        exit(1);
    }

    if (dividend.intCompact != INFLATED) {
        if ((divisor.intCompact != INFLATED)) {
            return divideLongByLong(dividend.intCompact, dividend.scale, divisor.intCompact, divisor.scale, scale, roundingMode);
        } else {
            return divideLongByBI(dividend.intCompact, dividend.scale, divisor.intVal, divisor.scale, scale, roundingMode);
        }
    } else {
        if ((divisor.intCompact != INFLATED)) {
            return divideBIByLong(dividend.intVal, dividend.scale, divisor.intCompact, divisor.scale, scale, roundingMode);
        } else {
            return divideBIByBI(dividend.intVal, dividend.scale, divisor.intVal, divisor.scale, scale, roundingMode);
        }
    }
}