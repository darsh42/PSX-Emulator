#include "../../include/dma.h"

static struct DMA dma;

struct DMA *get_dma(void) { return &dma; }

// helpers
static void dma_process_interrupts(void);
static void dma_gpu(void);
static void dma_otc(void);

// external interfaces
PSX_ERROR dma_reset(void) {
    dma.DMA0_MDEC_IN.MADR = (union D_MADR*) memory_pointer(ADDR_DMA0_MDEC_IN + 0);
    dma.DMA0_MDEC_IN.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA0_MDEC_IN + 4);
    dma.DMA0_MDEC_IN.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA0_MDEC_IN + 8);

    dma.DMA1_MDEC_OUT.MADR = (union D_MADR*) memory_pointer(ADDR_DMA1_MDEC_OUT + 0);
    dma.DMA1_MDEC_OUT.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA1_MDEC_OUT + 4);
    dma.DMA1_MDEC_OUT.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA1_MDEC_OUT + 8);

    dma.DMA2_GPU.MADR = (union D_MADR*) memory_pointer(ADDR_DMA2_GPU + 0);
    dma.DMA2_GPU.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA2_GPU + 4);
    dma.DMA2_GPU.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA2_GPU + 8);

    dma.DMA3_CDROM.MADR = (union D_MADR*) memory_pointer(ADDR_DMA3_CDROM + 0);
    dma.DMA3_CDROM.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA3_CDROM + 4);
    dma.DMA3_CDROM.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA3_CDROM + 8);

    dma.DMA4_SPU.MADR = (union D_MADR*) memory_pointer(ADDR_DMA4_SPU + 0);
    dma.DMA4_SPU.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA4_SPU + 4);
    dma.DMA4_SPU.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA4_SPU + 8);

    
    dma.DMA5_PIO.MADR = (union D_MADR*) memory_pointer(ADDR_DMA5_PIO + 0); 
    dma.DMA5_PIO.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA5_PIO + 4);
    dma.DMA5_PIO.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA5_PIO + 8);

    dma.DMA6_OTC.MADR = (union D_MADR*) memory_pointer(ADDR_DMA6_OTC + 0);
    dma.DMA6_OTC.BRC  = (union D_BRC*)  memory_pointer(ADDR_DMA6_OTC + 4);
    dma.DMA6_OTC.CHCR = (union D_CHCR*) memory_pointer(ADDR_DMA6_OTC + 8);

    dma.DPRC = (union DPRC*) memory_pointer(ADDR_DMA_DPRC);

    dma.DIRC = (union DIRC*) memory_pointer(ADDR_DMA_DIRC);

    // reset value from no$psx docs
    dma.DPRC->mdec_in_priority  = 1;
    dma.DPRC->mdec_out_priority = 2;
    dma.DPRC->gpu_priority      = 3;
    dma.DPRC->cdrom_priority    = 4;
    dma.DPRC->spu_priority      = 5;
    dma.DPRC->pio_priority      = 6;
    dma.DPRC->otc_priority      = 7;

    dma.accessing_memory = false;
    dma.device_ready     = false;

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
        case GPU: dma_gpu(); break;
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

// BUG: DMA2 does not work
void dma_gpu(void) {
    union D_MADR madr = *dma.DMA2_GPU.MADR;
    union D_BRC  brc  = *dma.DMA2_GPU.BRC;
    union D_CHCR chcr = *dma.DMA2_GPU.CHCR;
    
    if (!chcr.start_busy)
        return;
    
    switch (chcr.sync_mode) {
        case MANUAL:  set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
        case REQUEST: {
            static  int32_t step;
            static uint32_t address, data;
            static uint32_t block_count, block_size;
            
            // start of request transfer
            if (!dma.accessing_memory) {
                dma.accessing_memory = true;
                dma.device_ready = true;

                address = madr.base_address;
                block_count = brc.BA;
                block_size  = brc.BS;

                switch (chcr.address_step) {
                    case INCREMENT: step = +4; break;
                    case DECREMENT: step = -4; break;
                }

            }

            // process transfer
            switch (chcr.transfer_direction) {
                case DEV_TO_RAM: break;
                case RAM_TO_DEV: {
                    if (!dma.device_ready) {
                        memory_cpu_load_32bit(ADDR_GPUSTAT, &data);
                        dma.device_ready = (data >> 28) & 0b1;
                        return;
                    }

                    memory_cpu_load_32bit(address, &data);
                    memory_cpu_store_32bit(ADDR_GPUREAD, data);
                    address += step;
                    block_size--;

                    // if block transfered, request more
                    if (block_size <= 0) {
                        dma.device_ready = false;
                        block_size = brc.BS;
                        block_count--;
                    }

                    // if all blocks have been transferred transfer complete
                    if (block_count == -1) {
                        dma.DMA2_GPU.CHCR->start_busy = 0;
                        dma.DPRC->gpu_enable = DISABLE;
                        dma.accessing_memory = false;
                        return;
                    }

                    break;
                }
            }
            break;
        }
        case LINKED_LIST: {
            static  int32_t size = 0;
            static uint32_t next, address, header;

            if (!dma.accessing_memory) {
                dma.accessing_memory = true;
                next    = madr.base_address;
            }
            
            // if new packet
            if (size <= 0) {
                // move to next packet
                address = next;

                // check for terminator
                if (address == 0XFFFFFF) {
                    dma.DMA2_GPU.CHCR->start_busy = 0;
                    dma.DPRC->gpu_enable = DISABLE;
                    dma.accessing_memory = false;
                    return;
                }

                // get packet header
                memory_cpu_load_32bit(address, &header);
                // size of new packet (top 8bits)
                size    = (header >> 24) & 0b000000000000000011111111;
                // address of next packet (first 24bits)
                next    = (header >>  0) & 0b111111111111111111111111;
                // skip packet header
                address += 4;
            } else {
                uint32_t command;

                memory_cpu_load_32bit(address, &command);  // read ram bytes
                memory_cpu_store_32bit(ADDR_GP0, command); // send to gpu 

                address += 4;
                size    -= 1;
            }
            break;
        }
    }

}


void dma_otc(void) {
    union D_MADR madr = *dma.DMA6_OTC.MADR;
    union D_BRC  brc  = *dma.DMA6_OTC.BRC;
    union D_CHCR chcr = *dma.DMA6_OTC.CHCR;

    static  int32_t step, size;
    static uint32_t address;

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

                size    = brc.BC;
                address = madr.base_address;
                
                // no$psx docs "automatically cleared on beginning of transfer"
                dma.DMA6_OTC.CHCR->start_trigger = 0;
            }
            
            switch(chcr.transfer_direction) {
                case RAM_TO_DEV: set_PSX_error(UNSUPPORTED_DMA_TRANSFER_DIRECTION); break;
                case DEV_TO_RAM: 
                    if (size <= 1) {
                        // set last double word as the terminator
                        memory_cpu_store_32bit(address, 0XFFFFFF);
                        
                        // let system know dma transfer completed
                        dma.DMA6_OTC.MADR->base_address = 0XFFFFFF;
                        dma.DMA6_OTC.CHCR->start_busy = 0;
                        dma.DPRC->otc_enable = DISABLE;
                        // free bus for cpu
                        dma.accessing_memory = 0;
                    } else {
                        memory_cpu_store_32bit(address, address - 4);
                        address += step;
                        size -= 1;
                    }
                    break;
            }
            break;
        case REQUEST:     set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
        case LINKED_LIST: set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
    }
}


