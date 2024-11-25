#include "lc3assembler.h"

#include <stdlib.h>

#include "../../lexer/lexer.h"
#include "../instructions/lc3isa.h"

extern LabelledInstructionList* labelledInstructions;

void freeLabelledInstructions() {
    if (labelledInstructions == NULL) {
        return;
    }

    destroyLabelledInstructionList(labelledInstructions);
    labelledInstructions = NULL;
}

void assemble(LC3Context ctx) {
    // Register parser cleanup
    atexit(freeLabelledInstructions);

    // Setup the lexer with the input file.
    initLexer(ctx.inputFile);

    // Parse the input file
    yyparse();

    // Free the lexer resources.
    finalizeLexer();

    // Create the memory layout


    // Free the parsed instructions
    freeLabelledInstructions();
}

void ensureMemoryLayoutCanBeMade(){
    if(labelledInstructions->count == 0){
        fprintf(stderr, "No instructions found in the input file.\n");
        exit(1);
    }

    LabelledInstruction firstInstruction = labelledInstructions->instructions[0];

    if(firstInstruction.instruction.type != D_ORIG){
        fprintf(stderr, "First instruction must be an .ORIG directive.\n");
        exit(1);
    }

    int addr = firstInstruction.instruction.dOrig.address;
    
    if(addr < 0x3000){
        fprintf(stderr, "Origin address must be at least 0x3000.\n");
        exit(1);
    }

    if(addr > 0xFFFF){
        fprintf(stderr, "Origin address must be at most 0xFFFF.\n");
        exit(1);
    }
}

short* createMemoryLayout(LC3Context ctx) {
    ensureMemoryLayoutCanBeMade();

    short* memory = (short*)calloc(65536, sizeof(short));
    if (memory == NULL) {
        return NULL;
    }

    // Fill up memory with junk values if randomized
    if (ctx.randomized) {
        int seed = ctx.seed;
        srand(seed);
        for (int i = 0; i < 65536; i++) {
            memory[i] = rand() % 65536;
        }
    }

    return memory;
}

