#ifndef DEBUG_MEMORY_H_INCLUDED
#define DEBUG_MEMORY_H_INCLUDED

#include "../core/error.h"
#include "../core/common.h"
#include "../core/memory.h"
#include "../core/instruction.h"

#define print_debug_memory_error(func) print_error("debug_error", func)

extern struct MEMORY *_memory();
extern PSX_ERROR memory_load_bios(const char *filebios);
extern PSX_ERROR memory_cpu_load_32bit(uint32_t address, uint32_t *result);

#endif//DEBUG_MEMORY_H_INCLUDED
