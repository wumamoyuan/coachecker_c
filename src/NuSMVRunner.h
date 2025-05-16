#ifndef NUSMV_RUNNER_H
#define NUSMV_RUNNER_H

#include "AABACResult.h"

char *runOnSMCMode(char *modelCheckerPath, char *nusmvFilePath, char *resultFilePath, long timeout);
char *runOnBMCMode(char *modelCheckerPath, char *nusmvFilePath, char *resultFilePath, long timeout, char *bound);
AABACResult analyzeModelCheckerOutput(char *output, AABACInstance *pInst, char *boundStr, int showRules);

#endif // NUSMV_RUNNER_H
