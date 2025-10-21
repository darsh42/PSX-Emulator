#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#define DMA_PRIVATE
#include "dma.h"

#include "gpu.h"
#include "timer.h"
#include "memory.h"

#define DISABLE_DMAn_DPCR(channel) \
    dma.dpcr &= ~(1 << (4 * channel + 3))

static struct dma dma;

static void dma_transfer_manual_cdrom( void ) {
    union madr madr = { .value = dma.dma3_cdrom_madr };
    union brc   brc = { .value = dma.dma3_cdrom_brc  };
    union chcr chcr = { .value = dma.dma3_cdrom_chcr };
}
static void dma_transfer_manual_otc( void ) {
    union madr madr = { .value = dma.dma6_otc_madr };
    union brc   brc = { .value = dma.dma6_otc_brc  };
    union chcr chcr = { .value = dma.dma6_otc_chcr };

    /* Lock memory from CPU */
    dma.memory_locked = DMA_MEMORY_LOCKED;

    uint32_t source = madr.base_address;
    uint32_t size   = brc.bc;
     int32_t step   = 
         (chcr.address_step) ? -4: +4;

    TRACE_DMA("dma_transfer_manual_otc", "base address: %08x | size: %08x\n",
            source, size);

    /* clear */
    for (; size > 1; source += step, size--)
        memory_write(source, source + step, 4);

    /* last double word is a terminator */
    memory_write(source, 0xFFFFFF, 4);

    /* transfer complete */

    /* Unlock memory from CPU */
    dma.memory_locked = DMA_MEMORY_UNLOCKED;

    madr.base_address = 0xFFFFFF;
    chcr.start_busy   = 0;

    dma.dma6_otc_madr = madr.value;
    dma.dma6_otc_chcr = chcr.value;

    /* disable the master enable */
    DISABLE_DMAn_DPCR(DMA6_OTC);
}

static void dma_transfer_request_mdec_in( void ) {
    union madr madr = { .value = dma.dma0_mdec_in_madr };
    union brc   brc = { .value = dma.dma0_mdec_in_brc  };
    union chcr chcr = { .value = dma.dma0_mdec_in_chcr };
}
static void dma_transfer_request_mdec_out( void ) {
    union madr madr = { .value = dma.dma1_mdec_out_madr };
    union brc   brc = { .value = dma.dma1_mdec_out_brc  };
    union chcr chcr = { .value = dma.dma1_mdec_out_chcr };
}
static void dma_transfer_request_gpu( void ) {
    union gpustat gpustat;

    gpu_get_gpustat(&gpustat);

    if (!gpustat.ready_send_vram_cpu ||
        !gpustat.dma_data_request)
        return;

    /* vram read and write */
    union madr madr = { .value = dma.dma2_gpu_madr };
    union brc   brc = { .value = dma.dma2_gpu_brc  };
    union chcr chcr = { .value = dma.dma2_gpu_chcr };

    /* Lock memory from CPU */
    dma.memory_locked = DMA_MEMORY_LOCKED;

    /* dma transfer variables */
    uint32_t data;
    uint32_t step = 
        (chcr.address_step) ? -4: +4;

    uint32_t block_count = brc.ba;
    uint32_t block_size  = brc.bs;

    uint32_t ram_address = madr.base_address;

    TRACE_DMA("dma_transfer_request_gpu", "direction: %s, base address: %08x, block_count: %d, block_size: %d\n", 
            (chcr.transfer_direction) ? "RAM_TO_DEV": "DEV_TO_RAM", ram_address, block_count, block_size);

    switch (chcr.transfer_direction) {
    case RAM_TO_DEVICE: goto handle_ram_to_dev;
    case DEVICE_TO_RAM: goto handle_dev_to_ram;
    }

handle_ram_to_dev:
    while (block_count != 0) {
        memory_read(ram_address, &data, 4);
        memory_write_vram(gpu_get_vram_address(), 
                          data, 4);

        block_size--; ram_address+=step;

        /* if end of block go to next block */
        if (block_size == 0) {
            block_size = brc.bs;
            block_count--;
        }
    } goto finished;

handle_dev_to_ram:
    while (block_count != 0) {
        memory_read_vram(gpu_get_vram_address(), 
                         &data, 4);
        memory_write(ram_address,  data, 4);

        block_size--; ram_address+=step;

        /* if end of block go to next block */
        if (block_size == 0) {
            block_size = brc.bs;
            block_count--;
        }
    } goto finished;

finished:
    /* finish dma transfer */

    /* ensure all data has been transfered */
    assert(block_count == 0 && 
           block_size  == brc.bs);

    /* Unlock memory from CPU */
    dma.memory_locked = DMA_MEMORY_UNLOCKED;

    /* notify gpu of block end */
    gpustat.ready_recieve_dma_block = 0;
    gpu_set_gpustat(gpustat);

    chcr.start_busy   = 0;
    dma.dma2_gpu_chcr = chcr.value;

    /* disable the master enable */
    DISABLE_DMAn_DPCR(DMA2_GPU);
}
static void dma_transfer_request_spu( void ) {
    union madr madr = { .value = dma.dma4_spu_madr };
    union brc   brc = { .value = dma.dma4_spu_brc  };
    union chcr chcr = { .value = dma.dma4_spu_chcr };

    printf("SPU_DMA\n");
}

static void dma_transfer_linkedlist_gpu( void ) {
    union gpustat gpustat;

    gpu_get_gpustat(&gpustat);

    /* wait for gpustat dma data request bit    */
    /* wait for gpustat dma ready recieve block */
    if (!gpustat.ready_recieve_dma_block ||
        !gpustat.dma_data_request)
        return;

    /* lock the memory */

    static uint32_t next = 0x00FFFFFF;

    uint32_t source, size, header, command;

    /* new DMA transfer is determined by checking if the previous
     * DMA transfers end address was preserved                    */
    if (next == 0x00FFFFFF) {
        TRACE_DMA("dma_transfer_linkedlist_gpu", "LIST START\n", 0);

        /* get the start of the linked list */
        next = ((union madr) dma.dma2_gpu_madr).base_address;

        /* lock the memory for the CPU */
        dma.memory_locked = DMA_MEMORY_LOCKED;
    }

    /* load next source address */
    source = next;

    /* read packet header */
    memory_read(source, &header, 4);

    next = (header >>  0) & 0X00FFFFFF; /* store next address  */
    size = (header >> 24) & 0X000000FF; /* read size of packet */

    if (size > 0) {
        TRACE_DMA("dma_transfer_linkedlist_gpu", "source: %08x, header: %08x, next: %08x, size: %08x\n",
                    source, header, next, size);

        /* notify the gpu to process this block */
        gpustat.ready_recieve_dma_block = 0;
        gpu_set_gpustat(gpustat);
    }

    source += 4;

    while (size > 0) {
        memory_read(source, &command, 4);       /* read command from memory */
        memory_write(gp0_gpu_read, command, 4); /* write command to device  */

        TRACE_DMA("", "%08x\n", command);

        source += 4;
        size--;
    }

    /* end of link list is denoted by the packet 0x00FFFFFF */
    if (next == 0x00FFFFFF) {
        TRACE_DMA("dma_transfer_linkedlist_dma", "LIST END\n", 0);

        /* clear start busy */
        union chcr chcr = {.value = dma.dma2_gpu_chcr};
        dma.dma2_gpu_chcr = (chcr.start_busy = 0);

        /* Unlock the memory for the CPU */
        dma.memory_locked = DMA_MEMORY_UNLOCKED;

        /* disable the master enable */
        DISABLE_DMAn_DPCR(DMA2_GPU);
    }
}

static void dma_trace_linked_list( void ) {
    uint32_t source, size, next,
             header, command;

    fprintf(stderr, "Dma Linked List trace\n");

    next = ((union madr) dma.dma2_gpu_madr).base_address;

    while (next != 0xffffff) {
        source = next;

        memory_read(source, &header, 4);

        next = (header >>  0) & 0xffffff;
        size = (header >> 24) & 0x0000ff;

        if (size == 0)
            continue;

        fprintf(stderr, "   Packet: %08x\n", source);

        while (size > 0) {
            memory_read(source, &command, 4);

            fprintf(stderr, "       %08x\n", command);

            source += 4;
            size--;
        }
    }
}

uint32_t dma_memory_locked(void) {
    return dma.memory_locked;
}

uint32_t read_dma( uint32_t address ) {
    uint32_t data;
    switch ( address ) {
    case(dma0_mdec_in_madr ): data = dma.dma0_mdec_in_madr ; break;
    case(dma0_mdec_in_brc  ): data = dma.dma0_mdec_in_brc  ; break;
    case(dma0_mdec_in_chcr ): data = dma.dma0_mdec_in_chcr ; break;
    case(dma1_mdec_out_madr): data = dma.dma1_mdec_out_madr; break;
    case(dma1_mdec_out_brc ): data = dma.dma1_mdec_out_brc ; break;
    case(dma1_mdec_out_chcr): data = dma.dma1_mdec_out_chcr; break;
    case(dma2_gpu_madr     ): data = dma.dma2_gpu_madr     ; break;
    case(dma2_gpu_brc      ): data = dma.dma2_gpu_brc      ; break;
    case(dma2_gpu_chcr     ): data = dma.dma2_gpu_chcr     ; break;
    case(dma3_cdrom_madr   ): data = dma.dma3_cdrom_madr   ; break;
    case(dma3_cdrom_brc    ): data = dma.dma3_cdrom_brc    ; break;
    case(dma3_cdrom_chcr   ): data = dma.dma3_cdrom_chcr   ; break;
    case(dma4_spu_madr     ): data = dma.dma4_spu_madr     ; break;
    case(dma4_spu_brc      ): data = dma.dma4_spu_brc      ; break;
    case(dma4_spu_chcr     ): data = dma.dma4_spu_chcr     ; break;
    case(dma5_pio_madr     ): data = dma.dma5_pio_madr     ; break;
    case(dma5_pio_brc      ): data = dma.dma5_pio_brc      ; break;
    case(dma5_pio_chcr     ): data = dma.dma5_pio_chcr     ; break;
    case(dma6_otc_madr     ): data = dma.dma6_otc_madr     ; break;
    case(dma6_otc_brc      ): data = dma.dma6_otc_brc      ; break;
    case(dma6_otc_chcr     ): data = dma.dma6_otc_chcr     ; break;
    case(dpcr              ): data = dma.dpcr              ; break;
    case(dicr              ): data = dma.dicr              ; break;
    default:
        assert(0 && "Unhandled DMA register read");
    }

    TRACE_DMA("read_dma ", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_dma( uint32_t address, uint32_t data ) {
    switch ( address ) {
    case(dma0_mdec_in_madr ): dma.dma0_mdec_in_madr  = data; break;
    case(dma0_mdec_in_brc  ): dma.dma0_mdec_in_brc   = data; break;
    case(dma0_mdec_in_chcr ): dma.dma0_mdec_in_chcr  = data; break;
    case(dma1_mdec_out_madr): dma.dma1_mdec_out_madr = data; break;
    case(dma1_mdec_out_brc ): dma.dma1_mdec_out_brc  = data; break;
    case(dma1_mdec_out_chcr): dma.dma1_mdec_out_chcr = data; break;
    case(dma2_gpu_madr     ): dma.dma2_gpu_madr      = data; break;
    case(dma2_gpu_brc      ): dma.dma2_gpu_brc       = data; break;
    case(dma2_gpu_chcr     ): dma.dma2_gpu_chcr      = data; break;
    case(dma3_cdrom_madr   ): dma.dma3_cdrom_madr    = data; break;
    case(dma3_cdrom_brc    ): dma.dma3_cdrom_brc     = data; break;
    case(dma3_cdrom_chcr   ): dma.dma3_cdrom_chcr    = data; break;
    case(dma4_spu_madr     ): dma.dma4_spu_madr      = data; break;
    case(dma4_spu_brc      ): dma.dma4_spu_brc       = data; break;
    case(dma4_spu_chcr     ): dma.dma4_spu_chcr      = data; break;
    case(dma5_pio_madr     ): dma.dma5_pio_madr      = data; break;
    case(dma5_pio_brc      ): dma.dma5_pio_brc       = data; break;
    case(dma5_pio_chcr     ): dma.dma5_pio_chcr      = data; break;
    case(dma6_otc_madr     ): dma.dma6_otc_madr      = data; break;
    case(dma6_otc_brc      ): dma.dma6_otc_brc       = data; break;
    case(dma6_otc_chcr     ): dma.dma6_otc_chcr      = data; break;
    case(dpcr              ): dma.dpcr               = data; break;
    case(dicr              ): dma.dicr               = data; break;
    default:
        assert(0 && "Unhandled DMA register write");
    }

    TRACE_DMA("write_dma", "address: %08x | data: %08x\n", address, data);
}


void init_dma( void ) {
    dma.dpcr = 0x07654321;
}

void task_dma( void ) {
    // reverse iterate over all dma channels
    // until a channel has the following:
    //  - enabled in DPRC
    //  - enabled in its own CHCR
    //  - highest priority
    union chcr chcr;

    static const uint32_t *chcrs[] = {
        &dma.dma0_mdec_in_chcr,
        &dma.dma1_mdec_out_chcr,
        &dma.dma2_gpu_chcr,
        &dma.dma3_cdrom_chcr,
        &dma.dma4_spu_chcr,
        &dma.dma5_pio_chcr,
        &dma.dma6_otc_chcr
    };

    /* check if any channel is enabled */
    if (!(dma.dpcr & 0x08888888))
        return;

    dma.channel = DMAX_UNUSED;

    uint32_t dev_priority = 10;
    uint32_t channel_bits, priority, enabled;

    for (uint32_t _dpcr = dma.dpcr, i = 0; _dpcr > 0; _dpcr >>= 4, i++) {
        priority = _dpcr & 0x7;
        enabled  = _dpcr & 0x8;
        chcr.value = *chcrs[i];
        if (enabled && chcr.start_busy && priority < dev_priority) {
            dev_priority = priority;
            dma.channel  = i;
        }
    }

    /* set the correct channel or set to IDLE till next check */
    switch (dma.channel) {
    case DMA0_MDEC_IN:  dma_transfer_request_mdec_in();  break;
    case DMA1_MDEC_OUT: dma_transfer_request_mdec_out(); break;
    case DMA4_SPU:      dma_transfer_request_spu();      break;
    case DMA3_CDROM:    dma_transfer_manual_cdrom();     break;
    case DMA6_OTC:      dma_transfer_manual_otc();       break;
    case DMA2_GPU:
        switch (((union chcr) dma.dma2_gpu_chcr).sync_mode) {
        case REQUEST:     dma_transfer_request_gpu();    break;
        case LINKED_LIST: dma_transfer_linkedlist_gpu(); break;
        }
        break;
    default:
        break;
    }
}
