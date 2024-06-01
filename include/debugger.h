#ifndef DEBUGGER_H_INCLUDED
#define DEBUGGER_H_INCLUDED

#include "common.h"
#include "sdl.h"
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "instruction.h"
#include "coprocessor0.h"
#include "coprocessor2.h"

#define MAXLEN 100
#define CPU_REGISTER_MAXLEN 30

extern PSX_ERROR sdl_handle_events(SDL_Event *, void (*)(void), int (*)(SDL_Event *));
extern PSX_ERROR sdl_render_clear(void);
extern PSX_ERROR sdl_render_present(void);

char ***get_main_disassembly(void);
char ***get_bios_disassembly(void);
char ***get_exp1_disassembly(void);

// device refrence getters
extern struct CPU *get_cpu(void);
extern struct GPU *get_gpu(void);
extern struct DMA *get_dma(void);
extern struct MEMORY *get_memory(void);
extern struct SDL_HANDLER *sdl_return_handler(void);

// memory access functions
extern void memory_load_bios(const char *filebios);
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);

struct DEBUGGER {
    uint32_t running;
    uint32_t fontscale;

    struct nk_context  *ctx;

    // device refrences
    struct CPU *cpu;
    struct GPU *gpu;
    struct DMA *dma;
    struct MEMORY *memory;
    struct SDL_HANDLER *sdl_handler;

    // Disassembly
    uint32_t assembly_lines;
    char     bios_assembly[0X20000][MAXLEN];
    char     main_assembly[0X80000][MAXLEN];
    char     exp1_assembly[0X200000][MAXLEN];

    uint32_t asm_base_address;
    bool     focus_pc;

    // System Registers
    char cpu_registers[67][CPU_REGISTER_MAXLEN];

    // Breakpoints
    uint32_t *breakpoint_addrs;
    uint32_t  breakpoint_count;
    bool      breakpoint_hit;
};

// debug functions
extern void disassemble(void);
extern void peek_cpu_registers(void);


#endif // DEBUGGER_H_INCLUDED
