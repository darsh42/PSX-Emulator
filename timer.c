#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define TIMER_PRIVATE
#include "timer.h"
#include "memory.h"

#ifdef DEBUG
#include "stub.h"
#endif

pthread_cond_t system_tick_notify = PTHREAD_COND_INITIALIZER;
pthread_mutex_t system_tick_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    return data;
}

pthread_cond_t *write_timers( uint32_t address, uint32_t _data ) 
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
    return NULL;
}

/* allows devices to sleep till next system tick */
void wait_system_tick( int clocks )
{
    while (clocks--) assert(!pthread_cond_wait(&system_tick_notify, &system_tick_mutex));
}

void timer_reset( struct timer *timer )
{
    if ( timer->mode.reset_after )
    {
        /* counter reset when target reached */
        if ( timer->current_count >= timer->target_count )
        {
            timer->current_count   = 0;
            timer->mode.hit_target = 1;

            /* handle interrupt */
            if ( timer->mode.irq_when_target )
            {
            }
        }
    }
    else
    {
        /* counter reset on overflow         */
        if ( timer->current_count >= 0xFFFF )
        {
            timer->current_count = 0;
            timer->mode.hit_max  = 1;

            /* handle interrupt */
            if ( timer->mode.irq_when_max )
            {
            }
        }
    }
}

void *task_timers( void *ignore )
{
    printf("SYSTEM TIMER: %ld\n", pthread_self());
    while ( running )
    {
#ifdef DEBUG
        /* the gdb debugging stub needs to control the timer,   *
         * this will allow the stub to control the wider system *
         * as all of the components are being controlled by the *
         * timers                                               */
        if ( gdb_stub_pause )
        {
            wait_gdb_stub();
        }

#endif // DEBUG

         usleep(20);

        timers.t0.current_count++; timer_reset(&timers.t0);
        timers.t1.current_count++; timer_reset(&timers.t1);
        timers.t2.current_count++; timer_reset(&timers.t2);
        
        /* tell all devices to cycle */
        assert(!pthread_cond_broadcast(&system_tick_notify));
    }
}
