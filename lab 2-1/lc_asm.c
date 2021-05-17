/***************************************************************/
/*                                                             */
/*                  RISCV-LC Assembler                         */
/*                                                             */
/*                     CEG3420 Lab2                            */
/*                 cbai@cse.cuhk.edu.hk                        */
/*           The Chinese University of Hong Kong               */
/*                                                             */
/***************************************************************/

#include "lc_asm.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: asm <a *.asm file>\n");
        throw_error(4, "%s: error: lack of input file or output file", argv[0]);
    }

    handle_cmd(argv[1], argv[2]);

    return 0;
}

int is_reg(char *reg) {
    int i = 0;
    while (regs[i] != NULL) {
        if (!strncmp(reg, regs[i], strlen(regs[i])) && strlen(reg) == strlen(regs[i]))
            return 1;
        i++;
    }
    return 0;
}

int reg2num(char *reg, int lineNum) {
    int i = 0;
    if (!is_reg(reg))
        handle_err(7, lineNum);
    while (regs[i] != NULL) {
        if (!strncmp(reg, regs[i], strlen(regs[i])) && strlen(reg) == strlen(regs[i]))
            return regs_code[i];
        i++;
    }
}

int is_imm(char *immNum) {
    int i = 0, l = strlen(immNum), hex = 0;
    if (l > 3 && immNum[0] == '-' && immNum[1] == '0' && (tolower(immNum[2]) == 'x')) {
        i += 3;
        hex = 1;
    } else if (l > 2 && immNum[0] == '0' && (tolower(immNum[1]) == 'x')) {
        i += 2;
        hex = 1;
    } else if (l > 1 && immNum[0] == '-') {
        i += 1;
    }
    while (i < l && isxdigit(immNum[i]))
        i++;
    if (i == l)
        return 1 + hex;
    else
        return 0;
}

int is_imm_in_range(int val, int base) {
    
    int i = ((1 << base) - 1), j = -(1 << base);
    i = (val <= i); j = (val >= j);
    //printf("val = %d, returns = %d and %d\n", val,  i, j);
    //return (val <= ((1 << base) - 1) && val >= -(1 << base));
    return i && j;
}

int handle_label_imm(char *imm, TableEntry *pLabel, int cmdNum, int lineNum) {
    int val, ret;
    if (!(ret = is_imm(imm))) {
        // labal
        return label_val(imm, 20, pLabel, cmdNum, lineNum);
    } else {
        // immediate value
        val = (int)strtol(imm, NULL, (ret == 2) ? 16 : 10);
        if (is_imm_in_range(val, 19))
            return val;
        else
            handle_err(5, lineNum);
    }
}

int validate_imm(char *imm, int check, int lineNum) {
    int val, ret;
    if (!(ret = is_imm(imm)))
        handle_err(7, lineNum);
    else{
        val = (int)strtol(imm, NULL, (ret == 2) ? 16 : 10);
        //printf("value = %d\n", val);
    }
    if (check != -1) {
        if (is_imm_in_range(val, check))
            return val;
        else
            handle_err(3, lineNum);
    } else
        return val;
}

int lower5bit(char *imm, int lineNum) {
    int val, ret;
    if (!(ret = is_imm(imm)))
        handle_err(7, lineNum);
    else
        val = ((int)strtol(imm, NULL, (ret == 2) ? 16 : 10) & 0x1F);
    if (is_imm_in_range(val, 6))
        return val;
    else
        handle_err(5, lineNum);
}

int is_label(char *lLabel) {
    char *pLabel = lLabel;

    if (isdigit(*pLabel))
        return FIRST_CHAR_ERROR;
    else {
        pLabel++;
        while (pLabel != NULL && *pLabel != '\0') {
            /* all characters have been converted to lower-case */
            if (isalnum(*pLabel)) {
                pLabel++;
                continue;
            }
            else
                return OTHER_CHAR_ERROR;
        }
    }
    return VALID_LABEL;
}

int label_val(char *labelName, int labelWid, TableEntry *pLabel, int cmdNum, int lineNum) {
    if (is_label(labelName) != VALID_LABEL)
        handle_err(10, lineNum);
    TableEntry *tPtr = pLabel;
    while (*(tPtr->label) != '\0') {
        if (strncmp(tPtr->label, labelName, strlen(tPtr->label)) == 0 && \
            strlen(labelName) == strlen(tPtr->label)) {
            int offset = tPtr->address;
            if (tPtr->fill)
                offset += la_cnt;
            else
                offset += la_cmd[cmdNum];
            offset <<= 2;
            if (offset >= (1 << labelWid) || offset < -(1 << labelWid))
                handle_err(8, lineNum);
            return offset;
        }
        tPtr++;
    }
    handle_err(1, lineNum);
}

void validate_args(int opNum, char *pArg1, char *pArg2, char *pArg3, int lineNum) {
    switch (opNum) {
        case 0:
            if (!(pArg1 == NULL && pArg2 == NULL && pArg3 == NULL))
                handle_err(5, lineNum);
            break;
        case 1:
            if (!(pArg1 != NULL && pArg2 == NULL && pArg3 == NULL))
                handle_err(5, lineNum);
            break;
        case 2:
            if (!(pArg1 != NULL && pArg2 != NULL && pArg3 == NULL))
                handle_err(5, lineNum);
            break;
        case 3:
            if (!(pArg1 != NULL && pArg2 != NULL && pArg3 != NULL))
                handle_err(5, lineNum);
            break;
        default:
            printf("Illegal use of 'validate_args' Function!\n");
    }
}

PExpression * parse_parenthesized_expression(char *pArg2, int lineNum) {
    PExpression * ret;
    char * reg, * imm;
    char temp[MAX_LINE_LENGTH + 1];
    int i = 0;
    if ((ret = (PExpression *)malloc(sizeof(ret))) == NULL)
            handle_err(4, lineNum);
    while (pArg2[i] != '\0') {
        temp[i] = pArg2[i];
        i++;
    }
    temp[i] = '\0';
    ret->imm = 100;
    imm = strtok(temp, "( ");
    ret->imm = validate_imm(imm, 12, lineNum);
    reg = strtok(NULL, ")");
    strncpy(ret->reg, reg, strlen(reg));
    ret->reg[strlen(reg)] = '\0';
    return ret;
}

void handle_pseudo_cmd(FILE *file, int lineNum, int cmdNum, char *pOpcode, char *pArg1, char *pArg2,
    char *pArg3, TableEntry *pLabel) {
    if (!strncmp(pOpcode, "la", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         */
        // printf("Lab2-1 assignment: LA instruction\n");
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        int lui_instr = cmd2code(lineNum, cmdNum, "lui", pArg1, "0x0", pArg3, pLabel);
        
        int val = label_val(pArg2, 12, pLabel, cmdNum, lineNum);
        
        char str[50];
        int k = handle_label_imm(pArg2, pLabel, cmdNum, lineNum);
        sprintf(str, "%d", k & 0xfff);
        
        int addi_instr = cmd2code(lineNum, cmdNum, "addi", pArg1, pArg1, str, pLabel);
        fprintf(file, "0x%08x\n0x%08x\n", lui_instr, addi_instr);
        
    } else if (!strncmp(pOpcode, ".fill", 5) && strlen(pOpcode) == 5) {
        validate_args(1, pArg1, pArg2, pArg3, lineNum);
        //printf("validate = %s\n", pArg1);
        fprintf(file, "0x%08x\n", validate_imm(pArg1, 31, lineNum));
    } else if (!strncmp(pOpcode, "halt", 4) && strlen(pOpcode) == 4) {
        validate_args(0, pArg1, pArg2, pArg3, lineNum);
        fprintf(file, "0x%08x\n", 0x0000707f);
    }
}

int cmd2code(int lineNum, int cmdNum, char *pOpcode, char *pArg1, char *pArg2,
    char *pArg3, TableEntry *pLabel) {
    /* save the translated Machine Code */
    int code = 0;
    // Integer Register-Immediate Instructions
    if (!strncmp(pOpcode, "addi", 4) && strlen(pOpcode) == 4) {
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (MASK11_0(validate_imm(pArg3, 12, lineNum)) << 20);
        printf("Lab2-1 assignment: ADDI instruction\n");
    } else if (!strncmp(pOpcode, "slli", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `lower5bit`
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code += (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (1 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (validate_imm(pArg3, 12, lineNum) << 20);
        printf("Lab2-1 assignment: SLLI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "xori", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (4 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (MASK11_0(validate_imm(pArg3, 12, lineNum)) << 20);
        printf("Lab2-1 assignment: XORI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "srli", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `lower5bit`
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (5 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (validate_imm(pArg3, 12, lineNum) << 20);
        printf("Lab2-1 assignment: SRLI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "srai", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `lower5bit`
         */
        code += (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (5 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (validate_imm(pArg3, 12, lineNum) << 20);
        code += (16 << 26);
        printf("Lab2-1 assignment: SRAI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "ori", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (6 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (MASK11_0(validate_imm(pArg3, 12, lineNum)) << 20);
        printf("Lab2-1 assignment: ORI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "andi", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x04 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (7 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (MASK11_0(validate_imm(pArg3, 12, lineNum)) << 20);
        printf("Lab2-1 assignment: ANDI instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "lui", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        code = (0x0D << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        int k = validate_imm(pArg2, 20, lineNum);
        code += (k << 12);
        //printf("error: after validate in lui \n");
        printf("Lab2-1 assignment: LUI instruction\n");
        //exit(1);
    }

    // Integer Register-Register Operations
    else if (!strncmp(pOpcode, "add", 3) && strlen(pOpcode) == 3) {
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
    } else if (!strncmp(pOpcode, "sub", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        code += (32 << 25); //31..25=32
        printf("Lab2-1 assignment: SUB instruction\n");
        // printf("%d\n", code);
        //exit(1);
    } else if (!strncmp(pOpcode, "sll", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (1 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        printf("Lab2-1 assignment: SLL instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "xor", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (4 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        printf("Lab2-1 assignment: XOR instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "srl", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (5 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        printf("Lab2-1 assignment: SRL instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "sra", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (5 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        code += (32 << 25); // 31..25=32
        printf("Lab2-1 assignment: SRA instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "or", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (6 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        printf("Lab2-1 assignment: OR instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "and", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x0C << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (7 << 12);
        code += (reg2num(pArg2, lineNum) << 15);
        code += (reg2num(pArg3, lineNum) << 20);
        printf("Lab2-1 assignment: AND instruction\n");
        //exit(1);
    }

    // Unconditional Jumps
    else if (!strncmp(pOpcode, "jalr", 4) && strlen(pOpcode) == 4) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code = (0x19 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (temp->imm << 20);
        printf("Lab2-1 assignment: JALR instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "jal", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `handle_label_imm`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        code = (0x1b << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7); //rd
        // width of 20
        int val = handle_label_imm(pArg2, pLabel, cmdNum, lineNum);

        // imm ok
        //imm[19:12]
        code += (val & 0xFF000);
        // printf("[19:12] = %d ", (imm & 0xFF000));

        // imm[11]
        code += (val & 0x800) << 9;
        // printf(" [11] = %d ", (imm & 0x800));

        // imm[10:1]
        code += (val & 0x7FE) << 20;
        // printf(" [10:1] = %d ", (imm & 0x7FE));

        // imm[20]
        code += (val & 0x100000) << 11;
        // printf(" [20] = %d\n", (imm & 0x100000));
        
        printf("Lab2-1 assignment: JAL instruction\n");
        //exit(1);
    }

    // Conditional Branches
    else if (!strncmp(pOpcode, "beq", 3) && strlen(pOpcode) == 3) {
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code = (0x18 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 15);
        code += (reg2num(pArg2, lineNum) << 20);
        int offset;
        offset = label_val(pArg3, 12, pLabel, cmdNum, lineNum);
        //printf("offset = 0x%08x\n", offset);
        
        // imm[11] 0x800 => 11 zeros
        code += ((offset & 0x800) >> 4);
        // imm[4:1] 0x1e => 1 zero
        code += ((offset & 0x1E) << 7);
        // imm[10:5] 0x7e0 => 5 zeros
        code += ((offset & 0x7E0) << 20);
        // imm[12] 0x1000 => 12 zeros
        code += ((offset & 0x1000) << 19);
    } else if (!strncmp(pOpcode, "bne", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        printf("Lab2-1 assignment: BNE instruction\n");
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code += (0x18 << 2) + 0x03;
        code += (1 << 12);
        code += (reg2num(pArg1, lineNum) << 15);
        code += (reg2num(pArg2, lineNum) << 20);
        
        int offset;
        offset = label_val(pArg3, 12, pLabel, cmdNum, lineNum);
        //printf("offset = 0x%08x\n", offset);
        // imm[11] 
        code += ((offset & 0x800) >> 4);
        //printf("imm[11] = 0x%08x\n", (offset & 0x800));
        // imm[4:1]
        code += ((offset & 0x1E) << 7);
        //printf("imm[4:1] = 0x%08x\n", (offset & 0x1E));
        // imm[10:5]
        code += ((offset & 0x7E0) << 20);
        //printf("imm[10 5] = 0x%08x\n", (offset & 0x7E0));
        // imm[12]
        code += ((offset & 0x1000) << 19);
        //printf("imm[12] = 0x%08x\n", (offset & 0x1000));
        //exit(1);
        //printf("code = 0x%08x\n", code);
    } else if (!strncmp(pOpcode, "blt", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        printf("Lab2-1 assignment: BLT instruction\n");
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code += (0x18 << 2) + 0x03;
        code += (4 << 12);
        code += (reg2num(pArg1, lineNum) << 15);
        code += (reg2num(pArg2, lineNum) << 20);
       
        int offset;
        offset = label_val(pArg3, 12, pLabel, cmdNum, lineNum);
        //printf("offset = 0x%08x\n", offset);
        // imm[11] // changes made here
        code += ((offset & 0x800) >> 4);
        // imm[4:1]
        code += ((offset & 0x1E) << 7);
        // imm[10:5]
        code += ((offset & 0x7E0) << 20);
        // imm[12]
        code += ((offset & 0x1000) << 19);
        
        //exit(1);
    } else if (!strncmp(pOpcode, "bge", 3) && strlen(pOpcode) == 3) {
        /*
         * Lab2-1 assignment
         */
        printf("Lab2-1 assignment: BGE instruction\n");
        validate_args(3, pArg1, pArg2, pArg3, lineNum);
        code += (0x18 << 2) + 0x03;
        code += (5 << 12);
        code += (reg2num(pArg1, lineNum) << 15);
        code += (reg2num(pArg2, lineNum) << 20);
        
        int offset;
        offset = label_val(pArg3, 12, pLabel, cmdNum, lineNum);
        //printf("offset = 0x%08x\n", offset);
         // imm[11]
        code += ((offset & 0x800) >> 4);
        // imm[4:1]
        code += ((offset & 0x1E) << 7);
        // imm[10:5]
        code += ((offset & 0x7E0) << 20);
        // imm[12]
        code += ((offset & 0x1000) << 19);
        //exit(1);
    }

    // Load and Store Instructions
    else if (!strncmp(pOpcode, "lb", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        code = (0x00 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (temp->imm << 20);
        printf("Lab2-1 assignment: LB instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "lh", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        code = (0x00 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (1 << 12);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (temp->imm << 20);
        printf("Lab2-1 assignment: LH instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "lw", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        code = (0x00 << 2) + 0x03;
        code += (reg2num(pArg1, lineNum) << 7);
        code += (2 << 12);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (temp->imm << 20);
        printf("Lab2-1 assignment: LW instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "sb", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code = (0x08 << 2) + 0x03;
        // imm [4:0]
        code += ((temp->imm & 0x1F) << 7);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (reg2num(pArg1, lineNum) << 20);
        //imm[11:5] 0xfe0 have 5 zeros
        code += ((temp->imm & 0xFE0) << 20);
        printf("Lab2-1 assignment: SB instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "sh", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code = (0x08 << 2) + 0x03;
        // imm [4:0]
        code += ((temp->imm & 0x1F) << 7);
        code += (0x01 << 12);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (reg2num(pArg1, lineNum) << 20);
        //imm[11:5] 0xfe0 have 5 zeros
        code += ((temp->imm & 0xFE0) << 20);
        printf("Lab2-1 assignment: SH instruction\n");
        //exit(1);
    } else if (!strncmp(pOpcode, "sw", 2) && strlen(pOpcode) == 2) {
        /*
         * Lab2-1 assignment
         * tips: refer to the function `parse_parenthesized_expression`
         */
        validate_args(2, pArg1, pArg2, pArg3, lineNum);
        PExpression *temp = parse_parenthesized_expression(pArg2, lineNum);
        code = (0x08 << 2) + 0x03;
        // imm [4:0]
        code += ((temp->imm & 0x1F) << 7);
        code += (2 << 12);
        code += (reg2num(temp->reg, lineNum) << 15);
        code += (reg2num(pArg1, lineNum) << 20);
        //imm[11:5] 0xfe0 have 5 zeros
        code += ((temp->imm & 0xFE0) << 20);
        printf("Lab2-1 assignment: SW instruction\n");
        //exit(1);
    }
    //printf("0x%08x\n", code);
    return code;
}

int is_opcode(char *lPtr) {
    if (strncmp(lPtr, "addi", 4) == 0 && strlen(lPtr) == 4)
        return ADDI;
    else if (strncmp(lPtr, "slli", 4) == 0 && strlen(lPtr) == 4)
        return SLLI;
    else if (strncmp(lPtr, "xori", 4) == 0 && strlen(lPtr) == 4)
        return XORI;
    else if (strncmp(lPtr, "srli", 4) == 0 && strlen(lPtr) == 4)
        return SRLI;
    else if (strncmp(lPtr, "srai", 4) == 0 && strlen(lPtr) == 4)
        return SRAI;
    else if (strncmp(lPtr, "ori", 3) == 0 && strlen(lPtr) == 3)
        return ORI;
    else if (strncmp(lPtr, "andi", 4) == 0 && strlen(lPtr) == 4)
        return ANDI;
    else if (strncmp(lPtr, "lui", 3) == 0 && strlen(lPtr) == 3)
        return LUI;
    else if (strncmp(lPtr, "add", 3) == 0 && strlen(lPtr) == 3)
        return ADD;
    else if (strncmp(lPtr, "sub", 3) == 0 && strlen(lPtr) == 3)
        return SUB;
    else if (strncmp(lPtr, "sll", 3) == 0 && strlen(lPtr) == 3)
        return SLL;
    else if (strncmp(lPtr, "xor", 3) == 0 && strlen(lPtr) == 3)
        return XOR;
    else if (strncmp(lPtr, "srl", 3) == 0 && strlen(lPtr) == 3)
        return SRL;
    else if (strncmp(lPtr, "sra", 3) == 0 && strlen(lPtr) == 3)
        return SRA;
    else if (strncmp(lPtr, "or", 2) == 0 && strlen(lPtr) == 2)
        return OR;
    else if (strncmp(lPtr, "and", 3) == 0 && strlen(lPtr) == 3)
        return AND;
    else if (strncmp(lPtr, "jalr", 4) == 0 && strlen(lPtr) == 4)
        return JALR;
    else if (strncmp(lPtr, "jal", 3) == 0 && strlen(lPtr) == 3)
        return JAL;
    else if (strncmp(lPtr, "beq", 3) == 0 && strlen(lPtr) == 3)
        return BEQ;
    else if (strncmp(lPtr, "bne", 3) == 0 && strlen(lPtr) == 3)
        return BNE;
    else if (strncmp(lPtr, "blt", 3) == 0 && strlen(lPtr) == 3)
        return BLT;
    else if (strncmp(lPtr, "bge", 3) == 0 && strlen(lPtr) == 3)
        return BGE;
    else if (strncmp(lPtr, "lb", 2) == 0 && strlen(lPtr) == 2)
        return LB;
    else if (strncmp(lPtr, "lh", 2) == 0 && strlen(lPtr) == 2)
        return LH;
    else if (strncmp(lPtr, "lw", 2) == 0 && strlen(lPtr) == 2)
        return LW;
    else if (strncmp(lPtr, "sb", 2) == 0 && strlen(lPtr) == 2)
        return SB;
    else if (strncmp(lPtr, "sh", 2) == 0 && strlen(lPtr) == 2)
        return SH;
    else if (strncmp(lPtr, "sw", 2) == 0 && strlen(lPtr) == 2)
        return SW;
    else if (strncmp(lPtr, "la", 2) == 0 && strlen(lPtr) == 2)
        return LA;
    else if (strncmp(lPtr, ".fill", 5) == 0 && strlen(lPtr) == 5)
        return FILL;
    else if (strncmp(lPtr, "halt", 4) == 0 && strlen(lPtr) == 4)
        return HALT;
    else
        return NOTMATCH;
}

void calc_la(FILE *pInfile) {
    char pLine[MAX_LINE_LENGTH + 1];
    char *lPtr;
    int lineCnt = 0, k = 0;
    while (fgets(pLine, MAX_LINE_LENGTH, pInfile) != NULL) {
        for (int i = 0; i < strlen(pLine); i++)
            pLine[i] = tolower(pLine[i]);
        lPtr = pLine;
        while (*lPtr != '#' && *lPtr != '\0' && *lPtr != '\n' && *lPtr != '\r')
            lPtr++;

        *lPtr = '\0';
        if (!(lPtr = strtok(pLine, "\t\n ,")))
            continue;
        if (is_opcode(lPtr) == LA )
            la_cmd[lineCnt] = ++k;
        else {
            if (!(lPtr = strtok(NULL, "\t\n ,")))
                continue;
            if (is_opcode(lPtr) == LA)
                la_cmd[lineCnt] = ++k;
            else
                la_cmd[lineCnt] = k;
        }
        lineCnt++;
    }
    la_cnt = k;
}

int parse_cmd(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode,
    char ** pArg1, char ** pArg2, char ** pArg3) {
    char *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return(DONE);

    /* convert entire line to lowercase characters */
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* make the pointers point to the address of a '\0' */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != '#' && *lPtr != '\0' && *lPtr != '\n'  && *lPtr != '\r')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return(EMPTY_LINE);

    if (is_opcode(lPtr) == NOTMATCH && is_label(lPtr) == VALID_LABEL) {
        /* a label is found */
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
            return(OK);
    }
    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return(OK);
    *pArg1 = lPtr;
   
    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return(OK);
    *pArg2 = lPtr;
    
    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return(OK);
    *pArg3 = lPtr;

    return(OK);
}

void handle_cmd(char *infileName, char *outfileName) {
    FILE *lInfile = fopen(infileName, "r");
    FILE *lOutfile = fopen(outfileName, "w");
    char lLine[MAX_LINE_LENGTH + 1];
    char *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3;
    int lRet;
    int cmdCnt = 0;
    int lineCnt = 0;
    TableEntry labelTable[MAX_LABELS];
    TableEntry *pLabelEntry = labelTable;
    int i, j;

    if (lInfile == NULL)
        throw_error(4, "Error: cannot open input file %s", infileName);

    /* initialize `labelTable` */
    for (i = 0; i < MAX_LABELS; i++, pLabelEntry++)
        pLabelEntry->label[0] = '\0';
    pLabelEntry = labelTable;

    printf("Processing input file %s\n", infileName);
    printf("Writing result to output file %s\n", outfileName);

    do {
        lRet = parse_cmd(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3);
        if (lRet != DONE && lRet != EMPTY_LINE) {
            if (lLabel != NULL && lLabel[0] != '\0') {
                if ((i = is_label(lLabel)) == VALID_LABEL) {
                    /* check duplicated cases */
                    for (j = 0; j < MAX_LABELS; j++) {
                        if ('\0' == labelTable[j].label[0])
                            continue;
                        if (strncmp(lLabel, labelTable[j].label, strlen(labelTable[j].label)) == 0 && \
                            strlen(lLabel) == strlen(labelTable[j].label)) {
                            throw_error(4, "Error: line %d: current label has a conflit with another label in line %d",
                                lineCnt + 1, labelTable[j].lineCnt + 1);
                        }
                    }
                    /* add `lLabel` to `pLabelEntry` */
                    if (is_opcode(lOpcode) == FILL)
                        pLabelEntry->fill = 1;
                    strncpy(pLabelEntry->label, lLabel, MAX_LABEL_LENGTH);
                    pLabelEntry->label[MAX_LABEL_LENGTH] = '\0';
                    pLabelEntry->address = cmdCnt;
                    pLabelEntry->lineCnt = lineCnt;
                    pLabelEntry++;
                }
                else if (i == FIRST_CHAR_ERROR)
                    throw_error(4, "Error: line %d: the first character of a valid label should not be a number",
                        lineCnt + 1);
                else if (i == OTHER_CHAR_ERROR)
                    throw_error(4, "Error: line %d: a valid label is supposed to be consist of alphanumeric",
                        lineCnt + 1);
            }
            cmdCnt++;
            if (strncmp(lOpcode, "HALT", 4) == 0 && strlen(lOpcode) == 4) {
                if (lArg1 != NULL && lArg1[0] != '\0')
                    throw_error(4, "Error: line %d: unexpected operand", lineCnt + 1);
            }
        }
        lineCnt++;
    } while (lRet != DONE);

    if (lOutfile == NULL)
        throw_error(4, "Error: cannot open output file %s", outfileName);

    cmdCnt = 0;
    lineCnt = 0;
    rewind(lInfile);

    calc_la(lInfile);
    rewind(lInfile);

    do {
        lRet = parse_cmd(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3);
        if (lRet != DONE && lRet != EMPTY_LINE) {
            if (lOpcode != NULL && *lOpcode == '\0')
                lOpcode = NULL;
            if (lArg1 != NULL && *lArg1 == '\0')
                lArg1 = NULL;
            if (lArg2 != NULL && *lArg2 == '\0')
                lArg2 = NULL;
            if (lArg3 != NULL && *lArg3 == '\0')
                lArg3 = NULL;
            /* check labels with nothing followed */
            if (lOpcode == NULL && lArg1 == NULL && lArg2 == NULL && lArg3 == NULL)
                throw_error(2, "Error: line %d: invalid opcode", lineCnt + 1);
            if (is_opcode(lOpcode) == LA || is_opcode(lOpcode) == FILL || is_opcode(lOpcode) == HALT)
                handle_pseudo_cmd(lOutfile, lineCnt, cmdCnt, lOpcode,
                    lArg1, lArg2, lArg3, labelTable);
            else
                fprintf(lOutfile, "0x%08x\n",
                    cmd2code(lineCnt, cmdCnt, lOpcode, lArg1, lArg2, lArg3, labelTable));
            #ifdef DEBUG
                unsigned int code = cmd2code(lineCnt, cmdCnt, lOpcode, lArg1,
                    lArg2, lArg3, labelTable);
                printf("[DEBUG] lLine: %s\tlLabel: %s\n", lLine, lLabel);
                printf("        lOpcode: %s\tlArg1: %s\n", lOpcode, lArg1);
                printf("        lArg2: %s\tlArg3: %s\n", lArg2, lArg3);
                printf("        code: 0x%08x\n", code);
            #endif
            cmdCnt++;
        }
        lineCnt++;
    } while (lRet != DONE);

    fclose(lInfile);
    fclose(lOutfile);
}

void handle_err(int errCode, int errLine) {
    switch (errCode) {
        case 1:
            printf("Error 1: Undefined label in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 1);
            exit(1);
        case 2:
            printf("Error 2: Invalid opcode in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 2);
            exit(2);
        case 3:
            printf("Error 3: Invalid constant in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 3);
            exit(3);
        case 4:
            printf("Error 4: Invalid Reg operand in line %d.\n", errLine+1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 5:
            printf("Error 5: Wrong Number of operands in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 6:
            printf("Error 6: Error occurs in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 7:
            printf("Error 7: Unexpected operand in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 8:
            printf("Error 8: Label in line %d is too far to fetch.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 9:
            printf("Error 9: Invalid memory address to load program in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 3);
            exit(3);
        case 10:
            printf("Error 10: Invalid label name in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        /* reserved for other Error Code */
        case 98:
            printf("Error 98: Malloc error occured in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        case 99:
            printf("Error 99: An error occured in line %d.\n", errLine + 1);
            printf("exit code: %d\n", 4);
            exit(4);
        default:
            printf("Opps! The handle_err function has BUGS!\n");
            exit(4);
    }
}

void throw_error(int exitCode, const char *format, ...) {
    va_list argp;

    va_start(argp, format);
    vprintf(format, argp);
    va_end(argp);

    printf("\nexit code: %d\n", exitCode);

    exit(exitCode);
}
