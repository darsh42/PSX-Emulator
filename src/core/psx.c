#include "psx.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        set_PSX_error(INSUFFICIENT_ARGS);
        print_psx_error("main");
        printf("USEAGE: ./psx <bios.bin> game.psx\n");
        exit(1);
    }

    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main");
        exit(1);
    }

    set_debug_cpu();
                                           
    cpu_initialize();
    while (1) {
        cpu_fetch();
        cpu_decode();
        peek_cpu_instruction();
        cpu_execute();
        peek_cpu_R_registers();
    }

    return 0;
}
