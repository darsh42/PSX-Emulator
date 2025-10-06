#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRIVATE_SYSTEM
#define RENDERER_SDL
#include "system.h"
#include "main.h"

struct system                  sys;
struct system_shared_variables sys_shared;

/*================================ HELPERS ===================================*/
static void system_audio_swap_buffers(int16_t **A, int16_t **B) {
    int16_t *T = *A; *A = *B; *B =  T;
}

void notify_system_ready(void) {
    assert(!pthread_mutex_lock(&sys_shared.m_system_ready));
    /* set the system ready flag */
    sys_shared.system_ready = 1;
    assert(!pthread_cond_broadcast(&sys_shared.c_system_ready));
    assert(!pthread_mutex_unlock(&sys_shared.m_system_ready));
}

/*========================= SYSTEM SERVICING METHODS ======================= */
static void system_input(void) {
    SDL_Event keypress;
    while (SDL_PollEvent(&keypress)) {
        switch (keypress.type) {
        case SDL_QUIT: running = 0; break;
        case SDL_KEYDOWN: 
           switch (keypress.key.keysym.sym) { 
               case SDLK_q: running = 0; break; 
           } break;
        case SDL_KEYUP: 
           switch (keypress.key.keysym.sym) { 
               case SDLK_q: running = 0; break; 
           } break;
        }
    }
}

static void system_video(void) {
    int32_t p_ret = 
        pthread_mutex_trylock(&sys_shared.m_render_frame);
    
    assert(p_ret == 0 || p_ret == EBUSY);

    if (!p_ret) {
        assert(!pthread_mutex_lock(&sys_shared.m_framebuffer));
        if (sys_shared.render_frame) {
            /* invert the render frame request */
            sys_shared.render_frame = 0;
            /* clear the renderer */
            SDL_RenderClear(sys.renderer);
            /* update the screen texture */
            SDL_UpdateTexture(sys.screen, NULL, 
                              sys_shared.framebuffer, 
                              WIN_W * sizeof(uint32_t));
            /* copy the screen texture to the renderer and scale */
            SDL_RenderCopy(sys.renderer, 
                           sys.screen, 
                           NULL, &sys.scale);
            /* render the updated screen */
            SDL_RenderPresent(sys.renderer);
        }
        assert(!pthread_mutex_unlock(&sys_shared.m_framebuffer));
        assert(!pthread_mutex_unlock(&sys_shared.m_render_frame));
    }
}

static void system_audio(void *userdata, uint8_t *stream, int32_t len) {
    /* try to acquire the filled audio flag */
    int32_t p_ret = 
        pthread_mutex_trylock(&sys_shared.m_audiobuffer);
    assert(p_ret == 0 || p_ret == EBUSY);
    if (!p_ret) {
        /* check if the audio buffers have been filled */
        if (sys_shared.filled_audio) {
            /* swap the buffers so the consumer 
             * buffer owns the filled buffer */
            sys_shared.filled_audio = 0;
            system_audio_swap_buffers(&sys_shared.audio_consume, 
                                      &sys_shared.audio_complete);
        }
        assert(!pthread_mutex_unlock(&sys_shared.m_audiobuffer));
    }
    /* cast the stream */
    int16_t *output = (int16_t *) stream;
    /* copy the bytes */
    memcpy(output, sys_shared.audio_consume, len);
}

/*================================== INIT AND KILL ==================================*/
static void system_init_video(void) {
    /* create the window */
    SDL_CHECK_PTR(sys.window   = 
            SDL_CreateWindow(NAME, WIN_X, WIN_Y, 
                             WIN_W, WIN_H, WINDOW_FLAGS));
    /* create the renderer */
    SDL_CHECK_PTR(sys.renderer = 
            SDL_CreateRenderer(sys.window, -1, 
                               SDL_RENDERER_ACCELERATED));
    /* create the screen texture */
    SDL_CHECK_PTR(sys.screen   = 
            SDL_CreateTexture(sys.renderer, SDL_PIXELFORMAT_ARGB8888, 
                              SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H));
    /* create the scaling rectangle */
    sys.scale = (SDL_Rect) {0, 0, WIN_W, WIN_H};

    /* initialise the rendering threads */
    init_threads();
}

static void system_init_audio(void) {
    /* zero out the specs */
    SDL_zero(sys.au_want);
    SDL_zero(sys.au_have);
    /* set requirements for audio device */
    sys.au_want.freq     = AUDIO_WANT_FREQUENCY;
    sys.au_want.format   = AUDIO_WANT_FORMAT;
    sys.au_want.channels = AUDIO_WANT_CHANNELS;
    sys.au_want.samples  = AUDIO_WANT_BUFFER;
    sys.au_want.callback = system_audio;
    /* open the audio device */
    sys.au_device =
        SDL_OpenAudioDevice(NULL, 0, &sys.au_want, 
                            &sys.au_have, 0);
    /* check that the device is real */
    assert(sys.au_device != 0);
    /* verify minimum requirements */
    assert(sys.au_have.freq     >= sys.au_want.freq);
    assert(sys.au_have.format   == sys.au_want.format);
    assert(sys.au_have.channels == sys.au_want.channels);
    assert(sys.au_have.samples  >= sys.au_want.samples);
    assert(sys.au_have.callback == sys.au_want.callback);
    /* set the audio buffers */
    sys_shared.audio_produce  = 
        &sys_shared.audiobuffer[2 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    sys_shared.audio_complete = 
        &sys_shared.audiobuffer[1 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    sys_shared.audio_consume  = 
        &sys_shared.audiobuffer[0 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    /* unpause the audio */
    SDL_PauseAudioDevice(sys.au_device, 0);
}

static void system_kill_video(void) {
    /* destroy the screen texture */
    SDL_DestroyTexture(sys.screen);
    /* destroy the renderer */
    SDL_DestroyRenderer(sys.renderer);
    /* destroy the window */
    SDL_DestroyWindow(sys.window);
    /* free the rendering threads */
    free_threads();
}

static void system_kill_audio(void) {
    SDL_CloseAudioDevice(sys.au_device);
}

/*==================================== EXTERNAL API ======================================*/
void system_render_next_frame(void) {
    assert(!pthread_mutex_lock(&sys_shared.m_render_frame));
    sys_shared.render_frame = 1;
    assert(!pthread_mutex_unlock(&sys_shared.m_render_frame));
}

void system_audio_push_sample(int16_t left, int16_t right) {
    static int32_t sample = 0;

    /* pointer asserts? */
    assert(sys_shared.audio_produce);

    /* push the samples */
    sys_shared.audio_produce[2*sample + 0] = left;
    sys_shared.audio_produce[2*sample + 1] = right;

    /* increment samples */
    sample++;

    if (sample >= SAMPLES_BUFFER_SIZE) {
        /* reset sample counter */
        sample = 0;
        /* swap producer and complete buffers */
        /* set the filled buffer notification */
        assert(!pthread_mutex_lock(&sys_shared.m_audiobuffer));
        sys_shared.filled_audio = 1;
        system_audio_swap_buffers(&sys_shared.audio_produce, 
                                  &sys_shared.audio_complete);
        assert(!pthread_mutex_unlock(&sys_shared.m_audiobuffer));
    }
}

void wait_system_ready(void) {
    assert(!pthread_mutex_lock(&sys_shared.m_system_ready));
    while (!sys_shared.system_ready) {
        assert(!pthread_cond_wait(&sys_shared.c_system_ready, 
                                  &sys_shared.m_system_ready));
    }
    assert(!pthread_mutex_unlock(&sys_shared.m_system_ready));
}

void *task_system( void *ignore ) {
    printf("SYSTEM: %ld\n", pthread_self());

    SDL_CHECK_RET(SDL_Init(INITIALIZE_FLAGS));

    system_init_video();
    system_init_audio();

    notify_system_ready();

    while (running) {
        system_input();
        system_video();
    }

    system_kill_video();
    system_kill_audio();

    SDL_Quit();

    return NULL;
}
