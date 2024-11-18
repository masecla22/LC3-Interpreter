%{

#include <stdio.h>

void yyerror(char *msg);    // Function to handle parsing errors 
extern int yylex(void); // External function to get the next token (from lexer.fl)


%}

%{/** Tokens for LC-3 Base instructions */%}
%token ADD AND BR JMP JSR JSRR LD LDI LDR LEA NOT RET RTI ST STI STR TRAP

%{/** Tokens for LC-3 Pseudo instructions */%}
%token GETC OUT PUTS IN HALT

%{/** Tokens for LC-3 Condition flags */%}
%token P Z N PZ PN ZN PZN

%{/** Tokens for LC-3 Registers */%}
%token R0 R1 R2 R3 R4 R5 R6 R7

%{/** Tokens for LC-3 Directives */%}
%token ORIG FILL BLKW STRINGZ END

%{/** Tokens for LC-3 Literals */%}
%token DECIMAL_LITERAL
%token HEX_LITERAL

%{/** Miscellaneous tokens */%}
%token IDENTIFIER

%%

/* Overarching grammar rules */

Program : Statements;

Statements : Statement | Statements Statement;

Instruction : AddInstruction | AndInstruction 
              | BranchInstruction 
              | JumpInstruction | JumpSubroutineInstruction | JumpSubroutineRegisterInstruction 
              | LoadInstruction | LoadIndirectInstruction | LoadBaseOffsetInstruction | LoadEffectiveAddressInstruction 
              | NotInstruction 
              | ReturnInstruction | ReturnInterruptInstruction 
              | StoreInstruction | StoreIndirectInstruction | StoreBaseOffsetInstruction 
              | TrapInstruction 
              | GetCharacterInstruction | OutputInstruction | OutputStringInstruction | InputInstruction | HaltInstruction 
              | OriginDirective | FillDirective | BlockDirective | StringDirective | EndDirective;

Statement : Instruction | Label Instruction;

/* Low level definitions */
Register : R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7;
FlagSet : P | Z | N | PZ | PN | ZN | PZN | ;

Immediate : DECIMAL_LITERAL | HEX_LITERAL;

Label: IDENTIFIER;


/* Instruction definitions */
AddInstruction : ADD Register Register Register | ADD Register Register Immediate;
AndInstruction : AND Register Register Register | AND Register Register Immediate;

BranchInstruction : BR FlagSet Label;


JumpInstruction : JMP Register;
JumpSubroutineInstruction : JSR Label;
JumpSubroutineRegisterInstruction : JSRR Register;

LoadInstruction : LD Register Label;
LoadIndirectInstruction : LDI Register Label;
LoadBaseOffsetInstruction : LDR Register Register Immediate;
LoadEffectiveAddressInstruction : LEA Register Label;

NotInstruction : NOT Register Register;

ReturnInstruction : RET;
ReturnInterruptInstruction : RTI;

StoreInstruction : ST Register Label;
StoreIndirectInstruction : STI Register Label;
StoreBaseOffsetInstruction : STR Register Register Immediate;

TrapInstruction : TRAP Immediate;

/* Pseudo instruction definitions */

GetCharacterInstruction : GETC;
OutputInstruction : OUT;
OutputStringInstruction : PUTS;
InputInstruction : IN;
HaltInstruction : HALT;

/* Directive definitions */

OriginDirective : ORIG Immediate;
FillDirective : FILL Immediate;
BlockDirective : BLKW Immediate;
StringDirective : STRINGZ Immediate;
EndDirective : END;



%%

void yyerror(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}
