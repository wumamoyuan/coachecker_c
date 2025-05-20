#include "bn_java.h"

#define INFLATED INT64_MIN

typedef struct BigDecimal {
    BigInteger intVal;
    int scale;
    int precision;
    long intCompact;
} BigDecimal;

typedef enum {
    ROUND_UP,
    ROUND_DOWN,
    ROUND_CEILING,
    ROUND_FLOOR,
    ROUND_HALF_UP,
    ROUND_HALF_DOWN,
    ROUND_HALF_EVEN,
    ROUND_UNNECESSARY
} RoundingMode;

BigDecimal createFromBigInteger(BigInteger integer) {
    BigDecimal result;
    result.intVal = integer;
    result.scale = 0;
    result.intCompact = INFLATED;
    // todo: 精度需要计算
    result.precision = 0;
    return result;
}

static BigInteger bigMultiplyPowerTen(BigInteger value, int n) {
    // if (n <= 0)
    //     return value;
    // if (n < LONG_TEN_POWERS_TABLE.length) {
    //     return value.multiply(LONG_TEN_POWERS_TABLE[n]);
    // }
    // return value.multiply(bigTenToThe(n));
    return iBigInteger.createFromInt(1);
}

static BigDecimal divideAndRound(BigInteger bdividend, BigInteger bdivisor, int scale, int roundingMode, int preferredScale) {
    BigDecimal ret;
    return ret;
}

static BigDecimal divideBigIntegers(BigInteger dividend, int dividendScale, BigInteger divisor, int divisorScale, int scale, int roundingMode) {
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

    if (dividend.intCompact == INFLATED || divisor.intCompact == INFLATED) {
        return divideBigIntegers(dividend.intVal, dividend.scale, divisor.intVal, divisor.scale, scale, roundingMode);
    }

    printf("only support to divide big integer now\n");
    exit(1);
}