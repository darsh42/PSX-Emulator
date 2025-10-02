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

static inline void system_render(void) {
    /* clear the renderer */
    SDL_RenderClear(sys.renderer);
    /* update the screen texture */
    SDL_UpdateTexture(sys.screen, NULL, 
                      sys.frame_buffer, 
                      WIN_W * sizeof(uint32_t));
    /* copy the screen texture to the renderer and scale */
    SDL_RenderCopy(sys.renderer, 
                   sys.screen, 
                   NULL, &sys.scale);
    /* render the updated screen */
    SDL_RenderPresent(sys.renderer);
}

pthread_mutex_t audio_samples_lock = PTHREAD_MUTEX_INITIALIZER;
static inline void system_audio_swap_buffers(int32_t **A, int32_t **B) {
    /* switch the buffers */
    pthread_mutex_lock(&audio_samples_lock);
    int32_t *T = *A; *A = *B; *B =  T;
    pthread_mutex_unlock(&audio_samples_lock);
}

void system_audio_callback(void *userdata, uint8_t *stream, int32_t len) {
    /* swap the consumer and complete buffers */
    system_audio_swap_buffers(&sys.audio_consume, 
                              &sys.audio_complete);
    /* cast the stream */
    int16_t *output = (int16_t *) stream;
    /* copy the bytes */
    memcpy(output, sys.audio_consume, len);
}

void system_audio_push_sample(int16_t left, int16_t right) {
    static int32_t sample = 0;

    /* pointer asserts? */
    assert(sys.audio_produce);

    /* clamp the audio samples? */
    left  = (left  < INT16_MIN) ? INT16_MIN : (left  > INT16_MAX ? INT16_MAX : left);
    right = (right < INT16_MIN) ? INT16_MIN : (right > INT16_MAX ? INT16_MAX : right);

    /* push the samples */
    sys.audio_produce[2*sample + 0] = left;
    sys.audio_produce[2*sample + 1] = right;

    /* increment samples */
    sample++;

    if (sample >= SAMPLES_BUFFER_SIZE) {
        /* reset sample counter */
        sample = 0;
        /* swap producer and complete buffers */
        system_audio_swap_buffers(&sys.audio_produce, 
                                  &sys.audio_complete);
    }
}
static inline void system_init_video(void) {
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

static inline void system_init_audio(void) {
    /* zero out the specs */
    SDL_zero(sys.au_want);
    SDL_zero(sys.au_have);

    /* set requirements for audio device */
    sys.au_want.freq     = AUDIO_WANT_FREQUENCY;
    sys.au_want.format   = AUDIO_WANT_FORMAT;
    sys.au_want.channels = AUDIO_WANT_CHANNELS;
    sys.au_want.samples  = AUDIO_WANT_BUFFER;
    sys.au_want.callback = system_audio_callback;

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
    sys.audio_produce  = &sys.samples[2 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    sys.audio_complete = &sys.samples[1 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];
    sys.audio_consume  = &sys.samples[0 * SAMPLES_BUFFER_SIZE * AUDIO_WANT_CHANNELS];

    SDL_PauseAudioDevice(sys.au_device, 0);
}

static inline void system_kill_video(void) {
    /* destroy the screen texture */
    SDL_DestroyTexture(sys.screen);
    /* destroy the renderer */
    SDL_DestroyRenderer(sys.renderer);
    /* destroy the window */
    SDL_DestroyWindow(sys.window);

    /* free the rendering threads */
    free_threads();
}

static inline void system_kill_audio(void) {
    SDL_CloseAudioDevice(sys.au_device);
}

void system_render_next_frame(void) {
    /* mutex lock */
    sys.render_next_frame = 1;
    /* mutex unlock */
}

void wait_system_ready(void) {
    assert(!pthread_cond_wait(&system_notify, &system_mutex));
}

void *task_system( void *ignore )
{
    printf("SYSTEM: %ld\n", pthread_self());

    SDL_CHECK_RET(SDL_Init(INITIALIZE_FLAGS));

    system_init_video();
    system_init_audio();

    assert(!pthread_cond_broadcast(&system_notify));

    /* SYSTEM LOOP */
    while ( running ) {
        system_input();

        /* video */
        if (sys.render_next_frame) {
            system_render();
            sys.render_next_frame =
                !sys.render_next_frame;
        }
    }

    system_kill_video();
    system_kill_audio();

    SDL_Quit();

    return NULL;
}
