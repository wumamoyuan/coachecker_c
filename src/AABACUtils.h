#ifndef AABACUTILS_H
#define AABACUTILS_H

#include "hashMap.h"

typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
} LogLevel;

char *strtrim(char *str);

unsigned int StringHashCode(void *pStr);

int StringEqual(void *pStr1, void *pStr2);

unsigned int IntHashCode(void *pInt);

int IntEqual(void *pInt1, void *pInt2);

char *IntToString(void *pInt);

char *StringToString(void *pStr);

void logAABAC(const char *func, int line, int logType, LogLevel logLevel, const char *format, ...);

char *mapToString(HashMap *map, char *(*keyToString)(void *key), char *(*valueToString)(void *value));

#endif // AABACUTILS_H