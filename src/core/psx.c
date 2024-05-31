#include "../../include/psx.h"

void psx_reset(char **argv) {
    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main", "Cannot load BIOS file", NULL);
        exit(1);
    }

    cpu_reset();
    gpu_reset();
    dma_reset();

    // sdl_initialize();

#ifdef DEBUG
    // debugger_init();
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
    disassemble();
//     while (1) {
//         cpu_fetch();
// #ifdef DEBUG
//         // debugger_exec();
//         peek_cpu_instruction();
// #endif
//         cpu_decode();
//         cpu_execute();
//     }
    psx_destroy();
    return 0;
}
