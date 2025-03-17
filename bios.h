#ifndef BIOS_H_INCLUDED
#define BIOS_H_INCLUDED

#include "stdint.h"

#ifdef BIOS_PRIVATE

#include "trace.h"

#ifdef ENABLE_BIOS_TRACE
#define TRACE_BIOS(function, format, ...) trace("bios.c", function, format, #__VA_ARGS__)
#else
#define TRACE_BIOS(function, format, ...) 
#endif

struct bios
{
    FILE *fptty;

    const char *file_bios;
    const char *file_exe;
};

#endif // BIOS_PRIVATE

extern void init_bios(const char *bios,
                      const char *exe,
                      const char *tty);
extern void task_bios( void );

#endif // BIOS_H_INCLUDED
