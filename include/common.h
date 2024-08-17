#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"

#define DEBUG

enum PSX_ENABLE {
    DISABLE = false,
    ENABLE  = true
};

enum EXCEPTION_CAUSE {
    INT,
    MOD,
    TLBL,
    TLBS,
    ADEL,
    ADES,
    IBE,
    DBE,
    SYS,
    BP,
    RI,
    CPU,
    OVF
};

enum IO_PORT_ADDRESSES {
    ADDR_DMA0_MDEC_IN  = 0X1F801080,
    ADDR_DMA1_MDEC_OUT = 0X1F801090,
    ADDR_DMA2_GPU      = 0X1F8010A0,
    ADDR_DMA3_CDROM    = 0X1F8010B0,
    ADDR_DMA4_SPU      = 0X1F8010C0,
    ADDR_DMA5_PIO      = 0X1F8010D0,
    ADDR_DMA6_OTC      = 0X1F8010E0,
    ADDR_DMA_DPRC      = 0X1F8010F0,
    ADDR_DMA_DIRC      = 0X1F8010F4,
    ADDR_TIMER_0       = 0X1F801100,
    ADDR_TIMER_1       = 0X1F801110,
    ADDR_TIMER_2       = 0X1F801120,
    ADDR_GP0           = 0X1F801810,
    ADDR_GP1           = 0X1F801814,
    ADDR_GPUREAD       = 0X1F801810,
    ADDR_GPUSTAT       = 0X1F801814,
};

struct PSX {
    bool running;

    struct CPU *cpu;
    struct GPU *gpu;
    struct DMA *dma;
    struct MEMORY *memory;
    struct TIMERS *timers;

    uint32_t system_clock;
};


extern void print_error(const char *file, const char *function, const char *format, ...);
extern void print_warning(const char *file, const char *function, const char *format, ...);
extern PSX_ERROR set_PSX_warning(PSX_ERROR err);
extern PSX_ERROR set_PSX_error(PSX_ERROR err);

#endif//COMMON_H_INCLUDED
