#ifndef PSX_H_INCLUDED
#define PSX_H_INCLUDED

// utility headers
#include "error.h"
#include "common.h"

// device headers
#include "memory.h"

// macros
#define print_psx_error(func, format, ...) print_error("psx.c", func, format, __VA_ARGS__)

// device functions
// memory
extern PSX_ERROR memory_load_bios(const char *filebios);

// cpu
extern PSX_ERROR cpu_reset(void);
extern PSX_ERROR cpu_fetch(void);
extern PSX_ERROR cpu_decode(void);
extern PSX_ERROR cpu_execute(void);
extern PSX_ERROR coprocessor_initialize(void);

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
