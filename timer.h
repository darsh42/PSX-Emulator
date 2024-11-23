#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#ifdef TIMER_PRIVATE

union timer_mode {
    uint16_t value;
    struct {
        uint16_t sync_enable         : 1;
        uint16_t sync_mode           : 2;
        uint16_t reset_after         : 1;
        uint16_t irq_when_target     : 1;
        uint16_t irq_when_max        : 1;
        uint16_t irq_once_or_repeat  : 1;
        uint16_t irq_pulse_or_toggle : 1;
        uint16_t clock_source        : 2;
        uint16_t interrupt_request   : 1;
        uint16_t hit_target          : 1;
        uint16_t hit_max             : 1;
    };
};

struct timer {
    uint16_t current_count;
    uint16_t target_count;

    union timer_mode mode;
};

struct timers {
    struct timer t0;
    struct timer t1;
    struct timer t2;
};
#endif // TIMER_PRIVATE

extern void *task_timers( void *ignore );

extern void wait_system_tick( int clocks );

extern uint32_t        read_timers( uint32_t address );
extern pthread_cond_t *write_timers( uint32_t address, uint32_t data );

#endif // TIMER_H_INCLUDED
