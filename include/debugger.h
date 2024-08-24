#ifndef DEBUGGER_H_INCLUDED
#define DEBUGGER_H_INCLUDED

#include <signal.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <readline/rltypedefs.h>
#include "common.h"
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "psx.h"
#include "instruction.h"

#define MAX_BREAK 100
#define MAX_WATCH 100
#define MAX_MEMORY_VIEW 512

typedef enum breakpoint_type {
    BP_EMPTY,
    BP_INS,
    BP_CON
} bp_t;

typedef struct breakpoint {
    int number;

    bp_t type;

    uint32_t opcode;
    uint32_t address;
} bp;

typedef enum watchpoint_register {
    WP_NO_REG,
    WP_CPU,
    WP_GPU,
    WP_SPU,
} wp_r;

typedef enum watchpoint_type {
    WP_EMPTY,
    WP_MEM,
    WP_REG
} wp_t;

typedef struct watchpoint {
    int number;
    
    wp_t type;
    wp_r regtype;

    uint32_t value;
    uint32_t location;
} wp;

typedef struct command {
    char *name;
    char *help;
    rl_icpfunc_t *func;
} COMMAND;

typedef struct argument {
    char *name;
    char *help;
} ARGUMENT;

struct DEBUGGER {
    struct PSX *psx;

    /* debug variables */
    int bp_count;
    int wp_count;
    bp bps[MAX_BREAK];
    wp wps[MAX_WATCH];

    bool stepping;
    bool logging;
    volatile sig_atomic_t paused;
};

struct PSX *get_psx(void);

extern void peek_cpu_instruction();

extern void gdb_stub_init(void);
extern void gdb_stub_process(void);
extern void gdb_stub_deinit(void);
#endif // DEBUGGER_H_INCLUDED
