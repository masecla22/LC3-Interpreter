#ifndef LC3_ISA_H
#define LC3_ISA_H

typedef enum InstructionType {
    I_ADD,
    I_AND,
    I_BR,
    I_JMP,
    I_JSR,
    I_JSRR,
    I_LD,
    I_LDI,
    I_LDR,
    I_LEA,
    I_NOT,
    I_RET,
    I_RTI,
    I_ST,
    I_STI,
    I_STR,
    I_TRAP
} InstructionType;

typedef struct AddInstruction {
    unsigned int destinationRegister;
    unsigned int sr1;

    unsigned int isImm;

    union {
        unsigned int sr2;
        unsigned int imm5;
    };
} AddInstruction;

// AndInstruction is the same as AddInstruction (in terms of fields)
typedef AddInstruction AndInstruction;

typedef struct BranchInstruction {
    unsigned int negative;
    unsigned int zero;
    unsigned int positive;

    unsigned int pcOffset9;
} BranchInstruction;

typedef struct JumpInstruction {
    unsigned int sourceRegister;
} JumpInstruction;

typedef struct JumpSubroutineInstruction {
    unsigned int pcOffset11;
} JumpSubroutineInstruction;

typedef struct JumpSubroutineRegisterInstruction {
    unsigned int baseRegister;
} JumpSubroutineRegisterInstruction;

typedef struct LoadInstruction {
    unsigned int destinationRegister;
    unsigned int pcOffset9;
} LoadInstruction;

typedef LoadInstruction LoadIndirectInstruction;  // LDI is the same as LD (in terms of fields)

typedef struct LoadBaseOffsetInstruction {
    unsigned int destinationRegister;
    unsigned int baseRegister;
    unsigned int offset6;
} LoadBaseOffsetInstruction;

typedef LoadInstruction LoadEffectiveAddressInstruction;  // LEA is the same as LD (in terms of fields)

typedef struct NotInstruction {
    unsigned int destinationRegister;
    unsigned int sourceRegister;
} NotInstruction;

typedef struct StoreInstruction {
    unsigned int sourceRegister;
    unsigned int pcOffset9;
} StoreInstruction;

typedef StoreInstruction StoreIndirectInstruction;  // STI is the same as ST (in terms of fields)

typedef struct StoreBaseOffsetInstruction {
    unsigned int sourceRegister;
    unsigned int baseRegister;
    unsigned int offset6;
} StoreBaseOffsetInstruction;

typedef struct TrapInstruction {
    unsigned int trapVector8;
} TrapInstruction;

typedef struct LC3ParsedInstruction {
    InstructionType type;

    union {
        AddInstruction iAdd;
        AndInstruction iAnd;
        BranchInstruction iBr;
        JumpInstruction iJmp;
        JumpSubroutineInstruction iJsr;
        JumpSubroutineRegisterInstruction iJsrr;
        LoadInstruction iLd;
        NotInstruction iNot;
        // RET and RTI are not in the union because they don't have any fields
        StoreInstruction iSt;
        TrapInstruction iTrap;
    };
} LC3ParsedInstruction;

unsigned short lc3AssembleInstruction(LC3ParsedInstruction instruction);
LC3ParsedInstruction lc3DisassembleInstruction(unsigned short instruction);


#endif