#include "AABACIO.h"
#include "AABACSlice.h"
#include "AABACUtils.h"

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/**
 * 计算Global Pruning Rate, 如果resultFile不为NULL，则将结果与剪枝耗时以"instFile,oldRuleNums,newRuleNums,GPR,time"的格式保存到resultFile
 *
 * @param instFile 实例文件路径
 * @param resultFile 结果文件路径
 */
void computeGPR(char *instFile, char *resultFile) {
    AABACInstance *pInst = readAABACInstance(instFile);
    if (pInst == NULL) {
        printf("Failed to read instance file!\n");
        return;
    }

    init(pInst);
    clock_t startTime = clock();

    // 记录原始规则数
    int nRulesOld = iHashSet.Size(pInst->pSetRuleIdxes);

    // 进行剪枝，记录剪枝后的规则数
    int nRulesNew = 0;
    pInst = userCleaning(pInst);
    AABACResult result = {.code = AABAC_RESULT_UNKNOWN};
    pInst = slice(pInst, &result);
    if (result.code != AABAC_RESULT_REACHABLE && result.code != AABAC_RESULT_UNREACHABLE) {
        nRulesNew = iHashSet.Size(pInst->pSetRuleIdxes);
    }

    // 计算GPR的百分数
    double GPR = (nRulesOld - nRulesNew) * 100.0 / nRulesOld;
    logAABAC(__func__, __LINE__, 0, INFO, "oldRuleNums => %d, newRuleNums => %d, GPR => %.2f%%\n", nRulesOld, nRulesNew, GPR);

    // 将结果与剪枝耗时以"instFile, GPR, time"的格式保存到resultFile
    double time = (double)(clock() - startTime) / CLOCKS_PER_SEC * 1000;

    if (resultFile != NULL) {
        // 如果resultFile不存在，则创建它，并写入表头
        if (access(resultFile, F_OK) == -1) {
            FILE *fp = fopen(resultFile, "w");
            fprintf(fp, "instFile,oldRuleNums,newRuleNums,GPR,time\n");
            fprintf(fp, "%s,%d,%d,%.2f%%,%.2fms\n", instFile, nRulesOld, nRulesNew, GPR, time);
            fclose(fp);
        } else {
            FILE *fp = fopen(resultFile, "a");
            fprintf(fp, "%s,%d,%d,%.2f%%,%.2fms\n", instFile, nRulesOld, nRulesNew, GPR, time);
            fclose(fp);
        }
    }
}

void computeStatsAttrQueryVal(char *statisticsFilePath) {
    FILE *fp = fopen(statisticsFilePath, "r");
    if (fp == NULL) {
        printf("Statistics file not found!\n");
        return;
    }

    int nq = 5, nv = 10, ninst = 50;
    int oldRuleNums[nq][nv][ninst];
    int newRuleNums[nq][nv][ninst];

    // 用正则表达式匹配实例文件名
    char *pattern = "MC5-Q([0-9]+)-A60-R3000+/V([0-9]+)/test([0-9]+).aabac";
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED);
    regmatch_t pmatch[4];

    int q, v, id;
    char qStr[10], vStr[10], idStr[10];
    char *instFile, *oldRuleNumStr, *newRuleNumStr;

    char line[1024];
    // 跳过表头
    fgets(line, sizeof(line), fp);

    int lineNum = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineNum++;
        instFile = strtok(line, ",");
        oldRuleNumStr = strtok(NULL, ",");
        newRuleNumStr = strtok(NULL, ",");

        // 解析instFile
        if (regexec(&regex, instFile, 4, pmatch, 0) == 0) {
            memcpy(qStr, instFile + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
            qStr[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';
            memcpy(vStr, instFile + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
            vStr[pmatch[2].rm_eo - pmatch[2].rm_so] = '\0';
            memcpy(idStr, instFile + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
            idStr[pmatch[3].rm_eo - pmatch[3].rm_so] = '\0';
            q = atoi(qStr);
            v = atoi(vStr);
            id = atoi(idStr);

            if (q > 0 && q <= nq && v > 0 && v <= nv * 10 && id > 0 && id <= ninst) {
                oldRuleNums[q - 1][v / 10 - 1][id - 1] = atoi(oldRuleNumStr);
                newRuleNums[q - 1][v / 10 - 1][id - 1] = atoi(newRuleNumStr);
            } else {
                printf("Invalid instance file: %s, in line %d\n", instFile, lineNum);
                exit(1);
            }
        }
    }

    int oldRuleNum, newRuleNum;
    double sumPruningRate;
    double rateList[nv];
    for (q = 1; q <= nq; q++) {
        for (v = 1; v <= nv; v++) {
            // 计算平均剪枝率
            sumPruningRate = 0;
            for (int i = 1; i <= ninst; i++) {
                oldRuleNum = oldRuleNums[q - 1][v - 1][i - 1];
                newRuleNum = newRuleNums[q - 1][v - 1][i - 1];
                sumPruningRate += (oldRuleNum - newRuleNum) * 100.0 / oldRuleNum;
            }
            rateList[v - 1] = sumPruningRate / ninst;
        }

        // 将dlist中的内容以百分比形式打印出来，保留两位小数，每打印一个数字换一行，不用逗号隔开
        printf("MC5-Q%d-A60-R3000", q);
        for (int i = 0; i < nv; i++) {
            printf(",%.2f%%", rateList[i]);
        }
        printf("\n");
    }
}

void computeStatsMaxCondVal(char *statisticsFilePath) {
    FILE *fp = fopen(statisticsFilePath, "r");
    if (fp == NULL) {
        printf("Statistics file not found!\n");
        return;
    }

    int nmc = 4, nv = 10, ninst = 50;
    int oldRuleNums[nmc][nv][ninst];
    int newRuleNums[nmc][nv][ninst];

    // 用正则表达式匹配实例文件名
    char *pattern = "MC([0-9]+)-Q1-A90-R3000+/V([0-9]+)/test([0-9]+).aabac";
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED);
    regmatch_t pmatch[4];

    int mc, v, id;
    char mcStr[10], vStr[10], idStr[10];
    char *instFile, *oldRuleNumStr, *newRuleNumStr;

    char line[1024];
    // 跳过表头
    fgets(line, sizeof(line), fp);

    int lineNum = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineNum++;
        instFile = strtok(line, ",");
        oldRuleNumStr = strtok(NULL, ",");
        newRuleNumStr = strtok(NULL, ",");

        // 解析instFile
        if (regexec(&regex, instFile, 4, pmatch, 0) == 0) {
            memcpy(mcStr, instFile + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
            mcStr[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';
            memcpy(vStr, instFile + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
            vStr[pmatch[2].rm_eo - pmatch[2].rm_so] = '\0';
            memcpy(idStr, instFile + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
            idStr[pmatch[3].rm_eo - pmatch[3].rm_so] = '\0';
            mc = atoi(mcStr);
            v = atoi(vStr);
            id = atoi(idStr);

            if (mc > 2 && mc <= 2 + nmc && v > 0 && v <= nv * 10 && id > 0 && id <= ninst) {
                oldRuleNums[mc - 3][v / 10 - 1][id - 1] = atoi(oldRuleNumStr);
                newRuleNums[mc - 3][v / 10 - 1][id - 1] = atoi(newRuleNumStr);
            } else {
                printf("Invalid instance file: %s, in line %d\n", instFile, lineNum);
                exit(1);
            }
        }
    }

    int oldRuleNum, newRuleNum;
    double sumPruningRate;
    double rateList[nv];
    for (mc = 3; mc <= 2 + nmc; mc++) {
        for (v = 1; v <= nv; v++) {
            // 计算平均剪枝率
            sumPruningRate = 0;
            for (int i = 1; i <= ninst; i++) {
                oldRuleNum = oldRuleNums[mc - 3][v - 1][i - 1];
                newRuleNum = newRuleNums[mc - 3][v - 1][i - 1];
                sumPruningRate += (oldRuleNum - newRuleNum) * 100.0 / oldRuleNum;
            }
            rateList[v - 1] = sumPruningRate / ninst;
        }

        // 将dlist中的内容以百分比形式打印出来，保留两位小数，每打印一个数字换一行，不用逗号隔开
        printf("MC%d-Q1-A90-R3000", mc);
        for (int i = 0; i < nv; i++) {
            printf(",%.2f%%", rateList[i]);
        }
        printf("\n");
    }
}

void computeStatsAttrRule(char *statisticsFilePath) {
    FILE *fp = fopen(statisticsFilePath, "r");
    if (fp == NULL) {
        printf("Statistics file not found!\n");
        return;
    }

    int na = 4, nr = 10, ninst = 50;
    int oldRuleNums[na][nr][ninst];
    int newRuleNums[na][nr][ninst];

    // 用正则表达式匹配实例文件名
    char *pattern = "MC5-Q1-V20-A([0-9]+)/P([0-9]+)/test([0-9]+).aabac";
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED);
    regmatch_t pmatch[4];

    int a, r, id;
    char aStr[10], rStr[10], idStr[10];
    char *instFile, *oldRuleNumStr, *newRuleNumStr;

    char line[1024];
    // 跳过表头
    fgets(line, sizeof(line), fp);

    int lineNum = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineNum++;
        instFile = strtok(line, ",");
        oldRuleNumStr = strtok(NULL, ",");
        newRuleNumStr = strtok(NULL, ",");

        // 解析instFile
        if (regexec(&regex, instFile, 4, pmatch, 0) == 0) {
            memcpy(aStr, instFile + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
            aStr[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';
            memcpy(rStr, instFile + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
            rStr[pmatch[2].rm_eo - pmatch[2].rm_so] = '\0';
            memcpy(idStr, instFile + pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
            idStr[pmatch[3].rm_eo - pmatch[3].rm_so] = '\0';
            a = atoi(aStr);
            r = atoi(rStr);
            id = atoi(idStr);

            if (a > 30 && a <= 150 && r > 0 && r <= nr * 500 && id > 0 && id <= ninst) {
                oldRuleNums[a / 30 - 2][r / 500 - 1][id - 1] = atoi(oldRuleNumStr);
                newRuleNums[a / 30 - 2][r / 500 - 1][id - 1] = atoi(newRuleNumStr);
            } else {
                printf("Invalid instance file: %s, in line %d\n", instFile, lineNum);
                exit(1);
            }
        }
    }

    int oldRuleNum, newRuleNum;
    double sumPruningRate;
    double rateList[nr];
    for (a = 2; a <=  1 + na; a++) {
        for (r = 1; r <= nr; r++) {
            // 计算平均剪枝率
            sumPruningRate = 0;
            for (int i = 1; i <= ninst; i++) {
                oldRuleNum = oldRuleNums[a - 2][r - 1][i - 1];
                newRuleNum = newRuleNums[a - 2][r - 1][i - 1];
                sumPruningRate += (oldRuleNum - newRuleNum) * 100.0 / oldRuleNum;
            }
            rateList[r - 1] = sumPruningRate / ninst;
        }

        // 将dlist中的内容以百分比形式打印出来，保留两位小数，每打印一个数字换一行，不用逗号隔开
        printf("MC5-Q1-V20-A%d", a * 30);
        for (int i = 0; i < nr; i++) {
            printf(",%.2f%%", rateList[i]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    char *helpMessage = "Usage: %s <option> <argument>\n"
                        "Options:\n"
                        "  -m, --mode <gpr|stat-gpr-a-r|stat-gpr-mc-v|stat-gpr-aq-v>    Set the mode of the program\n"
                        "  -i, --input <arg>                                            Compute the global pruning rate of given instance\n"
                        "  -o, --output <arg>                                           Save the global pruning rate to given output file\n"
                        "  -s, --stat-file <arg>                                        Compute statistics of given output file\n";

    static struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"stat-file", required_argument, 0, 's'},
        {0, 0, 0, 0}};

    char *mode = NULL;
    char *statFilePath = NULL;
    char *instFilePath = NULL;
    char *outputFilePath = NULL;
    int unrecognized = 0;

    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long_only(argc, argv, "g:s:o:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'm':
            mode = (char *)malloc(strlen(optarg) + 1);
            strcpy(mode, optarg);
            break;
        case 'i':
            instFilePath = (char *)malloc(strlen(optarg) + 1);
            strcpy(instFilePath, optarg);
            break;
        case 'o':
            outputFilePath = (char *)malloc(strlen(optarg) + 1);
            strcpy(outputFilePath, optarg);
            break;
        case 's':
            statFilePath = (char *)malloc(strlen(optarg) + 1);
            strcpy(statFilePath, optarg);
            break;
        default:
            unrecognized = 1;
            break;
        }
    }

    if (unrecognized) {
        printf("Unrecognized option or mode is not set!\n");
        printf("%s\n", helpMessage);
        return 1;
    }
    if (mode == NULL) {
        printf("Mode is not specified!\n");
        printf("%s\n", helpMessage);
        return 1;
    }
    if (strcmp(mode, "gpr") == 0) {
        if (instFilePath == NULL) {
            printf("Instance file is not specified!\n");
            printf("%s\n", helpMessage);
            return 1;
        }
        computeGPR(instFilePath, outputFilePath);
        return 0;
    }
    if (strcmp(mode, "stat-gpr-aq-v") == 0) {
        if (statFilePath == NULL) {
            printf("Statistics file is not specified!\n");
            printf("%s\n", helpMessage);
            return 1;
        }
        computeStatsAttrQueryVal(statFilePath);
        return 0;
    }
    if (strcmp(mode, "stat-gpr-mc-v") == 0) {
        if (statFilePath == NULL) {
            printf("Statistics file is not specified!\n");
            printf("%s\n", helpMessage);
            return 1;
        }
        computeStatsMaxCondVal(statFilePath);
        return 0;
    }
    if (strcmp(mode, "stat-gpr-a-r") == 0) {
        if (statFilePath == NULL) {
            printf("Statistics file is not specified!\n");
            printf("%s\n", helpMessage);
            return 1;
        }
        computeStatsAttrRule(statFilePath);
        return 0;
    }

    printf("unrecogized mode: %s", mode);
    printf("%s\n", helpMessage);
    return 1;
}