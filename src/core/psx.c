#include "psx.h"

void psx_reset(char **argv) {
    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main", "Cannot load BIOS file", NULL);
        exit(1);
    }

    cpu_reset();
    gpu_reset();
    dma_reset();
}

int main(int argc, char **argv) {
    if (argc != 3) {
        set_PSX_error(INSUFFICIENT_ARGS);
        print_psx_error("main", "USEAGE: ./psx <bios.bin> game.psx", NULL);
        exit(1);
    }

    psx_reset(argv);

    set_debug_cpu();
    
    while (1) {
        cpu_fetch();
        peek_cpu_instruction();
        cpu_decode();
        cpu_execute();
    }

    return 0;
}
