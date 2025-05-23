#include "BigInteger.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * The threshold value for using Schoenhage recursive base conversion. If
 * the number of ints in the number are larger than this value,
 * the Schoenhage algorithm will be used.  In practice, it appears that the
 * Schoenhage routine is faster for any threshold down to 2, and is
 * relatively flat for thresholds between 2-25, so this choice may be
 * varied within this range for very small effect.
 */
#define SCHOENHAGE_BASE_CONVERSION_THRESHOLD 20

/**
 * The threshold value for using Karatsuba multiplication.  If the number
 * of ints in both mag arrays are greater than this number, then
 * Karatsuba multiplication will be used.   This value is found
 * experimentally to work well.
 */
#define MULTIPLY_SQUARE_THRESHOLD 20

/**
 * The threshold value for using Karatsuba multiplication.  If the number of
 * ints in the number are larger than this value, {@code multiply} will use
 * Karatsuba multiplication.  The choice of 32 is arbitrary, and Karatsuba
 * multiplication is faster for any threshold down to 2.
 */
#define KARATSUBA_THRESHOLD 80

/**
 * The threshold value for using 3-way Toom-Cook multiplication.
 * If the number of ints in each mag array is greater than the
 * Karatsuba threshold, and the number of ints in at least one of
 * the mag arrays is greater than this threshold, then Toom-Cook
 * multiplication will be used.
 */
#define TOOM_COOK_THRESHOLD 240

#define MAX_VAL 0xFFFFFFFFL

#define require(p, msg) assert(p &&msg)

static BigInteger createFromInt(int i) {
    if (i == 0) {
        return ZERO;
    }

    int signum;
    uint32_t *mag;

    if (i < 0) {
        signum = -1;
        i = -i;
    } else {
        signum = 1;
    }
    mag = (uint32_t *)malloc(sizeof(uint32_t));
    mag[0] = i;
    return (BigInteger){signum, 1, mag};
}

static BigInteger createFromHexString(char *hex) {
    int signum, magLen;
    uint32_t *mag;

    if (hex[0] == '-') {
        signum = -1;
        hex++;
    } else {
        signum = 1;
    }

    while (hex[0] == '0') {
        hex++;
    }

    int len = strlen(hex);
    if (len == 0) {
        return ZERO;
    }

    magLen = (len - 1) / 8 + 1;
    mag = (uint32_t *)malloc(magLen * sizeof(uint32_t));

    int i, j;
    char *p = hex + len - 8;
    for (i = 0; i < magLen - 1; i++) {
        mag[i] = 0;
        for (j = 0; j < 8; j++) {
            if ('0' <= *p && *p <= '9') {
                mag[i] = mag[i] << 4 | (*p - '0');
            } else if ('a' <= *p && *p <= 'f') {
                mag[i] = mag[i] << 4 | (*p - 'a' + 10);
            } else if ('A' <= *p && *p <= 'F') {
                mag[i] = mag[i] << 4 | (*p - 'A' + 10);
            }
            p++;
        }
        p -= 16;
    }

    p = hex;
    mag[i] = 0;
    for (j = 0; j < (len - 1) % 8 + 1; j++) {
        if ('0' <= *p && *p <= '9') {
            mag[i] = mag[i] << 4 | (*p - '0');
        } else if ('a' <= *p && *p <= 'f') {
            mag[i] = mag[i] << 4 | (*p - 'a' + 10);
        } else if ('A' <= *p && *p <= 'F') {
            mag[i] = mag[i] << 4 | (*p - 'A' + 10);
        }
        p++;
    }
    return (BigInteger){signum, magLen, mag};
}

static void finalize(BigInteger n) {
    if (n.mag != NULL) {
        free(n.mag);
        n.mag = NULL;
    }
}

static char *smallToString(BigInteger n, char *p, int digits) {
    char *str;
    require(1 > 2, "not implemented");
    return str;
}

static void recursiveToString(BigInteger n, char *p, int digits) {
    require(n.signum >= 0, "n must be non-negative");

    // If we're smaller than a certain threshold, use the smallToString
    // method, padding with leading zeroes when necessary unless we're
    // at the beginning of the string or digits <= 0. As u.signum() >= 0,
    // smallToString() will not prepend a negative sign.
    if (n.magLen <= SCHOENHAGE_BASE_CONVERSION_THRESHOLD) {
        smallToString(n, p, digits);
        return;
    }

    // todo: Schoenhage recursive base conversion
}

/**
 * 将大整数转换为字符串
 * @param n 大整数
 * @return 大整数的字符串表示
 */
static char *toString(BigInteger n) {
    char *str;
    if (n.signum == 0) {
        str = (char *)malloc(2 * sizeof(char));
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int strlen = n.magLen * 10 + 1;
    str = (char *)malloc((strlen + 1) * sizeof(char));
    char *p = str;
    if (n.signum < 0) {
        *p++ = '-';
        BigInteger abs = n;
        abs.signum = 1;
        n = abs;
    }

    // use recursive toString
    recursiveToString(n, p, 0);
    return str;
}

/**
 * Returns the number of zero bits preceding the highest-order
 * ("leftmost") one-bit in the two's complement binary representation
 * of the specified uint32_t value.  Returns 32 if the
 * specified value has no one-bits in its two's complement representation,
 * in other words if it is equal to zero.
 */
static int numberOfLeadingZeros(uint32_t i) {
    // HD, Count leading 0's
    if (i <= 0)
        return i == 0 ? 32 : 0;
    int n = 31;
    if (i >= 1 << 16) {
        n -= 16;
        i >>= 16;
    }
    if (i >= 1 << 8) {
        n -= 8;
        i >>= 8;
    }
    if (i >= 1 << 4) {
        n -= 4;
        i >>= 4;
    }
    if (i >= 1 << 2) {
        n -= 2;
        i >>= 2;
    }
    return n - (i >> 1);
}

/**
 * 将大整数转换为十六进制字符串，不包括0x前缀，返回的内存需要free
 * @param n 大整数
 * @return 大整数的十六进制字符串表示
 */
static char *toHexString(BigInteger n) {
    char *str;
    if (n.signum == 0) {
        str = (char *)malloc(2 * sizeof(char));
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int j = n.magLen - 1;
    uint32_t i = n.mag[j];
    int nBytes = 8 - numberOfLeadingZeros(i) / 4;

    int strlen = (n.magLen << 3) + nBytes - 7 + (n.signum < 0 ? 1 : 0);
    str = (char *)malloc(strlen * sizeof(char));
    char *p = str;
    if (n.signum < 0) {
        *p++ = '-';
    }

    char format[6];
    sprintf(format, "%%.%dx", nBytes);
    sprintf(p, format, n.mag[j--]);
    p += nBytes;

    while (j >= 0) {
        sprintf(p, "%.08x", n.mag[j--]);
        p += 8; /* step WORD_SIZE hex-byte(s) forward in the string. */
    }

    return str;
}

static int compareMagnitude(BigInteger n, BigInteger m) {
    if (n.magLen < m.magLen) {
        return -1;
    }
    if (n.magLen > m.magLen) {
        return 1;
    }
    int i;
    for (i = n.magLen - 1; i >= 0; i--) {
        if (n.mag[i] < m.mag[i]) {
            return -1;
        }
        if (n.mag[i] > m.mag[i]) {
            return 1;
        }
    }
    return 0;
}

/**
 * 将两个无符号大整数的绝对值数组相加，并返回相加后的大整数数组，返回的数组用完后需要释放内存
 * @param xMag 大整数x的数组
 * @param xlen 大整数x的数组长度
 * @param yMag 大整数y的数组
 * @param ylen 大整数y的数组长度
 * @param pzLen 相加后的大整数z的数组长度
 * @return 相加后的大整数z的数组
 */
static uint32_t *addArray(uint32_t *xMag, int xlen, uint32_t *yMag, int ylen, int *pzLen) {
    // 相加后的大整数z的数组长度，最大为xlen和ylen中较大的那个+1
    int zLen = (xlen > ylen ? xlen : ylen) + 1;
    int minLen = xlen < ylen ? xlen : ylen;
    uint32_t *zMag = (uint32_t *)malloc(zLen * sizeof(uint32_t));
    uint64_t sum;
    int carry = 0;
    // 从低位开始相加
    int i;
    for (i = 0; i < minLen; i++) {
        // 将xMag的每一位，yMag的每一位，以及上一位的进位相加
        sum = (uint64_t)xMag[i] + (uint64_t)yMag[i] + carry;
        // 将sum的低32位赋值给z的当前位
        zMag[i] = (uint32_t)(sum & MAX_VAL);
        // 更新进位
        carry = sum > MAX_VAL;
    }
    uint32_t *mag = xlen > ylen ? xMag : yMag;
    for (i = minLen; i < zLen - 1; i++) {
        sum = (uint64_t)mag[i] + carry;
        zMag[i] = (uint32_t)(sum & MAX_VAL);
        carry = sum > MAX_VAL;
    }
    // 将最高位的进位赋值给z的最后一位
    zMag[zLen - 1] = carry;
    // 如果最高位没有进位，则zLen减1
    if (!carry) {
        zLen--;
        zMag = (uint32_t *)realloc(zMag, zLen * sizeof(uint32_t));
    }
    *pzLen = zLen;
    return zMag;
}

/**
 * 将两个无符号大整数的绝对值数组相减，并返回相减后的大整数数组，返回的数组用完后需要释放内存
 * @param bigMag 被减数的数组
 * @param bigLen 被减数的数组长度
 * @param smallMag 减数的数组
 * @param smallLen 减数的数组长度
 * @param pzLen 相减后的大整数z的数组长度
 * @return 相减后的大整数z的数组
 */
static uint32_t *subArray(uint32_t *bigMag, int bigLen, uint32_t *smallMag, int smallLen, int *pzLen) {
    uint32_t *zMag = (uint32_t *)malloc(bigLen * sizeof(uint32_t));
    int borrow = 0;
    int i = 0;
    int nonZeroIndex = 0;
    for (i = 0; i < smallLen; i++) {
        zMag[i] = bigMag[i] - smallMag[i] - borrow;
        borrow = zMag[i] > bigMag[i];
        if (zMag[i] != 0) {
            nonZeroIndex = i;
        }
    }
    while (borrow) {
        borrow = ((zMag[smallLen] = bigMag[smallLen] - 1) == MAX_VAL);
        if (zMag[smallLen] != 0) {
            nonZeroIndex = smallLen;
        }
        smallLen++;
    }
    if (smallLen < bigLen) {
        nonZeroIndex = bigLen - 1;
        for (i = smallLen; i < bigLen; i++) {
            zMag[i] = bigMag[i];
        }
    }
    *pzLen = nonZeroIndex + 1;
    if (*pzLen < bigLen) {
        zMag = (uint32_t *)realloc(zMag, *pzLen * sizeof(uint32_t));
    }
    return zMag;
}

/**
 * 将两个大整数相加
 * @param n 大整数n
 * @param m 大整数m
 * @param z 相加后的大整数z
 */
static BigInteger add(BigInteger n, BigInteger m) {
    int signum;
    uint32_t *mag;
    int magLen;

    BigInteger *tmp = NULL;
    if (n.signum == 0) {
        tmp = &m;
    } else if (m.signum == 0) {
        tmp = &n;
    }
    if (tmp != NULL) {
        signum = tmp->signum;
        magLen = tmp->magLen;
        if (tmp->magLen > 0) {
            mag = (uint32_t *)malloc(magLen * sizeof(uint32_t));
            int i;
            for (i = 0; i < magLen; i++) {
                mag[i] = tmp->mag[i];
            }
        }
        return (BigInteger){signum, magLen, mag};
    }
    if (n.signum == m.signum) {
        mag = addArray(n.mag, n.magLen, m.mag, m.magLen, &magLen);
        return (BigInteger){n.signum, magLen, mag};
    }
    int cmp = compareMagnitude(n, m);
    if (cmp == 0) {
        return ZERO;
    }
    signum = cmp == n.signum ? 1 : -1;
    if (cmp < 0) {
        mag = subArray(m.mag, m.magLen, n.mag, n.magLen, &magLen);
    } else {
        mag = subArray(n.mag, n.magLen, m.mag, m.magLen, &magLen);
    }
    return (BigInteger){signum, magLen, mag};
}

/**
 * 将两个大整数相减
 * @param n 大整数n
 * @param m 大整数m
 * @return 相减后的大整数
 */
static BigInteger subtract(BigInteger n, BigInteger m) {
    BigInteger *tmp = NULL;
    int signum;
    if (n.signum == 0) {
        tmp = &m;
        signum = m.signum;
    } else if (m.signum == 0) {
        tmp = &n;
        signum = n.signum;
    }
    int magLen;
    uint32_t *mag;
    if (tmp != NULL) {
        magLen = tmp->magLen;
        if (tmp->magLen > 0) {
            mag = (uint32_t *)malloc(magLen * sizeof(uint32_t));
            int i;
            for (i = 0; i < magLen; i++) {
                mag[i] = tmp->mag[i];
            }
        }
        return (BigInteger){signum, magLen, mag};
    }

    if (n.signum != m.signum) {
        mag = addArray(n.mag, n.magLen, m.mag, m.magLen, &magLen);
        return (BigInteger){n.signum, magLen, mag};
    }

    int cmp = compareMagnitude(n, m);
    if (cmp == 0) {
        return ZERO;
    }
    signum = (cmp == n.signum ? 1 : -1);
    if (cmp < 0) {
        mag = subArray(m.mag, m.magLen, n.mag, n.magLen, &magLen);
    } else {
        mag = subArray(n.mag, n.magLen, m.mag, m.magLen, &magLen);
    }
    return (BigInteger){signum, magLen, mag};
}

/**
 * 将一个无符号大整数减去一个整数
 * @param n 大整数n
 * @param m 整数m
 * @return 相减后的大整数
 */
static BigInteger subtractByInt(BigInteger n, int m) {
    BigInteger tmp = createFromInt(m);
    BigInteger z = subtract(n, tmp);
    finalize(tmp);
    return z;
}

/**
 * 判断一个数是否是2的幂
 * @param n 要判断的数
 * @return 如果是2的幂，返回1，否则返回0
 */
static int isTwoPower(uint32_t n) {
    return (n & (n - 1)) == 0;
}

/**
 * 将一个无符号大整数的数组左移指定的位数，并返回左移后的数组，返回的数组用完后需要释放内存
 * @param xMag 大整数x的数组
 * @param xLen 大整数x的数组长度
 * @param shift 左移的位数
 * @param pzLen 左移后的大整数z的数组长度
 * @return 左移后的大整数z的数组
 */
static uint32_t *shiftLeft(uint32_t *xMag, int xLen, int shift, int *pzLen) {
    int shiftInt = shift >> 5;
    shift = shift & 0x1F;

    int zLen = xLen + shiftInt + 1;
    uint32_t *zMag = (uint32_t *)malloc(zLen * sizeof(uint32_t));

    uint64_t num = 0;
    uint32_t carry = 0;
    int zi, xi;
    for (zi = 0; zi < shiftInt; zi++) {
        zMag[zi] = 0;
    }
    for (xi = 0; xi < xLen; xi++) {
        num = (uint64_t)xMag[xi] << shift;
        zMag[zi++] = (uint32_t)(num & MAX_VAL) | carry;
        carry = (uint32_t)(num >> 32);
    }
    zMag[zi] = carry;
    if (carry == 0) {
        zLen--;
        zMag = (uint32_t *)realloc(zMag, zLen * sizeof(uint32_t));
    }
    *pzLen = zLen;
    return zMag;
}

/**
 * 将一个无符号大整数的数组乘以一个无符号整数，并返回乘积的数组，返回的数组用完后需要释放内存
 * @param xMag 大整数x的数组
 * @param xLen 大整数x的数组长度
 * @param y 乘数
 * @param pzLen 乘积z的数组长度
 * @return 乘积z的数组
 */
static uint32_t *multiplyByIntArray(uint32_t *xMag, int xLen, uint32_t y, int *pzLen) {
    require(y != 0, "y must not be 0");
    if (isTwoPower(y)) {
        int shift = 0;
        while (y != 1) {
            y >>= 1;
            shift++;
        }
        return shiftLeft(xMag, xLen, shift, pzLen);
    }

    int zLen = xLen + 1;
    uint32_t *zMag = (uint32_t *)malloc(zLen * sizeof(uint32_t));

    uint64_t product = 0;
    uint64_t carry = 0;
    int i;
    for (i = 0; i < xLen; i++) {
        product = (uint64_t)xMag[i] * y + carry;
        zMag[i] = (uint32_t)(product & MAX_VAL);
        carry = product >> 32;
    }
    zMag[i] = (uint32_t)carry;
    if (carry == 0) {
        zLen--;
        zMag = (uint32_t *)realloc(zMag, zLen * sizeof(uint32_t));
    }
    *pzLen = zLen;
    return zMag;
}

/**
 * 将两个无符号大整数的数组相乘，并返回乘积的数组，返回的数组用完后需要释放内存
 * @param xMag 大整数x的数组
 * @param xLen 大整数x的数组长度
 * @param yMag 大整数y的数组
 * @param yLen 大整数y的数组长度
 */
static uint32_t *multiplyArray(uint32_t *xMag, int xLen, uint32_t *yMag, int yLen, int *pzLen) {
    if (xLen <= 0 || yLen <= 0) {
        return NULL;
    }

    int zLen = xLen + yLen;
    uint32_t *zMag = (uint32_t *)malloc(zLen * sizeof(uint32_t));

    uint64_t product = 0;
    uint64_t carry = 0;

    int i, j, k;
    for (j = 0; j < yLen; j++) {
        product = (uint64_t)xMag[0] * yMag[j] + carry;
        zMag[j] = (uint32_t)(product & MAX_VAL);
        carry = product >> 32;
    }
    zMag[j] = (uint32_t)carry;

    for (i = 1; i < xLen; i++) {
        carry = 0;
        for (j = 0, k = i; j < yLen; j++, k++) {
            product = (uint64_t)xMag[i] * yMag[j] + zMag[k] + carry;
            zMag[k] = (uint32_t)(product & MAX_VAL);
            carry = product >> 32;
        }
        zMag[k] = (uint32_t)carry;
    }
    if (carry == 0) {
        zLen--;
        zMag = (uint32_t *)realloc(zMag, zLen * sizeof(uint32_t));
    }
    *pzLen = zLen;
    return zMag;
}

/**
 * Returns a new BigInteger representing n lower ints of the number.
 * This is used by Karatsuba multiplication and Karatsuba squaring.
 */
static BigInteger getLower(BigInteger x, int n) {
    uint32_t *mag;
    int magLen;

    int i;
    if (x.magLen <= n) {
        magLen = x.magLen;
        mag = (uint32_t *)malloc(magLen * sizeof(uint32_t));
        for (i = 0; i < magLen; i++) {
            mag[i] = x.mag[i];
        }
        return (BigInteger){1, magLen, mag};
    }

    mag = (uint32_t *)malloc(n * sizeof(uint32_t));
    for (i = 0; i < n; i++) {
        mag[i] = x.mag[i];
        if (mag[i] != 0) {
            magLen = i + 1;
        }
    }
    if (magLen < n) {
        mag = (uint32_t *)realloc(mag, n * sizeof(uint32_t));
    }
    return (BigInteger){1, magLen, mag};
}

/**
 * Returns a new BigInteger representing mag.length-n upper
 * ints of the number.  This is used by Karatsuba multiplication and
 * Karatsuba squaring.
 */
static BigInteger getUpper(BigInteger x, int n) {
    if (x.magLen <= n) {
        return ZERO;
    }

    int nonZeroIndex = 0;
    int upperLen = x.magLen - n;
    uint32_t *upperInts = (uint32_t *)malloc(upperLen * sizeof(uint32_t));
    int i;
    for (i = 0; i < upperLen; i++) {
        upperInts[i] = x.mag[i + n];
        if (upperInts[i] != 0) {
            nonZeroIndex = i;
        }
    }
    if (nonZeroIndex + 1 < upperLen) {
        upperLen = nonZeroIndex + 1;
        upperInts = (uint32_t *)realloc(upperInts, upperLen * sizeof(uint32_t));
    }

    return (BigInteger){1, upperLen, upperInts};
}

static BigInteger multiply(BigInteger x, BigInteger y);

/**
 * Multiplies two BigIntegers using the Karatsuba multiplication
 * algorithm.  This is a recursive divide-and-conquer algorithm which is
 * more efficient for large numbers than what is commonly called the
 * "grade-school" algorithm used in multiplyToLen.  If the numbers to be
 * multiplied have length n, the "grade-school" algorithm has an
 * asymptotic complexity of O(n^2).  In contrast, the Karatsuba algorithm
 * has complexity of O(n^(log2(3))), or O(n^1.585).  It achieves this
 * increased performance by doing 3 multiplies instead of 4 when
 * evaluating the product.  As it has some overhead, should be used when
 * both numbers are larger than a certain threshold (found experimentally).
 *
 * See:  http://en.wikipedia.org/wiki/Karatsuba_algorithm
 */
static BigInteger multiplyKaratsuba(BigInteger x, BigInteger y) {
    int xlen = x.magLen;
    int ylen = y.magLen;

    // The number of ints in each half of the number.
    int half = ((xlen > ylen ? xlen : ylen) + 1) / 2;

    // xl and yl are the lower halves of x and y respectively,
    // xh and yh are the upper halves.
    BigInteger xl = getLower(x, half);
    BigInteger xh = getUpper(x, half);
    BigInteger yl = getLower(y, half);
    BigInteger yh = getUpper(y, half);

    // p1 = xh * yh, p2 = xl * yl
    BigInteger p1 = multiply(xh, yh);
    BigInteger p2 = multiply(xl, yl);

    // p5 = (xh + xl) * (yh + yl)
    BigInteger p3 = add(xh, xl);
    BigInteger p4 = add(yh, yl);
    BigInteger p5 = multiply(p3, p4);
    finalize(xl);
    finalize(xh);
    finalize(yl);
    finalize(yh);
    finalize(p3);
    finalize(p4);

    // p7 = p5 - p1 - p2 = xh * yl + xl * yh
    BigInteger p6 = subtract(p5, p1);
    BigInteger p7 = subtract(p6, p2);
    finalize(p5);
    finalize(p6);

    // z = p1 * 2^(2 * 32 * half) + p7 * 2^(32 * half) + p2
    BigInteger p8, p10;
    p8.mag = shiftLeft(p1.mag, p1.magLen, 32 * half, &p8.magLen);
    p8.signum = 1;
    finalize(p1);

    BigInteger p9 = add(p8, p7);
    p10.mag = shiftLeft(p9.mag, p9.magLen, 32 * half, &p10.magLen);
    p10.signum = 1;
    finalize(p7);
    finalize(p8);
    finalize(p9);

    BigInteger z = add(p10, p2);
    finalize(p2);
    finalize(p10);

    if (x.signum != y.signum) {
        z.signum = -1;
    }
    return z;
}

static BigInteger multiplyByInt(BigInteger x, int y) {
    if (y == 0 || x.signum == 0) {
        return ZERO;
    }

    int signum;
    if (y < 0) {
        signum = -x.signum;
        y = -y;
    } else {
        signum = x.signum;
    }

    int magLen;
    uint32_t *mag = multiplyByIntArray(x.mag, x.magLen, (uint32_t)y, &magLen);
    return (BigInteger){signum, magLen, mag};
}

/**
 * 将两个大整数相乘
 * @param x 大整数x
 * @param y 大整数y
 * @return 相乘后的大整数z
 */
static BigInteger multiply(BigInteger x, BigInteger y) {
    if (x.signum == 0 || y.signum == 0) {
        return ZERO;
    }

    int xlen = x.magLen;
    // if (m == n && nlen > MULTIPLY_SQUARE_THRESHOLD) {
    //     square(n, z);
    //     return;
    // }

    int ylen = y.magLen;

    if ((xlen < KARATSUBA_THRESHOLD) || (ylen < KARATSUBA_THRESHOLD)) {
        int magLen;
        uint32_t *mag;
        if (ylen == 1) {
            mag = multiplyByIntArray(x.mag, xlen, y.mag[0], &magLen);
        } else if (xlen == 1) {
            mag = multiplyByIntArray(y.mag, ylen, x.mag[0], &magLen);
        } else {
            mag = multiplyArray(x.mag, xlen, y.mag, ylen, &magLen);
        }
        return (BigInteger){x.signum == y.signum ? 1 : -1, magLen, mag};
    }

    if ((xlen < TOOM_COOK_THRESHOLD) && (ylen < TOOM_COOK_THRESHOLD)) {
        return multiplyKaratsuba(x, y);
    }
    return multiplyKaratsuba(x, y);

    // if (!isRecursion) {
    //     // The bitLength() instance method is not used here as we
    //     // are only considering the magnitudes as non-negative. The
    //     // Toom-Cook multiplication algorithm determines the sign
    //     // at its end from the two signum values.
    //     if (bitLength(mag, mag.length) +
    //             bitLength(val.mag, val.mag.length) >
    //         32L * MAX_MAG_LENGTH) {
    //         reportOverflow();
    //     }
    // }

    // return multiplyToomCook3(this, val);
}

// /**
//  * This method is used for division. It multiplies an n word input a by one
//  * word input x, and subtracts the n word product from q. This is needed
//  * when subtracting qhat*divisor from dividend.
//  */
// static int mulsub(uint32_t *q, uint32_t *a, uint32_t x, int len, int offset) {
//     uint64_t xLong = x;
//     uint64_t carry = 0;
//     offset += len;

//     for (int j = len - 1; j >= 0; j--) {
//         long product = (a[j] & LONG_MASK) * xLong + carry;
//         long difference = q[offset] - product;
//         q[offset--] = (int)difference;
//         carry = (product >>> 32) + (((difference & LONG_MASK) > (((~(int)product) & LONG_MASK))) ? 1 : 0);
//     }
//     return (int)carry;
// }

// /**
//  * Divide this MutableBigInteger by the divisor.
//  * The quotient will be placed into the provided quotient object &
//  * the remainder object is returned.
//  */
// BigInteger divideMagnitude(BigInteger dividend, BigInteger div, BigInteger quotient, int needRemainder) {
//     // assert div.intLen > 1
//     // D1 normalize the divisor
//     int dlen = div.magLen;
//     int shift = numberOfLeadingZeros(div.mag[dlen - 1]);
//     // Copy divisor value to protect divisor
//     uint32_t *divisorMag;

//     int remMagLen;
//     uint32_t *remMag;
//     BigInteger rem; // Remainder starts as dividend with space for a leading zero
//     if (shift > 0) {
//         int divisorMagLen;
//         divisorMag = shiftLeft(div.mag, dlen, shift, &divisorMagLen);
//         assert(divisorMagLen == dlen);
//         remMag = shiftLeft(dividend.mag, dividend.magLen, shift, &remMagLen);
//         if (numberOfLeadingZeros(dividend.mag[dividend.magLen - 1]) >= shift) {
//             assert(remMagLen == dividend.magLen);
//         } else {
//             assert(remMagLen == dividend.magLen + 1);
//         }
//         remMag = (uint32_t *)realloc(remMag, (remMagLen + 1) * sizeof(uint32_t));
//         remMag[remMagLen] = 0;
//     } else {
//         int i;
//         divisorMag = (uint32_t *)malloc(dlen * sizeof(uint32_t));
//         for (i = 0; i < dlen; i++) {
//             divisorMag[i] = div.mag[i];
//         }
//         remMagLen = dividend.magLen;
//         remMag = (uint32_t *)malloc((remMagLen + 1) * sizeof(uint32_t));
//         for (i = 0; i < remMagLen; i++) {
//             remMag[i] = dividend.mag[i];
//         }
//         remMag[remMagLen] = 0;
//     }

//     // Set the quotient size
//     int limit = remMagLen - dlen + 1;
//     quotient.mag = (uint32_t *)malloc(limit * sizeof(uint32_t));
//     quotient.magLen = limit;
//     uint32_t *q = quotient.mag;

//     remMagLen++;

//     uint32_t dh = divisorMag[dlen - 1];
//     uint64_t dhLong = (uint64_t)dh;
//     uint32_t dl = divisorMag[dlen - 2];

//     // D2 Initialize j
//     for (int j = 0; j < limit - 1; j++) {
//         // D3 Calculate qhat
//         // estimate qhat
//         uint32_t qhat = 0;
//         uint32_t qrem = 0;
//         int skipCorrection = 0;
//         uint32_t nh = remMag[remMagLen - j - 1];
//         uint32_t nh2 = nh + 0x80000000;
//         uint32_t nm = remMag[remMagLen - j - 2];

//         if (nh == dh) {
//             // tofo: need to confirm
//             qhat = 0xFFFFFFFF;
//             qrem = nh + nm;
//             skipCorrection = qrem + 0x80000000 < nh2;
//         } else {
//             uint64_t nChunk = (((uint64_t)nh) << 32) | (uint64_t)nm;
//             // tofo: need to confirm the case of nChunk < 0
//             qhat = (uint32_t)(nChunk / dhLong);
//             qrem = (uint32_t)(nChunk - qhat * dhLong);
//         }

//         if (qhat == 0)
//             continue;

//         if (!skipCorrection) { // Correct qhat
//             uint64_t nl = (uint64_t)remMag[remMagLen - j - 3];
//             uint64_t rs = (((uint64_t)qrem) << 32) | nl;
//             uint64_t estProduct = ((uint64_t)dl) * ((uint64_t)qhat);

//             if (estProduct > rs) {
//                 qhat--;
//                 qrem = (uint32_t)(qrem + dhLong);
//                 if (qrem >= dhLong) {
//                     estProduct -= dl;
//                     rs = (((uint64_t)qrem) << 32) | nl;
//                     if (estProduct > rs)
//                         qhat--;
//                 }
//             }
//         }

//         // D4 Multiply and subtract
//         remMag[remMagLen - j - 1] = 0;
//         int borrow = mulsub(remMag, divisorMag, qhat, dlen, j);

//         // D5 Test remainder
//         if (borrow + 0x80000000 > nh2) {
//             // D6 Add back
//             divadd(divisorMag, remMag, j + 1);
//             qhat--;
//         }

//         // Store the quotient digit
//         q[j] = qhat;
//     } // D7 loop on j
//     // D3 Calculate qhat
//     // estimate qhat
//     int qhat = 0;
//     int qrem = 0;
//     int skipCorrection = 0;
//     int nh = remMag[limit - 1 + rem.offset];
//     int nh2 = nh + 0x80000000;
//     int nm = remMag[limit + rem.offset];

//     if (nh == dh) {
//         qhat = ~0;
//         qrem = nh + nm;
//         skipCorrection = qrem + 0x80000000 < nh2;
//     } else {
//         long nChunk = (((long)nh) << 32) | (nm & LONG_MASK);
//         if (nChunk >= 0) {
//             qhat = (int)(nChunk / dhLong);
//             qrem = (int)(nChunk - (qhat * dhLong));
//         } else {
//             long tmp = divWord(nChunk, dh);
//             qhat = (int)(tmp & LONG_MASK);
//             qrem = (int)(tmp >>> 32);
//         }
//     }
//     if (qhat != 0) {
//         if (!skipCorrection) { // Correct qhat
//             long nl = rem.value[limit + 1 + rem.offset] & LONG_MASK;
//             long rs = ((qrem & LONG_MASK) << 32) | nl;
//             long estProduct = (dl & LONG_MASK) * (qhat & LONG_MASK);

//             if (unsignedLongCompare(estProduct, rs)) {
//                 qhat--;
//                 qrem = (int)((qrem & LONG_MASK) + dhLong);
//                 if ((qrem & LONG_MASK) >= dhLong) {
//                     estProduct -= (dl & LONG_MASK);
//                     rs = ((qrem & LONG_MASK) << 32) | nl;
//                     if (unsignedLongCompare(estProduct, rs))
//                         qhat--;
//                 }
//             }
//         }

//         // D4 Multiply and subtract
//         int borrow;
//         remMag[limit - 1 + rem.offset] = 0;
//         if (needRemainder)
//             borrow = mulsub(remMag, divisorMag, qhat, dlen, limit - 1 + rem.offset);
//         else
//             borrow = mulsubBorrow(remMag, divisorMag, qhat, dlen, limit - 1 + rem.offset);

//         // D5 Test remainder
//         if (borrow + 0x80000000 > nh2) {
//             // D6 Add back
//             if (needRemainder)
//                 divadd(divisorMag, remMag, limit - 1 + 1 + rem.offset);
//             qhat--;
//         }

//         // Store the quotient digit
//         q[(limit - 1)] = qhat;
//     }

//     if (needRemainder) {
//         // D8 Unnormalize
//         if (shift > 0)
//             rem.rightShift(shift);
//         rem.normalize();
//     }
//     quotient.normalize();
//     return needRemainder ? rem : null;
// }

BigInteger ZERO = {0, 0, NULL};

BigIntegerInterface iBigInteger = {
    .createFromInt = createFromInt,
    .toString = toString,
    .createFromHexString = createFromHexString,
    .toHexString = toHexString,
    .finalize = finalize,
    .add = add,
    .subtractByInt = subtractByInt,
    .subtract = subtract,
    .multiplyByInt = multiplyByInt,
    .multiply = multiply,
};
