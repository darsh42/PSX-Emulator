#ifndef INSTRUCTION_H_INCLUDED 
#define INSTRUCTION_H_INCLUDED

#include "common.h"

union INSTRUCTION {
    // R-Type
    struct {
        uint32_t funct: 6;
        uint32_t shamt: 5;
        uint32_t rd: 5;
        uint32_t rt: 5;
        uint32_t rs: 5;
        uint32_t op: 6;
    };
    // COPn
    struct {
        uint32_t : 21;
        uint32_t cop_funct: 4;
        uint32_t cop_type: 1;
        uint32_t : 6;
    };
    uint32_t value;
    uint32_t target: 26;
    uint16_t immediate16;
    uint32_t immediate25: 25;
     int16_t relative;
};

#define FOREACH_MNEUMONIC(CMD) \
                 CMD(SLL)        \
                 CMD(SRL)        \
                 CMD(SRA)        \
                 CMD(SLLV)       \
                 CMD(SRLV)       \
                 CMD(SRAV)       \
                 CMD(JR)         \
                 CMD(JALR)       \
                 CMD(SYSCALL)    \
                 CMD(BREAK)      \
                 CMD(MFHI)       \
                 CMD(MTHI)       \
                 CMD(MFLO)       \
                 CMD(MTLO)       \
                 CMD(MULT)       \
                 CMD(MULTU)      \
                 CMD(DIV)        \
                 CMD(DIVU)       \
                 CMD(ADD)        \
                 CMD(ADDU)       \
                 CMD(SUB)        \
                 CMD(SUBU)       \
                 CMD(AND)        \
                 CMD(OR)         \
                 CMD(XOR)        \
                 CMD(NOR)        \
                 CMD(SLT)        \
                 CMD(SLTU)       \
                 CMD(BLTZ)       \
                 CMD(BGEZ)       \
                 CMD(BLTZAL)     \
                 CMD(BGEZAL)     \
                 CMD(J)          \
                 CMD(JAL)        \
                 CMD(BEQ)        \
                 CMD(BNE)        \
                 CMD(BLEZ)       \
                 CMD(BGTZ)       \
                 CMD(ADDI)       \
                 CMD(ADDIU)      \
                 CMD(SLTI)       \
                 CMD(SLTIU)      \
                 CMD(ANDI)       \
                 CMD(ORI)        \
                 CMD(XORI)       \
                 CMD(LUI)        \
                 CMD(LB)         \
                 CMD(LH)         \
                 CMD(LWL)        \
                 CMD(LW)         \
                 CMD(LBU)        \
                 CMD(LHU)        \
                 CMD(LWR)        \
                 CMD(SB)         \
                 CMD(SH)         \
                 CMD(SWL)        \
                 CMD(SW)         \
                 CMD(SWR)        \
                 CMD(COP0)       \
                 CMD(COP1)       \
                 CMD(COP2)       \
                 CMD(COP3)       \
                 CMD(LWC0)       \
                 CMD(LWC1)       \
                 CMD(LWC2)       \
                 CMD(LWC3)       \
                 CMD(SWC0)       \
                 CMD(SWC1)       \
                 CMD(SWC2)       \
                 CMD(SWC3)       \

#define FOREACH_OPCODE(CMD) \
             CMD(0X00 << 0) \
             CMD(0X02 << 0) \
             CMD(0X03 << 0) \
             CMD(0X04 << 0) \
             CMD(0X06 << 0) \
             CMD(0X07 << 0) \
             CMD(0X08 << 0) \
             CMD(0X09 << 0) \
             CMD(0X0C << 0) \
             CMD(0X0D << 0) \
             CMD(0X10 << 0) \
             CMD(0X11 << 0) \
             CMD(0X12 << 0) \
             CMD(0X13 << 0) \
             CMD(0X18 << 0) \
             CMD(0X19 << 0) \
             CMD(0X1A << 0) \
             CMD(0X1B << 0) \
             CMD(0X20 << 0) \
             CMD(0X21 << 0) \
             CMD(0X22 << 0) \
             CMD(0X23 << 0) \
             CMD(0X24 << 0) \
             CMD(0X25 << 0) \
             CMD(0X26 << 0) \
             CMD(0X27 << 0) \
             CMD(0X2A << 0) \
             CMD(0X2B << 0) \
             CMD(0X01 << 26 | 0X00 << 16) \
             CMD(0X01 << 26 | 0X01 << 16) \
             CMD(0X01 << 26 | 0X0F << 16) \
             CMD(0X01 << 26 | 0X10 << 16) \
             CMD(0X02 << 26) \
             CMD(0X03 << 26) \
             CMD(0X04 << 26) \
             CMD(0X05 << 26) \
             CMD(0X06 << 26) \
             CMD(0X07 << 26) \
             CMD(0X08 << 26) \
             CMD(0X09 << 26) \
             CMD(0X0A << 26) \
             CMD(0X0B << 26) \
             CMD(0X0C << 26) \
             CMD(0X0D << 26) \
             CMD(0X0E << 26) \
             CMD(0X0F << 26) \
             CMD(0X20 << 26) \
             CMD(0X21 << 26) \
             CMD(0X22 << 26) \
             CMD(0X23 << 26) \
             CMD(0X24 << 26) \
             CMD(0X25 << 26) \
             CMD(0X26 << 26) \
             CMD(0X28 << 26) \
             CMD(0X29 << 26) \
             CMD(0X2A << 26) \
             CMD(0X2B << 26) \
             CMD(0X2E << 26) \
             CMD(0X10 << 26) \
             CMD(0X11 << 26) \
             CMD(0X12 << 26) \
             CMD(0X13 << 26) \
             CMD(0X30 << 26) \
             CMD(0X31 << 26) \
             CMD(0X32 << 26) \
             CMD(0X33 << 26) \
             CMD(0X38 << 26) \
             CMD(0X39 << 26) \
             CMD(0X3A << 26) \
             CMD(0X3B << 26) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_OPCODES(INT) INT,

#endif//INSTRUCTION_H_INCLUDED
