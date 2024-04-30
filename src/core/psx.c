#include "psx.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        set_PSX_error(INSUFFICIENT_ARGS);
        print_psx_error("main", "USEAGE: ./psx <bios.bin> game.psx", NULL);
        exit(1);
    }

    if (memory_load_bios(*(++argv)) != NO_ERROR) {
        print_psx_error("main", "Cannot load BIOS file", NULL);
        exit(1);
    }

    set_debug_cpu();
                                           
    cpu_initialize();
    while (1) {
        cpu_fetch();
        cpu_decode();
        // peek_cpu_pc();
        peek_cpu_instruction();
        cpu_execute();
        // peek_cpu_R_registers();
        // peek_coprocessor_n_registers(0);
        // peek_coprocessor_n_registers(2);
    }

    return 0;
}
