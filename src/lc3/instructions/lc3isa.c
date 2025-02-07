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
    if (instruction.labels != NULL) {
        destroyLabels(instruction.labels);
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

void printUnresolvedInstruction(UnresolvedInstruction instruction) {
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

        case D_FILL: {
            if (instruction.dFill.isResolved) {
                int signedValue = instruction.dFill.value;
                if (signedValue < 256 && signedValue > -256) {
                    printf(".FILL #%d\n", signedValue);
                } else {
                    printf(".FILL x%04X\n", instruction.dFill.value);
                }
            } else {
                printf(".FILL %s\n", instruction.dFill.label);
            }
            break;
        }
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
    if (instruction.labels != NULL) {
        for (unsigned int i = 0; i < instruction.labels->count; i++) {
            printf("%s", instruction.labels->labels[i]);
            if (i < instruction.labels->count - 1) {
                printf(", ");
            } else {
                printf(" ");
            }
        }
    }

    printUnresolvedInstruction(instruction.instruction);
}

void printParsedInstruction(ParsedInstruction instruction) {
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
            printf(" #%d\n", instruction.iBr.pcOffset9);
            break;

        case I_JMP:
            printf("JMP R%d\n", instruction.iJmp.sourceRegister);
            break;

        case I_JSR:
            printf("JSR #%d\n", instruction.iJsr.pcOffset11);
            break;

        case I_JSRR:
            printf("JSRR R%d\n", instruction.iJsrr.baseRegister);
            break;

        case I_LD:
            printf("LD R%d, #%d\n", instruction.iLd.destinationRegister, instruction.iLd.pcOffset9);
            break;

        case I_LDI:
            printf("LDI R%d, #%d\n", instruction.iLdi.destinationRegister, instruction.iLdi.pcOffset9);
            break;

        case I_LDR:
            printf("LDR R%d, R%d, #%d\n", instruction.iLdr.destinationRegister, instruction.iLdr.baseRegister, instruction.iLdr.offset6);
            break;

        case I_LEA:
            printf("LEA R%d, #%d\n", instruction.iLea.destinationRegister, instruction.iLea.pcOffset9);
            break;

        case I_NOT:
            printf("NOT R%d, R%d\n", instruction.iNot.destinationRegister, instruction.iNot.sourceRegister);
            break;
        case I_ST:
            printf("ST R%d, #%d\n", instruction.iSt.sourceRegister, instruction.iSt.pcOffset9);
            break;
        case I_STI:
            printf("STI R%d, #%d\n", instruction.iSti.sourceRegister, instruction.iSti.pcOffset9);
            break;
        case I_STR:
            printf("STR R%d, R%d, #%d\n", instruction.iStr.sourceRegister, instruction.iStr.baseRegister, instruction.iStr.offset6);
            break;
        case I_TRAP:
            printf("TRAP x%02X\n", instruction.iTrap.trapVector8);
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
        case D_ORIG:
            printf(".ORIG x%04X\n", instruction.dOrig.address);
            break;
        case D_FILL:
            printf(".FILL #%d\n", instruction.dFill.value);
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
        default:
            printf("Unknown instruction type: %d\n", instruction.type);
            exit(1);
            break;
    }
}

ParsedInstructionList* createParsedInstructionList(void) {
    ParsedInstructionList* list = calloc(1, sizeof(ParsedInstructionList));
    list->instructions = calloc(16, sizeof(ParsedInstruction));
    list->count = 0;
    list->capacity = 16;

    return list;
}

void destroyParsedInstructionList(ParsedInstructionList* list) {
    free(list->instructions);
    free(list);
}

void addParsedInstruction(ParsedInstructionList* list, ParsedInstruction instruction) {
    if (list->count == list->capacity - 2) {
        list->capacity *= 2;
        list->instructions = realloc(list->instructions, sizeof(ParsedInstruction) * list->capacity);

        int oldCapacity = list->capacity / 2;
        for (unsigned int i = oldCapacity; i < list->capacity; i++) {
            list->instructions[i] = (ParsedInstruction){0};
        }
    }

    list->instructions[list->count++] = instruction;
}

Labels* createLabels(void) {
    Labels* labels = calloc(1, sizeof(Labels));
    labels->labels = calloc(16, sizeof(char*));
    labels->count = 0;
    labels->capacity = 16;

    return labels;
}

void destroyLabels(Labels* labels) {
    for (unsigned int i = 0; i < labels->count; i++) {
        free(labels->labels[i]);
    }

    free(labels->labels);
    free(labels);
}

void addLabel(Labels* labels, char* label) {
    if (labels->count == labels->capacity - 2) {
        labels->capacity *= 2;
        labels->labels = realloc(labels->labels, sizeof(char*) * labels->capacity);

        int oldCapacity = labels->capacity / 2;
        for (unsigned int i = oldCapacity; i < labels->capacity; i++) {
            labels->labels[i] = NULL;
        }
    }

    labels->labels[labels->count++] = label;
}