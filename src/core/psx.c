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
    return 0;
}
