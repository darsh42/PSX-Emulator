#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include <stdint.h>
#include <pthread.h>

extern uint32_t read_interrupts( uint32_t address );
extern void     write_interrupts( uint32_t address, uint32_t data );

#ifdef INTERRUPTS_PRIVATE

#include "trace.h"

#ifdef ENABLE_INTERRUPTS_TRACE
#define TRACE_INTERRUPTS(function, format, ...) trace("interrupts.c", function, format, __VA_ARGS__)
#else
#define TRACE_INTERRUPTS(function, format, ...) 
#endif

struct interrupts
{
    uint32_t interrupt_status;
    uint32_t interrupt_mask;
};

#endif // INTERRUPTS_PRIVATE

void init_interrupts( void );

#endif // INTERRUPTS_H_INCLUDED
