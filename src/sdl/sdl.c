#include "../../include/sdl.h"

struct SDL_HANDLER handler;

#ifdef DEBUG
struct SDL_HANDLER *sdl_return_handler(void) { return &handler; }
#endif

PSX_ERROR sdl_initialize(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        print_sdl_error("sdl_initialize", "SDL could not initialize, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_INIT);
    }
    handler.window = SDL_CreateWindow(WINDOW_NAME, 
                                      0, 
                                      0, 
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
    handler.status = CONTINUE;
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_destroy(void) {
    SDL_DestroyTexture(handler.native_display);
    SDL_DestroyRenderer(handler.renderer);
    SDL_DestroyWindow(handler.window);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_render_clear(void) {
    if (SDL_SetRenderDrawColor(handler.renderer, 0, 0, 0, 255) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not set color, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    if (SDL_RenderClear(handler.renderer) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not clear background, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }

    SDL_Delay(4);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_update_psx_screen(void) {
    // TODO: create a display retriever
    if (SDL_UpdateTexture(handler.native_display, NULL, (uint32_t *) memory_VRAM_pointer(), NATIVE_DISPLAY_WIDTH*3) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not update native display, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    if (SDL_RenderCopy(handler.renderer, handler.native_display, NULL, &handler.scaled_display) != 0) {
        print_sdl_error("sdl_render_screen", "SDL could not copy native display to window, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_RENDER_SCREEN);
    }
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_render_present(void) {
    SDL_RenderPresent(handler.renderer);
    return set_PSX_error(NO_ERROR);
}

// Callee passes created SDL event
// Callee passes context destruction function incase user quits application
// Callee passes context event handler in case it requires (Nuklear GUI)
//
PSX_ERROR sdl_handle_events(SDL_Event *event, int (*nk_sdl_input_handler) (SDL_Event *e)) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT: 
                handler.status = QUIT; break;
            case SDL_KEYUP:
                handler.status = CONTINUE;
            case SDL_KEYDOWN:
                switch (event->key.keysym.sym) {
                    case SDLK_ESCAPE:  exit(0); handler.status = QUIT;         break; // quit
                    case SDLK_SPACE:   handler.status = TOGGLE_PAUSE; break; // pause
                    case SDLK_F10:     handler.status = STEPINTO;     break; // step into
                    case SDLK_F11:     handler.status = STEP;         break; // step 
                    case SDLK_F12:     handler.status = STEPOVER;     break; // step over
                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                        switch (event->key.keysym.sym) {
                            case SDLK_p: handler.status = TOGGLE_FOLLOW_PC; break; // follow PC
                        }
                        break;
                }
            default:
                                      break;
        }
        nk_sdl_input_handler(event);
    }
    return set_PSX_error(NO_ERROR);
}
// PSX_ERROR sdl_handle_events(SDL_Event *event, void (*context_destroy)(void), void (*context_handler)(SDL_Event *)) {
//      while (SDL_PollEvent(event)) {
//          switch (event->type) {
//              case SDL_QUIT:
//                  if (context_destroy) {
//                      context_destroy();
//                  }
//                  return sdl_destroy();
//              case SDL_KEYDOWN:
//                  switch (event->key.keysym.sym) {
//                      case SDLK_ESCAPE:
//                      default:
//                  }
//  
//          }
//  
//  
//          context_handler(event);
//      }
//      return set_PSX_error(NO_ERROR);
//  }
