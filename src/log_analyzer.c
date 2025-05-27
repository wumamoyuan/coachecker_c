#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "AABACUtils.h"

static void convert(char *totalCost, char *result) {
    totalCost[strlen(totalCost) - 1] = '\0';
    char *minute = strtrim(strtok(totalCost, "m"));
    char *second = strtrim(strtok(NULL, "m"));
    double sec = atoi(minute) * 60 + atof(second);
    sprintf(result, "%.3f", sec);
}

static int analyzeLogFile(char *logFile, int tool, char *timecost) {
    FILE *fp = fopen(logFile, "r");
    if (fp == NULL) {
        printf("Failed to open file %s!\n", logFile);
        return 1;
    }

    int readResult = 0;
    char result[20] = {'\0'};

    // 动态分配初始缓冲区
    size_t buffer_size = 1024;
    char *line = (char *)malloc(buffer_size);
    if (line == NULL) {
        printf("Failed to allocate memory for line buffer\n");
        fclose(fp);
        return 1;
    }
    while (1) {
        // 尝试读取一行
        if (fgets(line, buffer_size, fp) == NULL) {
            // 文件结束或读取错误
            break;
        }

        // 检查是否需要扩展缓冲区
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] != '\n' && !feof(fp)) {
            // 行被截断，需要扩展缓冲区
            size_t new_size = buffer_size * 2;
            char *new_line = (char *)realloc(line, new_size);
            if (new_line == NULL) {
                printf("Failed to reallocate memory for line buffer\n");
                free(line);
                fclose(fp);
                return 1;
            }
            line = new_line;
            buffer_size = new_size;

            // 继续读取同一行的剩余部分
            while (fgets(line + line_len, buffer_size - line_len, fp) != NULL) {
                line_len = strlen(line);
                if (line_len > 0 && line[line_len - 1] == '\n') {
                    break; // 行已完整读取
                }
                if (feof(fp)) {
                    break; // 文件结束
                }
                // 再次扩展缓冲区
                new_size = buffer_size * 2;
                new_line = (char *)realloc(line, new_size);
                if (new_line == NULL) {
                    printf("Failed to reallocate memory for line buffer\n");
                    free(line);
                    fclose(fp);
                    return 1;
                }
                line = new_line;
                buffer_size = new_size;
            }
        }

        if (strncmp(line, "real", 4) == 0) {
            // printf("%s", line);
            char *timeLine = line + 4;
            timeLine = strtrim(timeLine);
            convert(timeLine, timecost);
        } else if (tool == 0) {
            if (strstr(line, "***RESULT***") != NULL) {
                readResult = 1;
            } else if (readResult) {
                strcpy(result, strtrim(line));
                readResult = 0;
            } else if (strncmp(line, "Exception in", 12) == 0 && strstr(line, "process hasn't exited") != NULL) {
                strcpy(result, "timeout");
            }
        } else if (tool == 1) {
            if (strstr(line, "[COMPLETED] Result:") != NULL || readResult) {
                readResult = 0;
                if (strstr(line, "ERROR_OCCURRED") != NULL) {
                    strcpy(result, "error");
                } else if (strstr(line, "TIMEOUT") != NULL) {
                    strcpy(result, "timeout");
                } else if (strstr(line, "GOAL_REACHABLE") != NULL) {
                    strcpy(result, "reachable");
                } else if (strstr(line, "GOAL_UNREACHABLE") != NULL) {
                    strcpy(result, "unreachable");
                } else {
                    readResult = 1;
                }
            }
        } else if (tool == 2) {
            if (strstr(line, "The ARBAC policy is safe") != NULL) {
                strcpy(result, "unreachable");
            } else if (strstr(line, "The ARBAC policy may not be safe") != NULL) {
                strcpy(result, "reachable");
            } else if (strstr(line, "There is something wrong with the analyzed file") != NULL) {
                strcpy(result, "error");
            }
        } else {
            printf("Unknown tool id: %d\n", tool);
            printf("tool, tool_id\n");
            printf("coachecker, 0\n");
            printf("mohawk, 1\n");
            printf("vac, 2\n");
            return 1;
        }
    }
    if (line != NULL) {
        free(line);
    }
    fclose(fp);

    if (strcmp(result, "reachable") != 0 && strcmp(result, "unreachable") != 0) {
        strcpy(timecost, result);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *helpMessage = "Usage: log_analyzer\
        \n[-l|--logdir] <arg>   t       he directory saving log files\
        \n[-c|--coachecker]             analyze coachecker log files\
        \n[-m|--mohawk]                 analyze mohawk log files\
        \n[-v|--vac]                    analyze vac log files\
        \n[-a|--ablation]               analyze ablation results\
        \n[-o|--output-csvfile] <arg>   the path of the output csv file\n";

    char *logDir = NULL;
    int coachecker = 0;
    int mohawk = 0;
    int vac = 0;
    int ablation = 0;
    char *outputCsvFile = NULL;

    int unrecognized = 0;

    static struct option long_options[] = {
        {"logdir", required_argument, 0, 'l'},
        {"coachecker", no_argument, 0, 'c'},
        {"mohawk", no_argument, 0, 'm'},
        {"vac", no_argument, 0, 'v'},
        {"ablation", no_argument, 0, 'a'},
        {"output-csvfile", required_argument, 0, 'o'},
        {0, 0, 0, 0}};

    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "l:cmvao:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'l':
            logDir = (char *)malloc(strlen(optarg) + 1);
            strcpy(logDir, optarg);
            break;
        case 'c':
            coachecker = 1;
            break;
        case 'm':
            mohawk = 1;
            break;
        case 'v':
            vac = 1;
            break;
        case 'a':
            ablation = 1;
            break;
        case 'o':
            outputCsvFile = (char *)malloc(strlen(optarg) + 1);
            strcpy(outputCsvFile, optarg);
            break;
        default:
            unrecognized = 1;
            printf("Unrecognized option: %s\n", argv[optind]);
            break;
        }
    }

    if (unrecognized) {
        printf("%s\n", helpMessage);
        return 1;
    }

    if (logDir == NULL) {
        printf("Error: log directory is required\n");
        printf("%s\n", helpMessage);
        return 1;
    }
    printf("log directory: %s\n", logDir);
    char *logFile = (char *)malloc(strlen(logDir) + 50);

    FILE *fpOutputCsv = NULL;
    if (outputCsvFile != NULL) {
        fpOutputCsv = fopen(outputCsvFile, "w");
        if (fpOutputCsv == NULL) {
            printf("Error: failed to open output csv file %s\n", outputCsvFile);
            return 1;
        }
    }

    char line[1024];
    // print the header
    strcpy(line, "index");
    if (coachecker || ablation) {
        strcat(line, ",coachecker");
    }
    if (ablation) {
        strcat(line, ",noslicing,noabsref,smc");
    }
    if (mohawk) {
        strcat(line, ",mohawk");
    }
    if (vac) {
        strcat(line, ",vac");
    }
    printf("%s\n", line);
    if (fpOutputCsv != NULL) {
        fprintf(fpOutputCsv, "%s\n", line);
    }

    char *ablationSuffixes[] = {"noslicing", "noabsref", "smc"};
    int ablationSuffixNum = 3;

    int ret;
    int instanceNum = 50;
    char timecost[50];
    for (int i = 1; i <= instanceNum; i++) {
        sprintf(line, "%d", i);

        if (coachecker || ablation) {
            sprintf(logFile, "%s/output%d-all.txt", logDir, i);
            ret = analyzeLogFile(logFile, 0, timecost);
            if (ret) {
                printf("Failed to analyze file %s!\n", logFile);
                if (fpOutputCsv != NULL) {
                    fclose(fpOutputCsv);
                }
                return 1;
            }
            strcat(line, ",");
            strcat(line, timecost);
        }
        if (ablation) {
            for (int j = 0; j < ablationSuffixNum; j++) {
                sprintf(logFile, "%s/output%d-%s.txt", logDir, i, ablationSuffixes[j]);
                ret = analyzeLogFile(logFile, 0, timecost);
                if (ret) {
                    printf("Failed to analyze file %s!\n", logFile);
                    if (fpOutputCsv != NULL) {
                        fclose(fpOutputCsv);
                    }
                    return 1;
                }
                strcat(line, ",");
                strcat(line, timecost);
            }
        }
        if (mohawk) {
            sprintf(logFile, "%s/output%d-mohawk.txt", logDir, i);
            ret = analyzeLogFile(logFile, 1, timecost);
            if (ret) {
                printf("Failed to analyze file %s!\n", logFile);
                if (fpOutputCsv != NULL) {
                    fclose(fpOutputCsv);
                }
                return 1;
            }
            strcat(line, ",");
            strcat(line, timecost);
        }
        if (vac) {
            sprintf(logFile, "%s/output%d-vac.txt", logDir, i);
            ret = analyzeLogFile(logFile, 2, timecost);
            if (ret) {
                printf("Failed to analyze file %s!\n", logFile);
                if (fpOutputCsv != NULL) {
                    fclose(fpOutputCsv);
                }
                return 1;
            }
            strcat(line, ",");
            strcat(line, timecost);
        }
        printf("%s\n", line);
        if (fpOutputCsv != NULL) {
            fprintf(fpOutputCsv, "%s\n", line);
        }
    }

    if (fpOutputCsv != NULL) {
        fclose(fpOutputCsv);
    }

    return 0;
}