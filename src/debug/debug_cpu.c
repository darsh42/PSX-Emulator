#include "debug_cpu.h"

static struct CPU *cpu;

void set_debug_cpu(void) {
    cpu = _cpu();
}

void peek_cpu_pc(void) {
    printf("PC: %08x\n", cpu->PC);
}

// TODO: translate register number into corresponding names
void peek_cpu_R_register(int reg) {
    printf("[DEBUG]: REGISTER: %d, Value: %d\n", reg, cpu->R[reg]);
}

void peek_cpu_R_registers(void) {
    for (int i = 0; i < 32; i++) {
        peek_cpu_R_register(i);
    }
}

void peek_cpu_mult_div_registers(void) {
    printf("[DEBUG]: REGISTER: HI, Value: %d\n", cpu->HI);
    printf("[DEBUG]: REGISTER: LO, Value: %d\n", cpu->LO);
}

// TODO: translate the opcodes and functions into corresponding mneumonics
void peek_cpu_instruction(void) {
    if (cpu->instruction_type == UNDECIDED) {
        printf("[WARNING]: Instruction has not been decoded yet\n");
        printf("[DEBUG]: As Generic: op=%d\n",cpu->instruction.generic.op);
    } else if (cpu->instruction_type == R_TYPE) {
        printf("[DEBUG]: As R-TYPE: funct=%d, shamt=%d, rd=%d, rt=%d, rs=%d, op=%d\n", cpu->instruction.R_TYPE.funct,
                                                                                       cpu->instruction.R_TYPE.shamt,
                                                                                       cpu->instruction.R_TYPE.rd,
                                                                                       cpu->instruction.R_TYPE.rt,
                                                                                       cpu->instruction.R_TYPE.rs,
                                                                                       cpu->instruction.R_TYPE.op);
    } else if (cpu->instruction_type == I_TYPE) {
        printf("[DEBUG]: As I-TYPE: immediate=%d, rt=%d, rs=%d, op=%d\n", cpu->instruction.I_TYPE.immediate,
                                                                          cpu->instruction.I_TYPE.rt,
                                                                          cpu->instruction.I_TYPE.rs,
                                                                          cpu->instruction.I_TYPE.op);
    } else {
        printf("[DEBUG]: As J-TYPE: target=%d, op=%d\n", cpu->instruction.J_TYPE.target, 
                                                         cpu->instruction.J_TYPE.op);
    }
}
