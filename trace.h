#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

// #define ENABLE_CPU_TRACE
#define ENABLE_GPU_TRACE
// #define ENABLE_DMA_TRACE
// #define ENABLE_MEMORY_TRACE
// #define ENABLE_TIMERS_TRACE
// #define ENABLE_INTERRUPTS_TRACE

extern void trace( char *file, char *function_call, char *format_string, ... );

#endif // TRACE_H_INCLUDED
