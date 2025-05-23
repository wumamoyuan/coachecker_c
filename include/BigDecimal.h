#include "BigInteger.h"

typedef struct BigDecimal {
    BigInteger intVal;
    long intCompact;
    // 表示数字intval与intCompact被放大了10的scale次方倍
    int scale;
    // 表示数字的有效位数
    int precision;
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