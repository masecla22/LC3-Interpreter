#include "lc3assembler.h"

#include "../../lexer/lexer.h"

void assemble(LC3Context ctx) {
    // Setup the lexer with the input file.
    initLexer(ctx.inputFile);

    // Parse the input file
    yyparse();

    // Free the lexer resources.
    finalizeLexer();
}