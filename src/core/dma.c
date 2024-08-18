#include "dma.h"

static struct DMA dma;

struct DMA *get_dma(void) { return &dma; }

// helpers
static int dma_get_channel_to_service(void);
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
    int dev = dma_get_channel_to_service();

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

int dma_get_channel_to_service(void) {
    // reverse iterate over all dma channels
    // until a channel has the following:
    //  - enabled in DPRC
    //  - enabled in its own CHCR
    //  - highest priority
    int dev = -1, dev_priority = 10;
    union D_CHCR chcr;
    uint32_t address = ADDR_DMA6_OTC + 8;
    
    for (int i = 6, priority, enabled; i >= 0; i--, address -= 0X10) {
        priority = (dma.DPRC->value >> i*4) & 0b0111;
        enabled  = (dma.DPRC->value >> i*4) & 0b1000;
        memory_cpu_load_32bit(address, &chcr.value);
        
        if (enabled && chcr.start_busy && priority < dev_priority) {
            dev_priority = priority;
            dev = i;
        }
    }

    return dev;
}

static void dma_gpu_request(union D_MADR madr, union D_BRC brc, union D_CHCR chcr);
static void dma_gpu_linked_list(union D_MADR madr, union D_BRC brc, union D_CHCR chcr);
static void dma_otc_manual(union D_MADR madr, union D_BRC brc, union D_CHCR chcr);

void dma_mdec_in(void) {
    union D_MADR madr = *dma.DMA0_MDEC_IN.MADR;
    union D_BRC  brc  = *dma.DMA0_MDEC_IN.BRC;
    union D_CHCR chcr = *dma.DMA0_MDEC_IN.CHCR;
    
    if (!chcr.start_busy)
        return;
    
    switch (chcr.sync_mode) {
        case MANUAL:      break; 
        case REQUEST:     break; 
        case LINKED_LIST: break; 
    }
}

void dma_gpu(void) {
    union D_MADR madr = *dma.DMA2_GPU.MADR;
    union D_BRC  brc  = *dma.DMA2_GPU.BRC;
    union D_CHCR chcr = *dma.DMA2_GPU.CHCR;
    
    if (!chcr.start_busy)
        return;
    
    switch (chcr.sync_mode) {
        case MANUAL:      set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
        case REQUEST:     dma_gpu_request(madr, brc, chcr); break;
        case LINKED_LIST: dma_gpu_linked_list(madr, brc, chcr); break;
    }
}

void dma_otc(void) {
    union D_MADR madr = *dma.DMA6_OTC.MADR;
    union D_BRC  brc  = *dma.DMA6_OTC.BRC;
    union D_CHCR chcr = *dma.DMA6_OTC.CHCR;

    switch (chcr.sync_mode) {
        case MANUAL:      dma_otc_manual(madr, brc, chcr); break;
        case REQUEST:     set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
        case LINKED_LIST: set_PSX_error(UNSUPPORTED_DMA_SYNC_MODE); break;
    }
}

void dma_gpu_request(union D_MADR madr, union D_BRC brc, union D_CHCR chcr) {
    switch (chcr.transfer_direction) {
        case DEV_TO_RAM: {
            static uint32_t address;
            static  int32_t step, block_count, block_size;

            if (!gpustat_ready_send_vram_cpu())
                return;

            if (!dma.accessing_memory) {
                dma.accessing_memory = true;

                block_count = brc.BA;
                block_size  = brc.BS;

                address = madr.base_address;

                step = (chcr.address_step) ? -4: +4;
            }

            if (block_count == 0) {
                dma.DMA2_GPU.CHCR->start_busy = false;
                dma.accessing_memory = false;
                return;
            }

            if (block_size == 0) {
                block_size = brc.BS;
                block_count--;
            }

            uint32_t data;
            
            memory_cpu_load_32bit(ADDR_GPUREAD, &data);
            memory_cpu_store_32bit(address, data);

            address += step;
            block_size--;
            break;
        }
        case RAM_TO_DEV: {
            static uint32_t address;
            static  int32_t step, block_count, block_size; 
            
            if (!gpustat_dma_data_request())
                 return;

            // if dma starting
            if (!dma.accessing_memory) {
                dma.accessing_memory = true;

                block_count = brc.BA;
                block_size  = brc.BS;

                address = madr.base_address;

                step = (chcr.address_step) ? -4: +4;
            }
            
            // reached end of dma transfer
            if (block_count == 0) {
                dma.DMA2_GPU.CHCR->start_busy = false;
                dma.accessing_memory = false;
                return;
            }

            // get the next block
            if (block_size == 0) {
                block_size = brc.BS;
                block_count--;
            }

            uint32_t data;

            memory_cpu_load_32bit(address, &data);
            memory_cpu_store_32bit(ADDR_GP0, data);
            
            address += step;
            block_size--;
            break;
        }
    }
}

void dma_gpu_linked_list(union D_MADR madr, union D_BRC brc, union D_CHCR chcr) {
    static  int32_t size = 0;
    static uint32_t header, next, address;

    if (!gpustat_dma_data_request())
        return;

    if (!gpustat_ready_recieve_dma_block())
        return;

    if (!dma.accessing_memory) {
        dma.accessing_memory = true;
        next    = madr.base_address;
    }

    if (size == 0) {
        // move to next packet
        address = next;

        // check for terminator
        if (address == 0XFFFFFF) {
            dma.DMA2_GPU.CHCR->start_busy = 0;
            dma.accessing_memory = false;
            return;
        }

        // get packet header
        memory_cpu_load_32bit(address, &header);
        next = (header >>  0) & 0X00FFFFFF;
        size = (header >> 24) & 0X000000FF;

        // skip packet header
        address += 4;
    } else {
        uint32_t command;

        memory_cpu_load_32bit(address, &command);  // read ram bytes
        memory_cpu_store_32bit(ADDR_GP0, command); // send to gpu 

        address += 4;
        size    -= 1;
    }
}

void dma_otc_manual(union D_MADR madr, union D_BRC brc, union D_CHCR chcr) {
    static  int32_t step, size;
    static uint32_t address;
    // if dma has just started, load all new values 
    // for further clock cycles 
    if (!chcr.start_trigger && !dma.accessing_memory)
        return;

    if (!dma.accessing_memory) {
        dma.accessing_memory = true;

        size    = brc.BC;
        address = madr.base_address;

        step = (chcr.address_step) ? -4: +4;
        
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

                // dma.DPRC->otc_enable = DISABLE;
                // free bus for cpu
                dma.accessing_memory = 0;
            } else {
                memory_cpu_store_32bit(address, address - 4);
                address += step;
                size -= 1;
            }
            break;
    }
}

