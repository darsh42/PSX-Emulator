#include "../../include/debug_cpu.h"

static void decode_instruction(union INSTRUCTION instruction);
static const char *register_names[32] = {"$zr", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
                                         "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
                                         "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", 
                                         "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

static struct CPU *cpu;

void set_debug_cpu(void) {
    cpu = get_cpu();
}

void peek_cpu_pc(void) {
    printf("[DEBUG]: PC: %08X\n", cpu->PC);
}

// TODO: translate register number into corresponding names
void peek_cpu_R_register(int reg) {
    printf("[DEBUG]: CPU REGISTER: %s, Value: %08X\n", register_names[reg], cpu->R[reg]);
}

void peek_cpu_R_registers(void) {
    for (int i = 0; i < 32; i++) {
        peek_cpu_R_register(i);
    }
}

void peek_cpu_mult_div_registers(void) {
    printf("[DEBUG]: HI-REGISTER: Value: %X\n", cpu->HI);
    printf("[DEBUG]: LO-REGISTER: Value: %X\n", cpu->LO);
}

void peek_coprocessor_n_register(int cop_n, int reg) {
    switch(cop_n) {
        case 0X00: if (cpu->cop0.R[reg] != NULL) {printf("[DEBUG]: CO PROCESSOR 0 REGISTER: %d, Value: %X\n", reg, *cpu->cop0.R[reg]);} break;
        case 0X02: if (cpu->cop2.R[reg] != NULL) {printf("[DEBUG]: CO PROCESSOR 2 REGISTER: %d, Value: %X\n", reg, *cpu->cop2.R[reg]);} break;
    }
}

void peek_coprocessor_n_registers(int cop_n) {
    for (int i = 0; i < 16; i++) {
        peek_coprocessor_n_register(cop_n, i);
    }
}

void peek_cpu_registers(void) {
    peek_cpu_R_registers();
    peek_cpu_mult_div_registers();
    peek_coprocessor_n_registers(0);
    peek_coprocessor_n_registers(2);
}
void peek_cpu_instruction(void) {
    decode_instruction(cpu->instruction);
}

static void decode_instruction(union INSTRUCTION instruction) {
    switch (debugger.psx->cpu->instruction.op) {
        case 0X00: 
            switch (instruction.funct) {
                case 0X00: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLL      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X02: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SRL      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X03: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SRA      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X04: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLLV     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X06: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SRLV     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X07: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SRAV     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X08: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    JR       $pc=%s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X09: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    JALR     %s, %s=0X%08X\n", debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rd], debugger.psx->cpu->PC); break;
                case 0X0C: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SYSCALL  \n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value); break;
                case 0X0D: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BREAK    \n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value); break;
                case 0X10: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MFHI     %s=$hi\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd]); break;
                case 0X11: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MTHI     %s=$lo\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd]); break;
                case 0X12: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MFLO     $hi=%s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X13: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MTLO     $lo=%s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs]); break;
                case 0X18: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MULT     %s, %s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X19: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    MULTU    %s, %s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X1A: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    DIV      %s, %s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X1B: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    DIVU     %s, %s\n",        debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X20: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ADD      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X21: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ADDU     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X22: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SUB      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X23: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SUBU     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X24: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    AND      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X25: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    OR       %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X26: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    XOR      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X27: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    NOR      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X2A: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLT      %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
                case 0X2B: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLTU     %s, %s, %s\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rd], register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt]); break;
            } 
            break;
        case 0X01: 
            switch (debugger.psx->cpu->instruction.rt) {
                case 0b00000: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BLTZ     %s, 0X%04X\n",             debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
                case 0b00001: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BGEZ     %s, 0X%04X\n",             debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
                case 0b10000: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BLTZAL   %s, 0X%04X %s = 0X%08X\n", debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16, register_names[31], debugger.psx->cpu->PC); break;
                case 0b10001: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BGEZAL   %s, 0X%04X %s = 0X%08X\n", debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16, register_names[31], debugger.psx->cpu->PC); break;
            }
            break;
        case 0X02: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    J        0X%08X\n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, (debugger.psx->cpu->PC & 0XF0000000) | (debugger.psx->cpu->instruction.target << 2)); break;
        case 0X03: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    JAL      0X%08X, %s = 0X%08X\n", debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, (debugger.psx->cpu->PC & 0XF0000000) | (debugger.psx->cpu->instruction.target << 2), register_names[31], debugger.psx->cpu->PC); break;
        case 0X04: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BEQ      %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16); break;
        case 0X05: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BNE      %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16); break;
        case 0X06: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BLEZ     %s, 0X%04X\n",          debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X07: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    BGTZ     %s, 0X%04X\n",          debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X08: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ADDI     %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X09: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ADDIU    %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0A: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLTI     %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0B: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SLTIU    %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0C: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ANDI     %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0D: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    ORI      %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0E: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    XORI     %s, %s, 0X%04X\n",      debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], register_names[debugger.psx->cpu->instruction.rs], debugger.psx->cpu->instruction.immediate16); break;
        case 0X0F: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LUI      %s, 0X%04X\n",          debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16); break;
        case 0X20: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LB       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X21: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LH       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X22: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWL      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X23: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LW       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X24: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LBU      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X25: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LHU      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X26: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWR      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X28: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SB       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X29: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SH       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X2A: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWL      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X2B: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SW       %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X2E: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWR      %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        // COsprOCESSOR instructions                        
        case 0X10: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    COP0     0X%08X\n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, debugger.psx->cpu->instruction.immediate25); break;
        case 0X11: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    COP1     0X%08X\n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, debugger.psx->cpu->instruction.immediate25); break;
        case 0X12: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    COP2     0X%08X\n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, debugger.psx->cpu->instruction.immediate25); break;
        case 0X13: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    COP3     0X%08X\n",              debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, debugger.psx->cpu->instruction.immediate25); break;
        case 0X30: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWC0     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X31: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWC1     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X32: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWC2     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X33: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    LWC3     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X38: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWC0     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X39: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWC1     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X3A: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWC2     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
        case 0X3B: fprintf(stdout, "[DISASSEMBLY]: PC: 0X%08X    HEX: 0X%08X    SWC3     %s, 0X%04X(%s)\n",    debugger.psx->cpu->PC, debugger.psx->cpu->instruction.value, register_names[debugger.psx->cpu->instruction.rt], debugger.psx->cpu->instruction.immediate16, register_names[debugger.psx->cpu->instruction.rs]); break;
    }
}

