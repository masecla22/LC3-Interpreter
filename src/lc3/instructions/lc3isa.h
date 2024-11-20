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
    I_TRAP,
    M_GETC,
    M_OUT,
    M_PUTS,
    M_IN,
    M_PUTSP,
    M_HALT,
    D_ORIG,
    D_FILL,
    D_BLKW,
    D_STRINGZ,
    D_END
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
    unsigned int nzp;

    unsigned int pcOffset9;
} BranchInstruction;


typedef struct UnresolvedBranchInstruction {
    unsigned int nzp;

    int isResolved;

    union {
        unsigned int pcOffset9;
        char* label;
    };
} UnresolvedBranchInstruction;

typedef struct JumpSubroutineInstruction {
    unsigned int pcOffset11;
} JumpSubroutineInstruction;

typedef struct UnresolvedJumpSubroutineInstruction {
    int isResolved;

    union {
        unsigned int pcOffset11;
        char* label;
    };
} UnresolvedJumpSubroutineInstruction;

typedef struct JumpSubroutineRegisterInstruction {
    unsigned int baseRegister;
} JumpSubroutineRegisterInstruction;

typedef struct JumpInstruction {
    unsigned int sourceRegister;
} JumpInstruction;

typedef struct LoadInstruction {
    unsigned int destinationRegister;
    unsigned int pcOffset9;
} LoadInstruction;

typedef struct UnresolvedLoadInstruction {
    unsigned int destinationRegister;    
    int isResolved;

    union {
        unsigned int pcOffset9;
        char* label;
    };
} UnresolvedLoadInstruction;

typedef LoadInstruction LoadIndirectInstruction;  // LDI is the same as LD (in terms of fields)
typedef UnresolvedLoadInstruction UnresolvedLoadIndirectInstruction; // LDI is the same as LD (in terms of fields)

typedef struct LoadBaseOffsetInstruction {
    unsigned int destinationRegister;
    unsigned int baseRegister;
    unsigned int offset6;
} LoadBaseOffsetInstruction;

typedef LoadInstruction LoadEffectiveAddressInstruction;  // LEA is the same as LD (in terms of fields)
typedef UnresolvedLoadInstruction UnresolvedLoadEffectiveAddressInstruction;  // LEA is the same as LD (in terms of fields)

typedef struct NotInstruction {
    unsigned int destinationRegister;
    unsigned int sourceRegister;
} NotInstruction;

typedef struct StoreInstruction {
    unsigned int sourceRegister;
    unsigned int pcOffset9;
} StoreInstruction;

typedef struct UnresolvedStoreInstruction {
    unsigned int sourceRegister;
    int isResolved;

    union {
        unsigned int pcOffset9;
        char* label;
    };
} UnresolvedStoreInstruction;

typedef StoreInstruction StoreIndirectInstruction;  // STI is the same as ST (in terms of fields)
typedef UnresolvedStoreInstruction UnresolvedStoreIndirectInstruction;  // STI is the same as ST (in terms of fields)


typedef struct StoreBaseOffsetInstruction {
    unsigned int sourceRegister;
    unsigned int baseRegister;
    unsigned int offset6;
} StoreBaseOffsetInstruction;

typedef struct TrapInstruction {
    unsigned int trapVector8;
} TrapInstruction;

typedef struct OrigDirective {
    unsigned int address;
} OrigDirective;

typedef struct FillDirective {
    unsigned int value;
} FillDirective;

typedef struct BlkwDirective {
    unsigned int count;
} BlkwDirective;

typedef struct StringzDirective {
    char* string;
} StringzDirective;


typedef struct ParsedInstruction {
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
} ParsedInstruction;

typedef struct UnresolvedInstruction {
    InstructionType type;

    union {
        AddInstruction iAdd;
        AndInstruction iAnd;
        UnresolvedBranchInstruction iBr;
        JumpInstruction iJmp;
        UnresolvedJumpSubroutineInstruction iJsr;
        JumpSubroutineRegisterInstruction iJsrr;
        UnresolvedLoadInstruction iLd;
        UnresolvedLoadIndirectInstruction iLdi;
        LoadBaseOffsetInstruction iLdr;
        UnresolvedLoadEffectiveAddressInstruction iLea;
        
        NotInstruction iNot;
        // RET and RTI are not in the union because they don't have any fields
        UnresolvedStoreInstruction iSt;
        UnresolvedStoreIndirectInstruction iSti;
        StoreBaseOffsetInstruction iStr;

        TrapInstruction iTrap;

        // Macros are not in the union because they don't have any fields

        OrigDirective dOrig;
        FillDirective dFill;
        BlkwDirective dBlkw;
        StringzDirective dStringz;

        // END is not in the union because it doesn't have any fields
    };
} UnresolvedInstruction;

void printInstruction(UnresolvedInstruction instruction);

typedef struct LabelledInstruction {
    char* label;
    UnresolvedInstruction instruction;
} LabelledInstruction;

void printLabelledInstruction(LabelledInstruction instruction);

typedef struct LabelledInstructionList {
    LabelledInstruction* instructions;
    unsigned int count;
    unsigned int capacity;
} LabelledInstructionList;

LabelledInstructionList* createLabelledInstructionList(void);
void destroyLabelledInstructionList(LabelledInstructionList* list);
void addLabelledInstruction(LabelledInstructionList* list, LabelledInstruction instruction);

unsigned short lc3AssembleInstruction(ParsedInstruction instruction);
ParsedInstruction lc3DisassembleInstruction(unsigned short instruction);

#endif