#ifndef PSX_H_INCLUDED
#define PSX_H_INCLUDED

// utility headers
#include "error.h"
#include "common.h"

// device headers
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "timers.h"

// macros
#define print_psx_error(func, format, ...) print_error("psx.c", func, format, __VA_ARGS__)

// device functions
// pointers
extern struct CPU *get_cpu(void);
extern struct GPU *get_gpu(void);
extern struct DMA *get_dma(void);
extern struct MEMORY *get_memory(void);
extern struct TIMERS *get_timers(void);

// memory
extern PSX_ERROR memory_load_bios(const char *filebios);

// timers
extern PSX_ERROR timers_create(void);
extern PSX_ERROR timers_step(void);

// cpu
extern PSX_ERROR cpu_reset(void);
extern PSX_ERROR cpu_step(void);
extern PSX_ERROR coprocessor_initialize(void);

// gpu
extern void gpu_reset(void);
extern void gpu_step(void);

// dma
extern PSX_ERROR dma_reset(void);
extern PSX_ERROR dma_step(void);

// gui and sdl
void debugger_reset(void);
void debugger_exec(void);
PSX_ERROR debugger_destroy(void);


void sdl_initialize(void);
void sdl_destroy(void);
PSX_ERROR sdl_update(void);

#ifdef DEBUG
// disassembler
extern void disassemble(void);

// cpu
extern void set_debug_cpu(void);
extern void peek_cpu_pc(void);
extern void peek_cpu_R_registers(void);
extern void peek_cpu_instruction(void);
extern void peek_cpu_mult_div_registers(void);

// co processor
extern void peek_coprocessor_n_registers(int cop_n);
#endif

#endif//PSX_H_INCLUDED
