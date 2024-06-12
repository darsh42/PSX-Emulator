#include "../../include/timers.h"

static struct TIMERS timers;

struct TIMERS *get_timers(void) { return &timers; }

// helpers
static void timer_reset(struct TIMER *timer);

PSX_ERROR timers_create(void) {
    timers.T0.current = (union TIMER_CURRENT *) memory_pointer(0X1F801100);
    timers.T0.mode    = (union TIMER_MODE *)    memory_pointer(0X1F801104);
    timers.T0.target  = (union TIMER_TARGET *)  memory_pointer(0X1F801108);
    timers.T0.old_mode = timers.T0.mode->value;

    timers.T1.current = (union TIMER_CURRENT *) memory_pointer(0X1F801110);
    timers.T1.mode    = (union TIMER_MODE *)    memory_pointer(0X1F801114);
    timers.T1.target  = (union TIMER_TARGET *)  memory_pointer(0X1F801118);
    timers.T1.old_mode = timers.T1.mode->value;

    timers.T2.current = (union TIMER_CURRENT *) memory_pointer(0X1F801120);
    timers.T2.mode    = (union TIMER_MODE *)    memory_pointer(0X1F801124);
    timers.T2.target  = (union TIMER_TARGET *)  memory_pointer(0X1F801128);
    timers.T2.old_mode = timers.T2.mode->value;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR timers_step(void) {
    timers.T0.current->count++;
    timers.T1.current->count++;
    timers.T2.current->count++;
    
    timer_reset(&timers.T0);
    timer_reset(&timers.T1);
    timer_reset(&timers.T2);

    return set_PSX_error(NO_ERROR);
}

void timer_reset(struct TIMER *timer) {
    union TIMER_CURRENT current = *timer->current;
    union TIMER_MODE    mode    = *timer->mode;
    union TIMER_TARGET  target  = *timer->target;
    
    if (mode.value != timer->old_mode) {
        timer->current->count = 0;
        timer->old_mode = timer->mode->value;
        return;
    }

    switch (mode.reset_after) {
        case MAXVAL: 
            if (current.count >= 0XFFFF) {
                timer->current->count = 0;
                timer->mode->hit_max  = 1;
                
                // handle interrupt
                if (mode.irq_when_max) {
                }
            }
            break;
        case TARGET: 
            if (current.count == target.count) {
                timer->current->count   = 0;
                timer->mode->hit_target = 1;

                // handle interrupt
                if (mode.irq_when_target) {
                }
            }
            break;
    }
}

