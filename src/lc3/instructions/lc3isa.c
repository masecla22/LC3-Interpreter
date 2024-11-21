#include "lc3isa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LabelledInstructionList* createLabelledInstructionList(void) {
    LabelledInstructionList* list = calloc(1, sizeof(LabelledInstructionList));
    list->instructions = calloc(16, sizeof(LabelledInstruction));
    list->count = 0;
    list->capacity = 16;

    return list;
}

void destroyLabelledInstruction(LabelledInstruction instruction) {
    if (instruction.label != NULL) {
        free(instruction.label);
    }

    switch (instruction.instruction.type) {
        case I_BR:
            if (!instruction.instruction.iBr.isResolved) {
                free(instruction.instruction.iBr.label);
            }
            break;
        case I_JSR:
            if (!instruction.instruction.iJsr.isResolved) {
                free(instruction.instruction.iJsr.label);
            }
            break;
        case I_LD:
            if (!instruction.instruction.iLd.isResolved) {
                free(instruction.instruction.iLd.label);
            }
            break;
        case I_LDI:
            if (!instruction.instruction.iLdi.isResolved) {
                free(instruction.instruction.iLdi.label);
            }
            break;
        case I_LEA:
            if (!instruction.instruction.iLea.isResolved) {
                free(instruction.instruction.iLea.label);
            }
            break;
        case I_ST:
            if (!instruction.instruction.iSt.isResolved) {
                free(instruction.instruction.iSt.label);
            }
            break;
        case I_STI:
            if (!instruction.instruction.iSti.isResolved) {
                free(instruction.instruction.iSti.label);
            }
            break;
        case D_STRINGZ:
            free(instruction.instruction.dStringz.string);
            break;

        default:
            break;
    }
}

void destroyLabelledInstructionList(LabelledInstructionList* list) {
    for (unsigned int i = 0; i < list->count; i++) {
        destroyLabelledInstruction(list->instructions[i]);
    }

    free(list->instructions);
    free(list);
}

void addLabelledInstruction(LabelledInstructionList* list, LabelledInstruction instruction) {
    printLabelledInstruction(instruction);

    if (list->count == list->capacity - 2) {
        list->capacity *= 2;
        list->instructions = realloc(list->instructions, sizeof(LabelledInstruction) * list->capacity);

        int oldCapacity = list->capacity / 2;
        for (unsigned int i = oldCapacity; i < list->capacity; i++) {
            list->instructions[i] = (LabelledInstruction){0};
        }
    }

    list->instructions[list->count++] = instruction;
}

void printInstruction(UnresolvedInstruction instruction) {
    switch (instruction.type) {
        case I_ADD:
            printf("ADD R%d, R%d, ", instruction.iAdd.destinationRegister, instruction.iAdd.sr1);
            if (instruction.iAdd.isImm) {
                printf("#%d\n", instruction.iAdd.imm5);
            } else {
                printf("R%d\n", instruction.iAdd.sr2);
            }
            break;

        case I_AND:
            printf("AND R%d, R%d, ", instruction.iAnd.destinationRegister, instruction.iAnd.sr1);
            if (instruction.iAnd.isImm) {
                printf("#%d\n", instruction.iAnd.imm5);
            } else {
                printf("R%d\n", instruction.iAnd.sr2);
            }
            break;

        case I_BR:
            printf("BR");
            if (instruction.iBr.nzp & 4) printf("n");
            if (instruction.iBr.nzp & 2) printf("z");
            if (instruction.iBr.nzp & 1) printf("p");
            if (instruction.iBr.isResolved) {
                printf(" #%d\n", instruction.iBr.pcOffset9);
            } else {
                printf(" %s\n", instruction.iBr.label);
            }
            break;

        case I_JMP:
            printf("JMP R%d\n", instruction.iJmp.sourceRegister);
            break;

        case I_JSR:
            if (instruction.iJsr.isResolved) {
                printf("JSR #%d\n", instruction.iJsr.pcOffset11);
            } else {
                printf("JSR %s\n", instruction.iJsr.label);
            }
            break;

        case I_JSRR:
            printf("JSRR R%d\n", instruction.iJsrr.baseRegister);
            break;

        case I_LD:
            if (instruction.iLd.isResolved) {
                printf("LD R%d, #%d\n", instruction.iLd.destinationRegister, instruction.iLd.pcOffset9);
            } else {
                printf("LD R%d, %s\n", instruction.iLd.destinationRegister, instruction.iLd.label);
            }
            break;

        case I_LDI:
            if (instruction.iLdi.isResolved) {
                printf("LDI R%d, #%d\n", instruction.iLdi.destinationRegister, instruction.iLdi.pcOffset9);
            } else {
                printf("LDI R%d, %s\n", instruction.iLdi.destinationRegister, instruction.iLdi.label);
            }
            break;

        case I_LDR:
            printf("LDR R%d, R%d, #%d\n", instruction.iLdr.destinationRegister, instruction.iLdr.baseRegister, instruction.iLdr.offset6);
            break;

        case I_LEA:
            if (instruction.iLea.isResolved) {
                printf("LEA R%d, #%d\n", instruction.iLea.destinationRegister, instruction.iLea.pcOffset9);
            } else {
                printf("LEA R%d, %s\n", instruction.iLea.destinationRegister, instruction.iLea.label);
            }
            break;

        case I_NOT:
            printf("NOT R%d, R%d\n", instruction.iNot.destinationRegister, instruction.iNot.sourceRegister);
            break;

        case I_ST:
            if (instruction.iSt.isResolved) {
                printf("ST R%d, #%d\n", instruction.iSt.sourceRegister, instruction.iSt.pcOffset9);
            } else {
                printf("ST R%d, %s\n", instruction.iSt.sourceRegister, instruction.iSt.label);
            }
            break;

        case I_STR:
            printf("STR R%d, R%d, #%d\n", instruction.iStr.sourceRegister, instruction.iStr.baseRegister, instruction.iStr.offset6);
            break;

        case I_TRAP:
            printf("TRAP x%02X\n", instruction.iTrap.trapVector8);
            break;

        case D_ORIG:
            printf(".ORIG x%04X\n", instruction.dOrig.address);
            break;

        case D_FILL:
            printf(".FILL x%04X\n", instruction.dFill.value);
            break;

        case D_BLKW:
            printf(".BLKW %d\n", instruction.dBlkw.count);
            break;

        case D_STRINGZ:
            printf(".STRINGZ \"%s\"\n", instruction.dStringz.string);
            break;

        case D_END:
            printf(".END\n");
            break;

        case M_GETC:
            printf("GETC\n");
            break;

        case M_OUT:
            printf("OUT\n");
            break;

        case M_PUTS:
            printf("PUTS\n");
            break;

        case M_IN:
            printf("IN\n");
            break;

        case M_PUTSP:
            printf("PUTSP\n");
            break;

        case M_HALT:
            printf("HALT\n");
            break;

        case I_RET:
            printf("RET\n");
            break;

        case I_RTI:
            printf("RTI\n");
            break;

        case I_STI:
            if (instruction.iSti.isResolved) {
                printf("STI R%d, #%d\n", instruction.iSti.sourceRegister, instruction.iSti.pcOffset9);
            } else {
                printf("STI R%d, %s\n", instruction.iSti.sourceRegister, instruction.iSti.label);
            }
            break;

        default:
            printf("Unknown instruction type: %d\n", instruction.type);
            exit(1);
            break;
    }
}

void printLabelledInstruction(LabelledInstruction instruction) {
    if (instruction.label != NULL) {
        printf("%s ", instruction.label);
    }

    printInstruction(instruction.instruction);
}

// unsigned short lc3AssembleInstruction(ParsedInstruction instruction);
// ParsedInstruction lc3DisassembleInstruction(unsigned short instruction);
