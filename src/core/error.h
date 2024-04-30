#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#define DEBUG

typedef enum PSX_ERROR {
    NO_ERROR,
    // PSX
    INSUFFICIENT_ARGS,
    // CPU
    CPU_FETCH_ERROR,
    CPU_DECODE_ERROR,
    CPU_EXECUTE_ERROR,
    // INSTRUCTIONS
    RTYPE_ERROR,
    ITYPE_ERROR,
    JTYPE_ERROR,
    UNKNOWN_OPCODE,
    
    // MEMORY
    BIOS_FILE_NOT_FOUND,
    BIOS_FILE_UNREADABLE,
    MEMORY_CPU_UNMAPPED_ADDRESS,
    MEMORY_UNALIGNED_ADDRESS,
} PSX_ERROR;

#ifdef DEBUG
extern void peek_cpu_registers(void);
#endif

#endif//ERROR_H_INCLUDED
