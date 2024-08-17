#ifndef COPROCESSOR0_H_INCLUDED
#define COPROCESSOR0_H_INCLUDED

#include "error.h"
#include "common.h"
#include "instruction.h"

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
        struct {
            uint32_t : 8;
            uint32_t Index: 6;
            uint32_t : 17;
            uint32_t P: 1;
        };
    } INDX;
    union {
        uint32_t value;
        struct {
            uint32_t : 8;
            uint32_t Index: 6;
            uint32_t : 18;
        };
    } RAND;
    union {
        uint32_t value;
        struct {
            uint32_t : 8;
            uint32_t G: 1;
            uint32_t V: 1;
            uint32_t D: 1;
            uint32_t N: 1;
            uint32_t FPN: 20;
        };
    } TLBL;
    union {
        uint32_t value;
    } BPC;
    union {
        uint32_t value;
        struct {
            uint32_t : 2;
            uint32_t BADV: 19;
            uint32_t PTEB: 11;
        };
    } CTXT;
    union {
        uint32_t value;
    } BDA;
    union {
        uint32_t value;
        struct {
        };
    } PIDMASK;
    union {
        uint32_t value;
    } DCIC;
    union {
        uint32_t value;
        struct {
            uint32_t BADV: 32;
        };
    } BADV;
    union {
        uint32_t value;
    } BDAM;
    union {
        uint32_t value;
        struct {
            uint32_t : 6;
            uint32_t PID: 6;
            uint32_t VPN: 20;
        };
    } TLBH;
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
        };
    } SR;
    union {
        uint32_t value;
        struct {
            uint32_t                  : 2;
            uint32_t           EXECODE: 5;
            uint32_t SoftwareInterrupt: 2;
            uint32_t                  : 1;
            uint32_t InterruptsPending: 6;
            uint32_t                  : 12;
            uint32_t  CoprocessorError: 2;
            uint32_t                  : 1;
            uint32_t       BranchDelay: 1;
        };
    } CAUSE;
    union {
        uint32_t value;
        struct {
            uint32_t return_address: 32;
        };
    } EPC;
    union {
        uint32_t value;
        struct {
            uint32_t Rev:8;
            uint32_t Imp:8;
            uint32_t :16;
        };
    } PIRD;
};

#endif//COPROCESSOR0_H_INCLUDED
