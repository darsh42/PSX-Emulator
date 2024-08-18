#ifndef PSX_H_INCLUDED
#define PSX_H_INCLUDED

// utility headers
#include "error.h"
#include "common.h"

// device headers
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "timers.h"
#include "renderer.h"

#include <SDL2/SDL.h>

// macros
#define print_psx_error(func, format, ...) print_error("psx.c", func, format, __VA_ARGS__)

struct PSX {
    bool running;

    SDL_Window   *window;
    SDL_GLContext context;

    struct CPU *cpu;
    struct GPU *gpu;
    struct DMA *dma;
    struct MEMORY *memory;
    struct TIMERS *timers;

    uint32_t system_clock;
};
extern PSX_ERROR coprocessor_initialize(void);

// gui and sdl
extern void debugger_reset(void);
extern void debugger_exec(void);
extern PSX_ERROR debugger_destroy(void);

#endif//PSX_H_INCLUDED
