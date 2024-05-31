#ifndef DEBUG_MEMORY_H_INCLUDED
#define DEBUG_MEMORY_H_INCLUDED

#include "error.h"
#include "common.h"
#include "memory.h"
#include "instruction.h"

#define print_debug_memory_error(func, format, ...) print_error("debug_error", func, format, __VA_ARGS__)

extern struct DEBUGGER *get_debugger();
extern struct MEMORY *get_memory();
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);

#endif//DEBUG_MEMORY_H_INCLUDED
