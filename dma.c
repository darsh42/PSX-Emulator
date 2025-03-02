#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#define DMA_PRIVATE
#include "dma.h"

#include "gpu.h"
#include "timer.h"
#include "memory.h"

static struct dma dma;

uint32_t read_dma( uint32_t address )
{
    uint32_t data;
    switch ( address )
    {
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
    }

    TRACE_DMA("read_dma ", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_dma( uint32_t address, uint32_t data )
{
    switch ( address )
    {
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
    }

    TRACE_DMA("write_dma", "address: %08x | data: %08x\n", address, data);
}

void dma_transfer_manual_cdrom( void ) 
{
    union madr madr = { .value = dma.dma3_cdrom_madr };
    union brc   brc = { .value = dma.dma3_cdrom_brc  };
    union chcr chcr = { .value = dma.dma3_cdrom_chcr };
}
void dma_transfer_manual_otc( void ) 
{
    TRACE_DMA("dma_transfer_manual_otc", "manual transfer otc\n", 0);

    union madr madr = { .value = dma.dma6_otc_madr };
    union brc   brc = { .value = dma.dma6_otc_brc  };
    union chcr chcr = { .value = dma.dma6_otc_chcr };

    /* lock memory */

    uint32_t source = madr.base_address;
    uint32_t size = brc.bc;
     int32_t step = (chcr.address_step) ? -4: +4;
    
    /* clear otc busy flag */
    
    /* clear */
    while ( size > 1 )
    {
        memory_write(source, source - 4, 4);
        source += step;
        size--;
    }
    
    /* last double word is a terminator */
    memory_write(source, 0xFFFFFF, 4);
    
    /* transfer complete */
    madr.base_address = 0xFFFFFF;
    chcr.start_busy   = 0;

    dma.dma6_otc_madr = madr.value;
    dma.dma6_otc_chcr = chcr.value;

    /* unlock memory */
}

void dma_transfer_request_mdec_in( void )
{
    union madr madr = { .value = dma.dma0_mdec_in_madr };
    union brc   brc = { .value = dma.dma0_mdec_in_brc  };
    union chcr chcr = { .value = dma.dma0_mdec_in_chcr };
}
void dma_transfer_request_mdec_out( void )
{
    union madr madr = { .value = dma.dma1_mdec_out_madr };
    union brc   brc = { .value = dma.dma1_mdec_out_brc  };
    union chcr chcr = { .value = dma.dma1_mdec_out_chcr };
}
void dma_transfer_request_gpu( void )
{
    if ( !gpu_gpustat_ready_send_vram_cpu() )
        return;

    TRACE_DMA("dma_transfer_request_gpu", "request transfer gpu\n", 0);

    /* vram read and write */
    union madr madr = { .value = dma.dma2_gpu_madr };
    union brc   brc = { .value = dma.dma2_gpu_brc  };
    union chcr chcr = { .value = dma.dma2_gpu_chcr };

    /* dma transfer variables */
    uint32_t data;
    uint32_t ram_address = madr.base_address;
    uint32_t gpu_address = 0;
    uint32_t block_count = brc.ba;
    uint32_t block_size  = brc.bs;
    uint32_t step = (chcr.address_step) ? -4: +4;
    
    while (block_count != 0 && block_size != 0)
    {
        /* if end of block go to next block */
        if (block_size == 0)
        {
            block_size = brc.bs;
            block_count--;
        }
        
        /* get next gpu address */
        gpu_address = gpu_get_vram_address();

        /* copy from source to destination depending on transfer direction */
        if ( chcr.transfer_direction == RAM_TO_DEVICE) { memory_read( ram_address, &data, 4); memory_write_vram( gpu_address,  data, 4); }
        else                                           { memory_read_vram( gpu_address, &data, 4); memory_write( ram_address,  data, 4); }

        block_size--;
        ram_address += step;
    }
}
void dma_transfer_request_spu( void )
{
    union madr madr = { .value = dma.dma4_spu_madr };
    union brc   brc = { .value = dma.dma4_spu_brc  };
    union chcr chcr = { .value = dma.dma4_spu_chcr };
}

void dma_transfer_linkedlist_gpu( void )
{
    /* wait for gpustat dma data request bit    */
    if ( !gpu_gpustat_dma_data_request() )
        return;

    /* wait for gpustat dma ready recieve block */
    if ( !gpu_gpustat_dma_ready_recieve_block() )
        return;

    TRACE_DMA("dma_transfer_linkedlist_gpu", "linked list transfer gpu\n", 0);

    /* lock the memory */
    
    static uint32_t next = 0x00FFFFFF;
    
    uint32_t source, size, header, command;
        
    /* new DMA transfer is determined by checking if the previous
     * DMA transfers end address was preserved                    */
    if (next == 0x00FFFFFF)
        next = ((union madr) dma.dma2_gpu_madr).base_address;
    
    /* load next source address */
    source = next;
    
    /* read packet header */
    memory_read(source, &header, 4);

    next = (header >>  0) & 0X00FFFFFF; /* store next address  */
    size = (header >> 24) & 0X000000FF; /* read size of packet */

    source += 4;

    while (size > 0)
    {
        memory_read(source, &command, 4);       /* read command from memory */
        memory_write(gp0_gpu_read, command, 4); /* write command to device  */
        
        source += 4;
        size--;
    }

    /* notify gpu of block end */
    gpu_notify_dma_block_end();

    /* end of link list is denoted by the packet 0x00FFFFFF, *
     * clear start busy                                      */
    if (next == 0x00FFFFFF)
    {
        union chcr chcr = {.value = dma.dma2_gpu_chcr};
        dma.dma2_gpu_chcr = (chcr.start_busy = 0);
    }

    /* unlock memory */
}

void init_dma( void ) {}
void task_dma( void )
{
    // reverse iterate over all dma channels
    // until a channel has the following:
    //  - enabled in DPRC
    //  - enabled in its own CHCR
    //  - highest priority
    union chcr chcr;
    
    const static uint32_t *chcrs[] = {
        &dma.dma0_mdec_in_chcr,
        &dma.dma1_mdec_out_chcr,
        &dma.dma2_gpu_chcr,
        &dma.dma3_cdrom_chcr,
        &dma.dma4_spu_chcr,
        &dma.dma5_pio_chcr,
        &dma.dma6_otc_chcr
    };
    
    dma.channel = DMAX_UNUSED;

    uint32_t dev_priority = 10;
    uint32_t channel_bits, priority, enabled;
    
    for (int32_t i = 6; i >= 1; i--) 
    {   
        channel_bits = (dma.dpcr >> (i * 4)) & 0xf;

        priority = channel_bits & 0x7; /* get 0b0111 */
        enabled  = channel_bits & 0x8; /* get 0b1000 */
        
        chcr.value = *chcrs[i];
        
        if (enabled && chcr.start_busy && priority < dev_priority) 
        {
            dev_priority = priority;
            dma.channel  = i;
        }
    }
    
    /* set the correct channel or set to IDLE till next check */
    switch (dma.channel)
    {
        case DMA0_MDEC_IN:  dma_transfer_request_mdec_in();  break;
        case DMA1_MDEC_OUT: dma_transfer_request_mdec_out(); break;
        case DMA4_SPU:      dma_transfer_request_spu();      break;
        case DMA3_CDROM:    dma_transfer_manual_cdrom();     break;
        case DMA6_OTC:      dma_transfer_manual_otc();       break;
        case DMA2_GPU:
            switch (((union chcr) dma.dma2_gpu_chcr).sync_mode)
            {
                case REQUEST:     dma_transfer_request_gpu();    break;
                case LINKED_LIST: dma_transfer_linkedlist_gpu(); break;
            }
            break;
        default: 
            break;
    }
}
