#include "../../include/debugger.h"

struct DEBUGGER debugger;

PSX_ERROR debugger_init(void) {
    debugger.running   = 1;
    debugger.fontscale = 1;

    sdl_return_handler(&debugger.sdl_handler);

    debugger.ctx = nk_sdl_init(debugger.sdl_handler->window, debugger.sdl_handler->renderer);
    // Load Fonts
    // Load Cursor
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;
        
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * debugger.fontscale, &config);
        nk_sdl_font_stash_end();

        font->handle.height /= debugger.fontscale;
        nk_style_set_font(debugger.ctx, &font->handle);
    }
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR debugger_exec(void) {
    while (debugger.running) {
        /* Input handler */
        SDL_Event event;
        nk_input_begin(debugger.ctx);
        sdl_handle_events(&event, nk_sdl_shutdown, nk_sdl_handle_event);
        nk_sdl_handle_grab();
        nk_input_end(debugger.ctx);

        /* Drawing GUI */
        if (nk_begin(debugger.ctx, "Debugger", nk_rect(0, 0, 1024, 512),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | 
                     NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) 
        {
            // Print disassembly
            nk_layout_row_begin();
            {
                
            }
            nk_layout_row_end();

        }
        nk_end(debugger.ctx);
        
        /* Rendering */
        sdl_render_clear();
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        sdl_render_present();
    }
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR debugger_destroy(void) {
    nk_sdl_shutdown();
    return set_PSX_error(NO_ERROR);
}
