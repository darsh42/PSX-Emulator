#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include <stdint.h>
#include <pthread.h>

enum irq {
    IRQ0  = 1,      // vblank
    IRQ1  = 2,      // gpu
    IRQ2  = 4,      // cdrom
    IRQ3  = 8,      // dma
    IRQ4  = 16,     // tmr0
    IRQ5  = 32,     // tmr1
    IRQ6  = 64,     // tmr2
    IRQ7  = 128,    // controller_and_memory_card
    IRQ8  = 256,    // sio
    IRQ9  = 512,    // spu
    IRQ10 = 1024,   // controller
};

union interrupt_status {
    uint32_t value;
    struct {
        uint32_t vblank:                     1;
        uint32_t gpu:                        1;
        uint32_t cdrom:                      1;
        uint32_t dma:                        1;
        uint32_t tmr0:                       1;
        uint32_t tmr1:                       1;
        uint32_t tmr2:                       1;
        uint32_t controller_and_memory_card: 1;
        uint32_t sio:                        1;
        uint32_t spu:                        1;
        uint32_t controller:                 1;
    };
};

struct interrupts {
    uint32_t interrupt_status;
    uint32_t interrupt_mask;
};

void init_interrupts( void );
void task_interrupts( void );
void interrupt_acknowledge( void );

uint32_t  read_interrupts( uint32_t address );
void     write_interrupts( uint32_t address, uint32_t data );

#endif // INTERRUPTS_H_INCLUDED
