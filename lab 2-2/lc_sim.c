/*
    Remove all unnecessary lines (including this one) in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name: [Steve Dexter B. Tamang]
    ID  : [115512468]
*/

/***************************************************************/
/*                                                             */
/*                  RISCV-LC Assembler                         */
/*                                                             */
/*                     CEG3420 Lab2                            */
/*                 cbai@cse.cuhk.edu.hk                        */
/*           The Chinese University of Hong Kong               */
/*                                                             */
/***************************************************************/

#include "lc_sim.h"

void help() {
    printf("----------------RISC-V LC SIM Help--------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?/h              -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

void cycle() {
    handle_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == TRAPVEC_BASE_ADDR) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != TRAPVEC_BASE_ADDR)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = start; address <= stop; address += 4)
        printf("  0x%08x (%d) : 0x%02x%02x%02x%02x\n", address, address,
            MEMORY[address + 3], MEMORY[address + 2], MEMORY[address + 1], MEMORY[address]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%08x..0x%08x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = start; address <= stop; address += 4)
        fprintf(dumpsim_file, " 0x%08x (%d) : 0x%02x%02x%02x%02x\n", address, address,
            MEMORY[address + 3], MEMORY[address + 2], MEMORY[address + 1], MEMORY[address]);
    fprintf(dumpsim_file, "\n");
}

void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%08x\n", CURRENT_LATCHES.PC);
    printf("Registers:\n");
    for (k = 0; k < LC_RISCV_REGS; k++)
        printf("%s\t[x%d]:\t0x%08x\n", regs[k], k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n"); 
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%04x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_RISCV_REGS; k++)
        fprintf(dumpsim_file, "%s\t[x%d]:\t0x%04x\n", regs[k], k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
}

void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-RISCV-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;
        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;
        case '?':
        case 'h':
        case 'H':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);
        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;
        default:
            printf("Invalid Command\n");
            break;
    }
}

void init_memory() {
    int i;

    for (i = 0; i < WORDS_IN_MEM; i++)
        MEMORY[i] = 0;
}

void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    program_base = CODE_BASE_ADDR;

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                    program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array.
         * Little endian
         */
        MEMORY[program_base + ii] = MASK7_0(word);
        MEMORY[program_base + ii + 1] = MASK15_8(word);
        MEMORY[program_base + ii + 2] = MASK23_16(word);
        MEMORY[program_base + ii + 3] = MASK31_24(word);
        ii += 4;
    }

    if (CURRENT_LATCHES.PC == 0){
        CURRENT_LATCHES.PC = CODE_BASE_ADDR;
    }

    printf("Read %d words (%d bytes) from program into memory.\n\n", ii, ii << 2);
}

void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
                argv[0]);
        exit(1);
    }

    printf("RISC-V LC Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ((dumpsim_file = fopen( "dumpsim", "w" )) == NULL) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);
}

/* Return the word content start from startAddr */
int read_mem(int startAddr) {
    /*
     * Lab2-2 assignment.
     */
    //printf("Lab2-2 assignment: read_mem\n");
    int i = (MEMORY[startAddr + 3] << 24);
    int j = (MEMORY[startAddr + 2] << 16);
    int k = (MEMORY[startAddr + 1] << 8);
    int l = (MEMORY[startAddr] << 0);
    int memory = i + j + k + l;
    return memory;
}

/*
 * Sign extend the imm to 32 bits
 * 'width' is the bit width before imm is extended
 */
int sext(int imm, int width) {
    /*
     * Lab2-2 assignment.
     */
    //printf("Lab2-2 assignment: sext\n");
    int r = 0, m = 1U << (width - 1);
    imm = imm & ((1U << width) - 1);
    r = (imm ^ m) - m;
    //printf("sext of %d is 0x%08x\n", imm, r);
    return r;
}

void handle_addi(unsigned int curInstr) {
    //printf("Lab2-2 NOT assignment: handle_addi\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    NEXT_LATCHES.REGS[rd] = CURRENT_LATCHES.REGS[rs1] + imm12;
}

void handle_slli(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_slli\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    int shamt = sext(MASK24_20(curInstr), 5);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] << shamt);
    
    //exit(1);
}

void handle_xori(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_xori\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] ^ imm12);
    
    //exit(1);
}

void handle_srli(unsigned int curInstr) {
    /*
     * Lab2-2 assignment, err could be imm[4:0] and imm[11:5]
     */
    //printf("Lab2-2 assignment: handle_srli\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    int shamt = sext(MASK24_20(curInstr), 5);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] >> shamt);

    
    //exit(1);
}

void handle_srai(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_srai\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    // int s = -((unsigned) imm12 >> 12); // extract the sign bit
    // int sra =  (s ^ imm12) >> 12 ^ s;
    int shamt = sext(MASK24_20(curInstr), 5);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] >> shamt);
    //exit(1);
}

void handle_ori(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_ori\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] | imm12);
    //exit(1);
}

void handle_andi(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_andi\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = sext(MASK31_20(curInstr), 12);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] & imm12);
    
    //exit(1);
}

void handle_lui(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_lui\n");
    unsigned int rd = MASK11_7(curInstr);
    int imm20 = sext(MASK31_12(curInstr), 20);
    NEXT_LATCHES.REGS[rd] = (imm20 << 12);
    //exit(1);
}

void handle_add(unsigned int curInstr) {
    //printf("Lab2-2 NOT assignment: handle_add\n");
    unsigned int rd = MASK11_7(curInstr),
		rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = CURRENT_LATCHES.REGS[rs1] + CURRENT_LATCHES.REGS[rs2];
}

void handle_sub(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_sub\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] - CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_sll(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_sll\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] << CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_xor(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_xor\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] ^ CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_srl(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_srl\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] >> CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_sra(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_sra\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] >> CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_or(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_or\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] | CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_and(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_and\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr),
        rs2 = MASK24_20(curInstr);
    NEXT_LATCHES.REGS[rd] = (CURRENT_LATCHES.REGS[rs1] & CURRENT_LATCHES.REGS[rs2]);
    
    //exit(1);
}

void handle_jalr(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_jalr\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = MASK31_20(curInstr);
    // rd = PC + 4
    CURRENT_LATCHES.REGS[rd] = CURRENT_LATCHES.PC + 4;
    // go to rs1 + imm12
	//printf("imm12 = 0x%08x\n", imm12);
    NEXT_LATCHES.PC = (CURRENT_LATCHES.REGS[rs1] + sext(imm12, 12));
    
    //exit(1);
}

void handle_jal(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_jal\n");
    unsigned int rd = MASK11_7(curInstr);
    int imm20 = (MASK31(curInstr) << 20) + \
            (MASK19_12(curInstr) << 12) + \
            (MASK20(curInstr) << 11) + \
            (MASK30_21(curInstr) << 1);
    
    //imm20 =  MASK31_12(curInstr);    
    // rd = PC + 4
    NEXT_LATCHES.REGS[rd] = CURRENT_LATCHES.PC + 4;
    // go to pc + imm
    //printf("imm12 = 0x%08x\n", imm20);
    NEXT_LATCHES.PC = sext(imm20, 20);
    
    //exit(1);
}

void handle_beq(unsigned int curInstr) {
    //printf("Lab2-2 NOT  assignment: handle_beq\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31(curInstr) << 12) + \
            (MASK7(curInstr) << 11) + \
            (MASK30_25(curInstr) << 5) + \
            (MASK11_8(curInstr) << 1);
    if (CURRENT_LATCHES.REGS[rs1] == CURRENT_LATCHES.REGS[rs2]){
        // go to PC + imm
        NEXT_LATCHES.PC = sext(imm12, 12);
    }
}

void handle_bne(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: bne\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31(curInstr) << 12) + \
            (MASK7(curInstr) << 11) + \
            (MASK30_25(curInstr) << 5) + \
            (MASK11_8(curInstr) << 1);
    if (CURRENT_LATCHES.REGS[rs1] != CURRENT_LATCHES.REGS[rs2]){
        NEXT_LATCHES.PC = sext(imm12, 12);
    }
    
    //exit(1);
}

void handle_blt(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_blt\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31(curInstr) << 12) + \
            (MASK7(curInstr) << 11) + \
            (MASK30_25(curInstr) << 5) + \
            (MASK11_8(curInstr) << 1);
    if (CURRENT_LATCHES.REGS[rs1] < CURRENT_LATCHES.REGS[rs2]){
        NEXT_LATCHES.PC = sext(imm12, 12);
    }
    
    //exit(1);
}

void handle_bge(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_bge\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31(curInstr) << 12) + \
            (MASK7(curInstr) << 11) + \
            (MASK30_25(curInstr) << 5) + \
            (MASK11_8(curInstr) << 1);
    if (CURRENT_LATCHES.REGS[rs1] >= CURRENT_LATCHES.REGS[rs2]){
        NEXT_LATCHES.PC = sext(imm12, 12);
    }
    
    //exit(1);
}

void handle_lb(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_lb\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = MASK31_20(curInstr);
    int byte = (CURRENT_LATCHES.REGS[rs1] + imm12);
    // load byte loads a 8-bit value then sign extends to 32 bit
    int l = (MEMORY[byte] << 0);
    NEXT_LATCHES.REGS[rd] = sext(l, 8);
   
    //exit(1);
}

void handle_lh(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_lh\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    int imm12 = MASK31_20(curInstr);
    int h_w = (CURRENT_LATCHES.REGS[rs1] + imm12);

    // load half-word loads a 16-bit value then sign extends to 32 bits
    int k = (MEMORY[h_w + 1] << 8);
    int l = (MEMORY[h_w] << 0);

    int result = (k + l); 

    NEXT_LATCHES.REGS[rd] = sext(result, 16);
    //exit(1);
}

void handle_lw(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_lw\n");
    unsigned int rd = MASK11_7(curInstr), rs1 = MASK19_15(curInstr);
    
    int imm12 = MASK31_20(curInstr);
    
    // load word loads a 32-bit value
    int word = read_mem(CURRENT_LATCHES.REGS[rs1] + imm12);
    // printf("word = 0x%08x\n", word);
    
    NEXT_LATCHES.REGS[rd] = word;
    
    //exit(1);
}

void handle_sb(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("sb instr = %d\t", curInstr);
    //printf("Lab2-2 assignment: handle_sb\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31_25(curInstr) << 5) + \
            (MASK11_7(curInstr) << 0);
    
    int t = CURRENT_LATCHES.REGS[rs1] + sext(imm12, 12);
    int word = CURRENT_LATCHES.REGS[rs2];
    
    // store byte stores a 8-bit value
    MEMORY[t] = MASK7_0(word);
    // MEMORY[t + 1] = MASK15_8(word);
    // MEMORY[t + 2] = MASK23_16(word);
    // MEMORY[t + 3] = MASK31_24(word);
    
    //exit(1);
}

void handle_sh(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_sh\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31_25(curInstr) << 5) + \
            (MASK11_7(curInstr) << 0);
    int t = CURRENT_LATCHES.REGS[rs1] + sext(imm12, 12);
    int word = CURRENT_LATCHES.REGS[rs2];
   
    // sh stores 16-bit
    MEMORY[t] = MASK7_0(word);
    MEMORY[t + 1] = MASK15_8(word);
    // MEMORY[t + 2] = MASK23_16(word);
    // MEMORY[t + 3] = MASK31_24(word);
    //MEMORY[CURRENT_LATCHES.REGS[rs2] + imm12] = CURRENT_LATCHES.REGS[rs1];
    
    //exit(1);
}

void handle_sw(unsigned int curInstr) {
    /*
     * Lab2-2 assignment
     */
    //printf("Lab2-2 assignment: handle_sw\n");
    unsigned int rs1 = MASK19_15(curInstr), rs2 = MASK24_20(curInstr);
    int imm12 = (MASK31_25(curInstr) << 5) + \
        (MASK11_7(curInstr) << 0);
    
    int t = CURRENT_LATCHES.REGS[rs1] + sext(imm12, 12);
    int word = CURRENT_LATCHES.REGS[rs2];
   
    // store word stores 32 bit
    MEMORY[t] = MASK7_0(word);
    MEMORY[t + 1] = MASK15_8(word);
    MEMORY[t + 2] = MASK23_16(word);
    MEMORY[t + 3] = MASK31_24(word);
    //exit(1);
}

void handle_halt(unsigned int curInstr) {
	unsigned int s0 = 8;
	NEXT_LATCHES.REGS[s0] = CURRENT_LATCHES.PC;
	NEXT_LATCHES.PC = TRAPVEC_BASE_ADDR;
}

/*  
 *  Process one instruction at a time  
 *  - Fetch one instruction
 *  - Decode 
 *  - Execute
 *  - Update NEXT_LATCHES
 */     
void handle_instruction() {
    
    unsigned int curInstr = read_mem(CURRENT_LATCHES.PC);
    unsigned int opCode = MASK6_0(curInstr), funct3 = MASK14_12(curInstr);
    printf("handle_instruction()| curInstr = 0x%08x\n", curInstr);

	CURRENT_LATCHES.REGS[0] = 0;
    NEXT_LATCHES = CURRENT_LATCHES;
    /*
     * Lab2-2 assignment: update NEXT_LATCHES
     * Think about how to update new PC
     */
    NEXT_LATCHES.PC = (CURRENT_LATCHES.PC + 4);
    
    /* opCode */
    switch(opCode) {
        case(0x7f):
            handle_halt(curInstr);
            break;
        case (0x04 << 2) + 0x03:
            /*
             * Integer Register-Immediate Instructions
             */
            switch(funct3) {
                    case 0:
                        handle_addi(curInstr);
                        break;
                    case 1:
                        handle_slli(curInstr);
                        break;
                    case 4:
                        handle_xori(curInstr);
                        break;
                    case 5:
                        if (MASK31_25(curInstr) == 0)
                            handle_srli(curInstr);
                        else
                            handle_srai(curInstr);
                        break;
                    case 6:
                        handle_ori(curInstr);
                        break;
                    case 7:
                        handle_andi(curInstr);
                        break;
                    default:
                        printf("Unknown opCode captured !\n");
                        exit(1);
                }
            break;
            /*
            * Lab2-2 assignment: Decode other types of RV32I instructions
            */
        case (0x0C << 2) + 0x03:
            /*
             * Integer Register-Register Instructions
             */
            switch(funct3) {
                case 0:
                    if (MASK31_25(curInstr) == 0)
                        handle_add(curInstr);
                    else
                        handle_sub(curInstr);
                    break;
                case 1:
                    handle_sll(curInstr);
                    break;
                case 4:
                    handle_xor(curInstr);
                    break;
                case 5:
                    if (MASK31_25(curInstr) == 0)
                        handle_srl(curInstr);
                    else
                        handle_sra(curInstr);
                    break;
                case 6:
                    handle_or(curInstr);
                    break;
                case 7:
                    handle_and(curInstr);
                    break;
                default:
                    printf("Unknown opCode captured !\n");
                    exit(1);

            }
            break;
        case (0x18 << 2) + 0x03:
            /*
             * Branch Instructions
             */
            switch(funct3) {
                case 0:
                    handle_beq(curInstr);
                    break;
                case 1:
                    handle_bne(curInstr);
                    break;
                case 4:
                    handle_blt(curInstr);
                    break;
                case 5:
                    handle_bge(curInstr);
                    break;
                default:
                    printf("Unknown opCode captured !\n");
                    exit(1);
            }
            break;

        case (0x19 << 2) + 0x03:
            // jalr
            //if(MASK14_12(curInstr) == 0){
                handle_jalr(curInstr);
            //}
            break;

        case (0x1b << 2) + 0x03:
            // jal 
            handle_jal(curInstr);
            break;
        
        case (0x00 << 2) + 0x03:
            /*
             * Load Instructions
             */
            switch(funct3) {
                case 0:
                    handle_lb(curInstr);
                    break;
                case 1:
                    handle_lh(curInstr);
                    break;
                case 2:
                    handle_lw(curInstr);
                    break;
                default:
                    printf("Unknown opCode captured !\n");
                    exit(1);
            }
            break;

        case (0x0D << 2) + 0x03:
            handle_lui(curInstr);
            break;
        
        case (0x08 << 2) + 0x03:
            /*
             * Store Instructions
             */
            switch(funct3) {
                case 0:
                    handle_sb(curInstr);
                    break;
                case 1:
                    handle_sh(curInstr);
                    break;
                case 2:
                    handle_sw(curInstr);
                    break;
                default:
                    printf("Unknown opCode captured !\n");
                    exit(1);
            }
            break;
        
        default:
            printf("Unknown opCode captured !\n");
            exit(1);
    }
}
