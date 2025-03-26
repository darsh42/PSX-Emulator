#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRIVATE_SYSTEM
#define RENDERER_SDL
#include "system.h"
#include "dma.h"
#include "gpu.h"
#include "memory.h"

pthread_cond_t renderer_notify = PTHREAD_COND_INITIALIZER;
pthread_mutex_t renderer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t system_notify = PTHREAD_COND_INITIALIZER;
pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;

struct system sys;

void render_three_point_polygon_monochrome(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
    bool semi_transparent
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             semi_transparent);
}
void render_four_point_polygon_monochrome(uint32_t c1, uint32_t v1, 
                                                       uint32_t v2, 
                                                       uint32_t v3, 
                                                       uint32_t v4, 
                                          bool semi_transparent
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             X(v4), Y(v4), R(c1), G(c1), B(c1),
             semi_transparent);
}
void render_three_point_polygon_textured(uint32_t c1, uint32_t v1, uint32_t t1_clut,
                                                      uint32_t v2, uint32_t t2_page,
                                                      uint32_t v3, uint32_t t3,
                                         bool semi_transparent, bool texture_blending) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             semi_transparent);
}
void render_four_point_polygon_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
                 uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             X(v4), Y(v4), R(c1), G(c1), B(c1),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             TX(t4     ), TY(t4     ),
             semi_transparent);
}
void render_three_point_polygon_shaded(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    bool semi_transparent
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             semi_transparent);
}
void render_four_point_polygon_shaded(uint32_t c1, uint32_t v1,
                                      uint32_t c2, uint32_t v2,
                                      uint32_t c3, uint32_t v3,
                                      uint32_t c4, uint32_t v4,
                                      bool semi_transparent) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             X(v4), Y(v4), R(c4), G(c4), B(c4),
             semi_transparent);
}
void render_three_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n \
             \n\ttextureblending: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             semi_transparent,
             texture_blending);
}
void render_four_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    uint32_t c4, uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n \
             \n\ttextureblending: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             X(v4), Y(v4), R(c4), G(c4), B(c4),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             TX(t4     ), TY(t4     ),
             semi_transparent,
             texture_blending);
}

static inline void system_input( void )
{
    SDL_Event keypress;

    while (SDL_PollEvent(&keypress))
    {
        switch (keypress.type)
        {
            case SDL_QUIT: running = 0; break;
            case SDL_KEYDOWN:
                switch (keypress.key.keysym.sym)
                {
                    case SDLK_q: running = 0; break;
                }
                break;
            case SDL_KEYUP:
                switch (keypress.key.keysym.sym)
                {
                    case SDLK_q: running = 0; break;
                }
                break;
        }
    }
}

void system_render( void )
{
    SDL_UpdateTexture(sys.screen, NULL, sys.frame_buffer, 1024*3);
    SDL_SetRenderDrawColor(sys.renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(sys.renderer);
    SDL_RenderCopy(sys.renderer, sys.screen, NULL, &sys.scale);
    SDL_RenderPresent(sys.renderer);
}

void system_write_audio_sample(int32_t sample)
{
    SDL_CHECK_RET(SDL_AudioStreamPut(sys.audio_stream, &sample, sizeof(sample)));
}

void wait_system_ready( void )
{
    assert(!pthread_cond_wait(&system_notify, &system_mutex));
}

void *task_system( void *ignore )
{
    printf("SYSTEM: %ld\n", pthread_self());

    SDL_CHECK_RET(SDL_Init(INITIALIZE_FLAGS));

    /* VIDEO  */
    SDL_CHECK_PTR(sys.window   = SDL_CreateWindow(NAME, WIN_X, WIN_Y, WIN_W, WIN_H, WINDOW_FLAGS));
    SDL_CHECK_PTR(sys.renderer = SDL_CreateRenderer(sys.window, -1, SDL_RENDERER_ACCELERATED));
    SDL_CHECK_PTR(sys.screen   = SDL_CreateTexture(sys.renderer, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H));
    
    sys.scale = (SDL_Rect) {0, 0, 1024, 512};

    /* AUDIO */
    SDL_CHECK_PTR(sys.audio_stream = SDL_NewAudioStream(AUDIO_S32, 1, 22050, AUDIO_F32, 2, 48000));

    assert(!pthread_cond_broadcast(&system_notify));

    while ( running )
    {
        system_input();
    }

cleanup:
    SDL_DestroyRenderer(sys.renderer);
    SDL_DestroyWindow(sys.window);
    SDL_Quit();

    return NULL;
}
