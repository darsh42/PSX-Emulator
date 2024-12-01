#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define INTERRUPTS_PRIVATE
#include "interrupts.h"

#include "timer.h"
#include "memory.h"

static struct interrupts interrupts;

uint32_t read_interrupts( uint32_t address )
{
    uint32_t data;
    switch ( address )
    {
        case (i_stat): data = interrupts.interrupt_status; break;
        case (i_mask): data = interrupts.interrupt_mask;   break;
        default:
            assert(0 && "invalid address to interrupts\n");
            break;
    }

    TRACE_INTERRUPTS("read_interrupts ", "address: %08x | data: %08x\n", address, data);

    return data;
}

pthread_cond_t *write_interrupts( uint32_t address, uint32_t data )
{
    switch ( address )
    {
        case (i_stat):interrupts.interrupt_status = data; break;
        case (i_mask):interrupts.interrupt_mask   = data; break;
        default:
            assert(0 && "invalid address to interrupts\n");
            break;
    }

    TRACE_INTERRUPTS("write_interrupts", "address: %08x | data: %08x\n", address, data);

    return NULL;
}
