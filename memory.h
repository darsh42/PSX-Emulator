#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#define SPU_SECTORS
#include "spu.h"

#include <pthread.h>
#include <stdint.h>

#include "trace.h"
#define TRACE_DEVMEM(file, function, format, ...) \
    trace(TRACE_MEMORY_EN, file " (devmem)", function, format, __VA_ARGS__)

#ifdef MEMORY_PRIVATE

#include "trace.h"
#define TRACE_MEM(function, format, ...) \
    trace(TRACE_MEMORY_EN, "memory.c", function, format, __VA_ARGS__)

#define SIZE_RAM   0x200000
#define SIZE_SCRPD 0x400
#define SIZE_BIOS  0x80000
#define SIZE_VRAM  0x100000
#define SIZE_SRAM  0x80000

enum read_write { WRITE = 0, READ };

struct memory {
    uint8_t        ram[SIZE_RAM];
    uint8_t scratchpad[SIZE_SCRPD];
    uint8_t       bios[SIZE_BIOS];
    uint8_t       vram[SIZE_VRAM];
    /* layout:
     * 00000h - 003ffh   CD audio left    *
     * 00400h - 007ffh   CD audio right   *
     * 00800h - 00bffh   voice 1 mono     *
     * 00C00h - 00fffh   voice 3 mono     *
     * 01000h - xxxxxh   ADPCM samples    *
     * xxxxxh - 7ffffh   Reverb work area */
    uint8_t  sram[SIZE_SRAM];
    
    /* memory control 1 */
    uint32_t expansion_1_base_address;
    uint32_t expansion_2_base_address;
    uint32_t expansion_1_delay_size;
    uint32_t expansion_3_delay_size;
    uint32_t bios_rom_delay_size;
    uint32_t spu_delay_size;
    uint32_t cdrom_delay_size;
    uint32_t expansion_2_delay_size;
    uint32_t com_delay_size;
    
    /* memory control 2 */

    /* cache control and KSEG2 */
    uint32_t cache_control;
};
#endif // MEMORY_PRIVATE

enum memory_map 
{
    /* general registers */
    expansion_1_base_address = 0x1F801000,
    expansion_2_base_address = 0x1F801004,
    expansion_1_delay_size   = 0x1F801008,
    expansion_3_delay_size   = 0x1F80100C,
    bios_rom_delay_size      = 0x1F801010,
    spu_delay_size           = 0x1F801014,
    cdrom_delay_size         = 0x1F801018,
    expansion_2_delay_size   = 0x1F80101C,
    com_delay_size           = 0x1F801020,
    
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
    
    /* spu registers */
    spu_voice_start                              = 0x1F801C00,
    spu_voice_end                                = 0x1F801D7F,

    /* voice registers come in chunks of 0x10, one for each voice */
    spu_voice_volume_left_base              = 0x1F801C00, // base, base + N * 0x10 for each voice
    spu_voice_volume_right_base             = 0x1F801C02, // base, base + N * 0x10 for each voice
    spu_voice_adpcm_sample_rate_base        = 0x1F801C04, // base, base + N * 0x10 for each voice 
    spu_voice_adpcm_start_address_base      = 0x1F801C06, // base, base + N * 0x10 for each voice 
    spu_voice_adsr_lower_base               = 0x1F801C08, // base, base + N * 0x10 for each voice 
    spu_voice_adsr_upper_base               = 0x1F801C0A, // base, base + N * 0x10 for each voice 
    spu_voice_adsr_current_volume_base      = 0x1F801C0C, // base, base + N * 0x10 for each voice 
    spu_voice_adpcm_repeat_address_base     = 0x1F801C0E, // base, base + N * 0x10 for each voice 

    spu_main_volume_left_right              = 0x1F801D80,
    spu_reverb_output_volume_left_right     = 0x1F801D84,
    spu_voice_key_on                        = 0x1F801D88,
    spu_voice_key_off                       = 0x1F801D8C,
    spu_channel_fm                          = 0x1F801D90,
    spu_channel_noise                       = 0x1F801D94,
    spu_channel_reverb                      = 0x1F801D98,
    spu_channel_status                      = 0x1F801D9C,
    spu_sram_reverb_work_area_start_address = 0x1F801DA2,
    spu_sram_irq_address                    = 0x1F801DA4,
    spu_sram_data_transfer_address          = 0x1F801DA6,
    spu_sram_data_transfer_fifo             = 0x1F801DA8,
    spucnt                                  = 0x1F801DAA,
    spu_sram_data_transfer_control          = 0x1F801DAC,
    spustat                                 = 0x1F801DAE,
    spu_cd_volume_left_right                = 0x1F801DB0,
    spu_extern_volume_left_right            = 0x1F801DB4,
    spu_current_main_volume_left_right      = 0x1F801DB8,

    /* cache control */
    cache_control            = 0xFFFE0130,
};

extern uint32_t *get_vram_pointer( void );

extern void memory_load_exe( const char *exe );
extern void memory_load_bios( const char *bios );

extern void memory_write(uint32_t address, uint32_t data, uint32_t size);
extern void memory_read(uint32_t address, uint32_t *data, uint32_t size);

extern void memory_write_vram(uint32_t address, uint32_t data, uint32_t size);
extern void memory_read_vram(uint32_t address, uint32_t *data, uint32_t size);

extern void memory_write_sram(uint32_t address, uint32_t data, uint32_t size);
extern void memory_read_sram(uint32_t address, uint32_t *data, uint32_t size);
extern void memory_read_sram_sector(uint32_t address, struct spu_adpcm_sector **sector);

#endif // MEMORY_H_INCLUDED
