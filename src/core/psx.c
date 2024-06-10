#include "../../include/psx.h"

struct PSX psx;

struct PSX *get_psx(void) { return &psx; }

void psx_reset(char **argv) {
    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main", "Cannot load BIOS file", NULL);
        exit(1);
    }
    
    psx.running = true;

    psx.cpu = get_cpu();
    psx.gpu = get_gpu();
    psx.dma = get_dma();
    psx.memory = get_memory();

    cpu_reset();
    gpu_reset();
    dma_reset();

    // sdl_initialize();

#ifdef DEBUG
    debugger_reset();
    set_debug_cpu();
#endif
}

void psx_destroy(void) {
#ifdef DEBUG
    // debugger_destroy();
#endif
    // sdl_destroy();
}

int main(int argc, char **argv) {
    if (argc != 3) {
        set_PSX_error(INSUFFICIENT_ARGS);
        print_psx_error("main", "USEAGE: ./psx <bios.bin> game.psx", NULL);
        exit(1);
    }

    psx_reset(argv);
    // disassemble();
    while (psx.running) {
#ifdef DEBUG
        debugger_exec();
#endif
        cpu_step();
    }
    psx_destroy();
    return 0;
}
