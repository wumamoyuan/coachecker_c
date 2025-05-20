#include "AABACBoundCalculator.h"
#include "AABACInstance.h"
#include "bn_java.h"

#include <unistd.h>

void computeBoundTightness(char *instFile, char *resultFile) {
    AABACInstance *pInst = readAABACInstance(instFile);
    if (pInst == NULL) {
        printf("Failed to read instance file!\n");
        return;
    }

    FILE *fp = NULL;
    if (resultFile != NULL) {
        if (access(resultFile, F_OK) == -1) {
            // 如果resultFile不存在，则创建它，并写入表头
            fp = fopen(resultFile, "w");
            fprintf(fp, "instFile,b1,b2,tightness\n");
        } else {
            fp = fopen(resultFile, "a");
        }
        if (fp == NULL) {
            printf("Failed to open result file!\n");
            return;
        }
    }

    init(pInst);

    // compute the trivial bound and the tightest bound
    BigInteger b1 = computeBound(pInst, 1);
    BigInteger b3 = computeBound(pInst, 3);

    //b1.
    double tightness = 0;//computeTightness(b1, b2);

    // write the result to the result file
    fprintf(fp, "%s,%s,%s,%.2f%%\n", instFile, b1, b3, tightness);
}
