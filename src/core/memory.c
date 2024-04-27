#include "memory.h"

static struct MEMORY memory;

#ifdef DEBUG
struct MEMORY *_memory() {return &memory;}
#endif

PSX_ERROR memory_load_bios(const char *filebios) {
    FILE *fp;
    if ((fp = fopen(filebios, "rb")) == NULL) {
	    return set_PSX_error(BIOS_FILE_NOT_FOUND);
    }
    if (fread(memory.BIOS.mem, 1, sizeof(memory.BIOS.mem), fp) == 0) {
	    return set_PSX_error(BIOS_FILE_UNREADABLE);
    }
    fclose(fp);
}

PSX_ERROR memory_cpu_load_32bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
         if (address >= 0X00000000 && address < 0X00200000) {address = address - 0X00000000; segment = memory.MAIN.mem;}
    else if (address >= 0X1F000000 && address < 0X1F800000) {address = address - 0X1F000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0X1F800000 && address < 0X1F801000) {address = address - 0X1F800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0X1F801000 && address < 0X1F802000) {address = address - 0X1F801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0X1F802000 && address < 0X1FA00000) {address = address - 0X1F802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0X1F8A0000 && address < 0X1FC00000) {address = address - 0X1F8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0X1FC00000 && address < 0X1FC80000) {address = address - 0X1FC00000; segment = memory.BIOS.mem;}
    else if (address >= 0X80000000 && address < 0X80200000) {address = address - 0X80000000; segment = memory.MAIN.mem;}
    else if (address >= 0X9F000000 && address < 0X9F800000) {address = address - 0X9F000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0X9F800000 && address < 0X9F801000) {address = address - 0X9F800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0X9F801000 && address < 0X9F802000) {address = address - 0X9F801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0X9F802000 && address < 0X9FA00000) {address = address - 0X9F802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0X9F8A0000 && address < 0X9FC00000) {address = address - 0X9F8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0X9FC00000 && address < 0X9FC80000) {address = address - 0X9FC00000; segment = memory.BIOS.mem;}
    else if (address >= 0XA0000000 && address < 0XA0200000) {address = address - 0XA0000000; segment = memory.MAIN.mem;}
    else if (address >= 0XBF000000 && address < 0XBF800000) {address = address - 0XBF000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0XBF800000 && address < 0XBF801000) {address = address - 0XBF800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0XBF801000 && address < 0XBF802000) {address = address - 0XBF801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0XBF802000 && address < 0XBFA00000) {address = address - 0XBF802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0XBF8A0000 && address < 0XBFC00000) {address = address - 0XBF8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0XBFC00000 && address < 0XBFC80000) {address = address - 0XBFC00000; segment = memory.BIOS.mem;}
    else return set_PSX_error(MEMORY_CPU_UNMAPPED_ADDRESS);

    uint8_t b0 = *(segment + address + 0);
    uint8_t b1 = *(segment + address + 1);
    uint8_t b2 = *(segment + address + 2);
    uint8_t b3 = *(segment + address + 3);

    *result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR memory_cpu_store_32bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
         if (address >= 0X00000000 && address < 0X00200000) {address = address - 0X00000000; segment = memory.MAIN.mem;}
    else if (address >= 0X1F000000 && address < 0X1F800000) {address = address - 0X1F000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0X1F800000 && address < 0X1F801000) {address = address - 0X1F800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0X1F801000 && address < 0X1F802000) {address = address - 0X1F801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0X1F802000 && address < 0X1FA00000) {address = address - 0X1F802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0X1F8A0000 && address < 0X1FC00000) {address = address - 0X1F8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0X1FC00000 && address < 0X1FC80000) {address = address - 0X1FC00000; segment = memory.BIOS.mem;}
    else if (address >= 0X80000000 && address < 0X80200000) {address = address - 0X80000000; segment = memory.MAIN.mem;}
    else if (address >= 0X9F000000 && address < 0X9F800000) {address = address - 0X9F000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0X9F800000 && address < 0X9F801000) {address = address - 0X9F800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0X9F801000 && address < 0X9F802000) {address = address - 0X9F801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0X9F802000 && address < 0X9FA00000) {address = address - 0X9F802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0X9F8A0000 && address < 0X9FC00000) {address = address - 0X9F8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0X9FC00000 && address < 0X9FC80000) {address = address - 0X9FC00000; segment = memory.BIOS.mem;}
    else if (address >= 0XA0000000 && address < 0XA0200000) {address = address - 0XA0000000; segment = memory.MAIN.mem;}
    else if (address >= 0XBF000000 && address < 0XBF800000) {address = address - 0XBF000000; segment = memory.EXPANSION_1.mem;}
    else if (address >= 0XBF800000 && address < 0XBF801000) {address = address - 0XBF800000; segment = memory.SCRATCH_PAD.mem;}
    else if (address >= 0XBF801000 && address < 0XBF802000) {address = address - 0XBF801000; segment = memory.IO_PORTS.mem;}
    else if (address >= 0XBF802000 && address < 0XBFA00000) {address = address - 0XBF802000; segment = memory.EXPANSION_2.mem;}
    else if (address >= 0XBF8A0000 && address < 0XBFC00000) {address = address - 0XBF8A0000; segment = memory.EXPANSION_3.mem;}
    else if (address >= 0XBFC00000 && address < 0XBFC80000) {address = address - 0XBFC00000; segment = memory.BIOS.mem;}
    else return set_PSX_error(MEMORY_CPU_UNMAPPED_ADDRESS);

    uint8_t b0 = (data >>  0) & 0X000000FF;
    uint8_t b1 = (data >>  8) & 0X000000FF;
    uint8_t b2 = (data >> 16) & 0X000000FF;
    uint8_t b3 = (data >> 24) & 0X000000FF;

    segment[address + 0] = b0; 
    segment[address + 1] = b1;
    segment[address + 2] = b2;
    segment[address + 3] = b3;

    return set_PSX_error(NO_ERROR);
}

