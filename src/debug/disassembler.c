#include "../../include/debugger.h"

extern struct DEBUGGER debugger;

static uint32_t     base_addresses[] = {0X00000000, 0X1F000000, 0X1FC00000};
static uint32_t      end_addresses[] = {0X00200000, 0X1F800000, 0X1FC80000};
static const char  *register_names[] = {"$zr", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
                                        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
                                        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", 
                                        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};
void disassemble(void) {
    uint32_t pc;
    union INSTRUCTION ins;

    for (int i = 0; i < 3; i++) {
        pc = base_addresses[i];
        for (int j = 0; pc < end_addresses[i]; pc+=4, j++) {
            char ptr[MAXLEN];
            memory_cpu_load_32bit(pc, &ins.value);
            switch (ins.op) {
                case 0X00: 
                    switch (ins.funct) {
                        case 0X00: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLL      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X02: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SRL      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X03: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SRA      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X04: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLLV     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X06: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SRLV     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X07: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SRAV     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rt], register_names[ins.rs]); break;
                        case 0X08: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    JR       $pc=%s                   \n",        pc, ins.value, register_names[ins.rs]); break;
                        case 0X09: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    JALR     %s, %s=0X%08X            \n", pc, ins.value, register_names[ins.rs], register_names[ins.rd], pc); break;
                        case 0X0C: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SYSCALL                           \n",              pc, ins.value); break;
                        case 0X0D: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BREAK                             \n",              pc, ins.value); break;
                        case 0X10: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MFHI     %s=$hi                   \n",        pc, ins.value, register_names[ins.rd]); break;
                        case 0X11: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MTHI     %s=$lo                   \n",        pc, ins.value, register_names[ins.rd]); break;
                        case 0X12: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MFLO     $hi=%s                   \n",        pc, ins.value, register_names[ins.rs]); break;
                        case 0X13: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MTLO     $lo=%s                   \n",        pc, ins.value, register_names[ins.rs]); break;
                        case 0X18: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MULT     %s, %s                   \n",        pc, ins.value, register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X19: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    MULTU    %s, %s                   \n",        pc, ins.value, register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X1A: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    DIV      %s, %s                   \n",        pc, ins.value, register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X1B: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    DIVU     %s, %s                   \n",        pc, ins.value, register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X20: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ADD      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X21: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ADDU     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X22: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SUB      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X23: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SUBU     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X24: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    AND      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X25: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    OR       %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X26: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    XOR      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X27: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    NOR      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X2A: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLT      %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;
                        case 0X2B: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLTU     %s, %s, %s               \n",    pc, ins.value, register_names[ins.rd], register_names[ins.rs], register_names[ins.rt]); break;

                    } 
                    break;
                case 0X01: 
                    switch (ins.rt) {
                        case 0b00000: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BLTZ     %s, 0X%04X            \n",             pc, ins.value, register_names[ins.rs], ins.immediate16); break;
                        case 0b00001: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BGEZ     %s, 0X%04X            \n",             pc, ins.value, register_names[ins.rs], ins.immediate16); break;
                        case 0b10000: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BLTZAL   %s, 0X%04X %s = 0X%08X\n", pc, ins.value, register_names[ins.rs], ins.immediate16, register_names[31], pc); break;
                        case 0b10001: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BGEZAL   %s, 0X%04X %s = 0X%08X\n", pc, ins.value, register_names[ins.rs], ins.immediate16, register_names[31], pc); break;
                    }
                    break;
                        case 0X02: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    J        0X%08X                   \n",              pc, ins.value, (pc & 0XF0000000) | (ins.target << 2)); break;
                        case 0X03: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    JAL      0X%08X, %s = 0X%08X      \n", pc, ins.value, (pc & 0XF0000000) | (ins.target << 2), register_names[31], pc); break;
                        case 0X04: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BEQ      %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rs], register_names[ins.rt], ins.immediate16); break;
                        case 0X05: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BNE      %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rs], register_names[ins.rt], ins.immediate16); break;
                        case 0X06: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BLEZ     %s, 0X%04X               \n",          pc, ins.value, register_names[ins.rs], ins.immediate16); break;
                        case 0X07: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    BGTZ     %s, 0X%04X               \n",          pc, ins.value, register_names[ins.rs], ins.immediate16); break;
                        case 0X08: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ADDI     %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X09: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ADDIU    %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0A: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLTI     %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0B: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SLTIU    %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0C: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ANDI     %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0D: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    ORI      %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0E: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    XORI     %s, %s, 0X%04X           \n",      pc, ins.value, register_names[ins.rt], register_names[ins.rs], ins.immediate16); break;
                        case 0X0F: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LUI      %s, 0X%04X               \n",          pc, ins.value, register_names[ins.rt], ins.immediate16); break;
                        case 0X20: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LB       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X21: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LH       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X22: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWL      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X23: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LW       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X24: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LBU      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X25: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LHU      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X26: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWR      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X28: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SB       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X29: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SH       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X2A: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWL      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X2B: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SW       %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X2E: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWR      %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        // COsprOCESSOR instructions                        
                        case 0X10: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    COP0     0X%08X                   \n",              pc, ins.value, ins.immediate25); break;
                        case 0X11: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    COP1     0X%08X                   \n",              pc, ins.value, ins.immediate25); break;
                        case 0X12: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    COP2     0X%08X                   \n",              pc, ins.value, ins.immediate25); break;
                        case 0X13: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    COP3     0X%08X                   \n",              pc, ins.value, ins.immediate25); break;
                        case 0X30: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWC0     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X31: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWC1     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X32: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWC2     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X33: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    LWC3     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X38: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWC0     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X39: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWC1     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X3A: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWC2     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
                        case 0X3B: sprintf(ptr, "PC: 0X%08X    HEX: 0X%08X    SWC3     %s, 0X%04X(%s)           \n",    pc, ins.value, register_names[ins.rt], ins.immediate16, register_names[ins.rs]); break;
            }
            switch (i) {
                case 0: strncpy(debugger.main_assembly[j], ptr, MAXLEN); break;
                case 1: strncpy(debugger.exp1_assembly[j], ptr, MAXLEN); break;
                case 2: strncpy(debugger.bios_assembly[j], ptr, MAXLEN); break;
            }
        }
    }
}
