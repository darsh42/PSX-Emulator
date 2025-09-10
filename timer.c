#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "timer.h"
#include "memory.h"
#include "gpu.h"

#include "trace.h"
#define TRACE_TIMERS(function, format, ...) \
    trace(TRACE_TIMERS_EN, "timers.c", function, format, __VA_ARGS__)

static struct timers timers;

uint32_t read_timers( uint32_t address )
{
    uint32_t data;
    switch ( address )
    {
        case(timer_0_current_counter): data = timers.t0.current_count; break;
        case(timer_0_mode           ): data = timers.t0.mode.value;    break;
        case(timer_0_target         ): data = timers.t0.target_count;  break;
        case(timer_1_current_counter): data = timers.t1.current_count; break;
        case(timer_1_mode           ): data = timers.t1.mode.value;    break;
        case(timer_1_target         ): data = timers.t1.target_count;  break;
        case(timer_2_current_counter): data = timers.t2.current_count; break;
        case(timer_2_mode           ): data = timers.t2.mode.value;    break;
        case(timer_2_target         ): data = timers.t2.target_count;  break;
    }
    TRACE_TIMERS("read_timers ", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_timers( uint32_t address, uint32_t _data )
{
    uint16_t data = (uint16_t) _data;
    switch ( address )
    {
        case(timer_0_current_counter): timers.t0.current_count = 0;    break;
        case(timer_0_mode           ): timers.t0.mode.value    = data; break;
        case(timer_0_target         ): timers.t0.target_count  = data; break;
        case(timer_1_current_counter): timers.t1.current_count = 0;    break;
        case(timer_1_mode           ): timers.t1.mode.value    = data; break;
        case(timer_1_target         ): timers.t1.target_count  = data; break;
        case(timer_2_current_counter): timers.t2.current_count = 0;    break;
        case(timer_2_mode           ): timers.t2.mode.value    = data; break;
        case(timer_2_target         ): timers.t2.target_count  = data; break;
    }

    TRACE_TIMERS("write_timers", "address: %08x | data: %08x\n", address, data);
}

static void timer_reset( struct timer *_timer )
{
    struct timer timer = *_timer;

    if ( timer.mode.reset_after )
    {
        /* counter reset when target reached */
        if ( timer.current_count >= timer.target_count )
        {
            timer.current_count   = 0;
            timer.mode.hit_target = 1;

            /* handle interrupt */
            if ( timer.mode.irq_when_target )
            {
            }
        }
    }
    else
    {
        /* counter reset on overflow */
        if ( timer.current_count >= 0xFFFF )
        {
            timer.current_count = 0;
            timer.mode.hit_max  = 1;

            /* handle interrupt */
            if ( timer.mode.irq_when_max )
            {
            }
        }
    }
}

static void timers_increment_timer0( void )
{
    if (timers.t0.mode.sync_enable)
    {
        // 0 = Pause counter during Hblank(s)
        // 1 = Reset counter to 0000h at Hblank(s)
        // 2 = Reset counter to 0000h at Hblank(s) and pause outside of Hblank
        // 3 = Pause until Hblank occurs once, then switch to Free Run

        if (gpu_hblank())
        {
            /* clear counter to 0000 at HBLANK for modes 1 and 2 */
            if (timers.t0.mode.sync_mode == 1 || timers.t0.mode.sync_mode == 2)
                timers.t0.current_count = 0;

            /* switch to free running after HBLANK for mode 3 */
            if (timers.t0.mode.sync_mode == 3)
                timers.t0.mode.sync_enable = 0;
        }
        else
        {
            /* pause counter until HBLANK (covers outside HBLANK aswell) */
            if (timers.t0.mode.sync_mode != 1)
                return;
        }
    }

    timers.t0.current_count++;
}

static void timers_increment_timer1( void )
{
    if (timers.t1.mode.sync_enable)
    {
        // 0 = Pause counter during Vblank(s)
        // 1 = Reset counter to 0000h at Vblank(s)
        // 2 = Reset counter to 0000h at Vblank(s) and pause outside of Vblank
        // 3 = Pause until Vblank occurs once, then switch to Free Run

        if (gpu_vblank())
        {
            /* clear counter to 0000 at VBLANK for modes 1 and 2 */
            if (timers.t1.mode.sync_mode == 1 || timers.t1.mode.sync_mode == 2)
                timers.t1.current_count = 0;

            /* switch to free running after VBLANK for mode 3 */
            if (timers.t1.mode.sync_mode == 3)
                timers.t1.mode.sync_enable = 0;
        }
        else
        {
            /* pause counter until VBLANK (covers outside VBLANK aswell) */
            if (timers.t1.mode.sync_mode != 1)
                return;
        }
    }

    timers.t1.current_count++;
}

static void timers_increment_timer2( void )
{
    if (timers.t2.mode.sync_enable)
    {
        if (timers.t2.mode.sync_mode == 0 || timers.t2.mode.sync_mode == 3)
            return;
    }

    timers.t2.current_count++;
}

void init_timers( void ) {}
void task_timers( void )
{
    //usleep(20);

    timers_increment_timer0(); timer_reset(&timers.t0);
    timers_increment_timer1(); timer_reset(&timers.t1);
    timers_increment_timer2(); timer_reset(&timers.t2);
}
