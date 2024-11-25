#ifndef LC3_ASSEMBLER_H
#define LC3_ASSEMBLER_H

#include "../instructions/lc3isa.h"
#include "../context/lc3context.h"

unsigned short* assemble(LC3Context ctx);

#endif // LC3_ASSEMBLER_H