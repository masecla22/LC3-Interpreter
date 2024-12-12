#ifndef EXPECTER_H
#define EXPECTER_H

#include <stdio.h>

typedef struct EmulatorInput {
    short replaceRegisters[8];
    short replaceMemory[65536];

    short registerReplacements[8];
    short memoryReplacements[65536];
} EmulatorInput;

typedef struct EmulatorOutput {
    short expectedRegisters[8]; // 1 if we expect the register to be replaced, 0 otherwise
    short expectedMemory[65536]; // 1 if we expect the memory to be replaced, 0 otherwise
} EmulatorOutput;

typedef struct EmulatorExpectations {
    EmulatorInput input;
    EmulatorOutput output;
} EmulatorExpectations;

EmulatorExpectations loadExpectationFromFile(FILE* expectationsFile);

#endif // EXPECTER_H