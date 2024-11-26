#include "lc3emulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    unsigned short opcode : 4;  // Opcode always takes up the first 4 bits

    union {
        struct {
            unsigned short dr : 3;
            unsigned short sr1 : 3;
            unsigned short isImm : 1;
            unsigned short imm5 : 5;  // This will be the SR2 field if isImm is 0
        } add_and;

        struct {
            unsigned short nzp : 3;
            unsigned short pcOffset9 : 9;
        } br;

        struct {
            unsigned short zeroZeroZero : 3;
            unsigned short baseRegister : 3;
            // The rest of the bits are unused
        } jmp_jsrr;

        struct {
            unsigned short one : 1;
            unsigned short pcOffset11 : 11;
        } jsr;

        struct {
            unsigned short destinationRegister : 3;
            unsigned short pcOffset9 : 9;
        } ld_ldi_lea;

        struct {
            unsigned short destinationRegister : 3;
            unsigned short baseRegister : 3;
            unsigned short offset6 : 6;
        } ldr;

        struct {
            unsigned short destinationRegister : 3;
            unsigned short sourceRegister : 3;
        } not ;

        struct {
            unsigned short sourceRegister : 3;
            unsigned short pcOffset9 : 9;
        } st_sti;

        struct {
            unsigned short sourceRegister : 3;
            unsigned short baseRegister : 3;
            unsigned short offset6 : 6;
        } str;

        struct {
            unsigned short zeroZeroZeroZero : 4;
            unsigned short trapVector8 : 8;
        } trap;
    };
} EmulatorInstruction;

static inline void stepBr(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    if (instruction->br.nzp & state->cc) {
        state->pc += instruction->br.pcOffset9;
    }
}

static inline void stepAdd(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    short sr1 = state->registers[instruction->add_and.sr1];
    short sr2 = instruction->add_and.isImm ? instruction->add_and.imm5 : state->registers[instruction->add_and.imm5];
    short result = sr1 + sr2;

    state->registers[instruction->add_and.dr] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;
}

static inline void stepLd(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    state->registers[instruction->ld_ldi_lea.destinationRegister] = state->memory[address];
    state->cc = state->memory[address] == 0 ? 2 : state->memory[address] < 0 ? 4 : 1;
}

static inline void stepSt(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short address = state->pc + instruction->st_sti.pcOffset9;
    state->memory[address] = state->registers[instruction->st_sti.sourceRegister];
}
static inline void stepJsrJsrr(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short pc = state->pc;
    state->pc += instruction->jsr.pcOffset11;
    state->registers[7] = pc;
}
static inline void stepAnd(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    short sr1 = state->registers[instruction->add_and.sr1];
    short sr2 = instruction->add_and.isImm ? instruction->add_and.imm5 : state->registers[instruction->add_and.imm5];
    short result = sr1 & sr2;

    state->registers[instruction->add_and.dr] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;
}
static inline void stepLdr(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short base = state->registers[instruction->ldr.baseRegister];
    unsigned short offset = instruction->ldr.offset6;
    unsigned short address = base + offset;

    state->registers[instruction->ldr.destinationRegister] = state->memory[address];
    state->cc = state->memory[address] == 0 ? 2 : state->memory[address] < 0 ? 4 : 1;
}
static inline void stepStr(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short base = state->registers[instruction->str.baseRegister];
    unsigned short offset = instruction->str.offset6;
    unsigned short address = base + offset;

    state->memory[address] = state->registers[instruction->str.sourceRegister];
}
static inline void stepRti(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    fprintf(stderr, "RTI encountered!\n");
    exit(1);
}
static inline void stepNot(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    short source = state->registers[instruction->not.sourceRegister];
    short result = ~source;

    state->registers[instruction->not.destinationRegister] = result;
    state->cc = result == 0 ? 2 : result < 0 ? 4 : 1;
}
static inline void stepLdi(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    unsigned short indirectAddress = state->memory[address];
    state->registers[instruction->ld_ldi_lea.destinationRegister] = state->memory[indirectAddress];
    state->cc = state->memory[indirectAddress] == 0 ? 2 : state->memory[indirectAddress] < 0 ? 4 : 1;
}

static inline void stepSti(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short address = state->pc + instruction->st_sti.pcOffset9;
    unsigned short indirectAddress = state->memory[address];
    state->memory[indirectAddress] = state->registers[instruction->st_sti.sourceRegister];
}

static inline void stepJmp(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short base = state->registers[instruction->jmp_jsrr.baseRegister];
    state->pc = base;
}

static inline void stepRes(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    fprintf(stderr, "Reserved opcode encountered!\n");
    exit(1);
}

static inline void stepLea(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    unsigned short address = state->pc + instruction->ld_ldi_lea.pcOffset9;
    state->registers[instruction->ld_ldi_lea.destinationRegister] = address;
}

static inline void stepTrap(LC3EmulatorState *state, EmulatorInstruction *instruction) {
    printf("TRAP x%02X\n", instruction->trap.trapVector8);
    exit(1);
}

void step(LC3Context *ctx, LC3EmulatorState *state) {
    unsigned short pc = state->pc;
    state->pc++;

    unsigned short instruction = state->memory[pc];
    EmulatorInstruction *emulatorInstruction = (EmulatorInstruction *)&instruction;

    unsigned short opcode = emulatorInstruction->opcode;

    switch (opcode) {
        case 0:
            stepBr(state, emulatorInstruction);
            break;
        case 1:
            stepAdd(state, emulatorInstruction);
            break;
        case 2:
            stepLd(state, emulatorInstruction);
            break;
        case 3:
            stepSt(state, emulatorInstruction);
            break;
        case 4:
            stepJsrJsrr(state, emulatorInstruction);
            break;
        case 5:
            stepAnd(state, emulatorInstruction);
            break;
        case 6:
            stepLdr(state, emulatorInstruction);
            break;
        case 7:
            stepStr(state, emulatorInstruction);
            break;
        case 8:
            stepRti(state, emulatorInstruction);
            break;
        case 9:
            stepNot(state, emulatorInstruction);
            break;
        case 10:
            stepLdi(state, emulatorInstruction);
            break;
        case 11:
            stepSti(state, emulatorInstruction);
            break;
        case 12:
            stepJmp(state, emulatorInstruction);
            break;
        case 13:
            stepRes(state, emulatorInstruction);
            break;
        case 14:
            stepLea(state, emulatorInstruction);
            break;
        case 15:
            stepTrap(state, emulatorInstruction);
            break;
    }
}

void emulate(LC3Context ctx, LC3EmulatorState state) {
    while (!state.haltSignal) {
        printf("big steppa!! PC: x%4x\n", state.pc);
        usleep(100000);
        step(&ctx, &state);
    }
}

// void main() {
//     printf("%d", sizeof(EmulatorInstruction));
// }