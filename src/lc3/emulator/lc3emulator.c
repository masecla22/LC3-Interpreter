#include "lc3emulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline unsigned short getRaw(unsigned short instruction, short at, short count) {
    return (instruction >> at) & ((1 << count) - 1);
}

static inline short getAsNumber(unsigned short instruction, short at, short count) {
    unsigned short raw = getRaw(instruction, at, count);
    return raw & (1 << (count - 1)) ? raw - (1 << count) : raw;
}

// typedef struct {
//     unsigned short opcode : 4;  // Opcode always takes up the first 4 bits

//     union {
//         // struct {
//         //     unsigned short dr : 3;
//         //     unsigned short sr1 : 3;
//         //     unsigned short isImm : 1;
//         //     unsigned short imm5 : 5;  // This will be the SR2 field if isImm is 0
//         // } __attribute__((packed)) add_and;

//         // struct {
//         //     unsigned short nzp : 3;
//         //     unsigned short pcOffset9 : 9;
//         // } __attribute__((packed)) br;

//         // struct {
//         //     unsigned short zeroZeroZero : 3;
//         //     unsigned short baseRegister : 3;
//         //     // The rest of the bits are unused
//         // } __attribute__((packed)) jmp_jsrr;

//         // struct {
//         //     unsigned short one : 1;
//         //     unsigned short pcOffset11 : 11;
//         // } __attribute__((packed)) jsr;

//         // struct {
//         //     unsigned short destinationRegister : 3;
//         //     unsigned short pcOffset9 : 9;
//         // } __attribute__((packed)) ld_ldi_lea;

//         // struct {
//         //     unsigned short destinationRegister : 3;
//         //     unsigned short baseRegister : 3;
//         //     unsigned short offset6 : 6;
//         // } __attribute__((packed)) ldr;

//         // struct {
//         //     unsigned short destinationRegister : 3;
//         //     unsigned short sourceRegister : 3;
//         // } __attribute__((packed)) not;

//         // struct {
//         //     unsigned short sourceRegister : 3;
//         //     unsigned short pcOffset9 : 9;
//         // } __attribute__((packed)) st_sti;

//         // struct {
//         //     unsigned short sourceRegister : 3;
//         //     unsigned short baseRegister : 3;
//         //     unsigned short offset6 : 6;
//         // } __attribute__((packed)) str;

//         // struct {
//         //     unsigned short zeroZeroZeroZero : 4;
//         //     unsigned short trapVector8 : 8;
//         // } __attribute__((packed)) trap;
//     } __attribute__((packed));
// } __attribute__((packed)) EmulatorInstruction;

static inline void stepBr(LC3EmulatorState *state, unsigned short instruction) {
    unsigned short nzp = getRaw(instruction, 9, 3);
    short pcOffset9 = getAsNumber(instruction, 0, 9);

    if (nzp & state->cc) {
        state->pc += pcOffset9;
    }
}

static inline void stepAdd(LC3EmulatorState *state, unsigned short instruction) {
    // short sr1 = state->registers[instruction->add_and.sr1];
    // short sr2 = instruction->add_and.isImm ? instruction->add_and.imm5 : state->registers[instruction->add_and.imm5];
    // short result = sr1 + sr2;

    // state->registers[instruction->add_and.dr] = result;
    // state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;

    short sr1 = state->registers[getRaw(instruction, 6, 3)];
    short sr2 = instruction & (1 << 5) ? getAsNumber(instruction, 0, 5) : state->registers[getRaw(instruction, 0, 3)];
    short result = sr1 + sr2;

    state->registers[getRaw(instruction, 9, 3)] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4
                                             : 1;
}

static inline void stepLd(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    // state->registers[instruction->ld_ldi_lea.destinationRegister] = state->memory[address].parsedNumber;
    // state->cc = state->memory[address].parsedNumber == 0 ? 2 : state->memory[address].parsedNumber < 0 ? 4 : 1;

    unsigned short address = state->pc + getAsNumber(instruction, 0, 9);
    state->registers[getRaw(instruction, 9, 3)] = state->memory[address].parsedNumber;
    state->cc = state->memory[address].parsedNumber == 0 ? 2 : state->memory[address].parsedNumber < 0 ? 4
                                                                                                       : 1;
}

static inline void stepSt(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short address = state->pc + instruction->st_sti.pcOffset9;
    // state->memory[address].parsedNumber = state->registers[instruction->st_sti.sourceRegister];

    unsigned short address = state->pc + getAsNumber(instruction, 0, 9);
    state->memory[address].parsedNumber = state->registers[getRaw(instruction, 9, 3)];
}

static inline void stepJsrJsrr(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short pc = state->pc;
    // state->pc += instruction->jsr.pcOffset11;
    // state->registers[7] = pc;

    unsigned short pc = state->pc;
    state->pc += getAsNumber(instruction, 0, 11);
    state->registers[7] = pc;
}
static inline void stepAnd(LC3EmulatorState *state, unsigned short instruction) {
    // short sr1 = state->registers[instruction->add_and.sr1];
    // short sr2 = instruction->add_and.isImm ? instruction->add_and.imm5 : state->registers[instruction->add_and.imm5];
    // short result = sr1 & sr2;

    // state->registers[instruction->add_and.dr] = result;
    // state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;

    short sr1 = state->registers[getRaw(instruction, 6, 3)];
    short sr2 = instruction & (1 << 5) ? getAsNumber(instruction, 0, 5) : state->registers[getRaw(instruction, 0, 3)];
    short result = sr1 & sr2;

    state->registers[getRaw(instruction, 9, 3)] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4
                                             : 1;
}
static inline void stepLdr(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short base = state->registers[instruction->ldr.baseRegister];
    // unsigned short offset = instruction->ldr.offset6;
    // unsigned short address = base + offset;

    // state->registers[instruction->ldr.destinationRegister] = state->memory[address].parsedNumber;
    // state->cc = state->memory[address].parsedNumber == 0 ? 2 : state->memory[address].parsedNumber < 0 ? 4 : 1;

    unsigned short base = state->registers[getRaw(instruction, 6, 3)];
    unsigned short offset = getAsNumber(instruction, 0, 6);
    unsigned short address = base + offset;

    state->registers[getRaw(instruction, 9, 3)] = state->memory[address].parsedNumber;
    state->cc = state->memory[address].parsedNumber == 0 ? 2 : state->memory[address].parsedNumber < 0 ? 4
                                                                                                       : 1;
}
static inline void stepStr(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short base = state->registers[instruction->str.baseRegister];
    // unsigned short offset = instruction->str.offset6;
    // unsigned short address = base + offset;

    // state->memory[address].parsedNumber = state->registers[instruction->str.sourceRegister];

    unsigned short base = state->registers[getRaw(instruction, 6, 3)];
    unsigned short offset = getAsNumber(instruction, 0, 6);
    unsigned short address = base + offset;

    state->memory[address].parsedNumber = state->registers[getRaw(instruction, 9, 3)];
}
static inline void stepRti(LC3EmulatorState *state, unsigned short instruction) {
    fprintf(stderr, "RTI encountered!\n");
    exit(1);
}
static inline void stepNot(LC3EmulatorState *state, unsigned short instruction) {
    // short source = state->registers[instruction->not.sourceRegister];
    // short result = ~source;

    // state->registers[instruction->not.destinationRegister] = result;
    // state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;

    short source = state->registers[getRaw(instruction, 0, 3)];
    short result = ~source;

    state->registers[getRaw(instruction, 3, 3)] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4
                                             : 1;
}
static inline void stepLdi(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    // unsigned short indirectAddress = state->memory[address].parsedNumber;
    // state->registers[instruction->ld_ldi_lea.destinationRegister] = state->memory[indirectAddress].parsedNumber;
    // state->cc = state->memory[indirectAddress].parsedNumber == 0 ? 2 : state->memory[indirectAddress].parsedNumber < 0 ? 4 : 1;

    unsigned short address = state->pc + getAsNumber(instruction, 0, 9);
    unsigned short indirectAddress = state->memory[address].parsedNumber;
    state->registers[getRaw(instruction, 9, 3)] = state->memory[indirectAddress].parsedNumber;
    state->cc = state->memory[indirectAddress].parsedNumber == 0 ? 2 : state->memory[indirectAddress].parsedNumber < 0 ? 4
                                                                                                                       : 1;
}

static inline void stepSti(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short address = state->pc + instruction->st_sti.pcOffset9;
    // unsigned short indirectAddress = state->memory[address].parsedNumber;
    // state->memory[indirectAddress].parsedNumber = state->registers[instruction->st_sti.sourceRegister];

    unsigned short address = state->pc + getAsNumber(instruction, 0, 9);
    unsigned short indirectAddress = state->memory[address].parsedNumber;
    state->memory[indirectAddress].parsedNumber = state->registers[getRaw(instruction, 9, 3)];
}

static inline void stepJmp(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short base = state->registers[instruction->jmp_jsrr.baseRegister];
    // state->pc = base;

    unsigned short base = state->registers[getRaw(instruction, 6, 3)];
    state->pc = base;
}

static inline void stepRes(LC3EmulatorState *state, unsigned short instruction) {
    fprintf(stderr, "Reserved opcode encountered!\n");
    exit(1);
}

static inline void stepLea(LC3EmulatorState *state, unsigned short instruction) {
    // unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    // state->registers[instruction->ld_ldi_lea.destinationRegister] = address;

    unsigned short address = state->pc + getAsNumber(instruction, 0, 9);
    state->registers[getRaw(instruction, 9, 3)] = address;
}

static inline void stepTrap(LC3EmulatorState *state, unsigned short instruction) {
    short trapVector = getAsNumber(instruction, 0, 8);

    if (trapVector == 0x20) {
        // GETC
        char c = getchar();
        state->registers[0] = c;
    } else if (trapVector == 0x21) {
        // OUT
        printf("-----\n");
        putchar(state->registers[0]);
        printf("-----\n");
    } else if (trapVector == 0x22) {
        // PUTS
        unsigned short *address = &state->memory[state->registers[0]].rawNumber;
        while (*address) {
            putchar(*address);
            address++;
        }
    } else if (trapVector == 0x23) {
        // IN
        printf("Input a character> ");
        char c = getchar();
        state->registers[0] = c;
        putchar(c);
    } else if (trapVector == 0x24) {
        // PUTSP
        unsigned short *address = &state->memory[state->registers[0]].rawNumber;
        while (*address) {
            char c = (*address) & 0xFF;
            putchar(c);

            c = (*address) >> 8;
            if (c == 0) break;
            putchar(c);

            address++;
        }
    } else if (trapVector == 0x25) {
        // HALT
        state->haltSignal = 1;
    }
}

void printState(LC3EmulatorState *state) {
    printf("PC: %d ", state->pc);
    printf("CC: %d ", state->cc);

    for (int i = 0; i < 8; i++) {
        if (state->registers[i] > 1000) {
            printf("R%d: x%4x ", i, state->registers[i]);
        } else {
            printf("R%d: %d ", i, state->registers[i]);
        }
    }
    printf("\n");
}

void step(LC3Context *ctx, LC3EmulatorState *state) {
    unsigned short pc = state->pc;
    state->pc++;

    unsigned short instruction = state->memory[pc].rawNumber;
    unsigned short opcode = getRaw(instruction, 12, 4);

    printState(state);

    switch (opcode) {
        case 0:
            stepBr(state, instruction);
            break;
        case 1:
            stepAdd(state, instruction);
            break;
        case 2:
            stepLd(state, instruction);
            break;
        case 3:
            stepSt(state, instruction);
            break;
        case 4:
            stepJsrJsrr(state, instruction);
            break;
        case 5:
            stepAnd(state, instruction);
            break;
        case 6:
            stepLdr(state, instruction);
            break;
        case 7:
            stepStr(state, instruction);
            break;
        case 8:
            stepRti(state, instruction);
            break;
        case 9:
            stepNot(state, instruction);
            break;
        case 10:
            stepLdi(state, instruction);
            break;
        case 11:
            stepSti(state, instruction);
            break;
        case 12:
            stepJmp(state, instruction);
            break;
        case 13:
            stepRes(state, instruction);
            break;
        case 14:
            stepLea(state, instruction);
            break;
        case 15:
            stepTrap(state, instruction);
            break;
    }
}

void emulate(LC3Context ctx, LC3EmulatorState state) {
    while (!state.haltSignal) {
        usleep(1000);
        step(&ctx, &state);
    }
}

// void main() {
//     printf("%d", sizeof(EmulatorInstruction));
// }