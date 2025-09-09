#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include <stdint.h>

enum tracing_device {
    TRACE_MAIN_EN       = 1,
    TRACE_STUB_EN       = 2,
    TRACE_SYSTEM_EN     = 4,

    TRACE_CPU_EN        = 8,
    TRACE_GPU_EN        = 16,
    TRACE_SPU_EN        = 32,
    TRACE_DMA_EN        = 64,
    TRACE_CDROM_EN      = 128,
    TRACE_TIMERS_EN     = 256,
    TRACE_MEMORY_EN     = 512,
    TRACE_INTERRUPTS_EN = 1024,
};

union logging {
    uint32_t profile;

    struct {
        uint32_t main:       1;
        uint32_t stub:       1;
        uint32_t system:     1;

        uint32_t cpu:        1;
        uint32_t gpu:        1;
        uint32_t spu:        1;
        uint32_t dma:        1;
        uint32_t cdrom:      1;
        uint32_t timers:     1;
        uint32_t memory:     1;
        uint32_t interrupts: 1;
    };
};

extern void trace_set_output( const char *filename );
extern void trace_set_profile( uint32_t profile );
extern void init_tracer( void );

extern void trace( enum tracing_device dev,
                   char *file,
                   char *function_call,
                   char *format_string,
                   ... );

#endif // TRACE_H_INCLUDED
