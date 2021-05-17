/***************************************************************/
/*                                                             */
/*                  RISCV-LC Assembler                         */
/*                                                             */
/*                     CEG3420 Lab2                            */
/*                 cbai@cse.cuhk.edu.hk                        */
/*           The Chinese University of Hong Kong               */
/*                                                             */
/***************************************************************/

/*
 * Header files
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>

/*
 * Macros
 */
#define MAX_LABELS 255
#define MAX_LABEL_LENGTH 20
#define MAX_LINE_LENGTH 255
#define MASK11_0(x) ((x) & 0xFFF)
/*
 * DEBUG is useful when you debug the program
 * you can just uncomment it and inject `#ifdef`
 * to your program and debug it
 */
// #define DEBUG 0

/*
 * Enumerations
 */
/*
 * label identifiers
 * you can check it in the function `is_label`
 */
enum {
    FIRST_CHAR_ERROR, OTHER_CHAR_ERROR, VALID_LABEL
};
/*
 * source code line identifiers
 * you can check it in the function `parse_cmd`
 */
enum {
    DONE, OK, EMPTY_LINE
};
/*
 * RV32I instruction identifiers
 * you can check it in the function `is_opcode`
 */
enum {
    ADDI, SLLI, XORI, SRLI, SRAI, ORI, ANDI, LUI, ADD, SUB, SLL, XOR,
    SRL, SRA, OR, AND, JALR, JAL, BEQ, BNE, BLT, BGE, LB, LH, LW,
    SB, SH, SW, LI, LA, FILL, HALT, NOTMATCH
};

/*
 * Global variables
 */
/* aliases for general-purpose registers */
const char * regs[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "t3", "t4",
    "t5", "t6", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "fp", "a0", "a1", "a2", "a3", "a4",
    "a5", "a6", "a7", "x0", "x1", "x2", "x3", "x4", "x5", "x6",
    "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
    "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24",
    "x25", "x26", "x27", "x28", "x29", "x30", "x31", NULL
};
/* #num for general-purpose registers */
const int regs_code[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 28, 29, 30, 31, 8, 9, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 8, 10, 11, 12, 13, 14, 15, 16, 17, 0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 3
};
/*
 * `TableEntry` stores the label table
 * `label` saves labels in original assembler source codes
 * `address` saves line numbers of labels
 * `lineCnt` saves 1st line number when the label emerges in the source code
 * `fill` distinguishes between a FILL definition and normal labels
 */
typedef struct label_table {
    int lineCnt;
    int address;
	int fill;
    char label[MAX_LABEL_LENGTH + 1];
} TableEntry;
/*
 * `PExpression` supports parentheses expresion
 * e.g., 8(t0)
 */
typedef struct parenthesized_expression {
    int imm;
    char reg[5];
} PExpression;
/*
 * Currently, LA is the only pseudo instruction that is tranlated
 * to two sequential base instructions, we need global variables
 * to supprt this machanism.
 * `la_cnt` saves #num of LA instructions in the source code
 * `la_cmd` accumulates #num of LA instructions according to the
 * line number
 */
int la_cnt;
int la_cmd[MAX_LINE_LENGTH];

/*
 * Function eclarations
 */
/* is it a general-purpose register? */
int is_reg(char *);
/* return the #num of a general-purpose register if it is legal */
int reg2num(char *, int);
/* is it an immediate value? */
int is_imm(char *);
/* is the immediate value in a suitable range? */
int is_imm_in_range(int, int);
/*
 * handle labels & immediate values
 * e.g., LA instruction
 */
int handle_label_imm(char *, TableEntry *, int, int);
/* validate if it is an immediate value
 * tips: compare between `validate_imm` and `is_imm`
 */
int validate_imm(char *, int, int);
/* is it a label? */
int is_label(char *);
/* get the address of the label */
int label_val(char *, int, TableEntry *, int, int);
/* validate arguments  */
void validate_args(int, char *, char *, char *, int);
/*
 * handle parentheses expresion
 * e.g., 8(t0)
 */
PExpression * parse_parenthesized_expression(char *, int);
/*
 * handle pseudo instructions
 */
void handle_pseudo_cmd(FILE *, int , int , char *, char *, char *, char *, TableEntry *);
/* translate from RV32I instructions to machine codes */
int cmd2code(int, int, char *, char *, char *, char *, TableEntry *);
/* is it an operation code? */
int is_opcode(char *);
/* calulates LA instructions
 * tips: refer to `la_cnt` and `la_cnt`
 */
void calc_la(FILE *);
/* parse RV32I instructions */
int parse_cmd(FILE *, char *, char **, char **, char **, char **, char **);
/* handle RV32I instructions */
void handle_cmd(char *, char *);
/* handle errors */
void handle_err(int, int);
/*
 * handle errors
 * tips: compare between `handle_err` and `throw_error`
 */
void throw_error(int, const char *, ...);
