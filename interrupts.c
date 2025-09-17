#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "interrupts.h"

#include "cp0.h"
#include "gpu.h"
#include "timer.h"
#include "memory.h"

#include "trace.h"
#define TRACE_INTERRUPTS(function, format, ...) \
    trace(TRACE_INTERRUPTS_EN, "interrupts.c", function, format, __VA_ARGS__)

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

    TRACE_INTERRUPTS("read_interrupts",
            "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_interrupts( uint32_t address, uint32_t data )
{
    switch ( address )
    {
        case (i_stat):interrupts.interrupt_status |= data; break;
        case (i_mask):interrupts.interrupt_mask    = data; break;
        default:
            assert(0 && "invalid address to interrupts\n");
            break;
    }

    TRACE_INTERRUPTS("write_interrupts",
            "address: %08x | data: %08x\n", address, data);
}

void interrupt_acknowledge( void ) {
    /* get masked status value */
    uint32_t masked_status = interrupts.interrupt_mask &
                             interrupts.interrupt_status;

    /* send i_status acknowledgement */
    interrupts.interrupt_status ^= masked_status;

    /* send device acknowledgement */
    switch (masked_status) {
    case IRQ1: {
        union gpustat gpustat;

        /* get current status */
        gpu_get_gpustat(&gpustat);

        /* acknowledge interrupt */
        gpustat.interrupt_request = 0;

        /* write back */
        gpu_set_gpustat(gpustat);
        break;
    }
    case IRQ2:
    case IRQ3:
    case IRQ7:
    case IRQ8:
    case IRQ9:
    case IRQ10:
    }

    interrupts.handling_irq = false;
}

void init_interrupts( void ) {}
void task_interrupts( void ) {
    /* get masked status value */
    uint32_t masked_status = interrupts.interrupt_mask &
                             interrupts.interrupt_status;

    if (masked_status && !interrupts.handling_irq) {
        TRACE_INTERRUPTS("task_interrupts",
                "handling: %04x\n", masked_status);

        interrupts.handling_irq = true;

        /* check for multiple interrupts at same time (value is
         * not a power of 2 then there are multiple interrupts)*/
        if ((masked_status & (masked_status - 1)) != 0) {
            union cp0_sr    sr;
            union cp0_cause cause;

            read_cp0_reg(CP0_SR,    &sr.value);
            read_cp0_reg(CP0_CAUSE, &cause.value);

            // set current interrupt enable
            sr.value |= (1 <<  0);
            // set interrupt mask bit
            sr.value |= (1 << 10);

            // set pending interrupt bit
            cause.value |= (1 << 10);

            write_cp0_reg(CP0_SR,    sr.value);
            write_cp0_reg(CP0_CAUSE, cause.value);
        }

        /* send irq request */
        cp0_exception(INT);
    }
}
