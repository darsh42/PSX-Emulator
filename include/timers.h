#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "common.h"

enum SYNCRONIZATION_ENABLE {
    FREE_RUN = 0,
    SYNCRONIZE = 1,
};

enum TIMER_SYNCRONIZATION_MODE {
    PAUSE_DURING_HBLANK,
    RESET_DURING_HBLANK,
    RESET_DURING_HBLANK_AND_PAUSE_OUTSIDE_HBLANK,
    PAUSE_UNTILL_HBLANK_THEN_FREE_RUN,
    PAUSE_DURING_VBLANK,
    RESET_DURING_VBLANK,
    RESET_DURING_VBLANK_AND_PAUSE_OUTSIDE_VBLANK,
    PAUSE_UNTILL_VBLANK_THEN_FREE_RUN,
    STOP_COUNTER,
    SET_FREE_RUN,
};

enum TIMER_RESET {
    MAXVAL = 0,
    TARGET = 1
};

union TIMER_CURRENT {
    uint32_t value;
    struct {
        uint32_t count: 16;
        uint32_t :16;
    };
};

union TIMER_TARGET {
    uint32_t value;
    struct {
        uint32_t count: 16;
        uint32_t :16;
    };
};

union TIMER_MODE {
    uint32_t value;
    struct {
        enum SYNCRONIZATION_ENABLE sync_enable: 1;
        uint32_t sync_mode: 2;
        enum TIMER_RESET reset_after: 1;
        enum PSX_ENABLE irq_when_target: 1;
        enum PSX_ENABLE irq_when_max: 1;
        uint32_t irq_once_or_repeat: 1;
        uint32_t irq_pulse_or_toggle: 1;
        uint32_t clock_source: 2;
        enum PSX_ENABLE interrupt_request: 1;
        enum PSX_ENABLE hit_target: 1;
        enum PSX_ENABLE hit_max: 1;
    };
};

struct TIMER {
    union TIMER_CURRENT *current;
    union TIMER_MODE    *mode;
    union TIMER_TARGET  *target;

    uint32_t old_mode;
};

struct TIMERS {
    struct TIMER T0;
    struct TIMER T1;
    struct TIMER T2;
};

// memory functions
extern uint8_t *memory_pointer(uint32_t address);

#endif // !TIMER_H_INCLUDED
