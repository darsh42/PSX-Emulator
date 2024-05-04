#ifndef DEBUG_MEMORY_H_INCLUDED
#define DEBUG_MEMORY_H_INCLUDED

#include "../core/error.h"
#include "../core/common.h"
#include "../core/memory.h"
#include "../core/instruction.h"

#define print_debug_memory_error(func, format, ...) print_error("debug_error", func, format, __VA_ARGS__)

extern struct MEMORY *_memory();
extern void memory_load_bios(const char *filebios);
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);

#endif//DEBUG_MEMORY_H_INCLUDED
