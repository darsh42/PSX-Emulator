#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <pthread.h>
#include <stdint.h>

#define RAM_START  0x00000000
#define RAM_END    0x1F000000
#define DEV_START  0x1F801000
#define DEV_END    0x1F802000
#define BIOS_START 0x1FC00000
#define BIOS_END   0x1FC80000

#ifdef MEMORY_PRIVATE
struct memory {
    uint8_t ram[0x200000];
    uint8_t bios[0x80000];

    uint32_t cache_control;
};
#endif // MEMORY_PRIVATE

enum memory_map {
    /* general registers */
    expansion_1_base_address = 0x1F801000,
    expansion_2_base_address = 0x1F801004,
    expansion_1              = 0x1F800008,
    expansion_3              = 0x1F80000C,
    bios_rom                 = 0x1F800010,
    spu_delay                = 0x1F800014,
    cdrom_delay              = 0x1F800018,
    expansion_2              = 0x1F80001C,
    com_delay                = 0x1F800020,
    
    /* interrupt control registers */
    i_stat                   = 0x1F801070,
    i_mask                   = 0x1F801074,
    
    /* dma registers */
    dma0_mdec_in_madr        = 0x1F801080,
    dma0_mdec_in_brc         = 0x1F801084,
    dma0_mdec_in_chcr        = 0x1F801088,
    dma1_mdec_out_madr       = 0x1F801090,
    dma1_mdec_out_brc        = 0x1F801094,
    dma1_mdec_out_chcr       = 0x1F801098,
    dma2_gpu_madr            = 0x1F8010A0,
    dma2_gpu_brc             = 0x1F8010A4,
    dma2_gpu_chcr            = 0x1F8010A8,
    dma3_cdrom_madr          = 0x1F8010B0,
    dma3_cdrom_brc           = 0x1F8010B4,
    dma3_cdrom_chcr          = 0x1F8010B8,
    dma4_spu_madr            = 0x1F8010C0,
    dma4_spu_brc             = 0x1F8010C4,
    dma4_spu_chcr            = 0x1F8010C8,
    dma5_pio_madr            = 0x1F8010D0,
    dma5_pio_brc             = 0x1F8010D4,
    dma5_pio_chcr            = 0x1F8010D8,
    dma6_otc_madr            = 0x1F8010E0,
    dma6_otc_brc             = 0x1F8010E4,
    dma6_otc_chcr            = 0x1F8010E8,
    dpcr                     = 0x1F8010F0,
    dicr                     = 0x1F8010F4,
    
    /* timer registers */
    timer_0_current_counter  = 0x1F801100,
    timer_0_mode             = 0x1F801104,
    timer_0_target           = 0x1F801108,
    timer_1_current_counter  = 0x1F801110,
    timer_1_mode             = 0x1F801114,
    timer_1_target           = 0x1F801118,
    timer_2_current_counter  = 0x1F801120,
    timer_2_mode             = 0x1F801124,
    timer_2_target           = 0x1F801128,
    
    /* gpu registers */
    gp0_gpu_read             = 0x1F801810,
    gp1_gpu_stat             = 0x1F801814,

    /* cache control */
    cache_control            = 0xFFFE0130,
};

extern void memory_load_bios( const char *bios );

extern void memory_write(uint32_t address, uint32_t data, uint32_t size);
extern void memory_read(uint32_t address, uint32_t *data, uint32_t size);

extern uint32_t running;

#endif // MEMORY_H_INCLUDED
