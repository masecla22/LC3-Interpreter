#ifndef LC3_EMULATOR
#define LC3_EMULATOR

#include "../context/lc3context.h"

typedef union {
    short parsedNumber;
    unsigned short rawNumber;
} MemoryCell;

typedef struct LC3EmulatorState {
    short registers[8];
    unsigned short pc;
    unsigned short cc;
    MemoryCell *memory;
    unsigned short haltSignal;
} LC3EmulatorState;

void emulate(LC3Context ctx, LC3EmulatorState state);

void dumpToFile(LC3EmulatorState *state, FILE *output);
LC3EmulatorState loadFromFile(FILE *input);


#endif // LC3_EMULATOR