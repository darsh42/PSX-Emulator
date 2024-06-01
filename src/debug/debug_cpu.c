#include "../../include/debugger.h"

static const char *register_names[32] = {"$zr", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
                                         "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
                                         "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", 
                                         "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

extern struct DEBUGGER debugger;

void peek_cpu_pc(void) {
    snprintf(debugger.cpu_registers[34], MAXLEN, "$pc = %08X\n", debugger.cpu->PC);
}

// TODO: translate register number into corresponding names
void peek_cpu_R_register(int reg) {
    snprintf(debugger.cpu_registers[reg], MAXLEN, "%s = %08X\n", register_names[reg], debugger.cpu->R[reg]);
}

void peek_cpu_R_registers(void) {
    for (int i = 0; i < 32; i++) {
        peek_cpu_R_register(i);
    }
}

void peek_cpu_mult_div_registers(void) {
    snprintf(debugger.cpu_registers[32], MAXLEN, "$hi = %08X\n", debugger.cpu->HI);
    snprintf(debugger.cpu_registers[33], MAXLEN, "$lo = %08X\n", debugger.cpu->LO);
}

void peek_coprocessor_n_register(int cop_n, int reg) {
    switch(cop_n) {
        case 0X00: 
            if (debugger.cpu->cop0.R[reg] != NULL) {
                snprintf(debugger.cpu_registers[35 + reg], MAXLEN, "cop0-%02d = %08X\n", reg, *debugger.cpu->cop0.R[reg]);
            } 
            break;
        case 0X02: 
            if (debugger.cpu->cop2.R[reg] != NULL) {
                snprintf(debugger.cpu_registers[51 + reg], MAXLEN, "cop2-%02d = %08X\n", reg, *debugger.cpu->cop2.R[reg]);
            }
            break;
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
    peek_cpu_pc();
    peek_coprocessor_n_registers(0);
    peek_coprocessor_n_registers(2);
}
