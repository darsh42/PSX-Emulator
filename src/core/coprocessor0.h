#ifndef COPROCESSOR0_H_INCLUDED
#define COPROCESSOR0_H_INCLUDED

#include "error.h"
#include "common.h"
#include "instruction.h"

#define copn_imm25 coprocessor.instruction.COPN_IMMEDIATE25.imm

struct COPROCESSOR0 {
    // REGISTERS
    // r0-r2      N/A
    // r3         BCP - break point on execute
    // r4         N/A
    // r5         BDA - break point on data access
    // r6         JUMPDEST - randomly memorized jump address
    // r7         DCIC - breakpoint control
    // r8         BaddVaddr - bad virtual address
    // r9         BDAM - data access breakpoint mask
    // r10        N/A
    // r11        BCPM - execute breakpoint mask
    // r12        SR - system status register
    // r13        CAUSE - describes most recently recognised exception
    // r14        EPC - return address from trap
    // r15        PIRD - processor id
    // r16-r31    Garbage 
    // r32-r63    N/A - control registers NOTE: (not implemented here)
    uint32_t R[32];

    union COPROCESSOR_INSTRUCTION instruction;
};

#endif//COPROCESSOR0_H_INCLUDED
