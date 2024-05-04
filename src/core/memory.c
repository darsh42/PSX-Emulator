#include "memory.h"

static PSX_ERROR memory_cpu_map(uint8_t **segment, uint32_t *address, uint32_t aligned);

static struct MEMORY memory;
static uint32_t segment_lookup[] = {
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, // KUSEG
    (uint32_t) 0X7FFFFFFF,                                     // KSEG0
    (uint32_t) 0X1FFFFFFF,                                     // KSEG1
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF                          // KSEG2
};

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

void memory_cpu_load_8bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 1) != NO_ERROR) {
        print_memory_error("memory_cpu_load_8bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    if (segment == NULL) {
        return;
    }


    uint8_t b0 = *(segment + address + 0);
    *result &= 0X00000000;
    *result  = b0;
}

void memory_cpu_store_8bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 1) != NO_ERROR) {
        print_memory_error("memory_cpu_store_8bit", "ADDRESS: 0X%08x", address);
        printf("%x\n", address);
        exit(1);
    }

    if (segment == NULL) {
        return;
    }

    uint8_t b0 = (data >> 0) & 0X000000FF;
    *(segment + address + 0);
}

void memory_cpu_load_16bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 2) != NO_ERROR) {
        print_memory_error("memory_cpu_load_16bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    uint8_t b0 = *(segment + address + 0);
    uint8_t b1 = *(segment + address + 1);

    if (segment == NULL) {
        return;
    }

    *result &= 0X00000000;
    *result  = b0 | (b1 << 8);
}

void memory_cpu_store_16bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 2) != NO_ERROR) {
        print_memory_error("memory_cpu_store_16bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }
    uint8_t b0 = (data >> 0) & 0X000000FF;
    uint8_t b1 = (data >> 8) & 0X000000FF;

    if (segment == NULL) {
        return;
    }

    *(segment + address + 0);
    *(segment + address + 1);
}

void memory_cpu_load_32bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 4) != NO_ERROR) {
        print_memory_error("memory_cpu_load_32bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    uint8_t b0 = *(segment + address + 0);
    uint8_t b1 = *(segment + address + 1);
    uint8_t b2 = *(segment + address + 2);
    uint8_t b3 = *(segment + address + 3);

    if (segment == NULL) {
        return;
    }

    *result &= 0X00000000;
    *result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void memory_cpu_store_32bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, 4) != NO_ERROR) {
        print_memory_error("memory_cpu_store_32bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    if (segment == NULL) {
        return;
    }

    uint8_t b0 = (data >>  0) & 0X000000FF;
    uint8_t b1 = (data >>  8) & 0X000000FF;
    uint8_t b2 = (data >> 16) & 0X000000FF;
    uint8_t b3 = (data >> 24) & 0X000000FF;

    segment[address + 0] = b0; 
    segment[address + 1] = b1;
    segment[address + 2] = b2;
    segment[address + 3] = b3;
}

PSX_ERROR memory_cpu_map(uint8_t **segment, uint32_t *address, uint32_t alignment) {
    if (*address % alignment != 0) {
        return set_PSX_error(MEMORY_UNALIGNED_ADDRESS);
    }

    uint32_t region = *address & segment_lookup[*address >> 29];

    // KUSEG, KSEG0, KSEG1
    if (region >= 0X00000000 && region < 0X00200000) {
        if (cop0_SR_Isc()) {*address = region - 0X00000000; *segment = memory.SCRATCH_PAD.mem;}
        else               {*address = region - 0X00000000; *segment = memory.MAIN.mem;}
    }
    else if (region >= 0X1F000000 && region < 0X1F800000) {*address = region - 0X1F000000; *segment = memory.EXPANSION_1.mem;}
    else if (region >= 0X1F800000 && region < 0X1F801000) {*address = region - 0X1F800000; *segment = memory.SCRATCH_PAD.mem;}
    else if (region >= 0X1F801000 && region < 0X1F802000) {*address = region - 0X1F801000; *segment = memory.IO_PORTS.mem;}   
    else if (region >= 0X1F802000 && region < 0X1FA00000) {*address = region - 0X1F802000; *segment = memory.EXPANSION_2.mem;}
    else if (region >= 0X1F8A0000 && region < 0X1FC00000) {*address = region - 0X1F8A0000; *segment = memory.EXPANSION_3.mem;}
    else if (region >= 0X1FC00000 && region < 0X1FC80000) {*address = region - 0X1FC00000; *segment = memory.BIOS.mem;}       

    // KSEG2
    else if (region >= 0XC0000000 && region < 0X100000000) {*address = region - 0XC0000000; *segment = memory.KSEG2.mem;}
    else                                                   {return set_PSX_error(MEMORY_CPU_UNMAPPED_ADDRESS);}

    return set_PSX_error(NO_ERROR);
}
