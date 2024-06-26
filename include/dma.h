#ifndef DMA_H_INCLUDED
#define DMA_H_INCLUDED

#include "common.h"

enum DMA_Direction {
    DEV_TO_RAM = false,
    RAM_TO_DEV = true
};

enum DMA_Step {
    INCREMENT = false,
    DECREMENT = true
};

enum DMA_Sync {
    MANUAL = 0,
    REQUEST = 1,
    LINKED_LIST = 2
};

union DPRC {
    uint32_t value;
    struct {
        uint32_t mdec_in_priority: 3;
        enum GENERAL_ENABLE mdec_in_enable: 1;
        uint32_t mdec_out_priority: 3;
        enum GENERAL_ENABLE mdec_out_enable: 1;
        uint32_t gpu_priority: 3;
        enum GENERAL_ENABLE gpu_enable: 1;
        uint32_t cdrom_priority: 3;
        enum GENERAL_ENABLE cdrom_enable: 1;
        uint32_t spu_priority: 3;
        enum GENERAL_ENABLE spu_enable: 1;
        uint32_t pio_priority: 3;
        enum GENERAL_ENABLE pio_enable: 1;
        uint32_t otc_priority: 3;
        enum GENERAL_ENABLE otc_enable: 1;
    };
};

union DIRC {
    uint32_t value;
    struct {
        uint32_t : 15;
        enum GENERAL_ENABLE forced_irq: 1;
        enum GENERAL_ENABLE irq_enable_dma0: 1;
        enum GENERAL_ENABLE irq_enable_dma1: 1;
        enum GENERAL_ENABLE irq_enable_dma2: 1;
        enum GENERAL_ENABLE irq_enable_dma3: 1;
        enum GENERAL_ENABLE irq_enable_dma4: 1;
        enum GENERAL_ENABLE irq_enable_dma5: 1;
        enum GENERAL_ENABLE irq_enable_dma6: 1;
        enum GENERAL_ENABLE irq_enable_master: 1;
        enum GENERAL_ENABLE irq_flag_dma0: 1;
        enum GENERAL_ENABLE irq_flag_dma1: 1;
        enum GENERAL_ENABLE irq_flag_dma2: 1;
        enum GENERAL_ENABLE irq_flag_dma3: 1;
        enum GENERAL_ENABLE irq_flag_dma4: 1;
        enum GENERAL_ENABLE irq_flag_dma5: 1;
        enum GENERAL_ENABLE irq_flag_dma6: 1;
        enum GENERAL_ENABLE irq_signal: 1;
    };
};

union D_MADR{
    uint32_t value;
    struct {
        uint32_t base_address: 24;
    };
};
union D_BRC {
    uint32_t value;
    struct {
        uint32_t BC: 16;
    };
    struct {
        uint32_t BA: 16;
        uint32_t BS: 16;
    };
};
union D_CHCR {
    uint32_t value;
    struct {
        enum DMA_Direction transfer_direction: 1;
        enum DMA_Step address_step: 1;
        uint32_t : 6;
        enum GENERAL_ENABLE chopping_enable: 1;
        enum DMA_Sync sync_mode: 2;
        uint32_t : 5;
        uint32_t chopping_dma_window_size: 3;
        uint32_t : 1;
        uint32_t chopping_cpu_window_size: 3;
        uint32_t : 1;
        enum GENERAL_ENABLE start_busy: 1;
        uint32_t : 3;
        enum GENERAL_ENABLE start_trigger: 1;
        uint32_t : 3;
    };
};

struct DMAn {
    union D_MADR *MADR;
    union D_BRC  *BRC;
    union D_CHCR *CHCR;
    uint32_t *unused;
};

struct DMA {
    struct DMAn DMA0_MDEC_IN;
    struct DMAn DMA1_MDEC_OUT;
    struct DMAn DMA2_GPU;
    struct DMAn DMA3_CDROM;
    struct DMAn DMA4_SPU;
    struct DMAn DMA5_PIO;
    struct DMAn DMA6_OTC;
    
    union DPRC *DPRC;
    union DIRC *DIRC;
};

// memory function
extern uint8_t *memory_pointer(uint32_t address);

#endif // DMA_H_INCLUDED
