#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#ifdef PRIVATE_SYSTEM

#ifdef RENDERER_OPENGL
#endif // RENDERER_OPENGL

#ifdef RENDERER_VULKAN
#endif // RENDERER_VULKAN

#include "trace.h"

#define X(v)  (((v) >>  0) & 0xffff)
#define Y(v)  (((v) >> 16) & 0xffff)
#define TX(t) (((t) >>  0) & 0xff)
#define TY(t) (((t) >>  8) & 0xff)
#define R(c)  (((c) >>  0) & 0xff)
#define G(c)  (((c) >>  8) & 0xff)
#define B(c)  (((c) >> 16) & 0xff)

#define NAME "psx"
#define WIN_W 642
#define WIN_H 482
#define WIN_X 0
#define WIN_Y 0

enum system_state {
    IDLE,
    RENDER
};

#define TRACE_SYS(function, format, ...) \
    trace(TRACE_SYSTEM_EN, "system.c", function, format, __VA_ARGS__)

#include <SDL2/SDL.h>

#define TRACE_SYS(function, format, ...) \
    trace(TRACE_SYSTEM_EN, "system_sdl.c", function, format, __VA_ARGS__)

#define INITIALIZE_FLAGS SDL_INIT_VIDEO | SDL_INIT_AUDIO
#define     WINDOW_FLAGS SDL_WINDOW_SHOWN

#define BUFFERING 3
#define SAMPLES_BUFFER_SIZE 512

#define AUDIO_WANT_FREQUENCY 44100
#define AUDIO_WANT_FORMAT    AUDIO_S16SYS
#define AUDIO_WANT_CHANNELS  2
#define AUDIO_WANT_BUFFER    SAMPLES_BUFFER_SIZE

#define SDL_CHECK_RET(expr) {assert((expr) == 0);}
#define SDL_CHECK_PTR(expr) {assert((expr) != NULL);}

struct box {
    int32_t minx, miny;
    int32_t maxx, maxy;
    int32_t height, width;
};

struct system {
    /* AUDIO */
    SDL_AudioSpec     au_want, au_have;
    SDL_AudioDeviceID au_device;

    int32_t *audio_produce;
    int32_t *audio_consume;
    int32_t *audio_complete;

    /* VIDEO */
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *screen;
    SDL_Rect      scale;

    size_t thread_count;
    size_t allocations;

    pthread_t  *threads;
    struct box *tiles;

    uint32_t render_next_frame;

    /* will create three audio buffers, 
     * one consumer and two producers  */
    int32_t  samples[BUFFERING * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    uint32_t frame_buffer[WIN_H][WIN_W];
};

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

extern void init_threads(void);
extern void free_threads(void);
extern void wait_system_ready( void );
extern void system_render_next_frame( void );
extern void system_audio_push_sample(int16_t left, int16_t right);
extern void *task_system( void *ignore );

#endif // SYSTEM_H_INCLUDED
