#ifndef COPROCESSOR0_H_INCLUDED
#define COPROCESSOR0_H_INCLUDED

#include "error.h"
#include "common.h"
#include "instruction.h"

#define copn_imm25 coprocessor.instruction.COPN_IMMEDIATE25.imm

struct COPROCESSOR_0 {
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
    uint32_t *R[16];
    union {
        uint32_t value;
    } BCP;
    union {
        uint32_t value;
    } BDA;
    union {
        uint32_t value;
    } JUMPDEST;
    union {
        uint32_t value;
    } DCIC;
    union {
        uint32_t value;
        struct {
            uint32_t badaddress:32;
        } reg;
    } BadVaddr;
    union {
        uint32_t value;
    } BDAM;
    union {
        uint32_t value;
    } BCPM;
    union {
        uint32_t value;
        struct {
            uint32_t IEc:1;  // Current interrupt enable (0=disable, 1=enable)
            uint32_t KUc:1;  // Current kernel usermode (0=kernel, 1=user)
            uint32_t IEp:1;  // Previos interrupt disable
            uint32_t KUp:1;  // Previos kernel mode
            uint32_t IEo:1;  // Old interrupt disable
            uint32_t KUo:1;  // Old kernal/user mode
            uint32_t    :1;
            uint32_t    :1;
            uint32_t  Im:8;  // 8 bit interrupt mask
            uint32_t Isc:1;  // Isolate cache (0=No, 1=Isolate) (when isolated all read and writes targeted to cache)
            uint32_t Swc:1;  // Swapped cache mode (0=Normal, 1=Swapped) (instructions will act as data cache)
            uint32_t  PZ:1;  // When set cache parity bits are written as 0
            uint32_t  CM:1;  // Shows result of last load operation with D-cache isolated
            uint32_t  PE:1;  // Cache parity error (no interrupt occurs)
            uint32_t  TS:1;  // TLB shutdown, (if 2 TLBs are matched)
            uint32_t BEV:1;  // Boot exception vectors in RAM/ROM (0=RAM/KSEG0, 1=ROM/KSEG1)
            uint32_t    :1;
            uint32_t    :1;
            uint32_t  RE:1;  // Reverse endianess (0=Normal, 1=Reversed)
            uint32_t    :1;
            uint32_t    :1;
            uint32_t CU0:1;  // COP0 enable (0=Enable only in kernal mode, 1=Kernal and user)
            uint32_t CU1:1;  // COP1 enable (0=disable, 1=enable) not in psx
            uint32_t CU2:1;  // COP2 enable (0=disable, 1=enable) GTE in psx
            uint32_t CU3:1;  // COP3 enable (0=disable, 1=enable) not in psx
        } reg;
    } SR;
    union {
        uint32_t value;
        struct {
            uint32_t       :1; 
            uint32_t       :1; 
            uint32_t excode:4; // Exception type that occured
            uint32_t       :1;  
            uint32_t     Ip:8; // Interrupt pending field
            uint32_t       :12;
            uint32_t     CE:2; // ??
            uint32_t       :1;
            uint32_t     BD:1; // Branch delay (set the exception point to the branch instruction instead of the branch delay instruction)
        } reg;
    } CAUSE;
    union {
        uint32_t value;
        struct {
            uint32_t return_address:32;
        } reg;
    } EPC;
    union {
        uint32_t value;
        struct {
            uint32_t revision:8;
            uint32_t implementation:8;
            uint32_t :16;
        } reg;
    } PIRD;
};

#endif//COPROCESSOR0_H_INCLUDED
