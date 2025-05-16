#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "AABACAbsRef.h"
#include "AABACBoundCalculator.h"
#include "AABACIO.h"
#include "AABACSlice.h"
#include "AABACTranslator.h"
#include "AABACUtils.h"
#include "NuSMVRunner.h"
#include "ccl/containers.h"
#include "hashMap.h"
#include "hashSet.h"
#include "precheck.h"

#define AABAC_SUFFIX ".aabac"
#define AABAC_SUFFIX_LEN 6
#define ARBAC_SUFFIX ".arbac"
#define ARBAC_SUFFIX_LEN 6
#define MOHAWK_SUFFIX ".mohawk"
#define MOHAWK_SUFFIX_LEN 7
#define SMV_SUFFIX ".smv"
#define SMV_SUFFIX_LEN 4

#define SLICING_RESULT_FILE_NAME "slicingResult"
#define SLICING_RESULT_FILE_NAME_LEN 13

#define ABSTRACTION_REFINEMENT_RESULT_FILE_NAME "abstractionRefinementResult"
#define ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN 27

#define NUSMV_FILE_NAME "lastSmvInstance"
#define NUSMV_FILE_NAME_LEN 15

#define RESULT_FILE_NAME "smvOutput"
#define RESULT_FILE_NAME_LEN 9

#define RESULT_SUFFIX ".txt"
#define RESULT_SUFFIX_LEN 4

static AABACResult verify(char *modelCheckerPath, char *instFilePath, int doPrechecking,
                          int doSlicing, int enableAbstractRefine, int useBMC, int bl, int showRules, long timeout) {
    AABACInstance *pInst = NULL;
    int instFilePathLen = strlen(instFilePath);
    if (instFilePathLen >= AABAC_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - AABAC_SUFFIX_LEN, AABAC_SUFFIX) == 0) {
        logAABAC(__func__, __LINE__, 0, INFO, "[start] parsing aabac instance file\n");
        pInst = readAABACInstance(instFilePath);
    } else if ((instFilePathLen >= ARBAC_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - ARBAC_SUFFIX_LEN, ARBAC_SUFFIX) == 0) ||
               (instFilePathLen >= MOHAWK_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - MOHAWK_SUFFIX_LEN, MOHAWK_SUFFIX) == 0)) {
        logAABAC(__func__, __LINE__, 0, ERROR, "unsupported file type: %s, %s\n", ARBAC_SUFFIX, MOHAWK_SUFFIX);
        return (AABACResult){.code = AABAC_RESULT_ERROR};
        // logAABAC(__func__, __LINE__, 0, INFO, "[start] translating arbac instance file\n");
        // readARBACInstance(instFilePath);
    } else {
        logAABAC(__func__, __LINE__, 0, ERROR, "illegal file type\n");
        return (AABACResult){.code = AABAC_RESULT_ERROR};
    }
    if (pInst == NULL) {
        return (AABACResult){.code = AABAC_RESULT_ERROR};
    }
    init(pInst);

    char *logDir = "/root/nn/coachecker_c/logs/";

    if (doPrechecking) {
        logAABAC(__func__, __LINE__, 0, INFO, "[start] pre-checking\n");
        clock_t startPreCheck = clock();
        AABACResult result = preCheck(pInst);
        clock_t endPreCheck = clock();
        double time_spent = (double)(endPreCheck - startPreCheck) / CLOCKS_PER_SEC * 1000;
        logAABAC(__func__, __LINE__, 0, INFO, "[end] pre-checking, cost ==> %.2fms\n", time_spent);
        if (result.code != AABAC_RESULT_UNKNOWN) {
            logAABAC(__func__, __LINE__, 0, INFO, "preCheck success\n");
            printResult(result, showRules);
            return result;
        }
        logAABAC(__func__, __LINE__, 0, INFO, "preCheck failed\n");
    }
    pInst = userCleaning(pInst);

    AABACResult result = {.code = AABAC_RESULT_UNKNOWN};
    if (doSlicing) {
        pInst = slice(pInst, &result);
        if (result.code != AABAC_RESULT_UNKNOWN) {
            printResult(result, showRules);
            return result;
        }

        char *writePath = (char *)malloc(strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN + AABAC_SUFFIX_LEN + 1);
        memcpy(writePath, logDir, strlen(logDir));
        memcpy(writePath + strlen(logDir), SLICING_RESULT_FILE_NAME, SLICING_RESULT_FILE_NAME_LEN);
        strcpy(writePath + strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN, AABAC_SUFFIX);
        writeAABACInstance(pInst, writePath);
        free(writePath);
    }

    AbsRef *pAbsRef;
    AABACInstance *next;
    char roundStr[10];
    char boundStr[15];
    char *nusmvFilePath, *resultFilePath, *nusmvOutput;

    // 开始abstraction refinement
    if (enableAbstractRefine) {
        pAbsRef = createAbsRef(pInst);
        next = abstract(pAbsRef);
    } else {
        next = pInst;
    }
    while (next != NULL) {
        sprintf(roundStr, "%d", pAbsRef->round);
        if (enableAbstractRefine) {
            char *writePath = (char *)malloc(strlen(logDir) + ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN + strlen(roundStr) + AABAC_SUFFIX_LEN + 1);
            memcpy(writePath, logDir, strlen(logDir));
            memcpy(writePath + strlen(logDir), ABSTRACTION_REFINEMENT_RESULT_FILE_NAME, ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN);
            memcpy(writePath + strlen(logDir) + ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN, roundStr, strlen(roundStr));
            strcpy(writePath + strlen(logDir) + ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN + strlen(roundStr), AABAC_SUFFIX);
            writeAABACInstance(next, writePath);
            free(writePath);

            if (doSlicing) {
                next = slice(next, &result);
                if (result.code == AABAC_RESULT_REACHABLE || (result.code == AABAC_RESULT_UNREACHABLE && !enableAbstractRefine)) {
                    printResult(result, showRules);
                    logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
                    return result;
                }
                if (result.code == AABAC_RESULT_UNREACHABLE) {
                    printResult(result, showRules);
                    next = refine(pAbsRef);
                    continue;
                }
                char *writePath = (char *)malloc(strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN + strlen(roundStr) + AABAC_SUFFIX_LEN + 1);
                memcpy(writePath, logDir, strlen(logDir));
                memcpy(writePath + strlen(logDir), SLICING_RESULT_FILE_NAME, SLICING_RESULT_FILE_NAME_LEN);
                memcpy(writePath + strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN, roundStr, strlen(roundStr));
                strcpy(writePath + strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN + strlen(roundStr), AABAC_SUFFIX);
                writeAABACInstance(next, writePath);
                free(writePath);
            }
        }

        // 将instance转化为NuSMV文件
        nusmvFilePath = (char *)malloc(strlen(logDir) + NUSMV_FILE_NAME_LEN + strlen(roundStr) + SMV_SUFFIX_LEN + 1);
        memcpy(nusmvFilePath, logDir, strlen(logDir));
        memcpy(nusmvFilePath + strlen(logDir), NUSMV_FILE_NAME, NUSMV_FILE_NAME_LEN);
        memcpy(nusmvFilePath + strlen(logDir) + NUSMV_FILE_NAME_LEN, roundStr, strlen(roundStr));
        strcpy(nusmvFilePath + strlen(logDir) + NUSMV_FILE_NAME_LEN + strlen(roundStr), SMV_SUFFIX);
        if (translate(next, nusmvFilePath, doSlicing) != 0) {
            logAABAC(__func__, __LINE__, 0, ERROR, "failed to translate instance to nusmv file\n");
            result.code = AABAC_RESULT_ERROR;
            printResult(result, showRules);
            return result;
        }

        // 调用model checker进行验证
        resultFilePath = (char *)malloc(strlen(logDir) + RESULT_FILE_NAME_LEN + strlen(roundStr) + RESULT_SUFFIX_LEN + 1);
        memcpy(resultFilePath, logDir, strlen(logDir));
        memcpy(resultFilePath + strlen(logDir), RESULT_FILE_NAME, RESULT_FILE_NAME_LEN);
        memcpy(resultFilePath + strlen(logDir) + RESULT_FILE_NAME_LEN, roundStr, strlen(roundStr));
        strcpy(resultFilePath + strlen(logDir) + RESULT_FILE_NAME_LEN + strlen(roundStr), RESULT_SUFFIX);

        int tooLarge = 0;
        if (!useBMC) {
            // 使用SMV进行验证
            nusmvOutput = runOnSMCMode(modelCheckerPath, nusmvFilePath, resultFilePath, timeout);
            result = analyzeModelCheckerOutput(nusmvOutput, next, NULL, showRules);
        } else {
            // 计算bound，然后使用BMC进行验证
            // 如果bound超过了int的范围，则使用INT_MAX作为bound
            BigInteger bound = computeBound(next, bl);
            printf("bound: %s\n", iBigInteger.toHexString(bound));
            if (bound.magLen > 1 || (bound.magLen == 1 && (bound.mag[0] >> 31) != 0)) {
                logAABAC(__func__, __LINE__, 0, WARNING, "bound is too large, use INT_MAX as bound\n");
                sprintf(boundStr, "%d", INT_MAX);
                tooLarge = 1;
            } else {
                sprintf(boundStr, "%d", bound.mag[0]);
            }
            iBigInteger.finalize(bound);
            nusmvOutput = runOnBMCMode(modelCheckerPath, nusmvFilePath, resultFilePath, timeout, boundStr);
            result = analyzeModelCheckerOutput(nusmvOutput, next, boundStr, showRules);
        }
        free(nusmvOutput);

        if (tooLarge && result.code == AABAC_RESULT_UNREACHABLE) {
            // todo: 如果界超过了int的范围，且模型检测结果为不可达，则应当以SMC模式重新检测
            nusmvOutput = runOnSMCMode(modelCheckerPath, nusmvFilePath, resultFilePath, timeout);
            result = analyzeModelCheckerOutput(nusmvOutput, next, NULL, showRules);
            free(nusmvOutput);
        }
        free(nusmvFilePath);
        free(resultFilePath);

        logAABAC(__func__, __LINE__, 0, INFO, "\n");
        printResult(result, showRules);
        
        if (!enableAbstractRefine || result.code != AABAC_RESULT_UNREACHABLE) {
            logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
            return result;
        }
        next = refine(pAbsRef);
    }
    logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
    return result;
}

// #include "bn_java.h"
// #include <math.h>

// char hexchars[] = "0123456789abcdef";

// char *randomHexString(int len) {
//     char *str = (char *)malloc(len + 1);
//     for (int i = 0; i < len; i++) {
//         str[i] = hexchars[rand() % 16];
//     }
//     str[len] = '\0';
//     return str;
// }

// int testBigInteger() {
//     BigInteger n1 = iBigInteger.createFromHexString(randomHexString(700));
//     BigInteger n2 = iBigInteger.createFromHexString(randomHexString(700));
//     BigInteger n3 = iBigInteger.createFromHexString(randomHexString(700));
//     printf("n1: %s\n", iBigInteger.toHexString(n1));
//     printf("n2: %s\n", iBigInteger.toHexString(n2));
//     printf("n3: %s\n", iBigInteger.toHexString(n3));
//     BigInteger tmp = iBigInteger.multiply(n1, n2);
//     printf("tmp: %s\n", iBigInteger.toHexString(tmp));
//     BigInteger result = iBigInteger.add(tmp, n2);
//     printf("result: %s\n", iBigInteger.toHexString(result));
//     tmp = iBigInteger.subtract(result, n3);
//     printf("tmp: %s\n", iBigInteger.toHexString(tmp));
//     result = iBigInteger.multiply(tmp, n3);
//     printf("result: %s\n", iBigInteger.toHexString(result));
//     return 0;
// }

int main(int argc, char *argv[]) {
    // testBigInteger();
    // return 0;
    int help = 0;
    int doPrechecking = 1;
    int doSlicing = 1;
    int enableAbstractRefine = 1;
    int useBMC = 1;
    int showRules = 1;

    int bl = 4;
    char *modelCheckerPath = NULL;
    char *inputFilePath = NULL;
    long timeout = 60;

    int unrecognized = 0;

    char *helpMessage = "Usage: aabac-verifier\
        \n-bl <arg>                   bound level, an integer in [1, 4]\
        \n-h                          print this help text\
        \n-input <arg>                aabac file path\
        \n-model_checker <arg>        nusmv file path\
        \n-no_absref                  no abstraction refinement\
        \n-no_precheck                no precheck\
        \n-no_slicing                 no slicing\
        \n-nuxmv <arg>                nuxmv file path\
        \n-no_rules                   do not show the rules associated with the actions in the result\
        \n-smc                        on smc mode\
        \n-timeout <arg>              timeout in seconds\n";

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"no_precheck", no_argument, 0, 'p'},
        {"no_slicing", no_argument, 0, 's'},
        {"no_absref", no_argument, 0, 'a'},
        {"smc", no_argument, 0, 'n'},
        {"bl", required_argument, 0, 'b'},
        {"no_rules", no_argument, 0, 'r'},
        {"model_checker", required_argument, 0, 'm'},
        {"input", required_argument, 0, 'i'},
        {"timeout", required_argument, 0, 't'},
        {0, 0, 0, 0}};

    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long_only(argc, argv, "hpsanb:rm:i:t:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 'h':
            help = 1;
            break;
        case 'p':
            doPrechecking = 0;
            break;
        case 's':
            doSlicing = 0;
            break;
        case 'a':
            enableAbstractRefine = 0;
            break;
        case 'n':
            useBMC = 0;
            break;
        case 'r':
            showRules = 0;
            break;
        case 'b':
            bl = atoi(optarg);
            if (bl < 1 || bl > 4) {
                printf("bound level must be in [1, 4]\n");
                return 0;
            }
            break;
        case 'm':
            modelCheckerPath = (char *)malloc(strlen(optarg) + 1);
            strcpy(modelCheckerPath, optarg);
            break;
        case 'i':
            inputFilePath = (char *)malloc(strlen(optarg) + 1);
            strcpy(inputFilePath, optarg);
            break;
        case 't':
            timeout = atol(optarg);
            break;
        default:
            unrecognized = 1;
            break;
        }
    }

    if (unrecognized) {
        printf("Found unrecognized option\n%s", helpMessage);
    } else if (help) {
        printf("%s", helpMessage);
    } else if (!inputFilePath) {
        printf("please input the file path of aabac instance\n%s", helpMessage);
    } else if (!modelCheckerPath) {
        printf("please input the file path of model checker\n%s", helpMessage);
    } else {
        clock_t start = clock();
        verify(modelCheckerPath, inputFilePath, doPrechecking, doSlicing, enableAbstractRefine, useBMC, bl, showRules, timeout);
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        logAABAC(__func__, __LINE__, 0, INFO, "end verification, cost => %.2fms\n", time_spent);
    }
    return 0;
}