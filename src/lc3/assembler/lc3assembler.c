#include "lc3assembler.h"

#include "../../lexer/lexer.h"
#include "../instructions/lc3isa.h"

#include <stdlib.h>

extern LabelledInstructionList *parsedInstructions;

void freeParsedInstructions() {
    if (parsedInstructions == NULL) {
        return;
    }

    destroyLabelledInstructionList(parsedInstructions);
    parsedInstructions = NULL;
}

void assemble(LC3Context ctx) {
    // Register parser cleanup
    atexit(freeParsedInstructions);

    // Setup the lexer with the input file.
    initLexer(ctx.inputFile);

    // Parse the input file
    yyparse();

    // Free the lexer resources.
    finalizeLexer();    

    // Free the parsed instructions
    freeParsedInstructions();
}