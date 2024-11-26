#include "lc3assembler.h"

#include <stdlib.h>
#include <string.h>

#include "../../lexer/lexer.h"
#include "../../map/string_map.h"
#include "../emulator/lc3emulator.h"
#include "../instructions/lc3isa.h"

extern LabelledInstructionList* labelledInstructions;

void freeLabelledInstructions() {
    if (labelledInstructions == NULL) {
        return;
    }

    destroyLabelledInstructionList(labelledInstructions);
    labelledInstructions = NULL;
}

void ensureMemoryLayoutCanBeMade() {
    if (labelledInstructions->count == 0) {
        fprintf(stderr, "No instructions found in the input file.\n");
        exit(1);
    }

    LabelledInstruction firstInstruction = labelledInstructions->instructions[0];

    if (firstInstruction.instruction.type != D_ORIG) {
        fprintf(stderr, "First instruction must be an .ORIG directive.\n");
        exit(1);
    }

    int addr = firstInstruction.instruction.dOrig.address;

    if (addr < 0x3000) {
        fprintf(stderr, "Origin address must be at least 0x3000.\n");
        exit(1);
    }

    if (addr > 0xFFFF) {
        fprintf(stderr, "Origin address must be at most 0xFFFF.\n");
        exit(1);
    }
}

int resolveInitialMemoryLayout() {
    ensureMemoryLayoutCanBeMade();

    LabelledInstruction firstInstruction = labelledInstructions->instructions[0];

    int addr = firstInstruction.instruction.dOrig.address;

    // Go through each instruction and resolve the memory location
    for (unsigned int i = 0; i < labelledInstructions->count; i++) {
        LabelledInstruction* instruction = &labelledInstructions->instructions[i];

        switch (instruction->instruction.type) {
            case D_ORIG:
                break;
            case D_BLKW:
                instruction->memoryLocation = addr;
                addr += instruction->instruction.dBlkw.count;
                break;
            case D_STRINGZ:
                instruction->memoryLocation = addr;
                addr += strlen(instruction->instruction.dStringz.string) + 1;
                break;
            case D_END:
                break;
            default:
                instruction->memoryLocation = addr;
                addr++;
                break;
        }
    }

    return firstInstruction.instruction.dOrig.address;
}

ParsedInstructionList* resolveReferences() {
    StringMap* labelMap = stringMapCreate();

    // Go through all labels and add their locations ot the StringMap
    for (unsigned int i = 0; i < labelledInstructions->count; i++) {
        LabelledInstruction instruction = labelledInstructions->instructions[i];
        if (instruction.label != NULL) {
            stringMapPut(labelMap, instruction.label, (void*)(long)(instruction.memoryLocation - 1));
        }
    }

    // Make sure no labels are unresolved
    for (unsigned int i = 0; i < labelledInstructions->count; i++) {
        LabelledInstruction* instruction = &labelledInstructions->instructions[i];

        char* labelNeedingChecking = NULL;
        switch (instruction->instruction.type) {
            case I_BR:
                if (!instruction->instruction.iBr.isResolved)
                    labelNeedingChecking = instruction->instruction.iBr.label;
                break;
            case I_JSR:
                if (!instruction->instruction.iJsr.isResolved)
                    labelNeedingChecking = instruction->instruction.iJsr.label;
                break;
            case I_LD:
                if (!instruction->instruction.iLd.isResolved)
                    labelNeedingChecking = instruction->instruction.iLd.label;
                break;
            case I_LDI:
                if (!instruction->instruction.iLdi.isResolved)
                    labelNeedingChecking = instruction->instruction.iLdi.label;
                break;
            case I_LEA:
                if (!instruction->instruction.iLea.isResolved)
                    labelNeedingChecking = instruction->instruction.iLea.label;
                break;
            case I_ST:
                if (!instruction->instruction.iSt.isResolved)
                    labelNeedingChecking = instruction->instruction.iSt.label;
                break;
            case I_STI:
                if (!instruction->instruction.iSti.isResolved)
                    labelNeedingChecking = instruction->instruction.iSti.label;
                break;
            default:
                break;
        }

        if (labelNeedingChecking != NULL) {
            // Check if the label is in the map
            void* result = stringMapGet(labelMap, labelNeedingChecking);
            if (result == NULL) {
                fprintf(stderr, "Label %s has not been declared anywhere!\n", labelNeedingChecking);
                exit(1);
            }
        }
    }

    ParsedInstructionList* instrList = createParsedInstructionList();

    // Go through all instructions and resolve the references
    for (unsigned int i = 0; i < labelledInstructions->count; i++) {
        LabelledInstruction instruction = labelledInstructions->instructions[i];
        ParsedInstruction parsedInstruction = {0};
        parsedInstruction.memoryLocation = instruction.memoryLocation;

        int currentAddress = instruction.memoryLocation;
        switch (instruction.instruction.type) {
            case I_ADD:
                parsedInstruction.type = I_ADD;
                parsedInstruction.iAdd = instruction.instruction.iAdd;
                break;
            case I_AND:
                parsedInstruction.type = I_AND;
                parsedInstruction.iAnd = instruction.instruction.iAnd;
                break;
            case I_BR:
                parsedInstruction.type = I_BR;
                parsedInstruction.iBr.nzp = instruction.instruction.iBr.nzp;
                if (instruction.instruction.iBr.isResolved) {
                    parsedInstruction.iBr.pcOffset9 = instruction.instruction.iBr.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iBr.label);
                    parsedInstruction.iBr.pcOffset9 = target - currentAddress;
                }
                break;
            case I_JMP:
                parsedInstruction.type = I_JMP;
                parsedInstruction.iJmp = instruction.instruction.iJmp;
                break;
            case I_JSR:
                parsedInstruction.type = I_JSR;
                if (instruction.instruction.iJsr.isResolved) {
                    parsedInstruction.iJsr.pcOffset11 = instruction.instruction.iJsr.pcOffset11;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iJsr.label);
                    parsedInstruction.iJsr.pcOffset11 = target - currentAddress;
                }
                break;
            case I_JSRR:
                parsedInstruction.type = I_JSRR;
                parsedInstruction.iJsrr = instruction.instruction.iJsrr;
                break;
            case I_LD:
                parsedInstruction.type = I_LD;
                parsedInstruction.iLd.destinationRegister = instruction.instruction.iLd.destinationRegister;
                if (instruction.instruction.iLd.isResolved) {
                    parsedInstruction.iLd.pcOffset9 = instruction.instruction.iLd.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iLd.label);
                    parsedInstruction.iLd.pcOffset9 = target - currentAddress;
                }
                break;
            case I_LDI:
                parsedInstruction.type = I_LDI;
                parsedInstruction.iLdi.destinationRegister = instruction.instruction.iLdi.destinationRegister;
                if (instruction.instruction.iLdi.isResolved) {
                    parsedInstruction.iLdi.pcOffset9 = instruction.instruction.iLdi.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iLdi.label);
                    parsedInstruction.iLdi.pcOffset9 = target - currentAddress;
                }
                break;
            case I_LDR:
                parsedInstruction.type = I_LDR;
                parsedInstruction.iLdr = instruction.instruction.iLdr;
                break;
            case I_LEA:
                parsedInstruction.type = I_LEA;
                parsedInstruction.iLea.destinationRegister = instruction.instruction.iLea.destinationRegister;
                if (instruction.instruction.iLea.isResolved) {
                    parsedInstruction.iLea.pcOffset9 = instruction.instruction.iLea.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iLea.label);
                    parsedInstruction.iLea.pcOffset9 = target - currentAddress;
                }
                break;
            case I_NOT:
                parsedInstruction.type = I_NOT;
                parsedInstruction.iNot = instruction.instruction.iNot;
                break;
            case I_RET:
            case I_RTI:
                parsedInstruction.type = instruction.instruction.type;
                break;
            case I_ST:
                parsedInstruction.type = I_ST;
                parsedInstruction.iSt.sourceRegister = instruction.instruction.iSt.sourceRegister;
                if (instruction.instruction.iSt.isResolved) {
                    parsedInstruction.iSt.pcOffset9 = instruction.instruction.iSt.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iSt.label);
                    parsedInstruction.iSt.pcOffset9 = target - currentAddress;
                }
                break;
            case I_STI:
                parsedInstruction.type = I_STI;
                parsedInstruction.iSti.sourceRegister = instruction.instruction.iSti.sourceRegister;
                if (instruction.instruction.iSti.isResolved) {
                    parsedInstruction.iSti.pcOffset9 = instruction.instruction.iSti.pcOffset9;
                } else {
                    int target = (int)(long)stringMapGet(labelMap, instruction.instruction.iSti.label);
                    parsedInstruction.iSti.pcOffset9 = target - currentAddress;
                }
                break;
            case I_STR:
                parsedInstruction.type = I_STR;
                parsedInstruction.iStr = instruction.instruction.iStr;
                break;
            case I_TRAP:
                parsedInstruction.type = I_TRAP;
                parsedInstruction.iTrap = instruction.instruction.iTrap;
                break;
            case M_GETC:
            case M_OUT:
            case M_PUTS:
            case M_IN:
            case M_PUTSP:
            case M_HALT:
                parsedInstruction.type = instruction.instruction.type;
                break;
            case D_ORIG:
                parsedInstruction.type = D_ORIG;
                parsedInstruction.dOrig = instruction.instruction.dOrig;
                break;
            case D_FILL:
                parsedInstruction.type = D_FILL;
                parsedInstruction.dFill = instruction.instruction.dFill;
                break;
            case D_BLKW:
                parsedInstruction.type = D_BLKW;
                parsedInstruction.dBlkw = instruction.instruction.dBlkw;
                break;
            case D_STRINGZ:
                parsedInstruction.type = D_STRINGZ;
                parsedInstruction.dStringz.string = strdup(instruction.instruction.dStringz.string);
                break;
            case D_END:
                parsedInstruction.type = D_END;
                break;
            default:
                printf("Unable to parse instruction: ");
                printLabelledInstruction(instruction);
                exit(1);
        }

        addParsedInstruction(instrList, parsedInstruction);
    }

    // Free the label map
    stringMapDestroy(labelMap, 0);
    labelMap = NULL;

    // Free the labelled instructions
    freeLabelledInstructions();

    return instrList;
}

MemoryCell* createMemoryLayout(LC3Context ctx) {
    MemoryCell* memory = calloc(65536, sizeof(MemoryCell));
    if (memory == NULL) {
        return NULL;
    }

    // Fill up memory with junk values if randomized
    if (ctx.randomized) {
        int seed = ctx.seed;
        srand(seed);
        for (int i = 0; i < 65536; i++) {
            memory[i].rawNumber = rand() % 65536;
        }
    }

    return memory;
}

unsigned short assembleAdd(AddInstruction instruction) {
    return (1 << 12) | (instruction.destinationRegister << 9) | (instruction.sr1 << 6) | (instruction.isImm << 5) | (instruction.isImm ? (instruction.imm5 & 0x1F) : (instruction.sr2 & 0x7));
}

unsigned short assembleAnd(AndInstruction instruction) {
    return (5 << 12) | (instruction.destinationRegister << 9) | (instruction.sr1 << 6) | (instruction.isImm << 5) | (instruction.isImm ? (instruction.imm5 & 0x1F) : (instruction.sr2 & 0x7));
}

unsigned short assembleBranch(BranchInstruction instruction) {
    return (instruction.nzp << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleJump(JumpInstruction instruction) {
    return (12 << 12) | (instruction.sourceRegister << 6);
}

unsigned short assembleJumpSubroutine(JumpSubroutineInstruction instruction) {
    return (4 << 12) | (instruction.pcOffset11 & 0x7FF);
}

unsigned short assembleJumpSubroutineRegister(JumpSubroutineRegisterInstruction instruction) {
    return (4 << 12) | (1 << 11) | (instruction.baseRegister << 6);
}

unsigned short assembleLoad(LoadInstruction instruction) {
    return (2 << 12) | (instruction.destinationRegister << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleLoadIndirect(LoadIndirectInstruction instruction) {
    return (10 << 12) | (instruction.destinationRegister << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleLoadBaseOffset(LoadBaseOffsetInstruction instruction) {
    return (6 << 12) | (instruction.destinationRegister << 9) | (instruction.baseRegister << 6) | (instruction.offset6 & 0x3F);
}

unsigned short assembleLoadEffectiveAddress(LoadEffectiveAddressInstruction instruction) {
    return (14 << 12) | (instruction.destinationRegister << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleNot(NotInstruction instruction) {
    return (9 << 12) | (instruction.destinationRegister << 9) | (instruction.sourceRegister << 6) | 0x3F;
}

unsigned short assembleRet() {
    return 0xC1C0;
}

unsigned short assembleRti() {
    return 0x8000;
}

unsigned short assembleStore(StoreInstruction instruction) {
    return (3 << 12) | (instruction.sourceRegister << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleStoreIndirect(StoreIndirectInstruction instruction) {
    return (11 << 12) | (instruction.sourceRegister << 9) | (instruction.pcOffset9 & 0x1FF);
}

unsigned short assembleStoreBaseOffset(StoreBaseOffsetInstruction instruction) {
    return (7 << 12) | (instruction.sourceRegister << 9) | (instruction.baseRegister << 6) | (instruction.offset6 & 0x3F);
}

unsigned short assembleTrap(TrapInstruction instruction) {
    return (0xF << 12) | (instruction.trapVector8 & 0xFF);
}

unsigned short assembleGetc() {
    return 0xF020;
}

unsigned short assembleOut() {
    return 0xF021;
}

unsigned short assemblePuts() {
    return 0xF022;
}

unsigned short assembleIn() {
    return 0xF023;
}

unsigned short assemblePutsp() {
    return 0xF024;
}

unsigned short assembleHalt() {
    return 0xF025;
}

void assembleInstructionsIntoMemory(MemoryCell* memory, ParsedInstructionList* instructionList) {
    for (unsigned int i = 0; i < instructionList->count; i++) {
        ParsedInstruction instruction = instructionList->instructions[i];
        switch (instruction.type) {
            case I_ADD:
                memory[instruction.memoryLocation].rawNumber = assembleAdd(instruction.iAdd);
                break;
            case I_AND:
                memory[instruction.memoryLocation].rawNumber = assembleAnd(instruction.iAnd);
                break;
            case I_BR:
                memory[instruction.memoryLocation].rawNumber = assembleBranch(instruction.iBr);
                break;
            case I_JMP:
                memory[instruction.memoryLocation].rawNumber = assembleJump(instruction.iJmp);
                break;
            case I_JSR:
                memory[instruction.memoryLocation].rawNumber = assembleJumpSubroutine(instruction.iJsr);
                break;
            case I_JSRR:
                memory[instruction.memoryLocation].rawNumber = assembleJumpSubroutineRegister(instruction.iJsrr);
                break;
            case I_LD:
                memory[instruction.memoryLocation].rawNumber = assembleLoad(instruction.iLd);
                break;
            case I_LDI:
                memory[instruction.memoryLocation].rawNumber = assembleLoadIndirect(instruction.iLdi);
                break;
            case I_LDR:
                memory[instruction.memoryLocation].rawNumber = assembleLoadBaseOffset(instruction.iLdr);
                break;
            case I_LEA:
                memory[instruction.memoryLocation].rawNumber = assembleLoadEffectiveAddress(instruction.iLea);
                break;
            case I_NOT:
                memory[instruction.memoryLocation].rawNumber = assembleNot(instruction.iNot);
                break;
            case I_RET:
                memory[instruction.memoryLocation].rawNumber = assembleRet();
                break;
            case I_RTI:
                memory[instruction.memoryLocation].rawNumber = assembleRti();
                break;
            case I_ST:
                memory[instruction.memoryLocation].rawNumber = assembleStore(instruction.iSt);
                break;
            case I_STI:
                memory[instruction.memoryLocation].rawNumber = assembleStoreIndirect(instruction.iSti);
                break;
            case I_STR:
                memory[instruction.memoryLocation].rawNumber = assembleStoreBaseOffset(instruction.iStr);
                break;
            case I_TRAP:
                memory[instruction.memoryLocation].rawNumber = assembleTrap(instruction.iTrap);
                break;
            case M_GETC:
                memory[instruction.memoryLocation].rawNumber = assembleGetc();
                break;
            case M_OUT:
                memory[instruction.memoryLocation].rawNumber = assembleOut();
                break;
            case M_PUTS:
                memory[instruction.memoryLocation].rawNumber = assemblePuts();
                break;
            case M_IN:
                memory[instruction.memoryLocation].rawNumber = assembleIn();
                break;
            case M_PUTSP:
                memory[instruction.memoryLocation].rawNumber = assemblePutsp();
                break;
            case M_HALT:
                memory[instruction.memoryLocation].rawNumber = assembleHalt();
                break;
            case D_ORIG:
                break;
            case D_FILL:
                memory[instruction.memoryLocation].rawNumber = instruction.dFill.value;
                break;
            case D_BLKW:
                for (unsigned int j = 0; j < instruction.dBlkw.count; j++) {
                    memory[instruction.memoryLocation + j].rawNumber = 0;
                }
                break;
            case D_STRINGZ:
                for (unsigned int j = 0; j < strlen(instruction.dStringz.string); j++) {
                    memory[instruction.memoryLocation + j].rawNumber = instruction.dStringz.string[j];
                }
                // Add the null terminator
                memory[instruction.memoryLocation + strlen(instruction.dStringz.string)].rawNumber = 0;

                // Since we're done with this string we can free it
                free(instruction.dStringz.string);

                break;
            case D_END:
                break;
            default:
                printf("Cannot assemble instruction: ");
                printParsedInstruction(instruction);
                break;
        }
    }
}

LC3EmulatorState prepareEmulatorState(LC3Context ctx, MemoryCell* memory, int initialPc) {
    LC3EmulatorState emulatorState = {0};

    emulatorState.cc = 2;
    emulatorState.haltSignal = 0;
    emulatorState.memory = memory;
    emulatorState.pc = initialPc;

    if (ctx.randomized) {
        // Set registers to random values
        for (int i = 0; i < 8; i++) {
            emulatorState.registers[i] = rand() % 65536;
        }
    }

    return emulatorState;
}

LC3EmulatorState assemble(LC3Context ctx) {
    // Register parser cleanup
    atexit(freeLabelledInstructions);

    // Setup the lexer with the input file.
    initLexer(ctx.inputFile);

    // Parse the input file
    yyparse();

    // Free the lexer resources.
    finalizeLexer();

    // Resolve the initial memory layout
    int initialPc = resolveInitialMemoryLayout();

    // Resolve labels
    ParsedInstructionList* parsed = resolveReferences();
    MemoryCell* memory = createMemoryLayout(ctx);

    // Assemble the parsed instructions into the memory
    assembleInstructionsIntoMemory(memory, parsed);

    // Free the parsed instructions
    destroyParsedInstructionList(parsed);

    LC3EmulatorState emulatorState = prepareEmulatorState(ctx, memory, initialPc);

    return emulatorState;
}
