#include "sdl.h"

struct SDL_HANDLER handler;

PSX_ERROR sdl_initialize(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        print_sdl_error("sdl_initialize", "SDL could not initialize, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_INIT);
    }
    handler.window = SDL_CreateWindow(WINDOW_NAME, 
                                      SDL_WINDOWPOS_CENTERED, 
                                      SDL_WINDOWPOS_CENTERED, 
                                      WINDOW_SIZE_WIDTH, 
                                      WINDOW_SIZE_HEIGHT, 
                                      SDL_WINDOW_SHOWN);
    if (handler.window == NULL) {
        print_sdl_error("sdl_initialize", "SDL could not create window, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_WINDOW_CREATION);
    }
    handler.renderer = SDL_CreateRenderer(handler.window,
                                          -1,
                                          SDL_RENDERER_ACCELERATED);
    if (handler.renderer == NULL) {
        print_sdl_error("sdl_initialize", "SDL could not create renderer, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDERER_CREATION);
    }
    handler.native_display = SDL_CreateTexture(handler.renderer,
                                               SDL_PIXELFORMAT_RGB24,
                                               SDL_TEXTUREACCESS_STREAMING,
                                               NATIVE_DISPLAY_WIDTH,
                                               NATIVE_DISPLAY_HEIGHT);
    if (handler.native_display == NULL) {
        print_sdl_error("sdl_initialize", "SDL could not create native display, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_TEXTURE_CREATION);
    }
    handler.scaled_display = (SDL_Rect) {0, 0, SCALED_DISPLAY_WIDTH, SCALED_DISPLAY_HEIGHT};
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_destroy(void) {
    SDL_DestroyTexture(handler.native_display);
    SDL_DestroyRenderer(handler.renderer);
    SDL_DestroyWindow(handler.window);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_render_screen(void) {
    if (SDL_SetRenderDrawColor(handler.renderer, 0XFF, 0XFF, 0XFF, 0XFF) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not set color, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    if (SDL_RenderClear(handler.renderer) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not clear background, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    // TODO: create a display retriever
    if (SDL_UpdateTexture(handler.native_display, NULL, (uint32_t *) memory_VRAM_pointer(), NATIVE_DISPLAY_WIDTH*3) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not update native display, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    if (SDL_RenderCopy(handler.renderer, handler.native_display, NULL, &handler.scaled_display) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not copy native display to window, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    SDL_RenderPresent(handler.renderer);
    return set_PSX_error(NO_ERROR);
}
