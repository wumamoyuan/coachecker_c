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

static AABACResult verify(char *modelCheckerPath, char *instFilePath, char *logDir, int doPrechecking,
                          int doSlicing, int enableAbstractRefine, int useBMC, int tl, int showRules, long timeout) {
    AABACInstance *pInst = NULL;

    // read the instance file
    int instFilePathLen = strlen(instFilePath);
    if (instFilePathLen >= AABAC_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - AABAC_SUFFIX_LEN, AABAC_SUFFIX) == 0) {
        logAABAC(__func__, __LINE__, 0, INFO, "[start] parsing aabac instance file\n");
        pInst = readAABACInstance(instFilePath);
    } else if ((instFilePathLen >= ARBAC_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - ARBAC_SUFFIX_LEN, ARBAC_SUFFIX) == 0) ||
               (instFilePathLen >= MOHAWK_SUFFIX_LEN && strcmp(instFilePath + instFilePathLen - MOHAWK_SUFFIX_LEN, MOHAWK_SUFFIX) == 0)) {
        logAABAC(__func__, __LINE__, 0, INFO, "[start] translating arbac instance file\n");
        pInst = readARBACInstance(instFilePath);
    } else {
        logAABAC(__func__, __LINE__, 0, ERROR, "illegal file type\n");
        return (AABACResult){.code = AABAC_RESULT_ERROR};
    }
    if (pInst == NULL) {
        return (AABACResult){.code = AABAC_RESULT_ERROR};
    }

    // initialize the instance
    init(pInst);

    // pre-checking
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
    char *writePath;

    AABACResult result = {.code = AABAC_RESULT_UNKNOWN};
    if (doSlicing) {
        // Global pruning
        pInst = slice(pInst, &result);
        if (result.code != AABAC_RESULT_UNKNOWN) {
            printResult(result, showRules);
            return result;
        }

        writePath = (char *)malloc(strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN + AABAC_SUFFIX_LEN + 2);
        sprintf(writePath, "%s/%s%s", logDir, SLICING_RESULT_FILE_NAME, AABAC_SUFFIX);
        writeAABACInstance(pInst, writePath);
        free(writePath);
    }

    AbsRef *pAbsRef;
    AABACInstance *next;
    char roundStr[10];
    char boundStr[15];
    char *nusmvFilePath, *resultFilePath, *nusmvOutput;

    if (enableAbstractRefine) {
        // Generate an abstract sub-policy
        pAbsRef = createAbsRef(pInst);
        next = abstract(pAbsRef);
    } else {
        // no abstraction refinement
        next = pInst;
    }

    // Start the loop of abstraction refinement
    while (next != NULL) {
        sprintf(roundStr, "%d", enableAbstractRefine ? pAbsRef->round : 0);

        if (enableAbstractRefine) {
            // Save the abstract sub-policy in the log directory
            writePath = (char *)malloc(strlen(logDir) + ABSTRACTION_REFINEMENT_RESULT_FILE_NAME_LEN + strlen(roundStr) + AABAC_SUFFIX_LEN + 2);
            sprintf(writePath, "%s/%s%s%s", logDir, ABSTRACTION_REFINEMENT_RESULT_FILE_NAME, roundStr, AABAC_SUFFIX);
            writeAABACInstance(next, writePath);
            free(writePath);

            if (doSlicing) {
                // Local pruning
                next = slice(next, &result);
                if (result.code == AABAC_RESULT_REACHABLE || (result.code == AABAC_RESULT_UNREACHABLE && !enableAbstractRefine)) {
                    // Abstraction refinement is disabled and the safety of the sub-policy is determined, output the result
                    // Abstraction refinement is enabled and the sub-policy is determined to be "unsafe", also output the result
                    printResult(result, showRules);
                    logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
                    return result;
                }
                if (result.code == AABAC_RESULT_UNREACHABLE) {
                    // Abstraction refinement is enabled and the sub-policy is determined to be "safe", need refinement and re-verification
                    printResult(result, showRules);
                    next = refine(pAbsRef);
                    continue;
                }

                // Save the pruned sub-policy in the log directory
                writePath = (char *)malloc(strlen(logDir) + SLICING_RESULT_FILE_NAME_LEN + strlen(roundStr) + AABAC_SUFFIX_LEN + 2);
                sprintf(writePath, "%s/%s%s%s", logDir, SLICING_RESULT_FILE_NAME, roundStr, AABAC_SUFFIX);
                writeAABACInstance(next, writePath);
                free(writePath);
            }
        }

        int tooLarge = 0;
        if (useBMC) {
            // Bound estimation, if the bound exceeds the range of int, use INT_MAX as the bound
            BigInteger bound = computeBound(next, tl);
            if (bound.magLen > 1 || (bound.magLen == 1 && (bound.mag[0] >> 31) != 0)) {
                logAABAC(__func__, __LINE__, 0, WARNING, "bound is too large, use INT_MAX as bound\n");
                sprintf(boundStr, "%d", INT_MAX);
                tooLarge = 1;
            } else {
                sprintf(boundStr, "%d", bound.mag[0]);
            }
            iBigInteger.finalize(bound);
        }

        // Translate the instance to a NuSMV file
        nusmvFilePath = (char *)malloc(strlen(logDir) + NUSMV_FILE_NAME_LEN + strlen(roundStr) + SMV_SUFFIX_LEN + 2);
        sprintf(nusmvFilePath, "%s/%s%s%s", logDir, NUSMV_FILE_NAME, roundStr, SMV_SUFFIX);
        if (translate(next, nusmvFilePath, doSlicing) != 0) {
            logAABAC(__func__, __LINE__, 0, ERROR, "failed to translate instance to nusmv file\n");
            result.code = AABAC_RESULT_ERROR;
            printResult(result, showRules);
            return result;
        }

        // Call the model checker to verify the instance and save the result in the log directory
        resultFilePath = (char *)malloc(strlen(logDir) + RESULT_FILE_NAME_LEN + strlen(roundStr) + RESULT_SUFFIX_LEN + 2);
        sprintf(resultFilePath, "%s/%s%s%s", logDir, RESULT_FILE_NAME, roundStr, RESULT_SUFFIX);
        nusmvOutput = runModelChecker(modelCheckerPath, nusmvFilePath, resultFilePath, timeout, useBMC ? boundStr : NULL);

        // Analyze the result of the model checker
        result = analyzeModelCheckerOutput(nusmvOutput, next, useBMC ? boundStr : NULL, showRules);
        free(nusmvOutput);

        if (tooLarge && result.code == AABAC_RESULT_UNREACHABLE) {
            // The bound exceeds the range of int and the model checker result is "unreachable", need re-verification in SMC mode
            nusmvOutput = runModelChecker(modelCheckerPath, nusmvFilePath, resultFilePath, timeout, NULL);
            result = analyzeModelCheckerOutput(nusmvOutput, next, NULL, showRules);
            free(nusmvOutput);
        }
        free(nusmvFilePath);
        free(resultFilePath);

        logAABAC(__func__, __LINE__, 0, INFO, "\n");
        printResult(result, showRules);

        // If abstraction refinement is disabled or the sub-policy is not determined to be "unsafe", output the result
        if (!enableAbstractRefine || result.code != AABAC_RESULT_UNREACHABLE) {
            logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
            return result;
        }

        // Abstraction refinement is enabled and the sub-policy is determined to be "unsafe", need refinement and re-verification
        next = refine(pAbsRef);
    }
    logAABAC(__func__, __LINE__, 0, INFO, "round => %s\n", roundStr);
    return result;
}

int main(int argc, char *argv[]) {
    // testBigInteger();
    // return 0;
    int help = 0;
    int doPrechecking = 1;
    int doSlicing = 1;
    int enableAbstractRefine = 1;
    int useBMC = 1;
    int showRules = 1;

    int tl = 2;
    char *modelCheckerPath = NULL;
    char *inputFilePath = NULL;
    char *logDir = NULL;
    long timeout = 60;

    int unrecognized = 0;

    char *helpMessage = "Usage: aabac-verifier\
        \n-tl <arg>                   tight level, either 1 (loose) or 2 (tight)\
        \n-h                          print this help text\
        \n-input <arg>                acoac file path\
        \n-model_checker <arg>        nusmv file path\
        \n-log_dir <arg>              directory for storing logs\
        \n-no_absref                  no abstraction refinement\
        \n-no_precheck                no precheck\
        \n-no_slicing                 no slicing\
        \n-no_rules                   do not show the rules associated with the actions in the result\
        \n-smc                        on smc mode\
        \n-timeout <arg>              timeout in seconds\n";

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"no_precheck", no_argument, 0, 'p'},
        {"no_slicing", no_argument, 0, 's'},
        {"no_absref", no_argument, 0, 'a'},
        {"smc", no_argument, 0, 'n'},
        {"tl", required_argument, 0, 'b'},
        {"no_rules", no_argument, 0, 'r'},
        {"model_checker", required_argument, 0, 'm'},
        {"input", required_argument, 0, 'i'},
        {"log_dir", required_argument, 0, 'l'},
        {"timeout", required_argument, 0, 't'},
        {0, 0, 0, 0}};

    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long_only(argc, argv, "hpsanb:rm:i:l:t:", long_options, &option_index);

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
            tl = atoi(optarg);
            if (tl != 1 && tl != 2) {
                printf("tight level should be either 1 (loose) or 2 (tight)\n");
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
        case 'l':
            logDir = (char *)malloc(strlen(optarg) + 1);
            strcpy(logDir, optarg);
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
        printf("please input the file path of acoac instance\n%s", helpMessage);
    } else if (!modelCheckerPath) {
        printf("please input the file path of model checker\n%s", helpMessage);
    } else if (!logDir) {
        printf("please input the directory for storing logs\n%s", helpMessage);
    } else if (timeout <= 0) {
        printf("timeout must be greater than 0\n%s", helpMessage);
    } else {
        clock_t start = clock();
        verify(modelCheckerPath, inputFilePath, logDir, doPrechecking, doSlicing, enableAbstractRefine, useBMC, tl, showRules, timeout);
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        logAABAC(__func__, __LINE__, 0, INFO, "end verification, cost => %.2fms\n", time_spent);
    }
    return 0;
}