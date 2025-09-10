#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <stdint.h>
#include <limits.h>

enum cpu_reg_e {
    CPU_ZERO = 0,
    CPU_AT,
    CPU_V0, CPU_V1,
    CPU_A0, CPU_A1, CPU_A2, CPU_A3,
    CPU_T0, CPU_T1, CPU_T2, CPU_T3, CPU_T4, CPU_T5, CPU_T6, CPU_T7,
    CPU_S0, CPU_S1, CPU_S2, CPU_S3, CPU_S4, CPU_S5, CPU_S6, CPU_S7,
    CPU_T8, CPU_T9,
    CPU_K0, CPU_K1,
    CPU_GP,
    CPU_SP,
    CPU_FP,
    CPU_RA,

    CPU_HI,
    CPU_LO,
    CPU_PC,
    CPU_CIR,
};

enum cpu_load_delay
{
    UNUSED,
    TRANSFER,
    DELAY
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

    /* sideloading exe */
    const char *sideload_exe;
};

uint32_t cpu_cop0_sr_isc( void );
void write_cpu_reg(enum cpu_reg_e r, uint32_t  data);
void  read_cpu_reg(enum cpu_reg_e r, uint32_t *data);

void init_cpu(const char *file_bios, const char *file_exe);
void task_cpu( void );

#endif // CPU_H_INCLUDED
