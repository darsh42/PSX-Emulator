#ifndef PSXSDL_H_INCLUDED
#define PSXSDL_H_INCLUDED

#include "../core/common.h"
#include <SDL2/SDL.h>

#define print_sdl_error(func, format, ...) print_error("sdl.c", func, format, __VA_ARGS__)

#define NATIVE_DISPLAY_WIDTH  1024
#define NATIVE_DISPLAY_HEIGHT 512
#define SCALE 1
#define SCALED_DISPLAY_WIDTH  SCALE * NATIVE_DISPLAY_WIDTH
#define SCALED_DISPLAY_HEIGHT SCALE * NATIVE_DISPLAY_HEIGHT

#define WINDOW_NAME "PSX-Emulator"
#define WINDOW_SIZE_WIDTH  SCALED_DISPLAY_WIDTH
#define WINDOW_SIZE_HEIGHT SCALED_DISPLAY_HEIGHT

struct SDL_HANDLER {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *native_display;
    SDL_Rect      scaled_display;
};

// retrieves the screen data
extern uint8_t *memory_VRAM_pointer(void);

#endif // PSXSDL_H_INCLUDED
