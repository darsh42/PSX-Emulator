#include "../../include/memory.h"

static PSX_ERROR memory_cpu_map(uint8_t **segment, uint32_t *address, uint32_t *mask, uint32_t aligned, bool load);

static struct MEMORY memory;

/* CPU and MAIN BUS memory map */
static uint32_t segment_lookup[] = {
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, // KUSEG
    (uint32_t) 0X7FFFFFFF,                                     // KSEG0
    (uint32_t) 0X1FFFFFFF,                                     // KSEG1
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF               // KSEG2
};

struct MEMORY *get_memory() {return &memory;}

PSX_ERROR memory_load_bios(const char *filebios) {
    FILE *fp;
    if ((fp = fopen(filebios, "rb")) == NULL) {
	    return set_PSX_error(BIOS_FILE_NOT_FOUND);
    }
    if (fread(memory.BIOS.mem, 1, sizeof(memory.BIOS.mem), fp) == 0) {
	    return set_PSX_error(BIOS_FILE_UNREADABLE);
    }
    fclose(fp);

    return set_PSX_error(NO_ERROR);
}

uint8_t *memory_VRAM_pointer(void) {
    return memory.VRAM.mem;
}

uint8_t *memory_pointer(uint32_t address) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, NULL, 4, true) != NO_ERROR) {
        print_memory_error("memory_cpu_store_32bit", "ADDRESS: 0X%08x\n", address);
        exit(1);
    }
    return segment + address;
}


void memory_cpu_load_8bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, NULL, 1, true) != NO_ERROR) {
        print_memory_error("memory_cpu_load_8bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    uint8_t b0 = *(segment + address + 0);
    *result &= 0X00000000;
    *result  = b0;
}

void memory_cpu_store_8bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    uint32_t mask = 0XFFFFFFFF;
    if (memory_cpu_map(&segment, &address, &mask, 1, false) != NO_ERROR) {
        print_memory_error("memory_cpu_store_8bit", "ADDRESS: 0X%08x", address);
        printf("%x\n", address);
        exit(1);
    }
    
    data &= mask;

    uint8_t b0 = (data >> 0) & 0X000000FF;
    *(segment + address + 0) = b0;
}

void memory_cpu_load_16bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, NULL, 2, true) != NO_ERROR) {
        print_memory_error("memory_cpu_load_16bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    uint8_t b0 = *(segment + address + 0);
    uint8_t b1 = *(segment + address + 1);

    *result &= 0X00000000;
    *result  = b0 | (b1 << 8);
}

void memory_cpu_store_16bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    uint32_t mask = 0XFFFFFFFF;
    if (memory_cpu_map(&segment, &address, &mask, 2, false) != NO_ERROR) {
        print_memory_error("memory_cpu_store_16bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }
    data &=mask;

    uint8_t b0 = (data >> 0) & 0X000000FF;
    uint8_t b1 = (data >> 8) & 0X000000FF;

    *(segment + address + 0) = b0;
    *(segment + address + 1) = b1;
}

void memory_cpu_load_32bit(uint32_t address, uint32_t *result) {
    uint8_t *segment;
    if (memory_cpu_map(&segment, &address, NULL, 4, true) != NO_ERROR) {
        print_memory_error("memory_cpu_load_32bit", "ADDRESS: 0X%08x", address);
        exit(1);
    }

    uint8_t b0 = *(segment + address + 0);
    uint8_t b1 = *(segment + address + 1);
    uint8_t b2 = *(segment + address + 2);
    uint8_t b3 = *(segment + address + 3);

    *result &= 0X00000000;
    *result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void memory_cpu_store_32bit(uint32_t address, uint32_t data) {
    uint8_t *segment;
    uint32_t mask = 0XFFFFFFFF;
    if (memory_cpu_map(&segment, &address, &mask, 4, false) != NO_ERROR) {
        print_memory_error("memory_cpu_store_32bit", "ADDRESS: 0X%08x\n", address);
        exit(1);
    }
    data &= mask;

    uint8_t b0 = (data >>  0) & 0X000000FF;
    uint8_t b1 = (data >>  8) & 0X000000FF;
    uint8_t b2 = (data >> 16) & 0X000000FF;
    uint8_t b3 = (data >> 24) & 0X000000FF;


    segment[address + 0] = b0; 
    segment[address + 1] = b1;
    segment[address + 2] = b2;
    segment[address + 3] = b3;

}
/* This is the main mapping function for each of the memory accessing routines                              *
 * segment   -> the memory array is being accessed, e.g. main RAM, IO ports, e.t.c.                         *
 * address   -> the virtual address that is being accessed, this is transformed into a real address         *
 * mask      -> the mask when writing to io ports, this is because some ports have "always zero" bits       *
 *                  for information on why the mask is a given value check the no$psx docs                  *
 * alignment -> stores the number of bytes being accessed and checks if the address is aligned accordingly  *
 * load      -> specifies if the address is being loaded from or stored to                                  */
PSX_ERROR memory_cpu_map(uint8_t **segment, uint32_t *address, uint32_t *mask, uint32_t alignment, bool load) {
    memory.address_accessed = *address; // used for debugging

    if (*address % alignment != 0) {
        if (load) cpu_exception(ADEL);
        else      cpu_exception(ADES);
    }

    uint32_t region = *address & segment_lookup[*address >> 29];

    // KUSEG, KSEG0, KSEG1
    if (region  >= 0X00000000 && region < 0X00200000) {
        if (cop0_SR_Isc()) {*address = (region - 0X00000000) & 0X3FF; *segment = memory.SCRATCH_PAD.mem;}
        else               {          *address = region - 0X00000000; *segment = memory.MAIN.mem;}
    }
    else if (region >= 0X1F000000 && region < 0X1F800000) {*address = region - 0X1F000000; *segment = memory.EXPANSION_1.mem;}
    else if (region >= 0X1F800000 && region < 0X1F801000) {*address = region - 0X1F800000; *segment = memory.SCRATCH_PAD.mem;}
    else if (region >= 0X1F801000 && region < 0X1F802000) {
        // GPU read and write registers
        if      ( load && region >= 0X1f801810 && region < 0X1f801814) {*address = 0; *segment = GPUREAD(); }
        else if (!load && region >= 0X1f801810 && region < 0X1f801814) {*address = 0; *segment = GP0(); }
        else if ( load && region >= 0X1f801814 && region < 0X1f801818) {*address = 0; *segment = GPUSTAT(); }
        else if (!load && region >= 0X1f801814 && region < 0X1f801818) {*address = 0; *segment = GP1(); }
        else {
            *address = region - 0X1F801000; *segment = memory.IO_PORTS.mem;
        }
    }   
    else if (region >= 0X1F802000 && region < 0X1FA00000) {*address = region - 0X1F802000; *segment = memory.EXPANSION_2.mem;}
    else if (region >= 0X1F8A0000 && region < 0X1FC00000) {*address = region - 0X1F8A0000; *segment = memory.EXPANSION_3.mem;}
    else if (region >= 0X1FC00000 && region < 0X1FC80000) {*address = region - 0X1FC00000; *segment = memory.BIOS.mem;}       

    // KSEG2
    else if (region >= 0XC0000000) {*address = region - 0XC0000000; *segment = memory.KSEG2.mem;}
    else                           {return set_PSX_error(MEMORY_CPU_UNMAPPED_ADDRESS);}

    return set_PSX_error(NO_ERROR);
}

/* GPU and VRAM memory map */

void memory_gpu_load_4bit(uint32_t address, uint8_t *data) {
    *data  = memory.VRAM.mem[address];
    *data &= 0X0000000F;
}

void memory_gpu_load_8bit(uint32_t address, uint32_t *data) {
    *data  = memory.VRAM.mem[address];
    *data &= 0X000000FF;
}

void memory_gpu_load_16bit(uint32_t address, uint32_t *data) {
    *data  = memory.VRAM.mem[address];
    *data &= 0X0000FFFF;
}

void memory_gpu_load_24bit(uint32_t address, uint32_t *data) {
    *data  = memory.VRAM.mem[address];
    *data &= 0X00FFFFFF;
}

void memory_gpu_store_4bit(uint32_t address, uint8_t data) {
    data &= 0X0000000F;
    memory.VRAM.mem[address] = data;
}

void memory_gpu_store_8bit(uint32_t address, uint32_t data) {
    data &= 0X000000FF;
    memory.VRAM.mem[address] = data;
}

void memory_gpu_store_16bit(uint32_t address, uint32_t data) {
    data &= 0X0000FFFF;
    memory.VRAM.mem[address] = data;
}

void memory_gpu_store_24bit(uint32_t address, uint32_t data) {
    data &= 0X00FFFFFF;
    memory.VRAM.mem[address] = data;
}
