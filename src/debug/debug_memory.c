#include "debug_memory.h"

void peak_memory(uint32_t address) {
    uint32_t result;
    if (memory_cpu_load_32bit(address, &result) != NO_ERROR) {
        print_debug_memory_error("peek_memory");
        return;
    }
    printf("address: %d, value: %d\n", address, result);
    return;
}

void peak_memory_chunk(uint32_t base, int chunk_size) {
    for (int i = 0; i < chunk_size; i++) {
        peak_memory(base + i);
    }
}

// TODO: disassembler
