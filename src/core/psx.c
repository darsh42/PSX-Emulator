#include "psx.h"
#include <SDL2/SDL.h>

#define SDL_CHECK_ZERO(expr) {assert((expr) == 0);}
#define SDL_CHECK_NULL(expr) {assert((expr) != NULL);}

struct PSX psx;

struct PSX *get_psx(void) { return &psx; }

/** create a psx instance */
void 
psx_create
( int argc, char **argv )
{
    if (argc != 3) { print_psx_error("main", "USEAGE: ./psx <bios.bin> game.psx", NULL); exit(-1); }

    if (memory_load_bios(*(++argv)) != NO_ERROR) { print_psx_error("main", "Cannot load BIOS file", NULL); exit(1); }

    // create psx SDL context
    SDL_CHECK_ZERO(SDL_Init(SDL_INIT_VIDEO));
    SDL_CHECK_NULL(psx.window = SDL_CreateWindow(WIN_NAME, 0, 0, WIN_W, WIN_H, WIN_FLAGS));
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_CHECK_NULL(psx.context = SDL_GL_CreateContext(psx.window));
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    glViewport(0, 0, WIN_W, WIN_H);
    
    // create the renderer context and load the shaders
    
    const char *shaders[] = {
        "shaders/screen.vs.glsl",
        "shaders/screen.fs.glsl"
    };
    renderer_create(shaders, 2);
    
    psx.cpu     = get_cpu();
    psx.gpu     = get_gpu();
    psx.dma     = get_dma();
    psx.memory  = get_memory();
    psx.timers  = get_timers();

    cpu_reset();
    gpu_reset();
    dma_reset();
    timers_create();

    #ifdef DEBUG
        // debugger_reset();
    #endif 

    // start the psx
    psx.running = true;
}

/** step the internal components of the psx */
void 
psx_step_components
( void ) 
{
    #ifdef DEBUG
    // debugger_exec();
    #endif

    timers_step();

    if ( !psx.dma->accessing_memory ) { cpu_step(); }

    gpu_step();
    dma_step();
}

/** step the external user interface of the psx */
void
psx_step_interface
( void )
{
    renderer_end_frame();

    SDL_GL_SwapWindow( psx.window );

    SDL_Event e;
    while ( SDL_PollEvent( &e ) )
    {
        if ( e.type == SDL_QUIT )
        {
            exit(0);
        }
    }
    glClearColor( 0.0f , 0.0f , 0.0f , 1.0f );

    psx.gpu->render_phase = RENDER;
    renderer_start_frame();
}

/** run the psx */
void
psx_main
( void )
{
    while ( psx.running ) 
    {
        if ( psx.gpu->render_phase == VBLANK )
        {
            psx_step_interface();
        }

        psx_step_components();
    }
}

/* destroy the psx instance */
void 
psx_destroy
( void ) 
{
    SDL_GL_DeleteContext(psx.context);
    SDL_DestroyWindow(psx.window);
    SDL_Quit();
}

int 
main
( int argc , char **argv )
{
    psx_create(argc, argv);
    psx_main();
    psx_destroy();
}
