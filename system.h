#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#ifdef PRIVATE_SYSTEM

#include "trace.h"

#define X(v)  (((v) >>  0) & 0xffff)
#define Y(v)  (((v) >> 16) & 0xffff)
#define TX(t) (((t) >>  0) & 0xff)
#define TY(t) (((t) >>  8) & 0xff)
#define R(c)  (((c) >>  0) & 0x1f)
#define G(c)  (((c) >>  8) & 0x1f)
#define B(c)  (((c) >> 16) & 0x1f)

#define NAME "psx"
#define WIN_W 1024
#define WIN_H 512
#define WIN_X 0
#define WIN_Y 0

enum system_state {
    IDLE,
    RENDER
};

#define TRACE_SYS(function, format, ...) \
    trace(TRACE_SYSTEM_EN, "system.c", function, format, __VA_ARGS__)

/* renderer type specific structures */
#ifdef RENDERER_SDL

#include <SDL2/SDL.h>

#define TRACE_SYS(function, format, ...) \
    trace(TRACE_SYSTEM_EN, "system_sdl.c", function, format, __VA_ARGS__)

#define INITIALIZE_FLAGS SDL_INIT_VIDEO | SDL_INIT_AUDIO
#define     WINDOW_FLAGS SDL_WINDOW_SHOWN

#define SDL_CHECK_RET(expr) {assert((expr) == 0);}
#define SDL_CHECK_PTR(expr) {assert((expr) != NULL);}

struct system 
{
    /* VIDEO */
    uint8_t frame_buffer[1024 * 512][3];

    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *screen;
    SDL_Rect      scale;
    
    /* AUDIO */
    SDL_AudioStream *audio_stream;

    uint32_t render_next_frame;
};

#endif // RENDERER_SDL

#ifdef RENDERER_OPENGL
#endif // RENDERER_OPENGL

#ifdef RENDERER_VULKAN
#endif // RENDERER_VULKAN

#endif // PRIVATE_SYSTEM

#include "fifo.h"

/* 
 * Parameters to gpu rendering functions 
 *      c<num> -> color 
 *      v<num> -> vertex
 *      t<num> -> tex coord + (clut/page) 
 *      s      -> size (rectangle only) 
 */
extern void render_line_monochrome(
        uint32_t c, uint32_t v1,
                    uint32_t v2,
        bool semi_transparent
);
extern void render_line_shaded(
        uint32_t c1, uint32_t v1,
        uint32_t c2, uint32_t v2,
        bool semi_transparent
);
extern void render_polyline_monochrome(
        struct fifo gp0,
        bool semi_transparent
);
extern void render_polyline_shaded(
        struct fifo gp0,
        bool semi_transparent
);

extern void render_rectangle_monochrome(
        uint32_t c, uint32_t v, uint32_t s,
        bool semi_transparent
);
extern void render_rectangle_textured(
        uint32_t c, uint32_t v, uint32_t t_clut, uint32_t s,
        bool semi_transparent, bool texture_blending
);

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

extern void wait_system_ready( void );
extern void system_render_next_frame( void );
extern void system_write_audio_sample(int32_t sample);
extern void *task_system( void *ignore );

#endif // SYSTEM_H_INCLUDED
