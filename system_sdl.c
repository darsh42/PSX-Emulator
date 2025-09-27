#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRIVATE_SYSTEM
#define RENDERER_SDL
#include "system.h"
#include "main.h"

pthread_cond_t renderer_notify = PTHREAD_COND_INITIALIZER;
pthread_mutex_t renderer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t system_notify = PTHREAD_COND_INITIALIZER;
pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;

struct system sys;

static inline void system_input( void )
{
    SDL_Event keypress;
    while (SDL_PollEvent(&keypress))
    {
        switch (keypress.type)
        {
            case SDL_QUIT: running = 0; break;
            case SDL_KEYDOWN: switch (keypress.key.keysym.sym) { case SDLK_q: running = 0; break; } break;
            case SDL_KEYUP:   switch (keypress.key.keysym.sym) { case SDLK_q: running = 0; break; } break;
        }
    }
}

void system_render( void ) {
    SDL_UpdateTexture(sys.screen, NULL, sys.frame_buffer, WIN_W * sizeof(uint32_t));
    SDL_RenderClear(sys.renderer);
    SDL_RenderCopy(sys.renderer, sys.screen, NULL, &sys.scale);
    SDL_RenderPresent(sys.renderer);
}

void system_render_next_frame( void ) {
    sys.render_next_frame = 1;
}

void system_write_audio_sample(int32_t sample) {
    SDL_CHECK_RET(SDL_AudioStreamPut(sys.audio_stream, &sample, sizeof(sample)));
}

void wait_system_ready( void ) {
    assert(!pthread_cond_wait(&system_notify, &system_mutex));
}

void *task_system( void *ignore )
{
    printf("SYSTEM: %ld\n", pthread_self());

    SDL_CHECK_RET(SDL_Init(INITIALIZE_FLAGS));

    /* VIDEO  */
    SDL_CHECK_PTR(sys.window   = SDL_CreateWindow(NAME, WIN_X, WIN_Y, WIN_W, WIN_H, WINDOW_FLAGS));
    SDL_CHECK_PTR(sys.renderer = SDL_CreateRenderer(sys.window, -1, SDL_RENDERER_ACCELERATED));
    SDL_CHECK_PTR(sys.screen   = SDL_CreateTexture(sys.renderer, SDL_PIXELFORMAT_ABGR8888, 
                                                   SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H));

    sys.scale = (SDL_Rect) {0, 0, WIN_W, WIN_H};

    /* AUDIO */
    SDL_CHECK_PTR(sys.audio_stream = 
            SDL_NewAudioStream(AUDIO_S32, 1, 22050, AUDIO_F32, 2, 48000));

    assert(!pthread_cond_broadcast(&system_notify));

    /* SYSTEM LOOP */
    while ( running ) {
        system_input();

        if (sys.render_next_frame) {
            system_render();
            sys.render_next_frame =
                ~sys.render_next_frame;
        }
    }

    /* AUDIO */
    SDL_FreeAudioStream(sys.audio_stream);

    /* VIDEO*/
    SDL_DestroyTexture(sys.screen);
    SDL_DestroyRenderer(sys.renderer);
    SDL_DestroyWindow(sys.window);
    SDL_Quit();

    return NULL;
}
