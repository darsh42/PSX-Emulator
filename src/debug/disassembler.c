#include "../core/common.h"
#include "../core/instruction.h"

extern void memory_load_bios(const char *filebios);
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);

struct disassembler {
    uint32_t pc;
    union INSTRUCTION current;
} DA;

static void print_r_type(const char *mneumonic) {
    printf("[DISASSEMBLY]: PC: %08X:      HEX: %08X      ", DA.pc, DA.current.value);
    printf("%s rd=%02d, rt=%02d, rs=%02d, op=%02X, funct=%X, shamt=%X\n", mneumonic,
                                                                          DA.current.rd,
                                                                          DA.current.rt,
                                                                          DA.current.rs,
                                                                          DA.current.op,
                                                                          DA.current.funct,
                                                                          DA.current.shamt);
}

static void print_i_type(const char *mneumonic) {
    printf("[DISASSEMBLY]: PC: %08X:      HEX: %08X      ", DA.pc, DA.current.value);
    printf("%s         rt=%02d, rs=%02d, op=%02X, immediate=%X\n", mneumonic,
                                                                   DA.current.rt,
                                                                   DA.current.rs,
                                                                   DA.current.op, 
                                                                   DA.current.immediate16);
}
static void print_j_type(const char *mneumonic) {
    uint32_t target = (DA.pc & 0Xf0000000) + (DA.current.target << 2);
    printf("[DISASSEMBLY]: PC: %08X:      HEX: %08X      ", DA.pc, DA.current.value);
    printf("%s                       op=%02X, target=%X\n", mneumonic,
                                                            DA.current.op, 
                                                            target);
}

void disassemble(void) {
    DA.pc = 0XBFC00000;

    while (DA.pc < 0XBFC80000) {
        memory_cpu_load_32bit(DA.pc, &DA.current.value);
        switch (DA.current.op) {
            case 0X00: 
                // RTYPE
                switch (DA.current.funct) {
                    case 0X00: print_r_type("SLL     "); break;
                    case 0X02: print_r_type("SRL     "); break;
                    case 0X03: print_r_type("SRA     "); break;
                    case 0X04: print_r_type("SLLV    "); break;
                    case 0X06: print_r_type("SRLV    "); break;
                    case 0X07: print_r_type("SRAV    "); break;
                    case 0X08: print_r_type("JR      "); break;
                    case 0X09: print_r_type("JALR    "); break;
                    case 0X0C: print_r_type("SYSCALL "); break;
                    case 0X0D: print_r_type("BREAK   "); break;
                    case 0X10: print_r_type("MFHI    "); break;
                    case 0X11: print_r_type("MTHI    "); break;
                    case 0X12: print_r_type("MFLO    "); break;
                    case 0X13: print_r_type("MTLO    "); break;
                    case 0X18: print_r_type("MULT    "); break;
                    case 0X19: print_r_type("MULTU   "); break;
                    case 0X1A: print_r_type("DIV     "); break;
                    case 0X1B: print_r_type("DIVU    "); break;
                    case 0X20: print_r_type("ADD     "); break;
                    case 0X21: print_r_type("ADDU    "); break;
                    case 0X22: print_r_type("SUB     "); break;
                    case 0X23: print_r_type("SUBU    "); break;
                    case 0X24: print_r_type("AND     "); break;
                    case 0X25: print_r_type("OR      "); break;
                    case 0X26: print_r_type("XOR     "); break;
                    case 0X27: print_r_type("NOR     "); break;
                    case 0X2A: print_r_type("SLT     "); break;
                    case 0X2B: print_r_type("SLTU    "); break;

                } 
                break;
            case 0X01: 
                switch (DA.current.rt) {
                    case 0b00000: print_i_type("BLTZ   "); break;
                    case 0b00001: print_i_type("BGEZ   "); break;
                    case 0b10000: print_i_type("BLTZAL "); break;
                    case 0b10001: print_i_type("BGEZAL "); break;
                }
                break;
            case 0X02: print_j_type("J      "); break;
            case 0X03: print_j_type("JAL    "); break;
            case 0X04: print_i_type("BEQ    "); break;
            case 0X05: print_i_type("BNE    "); break;
            case 0X06: print_i_type("BLEZ   "); break;
            case 0X07: print_i_type("BGTZ   "); break;
            case 0X08: print_i_type("ADDI   "); break;
            case 0X09: print_i_type("ADDIU  "); break;
            case 0X0A: print_i_type("SLTI   "); break;
            case 0X0B: print_i_type("SLTIU  "); break;
            case 0X0C: print_i_type("ANDI   "); break;
            case 0X0D: print_i_type("ORI    "); break;
            case 0X0E: print_i_type("XORI   "); break;
            case 0X0F: print_i_type("LUI    "); break;
            case 0X20: print_i_type("LB     "); break;
            case 0X21: print_i_type("LH     "); break;
            case 0X22: print_i_type("LWL    "); break;
            case 0X23: print_i_type("LW     "); break;
            case 0X24: print_i_type("LBU    "); break;
            case 0X25: print_i_type("LHU    "); break;
            case 0X26: print_i_type("LWR    "); break;
            case 0X28: print_i_type("SB     "); break;
            case 0X29: print_i_type("SH     "); break;
            case 0X2A: print_i_type("SWL    "); break;
            case 0X2B: print_i_type("SW     "); break;
            case 0X2E: print_i_type("SWR    "); break;
            // COPROCESSOR instructions
            case 0X10: print_i_type("COP0   "); break;
            case 0X11: print_i_type("COP1   "); break;
            case 0X12: print_i_type("COP2   "); break;
            case 0X13: print_i_type("COP3   "); break;
            case 0X30: print_i_type("LWC0   "); break;
            case 0X31: print_i_type("LWC1   "); break;
            case 0X32: print_i_type("LWC2   "); break;
            case 0X33: print_i_type("LWC3   "); break;
            case 0X38: print_i_type("SWC0   "); break;
            case 0X39: print_i_type("SWC1   "); break;
            case 0X3A: print_i_type("SWC2   "); break;
            case 0X3B: print_i_type("SWC3   "); break;
        }
        DA.pc += 4;
    }
}
