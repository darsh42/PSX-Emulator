#ifndef DEBUGGER_H_INCLUDED
#define DEBUGGER_H_INCLUDED

#include "common.h"
#include "sdl.h"
#include "cpu.h"
#include "gpu.h"
#include "memory.h"
#include "instruction.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VAARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

extern PSX_ERROR sdl_return_handler(struct SDL_HANDLER **);
extern PSX_ERROR sdl_handle_events(SDL_Event *, void (*)(void), int (*)(SDL_Event *));
extern PSX_ERROR sdl_render_clear(void);
extern PSX_ERROR sdl_render_present(void);

char ***get_main_disassembly(void);
char ***get_bios_disassembly(void);
char ***get_exp1_disassembly(void);

struct DEBUGGER {
    uint32_t running;
    uint32_t fontscale;

    struct nk_context  *ctx;
    struct SDL_HANDLER *sdl_handler;
};

#endif // DEBUGGER_H_INCLUDED
