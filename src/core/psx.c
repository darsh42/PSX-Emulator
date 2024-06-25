#include "../../include/psx.h"

struct PSX psx;

struct PSX *get_psx(void) { return &psx; }

void psx_create(int argc, char **argv) {
    if (argc != 3) {
        set_PSX_error(INSUFFICIENT_ARGS);
        print_psx_error("main", "USEAGE: ./psx <bios.bin> game.psx", NULL);
        exit(1);
    }

    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main", "Cannot load BIOS file", NULL);
        exit(1);
    }
    
    psx.running = true;

    psx.cpu = get_cpu();
    psx.gpu = get_gpu();
    psx.dma = get_dma();
    psx.memory = get_memory();
    psx.timers = get_timers();

    cpu_reset();
    gpu_reset();
    dma_reset();
    timers_create();

    sdl_initialize();

#ifdef DEBUG
    debugger_reset();
#endif
}

void psx_step(void) {
#ifdef DEBUG
    debugger_exec();
#endif

    timers_step();

    if (!psx.dma->accessing_memory)
        cpu_step();

    gpu_step();
    dma_step();
}

void psx_destroy(void) {
    sdl_destroy();
}

int main(int argc, char **argv) {

    psx_create(argc, argv);
    while (psx.running) {
        // render and take input
        sdl_update();
        
        // run the emulator until VBLANK has been hit
        while (psx.gpu->render_phase != VBLANK) {
            psx_step();
        }

        psx.gpu->render_phase = RENDER;
    }
    psx_destroy();

    return 0;
}
