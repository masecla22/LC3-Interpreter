#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

void initLexer(FILE* f);
void finalizeLexer();

int yyparse(void);

#endif