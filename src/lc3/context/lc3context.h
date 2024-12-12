#ifndef LC3_CONTEXT_H
#define LC3_CONTEXT_H

#include <stdio.h>

typedef struct LC3Context {
    FILE* inputFile;
    FILE* outputFile;

    int randomized;
    int seed;
    int maxCycleCount;
} LC3Context;

#endif // LC3_CONTEXT_H