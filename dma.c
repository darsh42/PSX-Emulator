#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#define DMA_PRIVATE
#include "dma.h"
#include "gpu.h"
#include "timer.h"
#include "memory.h"

static struct dma dma;

static pthread_cond_t  dma_notify = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t dma_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    return data;
}

pthread_cond_t *write_dma( uint32_t address, uint32_t data )
{
    pthread_cond_t *notify = NULL;
    switch ( address )
    {
        case(dma0_mdec_in_madr ): dma.dma0_mdec_in_madr  = data; break;
        case(dma0_mdec_in_brc  ): dma.dma0_mdec_in_brc   = data; break;
        case(dma0_mdec_in_chcr ): dma.dma0_mdec_in_chcr  = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma1_mdec_out_madr): dma.dma1_mdec_out_madr = data; break;
        case(dma1_mdec_out_brc ): dma.dma1_mdec_out_brc  = data; break;
        case(dma1_mdec_out_chcr): dma.dma1_mdec_out_chcr = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma2_gpu_madr     ): dma.dma2_gpu_madr      = data; break;
        case(dma2_gpu_brc      ): dma.dma2_gpu_brc       = data; break;
        case(dma2_gpu_chcr     ): dma.dma2_gpu_chcr      = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma3_cdrom_madr   ): dma.dma3_cdrom_madr    = data; break;
        case(dma3_cdrom_brc    ): dma.dma3_cdrom_brc     = data; break;
        case(dma3_cdrom_chcr   ): dma.dma3_cdrom_chcr    = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma4_spu_madr     ): dma.dma4_spu_madr      = data; break;
        case(dma4_spu_brc      ): dma.dma4_spu_brc       = data; break;
        case(dma4_spu_chcr     ): dma.dma4_spu_chcr      = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma5_pio_madr     ): dma.dma5_pio_madr      = data; break;
        case(dma5_pio_brc      ): dma.dma5_pio_brc       = data; break;
        case(dma5_pio_chcr     ): dma.dma5_pio_chcr      = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dma6_otc_madr     ): dma.dma6_otc_madr      = data; break;
        case(dma6_otc_brc      ): dma.dma6_otc_brc       = data; break;
        case(dma6_otc_chcr     ): dma.dma6_otc_chcr      = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dpcr              ): dma.dpcr               = data; notify = &dma_notify; dma.state = DMA_CHECK_CHANNEL; break;
        case(dicr              ): dma.dicr               = data; break;
    }
    return notify;
}

void dma_check_channels( void )
{
    /* wait for system tick */
    wait_system_tick( 1 );

    // reverse iterate over all dma channels
    // until a channel has the following:
    //  - enabled in DPRC
    //  - enabled in its own CHCR
    //  - highest priority
    union chcr chcr;

     int32_t dev          = -1;
    uint32_t dev_priority = 10;
    uint32_t address      = dma6_otc_chcr;
    uint32_t priority, enabled;
    
    for (int32_t i = 6; i >= 0; i--, address -= 0X10) {
        priority = (dma.dpcr >> i*4) & 0x7;
        enabled  = (dma.dpcr >> i*4) & 0x8;

        memory_read(address, &chcr.value, 4);
        
        if (enabled && chcr.start_busy && priority < dev_priority) {
            dev_priority = priority;
            dev = i;
        }
    }
    
    /* set the correct channel or set to IDLE till next check */
    switch ( dev )
    {
        case ( DMA0_MDEC_IN  ):
        case ( DMA1_MDEC_OUT ):
        case ( DMA2_GPU      ):
        case ( DMA3_CDROM    ):
        case ( DMA4_SPU      ):
        case ( DMA5_PIO      ):
        case ( DMA6_OTC      ):
            dma.channel = dev;
            dma.state   = DMA_TRANSFER;
            break;
        default: 
            dma.state = DMA_IDLE; 
            break;
    }
}

void dma_transfer_data( void )
{
    uint32_t destination, source, clocks;

    union madr madr;
    union  brc  brc;
    union chcr chcr;
    
    /* configure the dma transfer for the device and direction */
    switch ( dma.channel )
    {
        case( DMA0_MDEC_IN  ): 
            clocks = 1;

            madr.value = dma.dma0_mdec_in_madr;
            chcr.value = dma.dma0_mdec_in_chcr;
            brc.value  = dma.dma0_mdec_in_brc;

            break;
        case( DMA1_MDEC_OUT ): 
            clocks = 1;

            madr.value = dma.dma1_mdec_out_madr;
            chcr.value = dma.dma1_mdec_out_chcr;
            brc.value  = dma.dma1_mdec_out_brc;

            break;
        case( DMA2_GPU      ): 
            clocks = 1;

            madr.value = dma.dma2_gpu_madr;
            chcr.value = dma.dma2_gpu_chcr;
            brc.value  = dma.dma2_gpu_brc;

            switch ( chcr.transfer_direction )
            {
                case RAM_TO_DEVICE: destination = gp0_gpu_read; source = madr.base_address; break;
                case DEVICE_TO_RAM: destination = madr.base_address; source = gp0_gpu_read; break;
            }

            break;
        case( DMA3_CDROM    ): 
            /* TODO: bios - 24clks, games - 40clks */
            clocks = 24;

            madr.value = dma.dma3_cdrom_madr;
            chcr.value = dma.dma3_cdrom_chcr;
            brc.value  = dma.dma3_cdrom_brc;

            break;
        case( DMA4_SPU      ): 
            clocks = 4;

            madr.value = dma.dma4_spu_madr;
            chcr.value = dma.dma4_spu_chcr;
            brc.value  = dma.dma4_spu_brc;

            break;
        case( DMA5_PIO      ): 
            clocks = 20;

            madr.value = dma.dma5_pio_madr;
            chcr.value = dma.dma5_pio_chcr;
            brc.value  = dma.dma5_pio_brc;

            break;
        case( DMA6_OTC      ): 
            clocks = 1;

            madr.value = dma.dma6_otc_madr;
            chcr.value = dma.dma6_otc_chcr;
            brc.value  = dma.dma6_otc_brc;

            break;
    }
    
    /* complete specified transfer */
    switch ( chcr.sync_mode )
    {
        /* asserts are to make sure no device uses an unsupported transfer type */
        case MANUAL:
        {
            /* expected: CDROM, OTC */
            assert(dma.channel != DMA0_MDEC_IN);
            assert(dma.channel != DMA1_MDEC_OUT);
            assert(dma.channel != DMA2_GPU);
            assert(dma.channel != DMA4_SPU);
            assert(dma.channel != DMA5_PIO);

            /* lock memory */

            uint32_t size = brc.bc;
            uint32_t step = (chcr.address_step) ? -4: +4;
            
            /* OTC is a unique case */
            if ( dma.channel == DMA6_OTC )
            {
                /* clear otc busy flag */
                
                /* clear */
                while ( size > 1 )
                {
                    /* wait for system tick */
                    wait_system_tick( clocks );

                    memory_write(source, source - 4, 4);
                    source += step;
                    size--;
                }
                
                /* last double word is a terminator */
                memory_write(source, 0xFFFFFF, 4);
                
                /* transfer complete */
                madr.base_address = 0xFFFFFF;
                chcr.start_busy   = 0;
            }

            /* unlock memory */
            
            break;
        }
        case REQUEST:
        {
            /* expected: MDEC, SPU, GPU*/
            assert(dma.channel != DMA3_CDROM);
            assert(dma.channel != DMA5_PIO);
            assert(dma.channel != DMA6_OTC);

            /* wait for device condition */
        
            /* lock the memory */
            
            uint32_t data;
            uint32_t block_count = brc.ba;
            uint32_t block_size  = brc.bs;
            uint32_t step = (chcr.address_step) ? -4: +4;
            
            while (block_count != 0 && block_size != 0)
            {
                /* wait for system tick */
                wait_system_tick( clocks );
                
                /* if end of block go to next block */
                if (block_size == 0)
                {
                    block_size = brc.bs;
                    block_count--;
                }

                memory_read(source, &data, 4);
                memory_write(destination, data, 4);

                block_size--;
                source += step;
            }

            /* clear start busy */

            /* unlock memory    */

            break;
        }
        case LINKED_LIST:
        {
            /* expected: GPU*/
            assert(dma.channel != DMA0_MDEC_IN);
            assert(dma.channel != DMA1_MDEC_OUT);
            assert(dma.channel != DMA3_CDROM);
            assert(dma.channel != DMA4_SPU);
            assert(dma.channel != DMA5_PIO);
            assert(dma.channel != DMA6_OTC);

            /* wait for gpustat dma data request bit    */
            wait_gpu_gpustat_dma_data_request();

            /* lock the memory */
            
            uint32_t next = source; 
            uint32_t size = 0;
            uint32_t header = 0;
            uint32_t command = 0;
            
            /* end of link list is denoted by the packet 0x00FFFFFF */
            while (size != 0 && source != 0x00FFFFFF)
            {
                /* wait for system clock */
                wait_system_tick( clocks );

                /* if at the end of packet */
                if (size == 0)
                {
                    /* wait for gpustat dma ready recieve block */
                    wait_gpu_gpustat_dma_ready_recieve_block();

                    /* load next source address */
                    source = next;
                    
                    /* read packet header */
                    memory_read(source, &header, 4);
                    
                    next = (header >>  0) & 0X00FFFFFF; /* store next address  */
                    size = (header >> 24) & 0X000000FF; /* read size of packet */
                }
                else
                {
                    memory_read(source, &command, 4);      /* read command from memory */
                    memory_write(destination, command, 4); /* write command to device  */
                    
                    size--;
                }
                source += 4;
            }

            /* clear start busy */
            dma.dma2_gpu_chcr = (chcr.start_busy = 0);

            /* unlock memory */

            break;
        }
    }

    dma.state = DMA_IDLE;
}

/* only used if external devices need to notify dma without writing to it */
void unlock_dma( void )
{
    assert(!pthread_cond_signal(&dma_notify));
}

void *task_dma( void *ignore )
{
    printf("DMA ENGINE: %ld\n", pthread_self());
    while (running)
    {
        switch (dma.state)
        {
            case DMA_IDLE:        
                assert(!pthread_cond_wait(&dma_notify, &dma_mutex)); 
                break;
            case DMA_TRANSFER:      dma_transfer_data();  break;
            case DMA_CHECK_CHANNEL: dma_check_channels(); break;
        }
    }

    return NULL;
}
