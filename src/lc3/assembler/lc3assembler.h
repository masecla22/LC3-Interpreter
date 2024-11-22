#ifndef LC3_ASSEMBLER_H
#define LC3_ASSEMBLER_H

#include "../instructions/lc3isa.h"
#include "../context/lc3context.h"

void assemble(LC3Context ctx);

unsigned short lc3AssembleInstruction(ParsedInstruction instruction);
ParsedInstruction lc3DisassembleInstruction(unsigned short instruction);


#endif // LC3_ASSEMBLER_H