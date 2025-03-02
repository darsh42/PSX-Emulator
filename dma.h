#ifndef DMA_H_INCLUDED
#define DMA_H_INCLUDED

#include <stdint.h>

#ifdef DMA_PRIVATE

#include "trace.h"

#ifdef ENABLE_DMA_TRACE
#define TRACE_DMA(function, format, ...) trace("dma.c", function, format, __VA_ARGS__)
#else
#define TRACE_DMA(function, format, ...) 
#endif

enum dma_channels 
{
    DMA0_MDEC_IN  = 0,
    DMA1_MDEC_OUT = 1,
    DMA2_GPU      = 2,
    DMA3_CDROM    = 3,
    DMA4_SPU      = 4,
    DMA5_PIO      = 5,
    DMA6_OTC      = 6,

    DMAX_UNUSED = -1
};

enum dma_sync_mode 
{
    MANUAL      = 0,
    REQUEST     = 1,
    LINKED_LIST = 2
};

enum dma_direction 
{
    DEVICE_TO_RAM = 0,
    RAM_TO_DEVICE = 1
};

union madr
{
    uint32_t value;
    struct 
    {
        uint32_t base_address: 24;
    };
};
union brc 
{
    uint32_t value;
    struct 
    {
        uint32_t bc: 16;
        uint32_t   : 16;
    };
    struct 
    {
        uint32_t bs: 16;
        uint32_t ba: 16;
    };
};
union chcr 
{
    uint32_t value;
    struct 
    {
        uint32_t transfer_direction       : 1;
        uint32_t address_step             : 1;
        uint32_t                          : 6;
        uint32_t chopping_enable          : 1;
        uint32_t sync_mode                : 2;
        uint32_t                          : 5;
        uint32_t chopping_dma_window_size : 3;
        uint32_t                          : 1;
        uint32_t chopping_cpu_window_size : 3;
        uint32_t                          : 1;
        uint32_t start_busy               : 1;
        uint32_t                          : 3;
        uint32_t start_trigger            : 1;
        uint32_t                          : 3;
    };
};


struct dma 
{
    uint32_t dma0_mdec_in_madr;
    uint32_t dma0_mdec_in_brc;
    uint32_t dma0_mdec_in_chcr;
    uint32_t dma1_mdec_out_madr;
    uint32_t dma1_mdec_out_brc;
    uint32_t dma1_mdec_out_chcr;
    uint32_t dma2_gpu_madr;
    uint32_t dma2_gpu_brc;
    uint32_t dma2_gpu_chcr;
    uint32_t dma3_cdrom_madr;
    uint32_t dma3_cdrom_brc;
    uint32_t dma3_cdrom_chcr;
    uint32_t dma4_spu_madr;
    uint32_t dma4_spu_brc;
    uint32_t dma4_spu_chcr;
    uint32_t dma5_pio_madr;
    uint32_t dma5_pio_brc;
    uint32_t dma5_pio_chcr;
    uint32_t dma6_otc_madr;
    uint32_t dma6_otc_brc;
    uint32_t dma6_otc_chcr;
    uint32_t dpcr;
    uint32_t dicr;
    
    enum dma_channels channel;
};
#endif // DMA_PRIVATE

extern uint32_t  read_dma( uint32_t address );
extern void     write_dma( uint32_t address, uint32_t data );

extern void init_dma( void );
extern void task_dma( void );

#endif // DMA_H_INCLUDED
