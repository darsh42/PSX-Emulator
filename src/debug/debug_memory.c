#include "../../include/debugger.h"

extern struct DEBUGGER debugger;

void peek_memory(uint32_t address) {
    uint32_t b0, b1, b2, b3;
    memory_cpu_load_8bit(address + 0, &b0);
    memory_cpu_load_8bit(address + 1, &b1);
    memory_cpu_load_8bit(address + 2, &b2);
    memory_cpu_load_8bit(address + 3, &b3);
    printf("address: 0X%08X, value: 0X%08X\n", address, (b3 << 24) | 
                                                        (b2 << 16) | 
                                                        (b1 << 8)  | 
                                                        (b0 << 0));
    return;
}

void peek_memory_chunk(uint32_t base, int chunk_size) {
    for (int i = 0; i < chunk_size; i++) {
        peek_memory(base + i*4);
    }
}
