#include "../../include/dma.h"
#include <error.h>

static struct DMA dma;

struct DMA *get_dma(void) { return &dma; }

// helpers
static void dma_process_interrupts(void);
static void dma_otc(void);

// external interfaces
PSX_ERROR dma_reset(void) {
    dma.DMA0_MDEC_IN.MADR = (union D_MADR*) memory_pointer(0X1F801080);
    dma.DMA0_MDEC_IN.BRC  = (union D_BRC*)  memory_pointer(0X1F801084);
    dma.DMA0_MDEC_IN.CHCR = (union D_CHCR*) memory_pointer(0X1F801088);

    dma.DMA1_MDEC_OUT.MADR = (union D_MADR*) memory_pointer(0X1F801090);
    dma.DMA1_MDEC_OUT.BRC  = (union D_BRC*)  memory_pointer(0X1F801094);
    dma.DMA1_MDEC_OUT.CHCR = (union D_CHCR*) memory_pointer(0X1F801098);

    dma.DMA2_GPU.MADR = (union D_MADR*) memory_pointer(0X1F8010A0);
    dma.DMA2_GPU.BRC  = (union D_BRC*)  memory_pointer(0X1F8010A4);
    dma.DMA2_GPU.CHCR = (union D_CHCR*) memory_pointer(0X1F8010A8);

    dma.DMA3_CDROM.MADR = (union D_MADR*) memory_pointer(0X1F8010B0);
    dma.DMA3_CDROM.BRC  = (union D_BRC*)  memory_pointer(0X1F8010B4);
    dma.DMA3_CDROM.CHCR = (union D_CHCR*) memory_pointer(0X1F8010B8);

    dma.DMA4_SPU.MADR = (union D_MADR*) memory_pointer(0X1F8010C0);
    dma.DMA4_SPU.BRC  = (union D_BRC*)  memory_pointer(0X1F8010C4);
    dma.DMA4_SPU.CHCR = (union D_CHCR*) memory_pointer(0X1F8010C8);

    dma.DMA5_PIO.MADR = (union D_MADR*) memory_pointer(0X1F8010D0);
    dma.DMA5_PIO.BRC  = (union D_BRC*)  memory_pointer(0X1F8010D4);
    dma.DMA5_PIO.CHCR = (union D_CHCR*) memory_pointer(0X1F8010D8);

    dma.DMA6_OTC.MADR = (union D_MADR*) memory_pointer(0X1F8010E0);
    dma.DMA6_OTC.BRC  = (union D_BRC*)  memory_pointer(0X1F8010E4);
    dma.DMA6_OTC.CHCR = (union D_CHCR*) memory_pointer(0X1F8010E8);

    dma.DPRC = (union DPRC*) memory_pointer(0X1F8010F0);

    dma.DIRC = (union DIRC*) memory_pointer(0X1F8010F4);

    // reset value from no$psx docs
    dma.DPRC->mdec_in_priority  = 1;
    dma.DPRC->mdec_out_priority = 2;
    dma.DPRC->gpu_priority      = 3;
    dma.DPRC->cdrom_priority    = 4;
    dma.DPRC->spu_priority      = 5;
    dma.DPRC->pio_priority      = 6;
    dma.DPRC->otc_priority      = 7;

    dma.accessing_memory = false;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR dma_step(void) {
    int dev = -1, dev_priority = 10;

    for (int i = 6, priority, enable; i >= 0; i--) {
        priority = (dma.DPRC->value >> i*4) & 0b0111;
        enable = (dma.DPRC->value >> i*4) & 0b1000;

        if (enable && priority < dev_priority) {
            dev_priority = priority;
            dev = i;
        }
    }

    switch (dev) {
        case MDEC_IN: break;
        case MDEC_OUT: break;
        case GPU: break;
        case CDROM: break;
        case SPU: break;
        case PIO: break;
        case OTC: dma_otc(); break;
        default: break;
    }
    return set_PSX_error(NO_ERROR);
}

void dma_process_interrupts(void) {
    bool forced = dma.DIRC->forced_irq;
    bool master = dma.DIRC->irq_enable_master;
    
    uint32_t irq_sum = dma.DIRC->irq_flag_sum & dma.DIRC->irq_enable_sum;

    dma.interrupt_request = forced || (master && (irq_sum> 0));
}

void dma_otc(void) {
    union D_MADR madr = *dma.DMA6_OTC.MADR;
    union D_BRC  brc  = *dma.DMA6_OTC.BRC;
    union D_CHCR chcr = *dma.DMA6_OTC.CHCR;

    static  int32_t step;
    static uint32_t size, base, address;

    switch (chcr.sync_mode) {
        case MANUAL:      
            // if dma has just started, load all new values 
            // for further clock cycles 
            if (!chcr.start_trigger && !dma.accessing_memory)
                return;

            if (!dma.accessing_memory) {
                dma.accessing_memory = true;

                switch (chcr.address_step) {
                    case INCREMENT: step = +4; break;
                    case DECREMENT: step = -4; break;
                }

                size = brc.BC;
                base = madr.base_address;
                address = base;
                
                // no$psx docs "automatically cleared on beggining of transfer"
                dma.DMA6_OTC.CHCR->start_trigger = 0;
            }
            
            switch(chcr.transfer_direction) {
                case RAM_TO_DEV: set_PSX_error(UNSUPPORTED_DMA_TRANSFER_DIRECTION); break;
                case DEV_TO_RAM: 
                    if (abs(address - base) < size - 4) {
                        // zero out order table 
                        memory_cpu_store_32bit(address, 0);
                    } else {
                        // set last double word as the terminator
                        memory_cpu_store_32bit(address, -1);
                        printf("Order table has cleared");
                        // set dma access to off
                        dma.accessing_memory = 0;
                        dma.DMA6_OTC.CHCR->start_busy = 0;
                    }
                    address += step;
                    break;
            }
            break;
        case REQUEST:     set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
        case LINKED_LIST: set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
    }
}

