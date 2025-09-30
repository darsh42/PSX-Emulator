#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <limits.h>
#include <stdint.h>

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
    CPU_REG_MAX
};

enum cpu_load_delay
{
    UNUSED,
    TRANSFER,
    DELAY
};

struct stack_entry {
    uint32_t call_to;
    uint32_t call_at;
};
#define SS_SIZE 32
struct shadow_stack {
    uint32_t size;
    uint32_t head;

    struct stack_entry items[SS_SIZE];
};
struct cpu {
    uint32_t r[32];

    uint32_t cycles;

    uint32_t pc;
    uint32_t cir;

    /* load delay destinaion and value */
    uint32_t load_d, load_v;
    enum cpu_load_delay load_s;

    /* branch delay value */
    uint32_t branch_v;
    enum cpu_load_delay branch_s;

    uint32_t hi, lo;

    uint32_t imm25;
    uint32_t target;
    uint32_t relative;
    
    /* instruction components */
    uint8_t funct;
    uint8_t shamt;
    uint8_t rd;
    uint8_t rt;
    uint8_t rs;
    uint8_t op;

    uint16_t imm16;

    /* sideloading exe */
    const char *sideload_exe;

    /* shadow stack */
    struct shadow_stack ss;
};

uint32_t cpu_cop0_sr_isc( void );
void write_cpu_reg(enum cpu_reg_e r, uint32_t  data);
void  read_cpu_reg(enum cpu_reg_e r, uint32_t *data);

void init_cpu(const char *file_bios, const char *file_exe);
void task_cpu( void );

#endif // CPU_H_INCLUDED
