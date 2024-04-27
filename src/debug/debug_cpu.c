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
void decode_instruction(union INSTRUCTION instruction, enum INSTRUCTION_TYPE instruction_type) {
        //     I-TYPE instruction             R-TYPE instructions             J-TYPE instructions
    if (instruction_type == UNDECIDED) {
        printf("[WARNING]: Instruction has not been decoded yet\n");
        printf("[DEBUG]: As Generic: op=%d\n",instruction.generic.op);
    } else if (instruction_type == R_TYPE) {
        printf("[DEBUG]: As R-TYPE: funct=%d, shamt=%d, rd=%d, rt=%d, rs=%d, op=%d\n", instruction.R_TYPE.funct,
                                                                                       instruction.R_TYPE.shamt,
                                                                                       instruction.R_TYPE.rd,
                                                                                       instruction.R_TYPE.rt,
                                                                                       instruction.R_TYPE.rs,
                                                                                       instruction.R_TYPE.op);
        printf("[DEBUG]: mneumonic: ");
        switch (instruction.R_TYPE.funct) {
            case 0X00: printf("SLL\n"    ); break;
            case 0X02: printf("SRL\n"    ); break;
            case 0X03: printf("SRA\n"    ); break;
            case 0X04: printf("SLLV\n"   ); break;
            case 0X06: printf("SRLV\n"   ); break;
            case 0X07: printf("SRAV\n"   ); break;
            case 0X08: printf("JR\n"     ); break;
            case 0X09: printf("JALR\n"   ); break;
            case 0X0C: printf("SYSCALL\n"); break;
            case 0X0D: printf("BREAK\n"  ); break;
            case 0X10: printf("MFHI\n"   ); break;
            case 0X11: printf("MTHI\n"   ); break;
            case 0X12: printf("MFLO\n"   ); break;
            case 0X13: printf("MTLO\n"   ); break;
            case 0X18: printf("MULT\n"   ); break;
            case 0X19: printf("MULTU\n"  ); break;
            case 0X1A: printf("DIV\n"    ); break;
            case 0X1B: printf("DIVU\n"   ); break;
            case 0X20: printf("ADD\n"    ); break;
            case 0X21: printf("ADDU\n"   ); break;
            case 0X22: printf("SUB\n"    ); break;
            case 0X23: printf("SUBU\n"   ); break;
            case 0X24: printf("AND\n"    ); break;
            case 0X25: printf("OR\n"     ); break;
            case 0X26: printf("XOR\n"    ); break;
            case 0X27: printf("NOR\n"    ); break;
            case 0X2A: printf("SLT\n"    ); break;
            case 0X2B: printf("SLTU\n"   ); break;
        }
    } else if (instruction_type == I_TYPE) {
        printf("[DEBUG]: As I-TYPE: immediate=%d, rt=%d, rs=%d, op=%d\n", instruction.I_TYPE.immediate,
                                                                          instruction.I_TYPE.rt,
                                                                          instruction.I_TYPE.rs,
                                                                          instruction.I_TYPE.op);
        printf("[DEBUG]: mneumonic: ");
        switch (instruction.I_TYPE.op) {
            case 0X01: printf("BCONDZ\n"); break;  
            case 0X04: printf("BEQ\n"   ); break;  
            case 0X05: printf("BNE\n"   ); break;  
            case 0X06: printf("BLEZ\n"  ); break;  
            case 0X07: printf("BTGZ\n"  ); break;  
            case 0X08: printf("ADDI\n"  ); break;  
            case 0X09: printf("ADDIU\n" ); break;  
            case 0X0A: printf("SLTI\n"  ); break;  
            case 0X0B: printf("SLTIU\n" ); break;  
            case 0X0C: printf("ANDI\n"  ); break;  
            case 0X0D: printf("ORI\n"   ); break;  
            case 0X0E: printf("XORI\n"  ); break;  
            case 0X0F: printf("LUI\n"   ); break;  
            case 0X10: printf("COP0\n"  ); break;  
            case 0X11: printf("COP1\n"  ); break;  
            case 0X12: printf("COP2\n"  ); break;  
            case 0X13: printf("COP3\n"  ); break;  
            case 0X20: printf("LB\n"    ); break;  
            case 0X21: printf("LH\n"    ); break;  
            case 0X22: printf("LWL\n"   ); break;  
            case 0X23: printf("LW\n"    ); break;  
            case 0X24: printf("LBU\n"   ); break;  
            case 0X25: printf("LHU\n"   ); break;  
            case 0X26: printf("LWR\n"   ); break;  
            case 0X28: printf("SB\n"    ); break;  
            case 0X29: printf("SH\n"    ); break;  
            case 0X2A: printf("SWL\n"   ); break;  
            case 0X2B: printf("SW\n"    ); break;  
            case 0X2E: printf("SWR\n"   ); break; 
            case 0X30: printf("LWC0\n"  ); break; 
            case 0X31: printf("LWC1\n"  ); break; 
            case 0X32: printf("LWC2\n"  ); break; 
            case 0X33: printf("LWC3\n"  ); break; 
            case 0X38: printf("SWC0\n"  ); break; 
            case 0X39: printf("SWC1\n"  ); break; 
            case 0X3A: printf("SWC2\n"  ); break; 
            case 0X3B: printf("SWC3\n"  ); break; 
        }
    } else {
        printf("[DEBUG]: As J-TYPE: target=%d, op=%d\n", instruction.J_TYPE.target, 
                                                         instruction.J_TYPE.op);
        printf("[DEBUG]: mneumonic: ");
        switch(instruction.J_TYPE.op) {
            case 0X02: printf("J\n"  ); break;
            case 0X03: printf("JAL\n"); break;
        }
    }
}

void peek_cpu_instruction(void) {
    decode_instruction(cpu->instruction, cpu->instruction_type);
}
