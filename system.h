#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#ifdef PRIVATE_SYSTEM

#define NAME "psx"
#define W 1920 - 640
#define H 0
#define X 0
#define Y 0

enum system_state {
    IDLE,
    RENDER
};

/* renderer type specific structures */
#ifdef RENDERER_SDL

#include <SDL2/SDL.h>

#define INITIALIZE_FLAGS SDL_INIT_VIDEO
#define WINDOW_FLAGS 

#define SDL_CHECK_RET(expr) {assert((expr) == 0);}
#define SDL_CHECK_PTR(expr) {assert((expr) != NULL);}

struct system 
{
    SDL_Window   *window;
    SDL_Renderer *renderer;

    uint32_t render_next_frame;
};

#endif // RENDERER_SDL

#ifdef RENDERER_OPENGL
#endif // RENDERER_OPENGL

#ifdef RENDERER_VULKAN
#endif // RENDERER_VULKAN

#endif // PRIVATE_SYSTEM

extern void render_three_point_polygon_monochrome(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
    bool semi_transparent
);
extern void render_four_point_polygon_monochrome(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
                 uint32_t v4,
    bool semi_transparent
);
extern void render_three_point_polygon_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
);
extern void render_four_point_polygon_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
                 uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
);
extern void render_three_point_polygon_shaded(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    bool semi_transparent
);
extern void render_four_point_polygon_shaded(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    uint32_t c4, uint32_t v4,
    bool semi_transparent
);
extern void render_three_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
);
extern void render_four_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    uint32_t c4, uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
);

extern void *task_system( void *ignore );

#endif // SYSTEM_H_INCLUDED
