%{

#include <stdio.h>

extern int yylex(void); // External function to get the next token (from lexer.fl)
extern void showErrorLine(); // Function to show the line where the error occurred (from lexer.fl)
extern void printToken(int token); // Function to print the token (from lexer.fl)

void yyerror(char *msg);    // Function to handle parsing errors


%}

%code requires {
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "../../src/lc3/instructions/lc3isa.h"
}


%union 
{
    int ival;
    char *sval;

    UnresolvedInstruction instruction;
}

%{/** Tokens for LC-3 Base instructions */%}
%token ADD AND JMP JSR JSRR LD LDI LDR LEA NOT RET RTI ST STI STR TRAP

%{/** Tokens for LC-3 Pseudo instructions */%}
%token GETC OUT PUTS IN HALT

%{/** Tokens for LC-3 Condition flags */%}
%token BR BR_P BR_Z BR_N BR_PZ BR_PN BR_ZN BR_PZN

%{/** Tokens for LC-3 Registers */%}
%token R0 R1 R2 R3 R4 R5 R6 R7

%{/** Tokens for LC-3 Directives */%}
%token ORIG FILL BLKW STRINGZ END

%{/** Tokens for LC-3 Literals */%}
%token <ival> DECIMAL_LITERAL HEX_LITERAL

%{/** Miscellaneous tokens */%}
%token <sval> IDENTIFIER
%type <ival> Register
%type <ival> Immediate

%type <instruction> TrapInstruction
%type <instruction> AddInstruction

%start Program

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

Statement : Label Instruction | Instruction;

/* Low level definitions */
Register : R0 {$$ = 0;} 
         | R1 {$$ = 1;} 
         | R2 {$$ = 2;} 
         | R3 {$$ = 3;} 
         | R4 {$$ = 4;} 
         | R5 {$$ = 5;} 
         | R6 {$$ = 6;} 
         | R7 {$$ = 7;};

Immediate : DECIMAL_LITERAL | HEX_LITERAL;

Label: IDENTIFIER;


/* Instruction definitions */
AddInstruction : ADD Register Register Register 
                {
                  UnresolvedInstruction instruction = {0};
                  instruction.type = I_ADD;
                  instruction.iAdd.destinationRegister = $2;
                  instruction.iAdd.sr1 = $3;
                  instruction.iAdd.sr2 = $4;
                  instruction.iAdd.isImm = 0;
                  $$ = instruction;
                }
               | ADD Register Register Immediate
                {
                  UnresolvedInstruction instruction = {0};
                  instruction.type = I_ADD;
                  instruction.iAdd.destinationRegister = $2;
                  instruction.iAdd.sr1 = $3;
                  instruction.iAdd.imm5 = $4;
                  instruction.iAdd.isImm = 1;
                  $$ = instruction;
                };

AndInstruction : AND Register Register Register | AND Register Register Immediate;

BranchBase : BR_P | BR_Z | BR_N | BR_PZ | BR_PN | BR_ZN | BR_PZN | BR;
BranchInstruction : 
        BranchBase Label
      | BranchBase Immediate;


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

TrapInstruction : TRAP Immediate {
  UnresolvedInstruction instruction = {0};
  instruction.type = I_TRAP;
  instruction.iTrap.trapVector8 = $2;
  $$ = instruction;
};

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

void printToken(int token){
  switch(token){
    case ADD: fprintf(stderr, "ADD"); break;
    case AND: fprintf(stderr, "AND"); break;
    case BR: fprintf(stderr, "BR"); break;
    case JMP: fprintf(stderr, "JMP"); break;
    case JSR: fprintf(stderr, "JSR"); break;
    case JSRR: fprintf(stderr, "JSRR"); break;
    case LD: fprintf(stderr, "LD"); break;
    case LDI: fprintf(stderr, "LDI"); break;
    case LDR: fprintf(stderr, "LDR"); break;
    case LEA: fprintf(stderr, "LEA"); break;
    case NOT: fprintf(stderr, "NOT"); break;
    case RET: fprintf(stderr, "RET"); break;
    case RTI: fprintf(stderr, "RTI"); break;
    case ST: fprintf(stderr, "ST"); break;
    case STI: fprintf(stderr, "STI"); break;
    case STR: fprintf(stderr, "STR"); break;
    case TRAP: fprintf(stderr, "TRAP"); break;
    case GETC: fprintf(stderr, "GETC"); break;
    case OUT: fprintf(stderr, "OUT"); break;
    case PUTS: fprintf(stderr, "PUTS"); break;
    case IN: fprintf(stderr, "IN"); break;
    case HALT: fprintf(stderr, "HALT"); break;
    case ORIG: fprintf(stderr, "ORIG"); break;
    case FILL: fprintf(stderr, "FILL"); break;
    case BLKW: fprintf(stderr, "BLKW"); break;
    case STRINGZ: fprintf(stderr, "STRINGZ"); break;
    case END: fprintf(stderr, "END"); break;
    case HEX_LITERAL: fprintf(stderr, "HEX_LITERAL"); break;
    case DECIMAL_LITERAL: fprintf(stderr, "DECIMAL_LITERAL"); break;
    case IDENTIFIER: fprintf(stderr, "IDENTIFIER"); break;
    case R0: fprintf(stderr, "R0"); break;
    case R1: fprintf(stderr, "R1"); break;
    case R2: fprintf(stderr, "R2"); break;
    case R3: fprintf(stderr, "R3"); break;
    case R4: fprintf(stderr, "R4"); break;
    case R5: fprintf(stderr, "R5"); break;
    case R6: fprintf(stderr, "R6"); break;
    case R7: fprintf(stderr, "R7"); break;
    case BR_P: fprintf(stderr, "BR_P"); break;
    case BR_Z: fprintf(stderr, "BR_Z"); break;
    case BR_N: fprintf(stderr, "BR_N"); break;
    case BR_PZ: fprintf(stderr, "BR_PZ"); break;
    case BR_PN: fprintf(stderr, "BR_PN"); break;
    case BR_ZN: fprintf(stderr, "BR_ZN"); break;
    case BR_PZN: fprintf(stderr, "BR_PZN"); break;
    default: fprintf(stderr, "UNKNOWN TOKEN");
  }
}

void yyerror(char *msg) {
  showErrorLine();
  fprintf(stderr, "%s (detected at token=", msg);
  printToken(yychar);
  fprintf(stderr, ").\n");

  printf("ERRORS: 1\nREJECTED\n");
}