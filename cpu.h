#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <pthread.h>
#include <stdint.h>
#include <limits.h>

#ifdef CPU_PRIVATE

#define STRINGIZE_NX(A) #A

/* main opcode breakdown */
#define FUNCT    ((cpu.cir >>  0) & 0x3F)
#define SHAMT    ((cpu.cir >>  6) & 0x1F)
#define RD       ((cpu.cir >> 11) & 0x1F)
#define RT       ((cpu.cir >> 16) & 0x1F)
#define RS       ((cpu.cir >> 21) & 0x1F)
#define OP       ((cpu.cir >> 26) & 0x3F)
#define TARGET    (cpu.cir & ((1 << 26) - 1))
#define IMM16     (cpu.cir & ((1 << 16) - 1))
#define IMM25     (cpu.cir & ((1 << 25) - 1))
#define RELATIVE  (cpu.cir & ((1 << 16) - 1))

/* coprocessor opcode breakdown */
#define COP_TYPE ((cpu.cir >> 25) & 0x1)
#define COP_FUNC ((cpu.cir >> 21) & 0x7)

/* coprocessor register breakdown */
#define COP0_BPC         2
#define COP0_BDA         4
#define COP0_JUMPDEST    5
#define COP0_DCIC        6
#define COP0_BAD_VADDR   7
#define COP0_BDAM        8
#define COP0_SR          9
#define COP0_CAUSE      10
#define COP0_EPC        11
#define COP0_PRID       12

/* cop0 status register*/

#define sign8(a)  (int32_t) (int8_t)  a
#define sign16(a) (int32_t) (int16_t) a
#define sign32(a) (int32_t)           a
#define sign64(a) (int64_t) (int32_t) a
#define overflow(a, b) (a > 0 && (a + b) > 0xffffffff)
#define underflow(a,b) ((b < 0) && (a > INT_MAX + b)) 

#define reg(R) cpu.r[R]
#define copn_reg(N, R) cpu.STRCAT(cop, N)[R]

enum cpu_exception_type {
    OVF,
    CPU,
    SYS,
    BP
};

enum cpu_load_delay {
    UNUSED,
    TRANSFER,
    DELAY
};

struct cpu {
    uint32_t pc;
    uint32_t cir;
    uint32_t r[32];
    uint32_t hi, lo;
    
    /* coprocessor registers */
    uint32_t cop0[16];
    uint32_t cop2[64];

    uint32_t cycles;
    
    /* load delay destinaion and value */
    uint32_t load_d, load_v;
    enum cpu_load_delay load_s;

    /* branch delay value */
    uint32_t branch_v;
    enum cpu_load_delay branch_s;
};

#endif // CPU_PRIVATE

extern void *task_cpu( void *ignore );

#endif // CPU_H_INCLUDED
