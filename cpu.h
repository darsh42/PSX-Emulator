#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <pthread.h>
#include <stdint.h>
#include <limits.h>

#ifdef CPU_PRIVATE

#include "trace.h"

#ifdef ENABLE_CPU_TRACE
#define TRACE_CPU(function, format, ...) trace("cpu.c", function, format, __VA_ARGS__)
#else
#define TRACE_CPU(function, format, ...) 
#endif

/* main opcode breakdown */
#define FUNCT    ((cpu.cir >>  0) & 0x3F)
#define SHAMT    ((cpu.cir >>  6) & 0x1F)
#define RD       ((cpu.cir >> 11) & 0x1F)
#define RT       ((cpu.cir >> 16) & 0x1F)
#define RS       ((cpu.cir >> 21) & 0x1F)
#define OP       ((cpu.cir >> 26) & 0x3F)
#define TARGET    (cpu.cir & ((1 << 26) - 1))
#define IMM16     (cpu.cir & ((1 << 16) - 1))
#define S_IMM16   sign16(IMM16)
#define IMM25     (cpu.cir & ((1 << 25) - 1))
#define RELATIVE  (cpu.cir & ((1 << 16) - 1))

/* coprocessor opcode breakdown */
#define COP_TYPE ((cpu.cir >> 25) & 0x1)
#define COP_FUNC ((cpu.cir >> 21) & 0x7)

/* coprocessor register breakdown */
#define COP0_BPC         3
#define COP0_BDA         5
#define COP0_JUMPDEST    6
#define COP0_DCIC        7
#define COP0_BAD_VADDR   8
#define COP0_BDAM        9
#define COP0_BPCM       11
#define COP0_SR         12
#define COP0_CAUSE      13
#define COP0_EPC        14
#define COP0_PRID       15

/* cop0 status register*/

#define sign8(a)  (int32_t) (int8_t)  a
#define sign16(a) (int32_t) (int16_t) a
#define sign32(a) (int32_t)           a
#define sign64(a) (int64_t) (int32_t) a
#define overflow(a, b) (a > 0 && (a + b) > 0xffffffff)
#define underflow(a,b) ((b < 0) && (a > INT_MAX + b)) 

#define reg(R) cpu.r[R]
#define copn_reg(N, R) cpu.STRCAT(cop, N)[R]

enum cpu_exception_type 
{
    INT     = 0x00,
    MOD     = 0x01,
    TLBL    = 0x02,
    TLBS    = 0x03,
    AdEL    = 0x04,
    AdES    = 0x05,
    IBE     = 0x06,
    DBE     = 0x07,
    SYSCALL = 0x08,
    BP      = 0x09,
    RI      = 0x0A,
    CpU     = 0x0B,
    Ov      = 0x0C
};

enum cpu_load_delay 
{
    UNUSED,
    TRANSFER,
    DELAY
};

/* coprocessor 0 SR struct */
union cop0_sr
{
    uint32_t value;
    struct {
        uint32_t IEc: 1;
        uint32_t KUc: 1;
        uint32_t IEp: 1;
        uint32_t KUp: 1;
        uint32_t IEo: 1;
        uint32_t KUo: 1;
        uint32_t    : 2;
        uint32_t Im : 8;
        uint32_t Isc: 1;
        uint32_t Swc: 1;
        uint32_t PZ : 1;
        uint32_t CM : 1;
        uint32_t PE : 1;
        uint32_t TS : 1;
        uint32_t BEV: 1;
        uint32_t    : 2;
        uint32_t RE : 1;
        uint32_t    : 2;
        uint32_t CU0: 1;
        uint32_t CU1: 1;
        uint32_t CU2: 1;
        uint32_t CU3: 1;
    };
};

/* coprocessor 0 CAUSE struct */
union cop0_cause
{
    uint32_t value;
    struct {
        uint32_t             :  2;
        uint32_t excode      :  5;
        uint32_t             :  1;
        uint32_t Ip          :  8;
        uint32_t             : 12;
        uint32_t CE          :  2;
        uint32_t             :  1;
        uint32_t branch_delay:  1;
    };
};

struct cpu 
{
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

extern uint32_t cpu_cop0_sr_isc( void );

extern void *task_cpu( void *ignore );

#endif // CPU_H_INCLUDED
