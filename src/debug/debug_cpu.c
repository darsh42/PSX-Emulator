#include "debug_cpu.h"

static void decode_instruction(union INSTRUCTION instruction);

static struct CPU *cpu;

void set_debug_cpu(void) {
    cpu = _cpu();
}

void peek_cpu_pc(void) {
    printf("[DEBUG]: PC: %08X\n", cpu->PC);
}

// TODO: translate register number into corresponding names
void peek_cpu_R_register(int reg) {
    printf("[DEBUG]: CPU REGISTER: %02d, Value: %08X\n", reg, cpu->R[reg]);
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
        case 0X00: if (cpu->coprocessor0.R[reg] != NULL) {printf("[DEBUG]: CO PROCESSOR 0 REGISTER: %d, Value: %X\n", reg, *cpu->coprocessor0.R[reg]);} break;
        case 0X02: if (cpu->coprocessor2.R[reg] != NULL) {printf("[DEBUG]: CO PROCESSOR 2 REGISTER: %d, Value: %X\n", reg, *cpu->coprocessor2.R[reg]);} break;
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

// TODO: translate the opcodes and functions into corresponding mneumonics
static void decode_instruction(union INSTRUCTION instruction) {
    printf("[DEBUG]: PC: %08X  ", cpu->PC);

    if (instruction.op == 0X00) {
        printf(" R-TYPE: "); 
        switch (instruction.funct) {
            case 0X00: printf("SLL    "); break;
            case 0X02: printf("SRL    "); break;
            case 0X03: printf("SRA    "); break;
            case 0X04: printf("SLLV   "); break;
            case 0X06: printf("SRLV   "); break;
            case 0X07: printf("SRAV   "); break;
            case 0X08: printf("JR     "); break;
            case 0X09: printf("JALR   "); break;
            case 0X0C: printf("SYSCALL"); break;
            case 0X0D: printf("BREAK  "); break;
            case 0X10: printf("MFHI   "); break;
            case 0X11: printf("MTHI   "); break;
            case 0X12: printf("MFLO   "); break;
            case 0X13: printf("MTLO   "); break;
            case 0X18: printf("MULT   "); break;
            case 0X19: printf("MULTU  "); break;
            case 0X1A: printf("DIV    "); break;
            case 0X1B: printf("DIVU   "); break;
            case 0X20: printf("ADD    "); break;
            case 0X21: printf("ADDU   "); break;
            case 0X22: printf("SUB    "); break;
            case 0X23: printf("SUBU   "); break;
            case 0X24: printf("AND    "); break;
            case 0X25: printf("OR     "); break;
            case 0X26: printf("XOR    "); break;
            case 0X27: printf("NOR    "); break;
            case 0X2A: printf("SLT    "); break;
            case 0X2B: printf("SLTU   "); break;
        }
        printf(" rd=%02d, rt=%02d, rs=%02d, op=%02X, funct=%X, shamt=%X\n", instruction.rd,
                                                                            instruction.rt,
                                                                            instruction.rs,
                                                                            instruction.op,
                                                                            instruction.funct,
                                                                            instruction.shamt);
    } else if (instruction.op == 0X02 || 
               instruction.op == 0X03) {
        printf(" J-TYPE: ");
        switch(instruction.op) {
            case 0X02: printf("J     "); break;
            case 0X03: printf("JAL   "); break;
        }
        printf("                       op=%02X, target=%X\n", instruction.op, 
                                                              instruction.target);
    } else {
        printf(" I-TYPE: ");
        switch (instruction.op) {
            case 0X01: 
                switch (cpu->instruction.rt) {
                    case 0b00000: printf("BLTZ   "); break;
                    case 0b00001: printf("BGEZ   "); break;
                    case 0b10000: printf("BLTZAL "); break;
                    case 0b10001: printf("BGEZAL "); break;
                }
                break;
            case 0X04: printf("BEQ    "); break;  
            case 0X05: printf("BNE    "); break;  
            case 0X06: printf("BLEZ   "); break;  
            case 0X07: printf("BTGZ   "); break;  
            case 0X08: printf("ADDI   "); break;  
            case 0X09: printf("ADDIU  "); break;  
            case 0X0A: printf("SLTI   "); break;  
            case 0X0B: printf("SLTIU  "); break;  
            case 0X0C: printf("ANDI   "); break;  
            case 0X0D: printf("ORI    "); break;  
            case 0X0E: printf("XORI   "); break;  
            case 0X0F: printf("LUI    "); break;  
            case 0X10: printf("COP0   "); break;  
            case 0X11: printf("COP1   "); break;  
            case 0X12: printf("COP2   "); break;  
            case 0X13: printf("COP3   "); break;  
            case 0X20: printf("LB     "); break;  
            case 0X21: printf("LH     "); break;  
            case 0X22: printf("LWL    "); break;  
            case 0X23: printf("LW     "); break;  
            case 0X24: printf("LBU    "); break;  
            case 0X25: printf("LHU    "); break;  
            case 0X26: printf("LWR    "); break;  
            case 0X28: printf("SB     "); break;  
            case 0X29: printf("SH     "); break;  
            case 0X2A: printf("SWL    "); break;  
            case 0X2B: printf("SW     "); break;  
            case 0X2E: printf("SWR    "); break; 
            case 0X30: printf("LWC0   "); break; 
            case 0X31: printf("LWC1   "); break; 
            case 0X32: printf("LWC2   "); break; 
            case 0X33: printf("LWC3   "); break; 
            case 0X38: printf("SWC0   "); break; 
            case 0X39: printf("SWC1   "); break; 
            case 0X3A: printf("SWC2   "); break; 
            case 0X3B: printf("SWC3   "); break; 
        }
        printf("        rt=%02d, rs=%02d, op=%02X, immediate=%X\n", instruction.rt,
                                                                    instruction.rs,
                                                                    instruction.op, 
                                                                    instruction.immediate16);
    }
}

