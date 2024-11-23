#ifndef DMA_H_INCLUDED
#define DMA_H_INCLUDED

#include <pthread.h>
#include <stdint.h>

#ifdef DMA_PRIVATE

enum dma_commands {
    END = 0x0,
    START_TRANSFER = 0x1,
};

enum dma_channels {
    DMA0_MDEC_IN,
    DMA1_MDEC_OUT,
    DMA2_GPU,
    DMA3_CDROM,
    DMA4_SPU,
    DMA5_PIO,
    DMA6_OTC
};

enum dma_sync_mode {
    MANUAL      = 0,
    REQUEST     = 1,
    LINKED_LIST = 2
};

enum dma_direction {
    DEVICE_TO_RAM = 0,
    RAM_TO_DEVICE = 1
};

enum dma_state {
    DMA_IDLE,
    DMA_TRANSFER,
    DMA_CHECK_CHANNEL
};

union madr{
    uint32_t value;
    struct {
        uint32_t base_address: 24;
    };
};
union brc {
    uint32_t value;
    struct {
        uint32_t bc: 16;
    };
    struct {
        uint32_t bs: 16;
        uint32_t ba: 16;
    };
};
union chcr {
    uint32_t value;
    struct {
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


struct dma {
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
    enum dma_state state;
};
#endif

extern uint32_t         read_dma( uint32_t address );
extern pthread_cond_t *write_dma( uint32_t address, uint32_t data );

extern void unlock_dma( void );

extern void *task_dma( void *ignore );

#endif // DMA_H_INCLUDED
