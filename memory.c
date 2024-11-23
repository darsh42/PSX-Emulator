#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define MEMORY_PRIVATE
#include "memory.h"

// devices
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "timer.h"

static struct memory memory;

/* virtual to physical memory lookup table */
static uint32_t segment_lookup[] = {
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, // KUSEG
    (uint32_t) 0X7FFFFFFF,                                     // KSEG0
    (uint32_t) 0X1FFFFFFF,                                     // KSEG1
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF               // KSEG2
};

static pthread_rwlock_t memory_lock = PTHREAD_RWLOCK_INITIALIZER;

void memory_load_bios( const char *bios )
{
    FILE *fp;
    printf("%s\n", bios);

    assert((fp = fopen(bios, "rb")));
    assert(fread(memory.bios, 1, sizeof(memory.bios), fp));
    assert(!fclose(fp));
}

void memory_write(uint32_t address, uint32_t data, uint32_t size)
{
    assert(size == 4 || size == 2 || size == 1);

    /* virtual to physical memory lookup */
    uint32_t physical = address & segment_lookup[address >> 29];

    if ( ( physical >= 0x1F801000 && physical < 0x1F802000 ) || physical == 0xFFFE0130 ) 
    {
        pthread_cond_t *notify = NULL;
        switch ((enum memory_map) address - DEV_START)
        {
            /* DMA REGISTERS */
            case(dma0_mdec_in_madr ):
            case(dma0_mdec_in_brc  ):
            case(dma0_mdec_in_chcr ):
            case(dma1_mdec_out_madr):
            case(dma1_mdec_out_brc ):
            case(dma1_mdec_out_chcr):
            case(dma2_gpu_madr     ):
            case(dma2_gpu_brc      ):
            case(dma2_gpu_chcr     ):
            case(dma3_cdrom_madr   ):
            case(dma3_cdrom_brc    ):
            case(dma3_cdrom_chcr   ):
            case(dma4_spu_madr     ):
            case(dma4_spu_brc      ):
            case(dma4_spu_chcr     ):
            case(dma5_pio_madr     ):
            case(dma5_pio_brc      ):
            case(dma5_pio_chcr     ):
            case(dma6_otc_madr     ):
            case(dma6_otc_brc      ):
            case(dma6_otc_chcr     ):
            case(dpcr              ):
            case(dicr              ):
                notify = write_dma(physical, data);
                break;
            /* TIMER REGISTERS */
            case(timer_0_current_counter):
            case(timer_0_mode           ):
            case(timer_0_target         ):
            case(timer_1_current_counter):
            case(timer_1_mode           ):
            case(timer_1_target         ):
            case(timer_2_current_counter):
            case(timer_2_mode           ):
            case(timer_2_target         ):
                notify = write_timers(physical, data);
                break;
            /* GPU REGISTERS */
            case(gp0_gpu_read      ):
            case(gp1_gpu_stat      ):
                notify = write_gpu(physical, data);
                break;
            case (cache_control    ):
                memory.cache_control = 0;
                memory.cache_control |= (uint8_t) (data >> 24); memory.cache_control <<= 8;
                memory.cache_control |= (uint8_t) (data >> 16); memory.cache_control <<= 8;
                memory.cache_control |= (uint8_t) (data >>  8); memory.cache_control <<= 8;
                memory.cache_control |= (uint8_t) (data >>  0); memory.cache_control <<= 0;
                break;
        }
        
        /* if the write triggers a change in state send a signal */
        if (notify)
            assert(!pthread_cond_signal(notify));

        return;
    }


    
    uint8_t *segment = NULL;

         if (physical >= 0x00000000 && physical < 0x00200000) { segment =  memory.ram; physical -= 0x00000000; }
    else if (physical >= 0x1FC00000 && physical < 0x1FC80000) { segment = memory.bios; physical -= 0x1FC00000; }
    else                                                      { return;                                        }

    assert(segment);
    assert(!pthread_rwlock_rdlock(&memory_lock));
    
    switch ( size )
    {
        case 1:
            *(segment + physical + 0) = (uint8_t) (data >>  0);
            break;
        case 2:
            *(segment + physical + 0) = (uint8_t) (data >>  0);
            *(segment + physical + 1) = (uint8_t) (data >>  8);
            break;
        case 4:
            *(segment + physical + 0) = (uint8_t) (data >>  0);
            *(segment + physical + 1) = (uint8_t) (data >>  8);
            *(segment + physical + 2) = (uint8_t) (data >> 16);
            *(segment + physical + 3) = (uint8_t) (data >> 24);
            break;
    }

    assert(!pthread_rwlock_unlock(&memory_lock));
}

void memory_read(uint32_t address, uint32_t *data, uint32_t size)
{
    assert(data);
    assert(size == 4 || size == 2 || size == 1);
    
    /* clear data pointer */
    *data = 0;

    /* virtual to physical memory lookup */
    uint32_t physical = address & segment_lookup[address >> 29];

    if ( ( physical >= 0x1F801000 && physical < 0x1F802000 ) || physical == 0xFFFE0130 ) 
    {
        switch ((enum memory_map) address - DEV_START)
        {
            /* DMA REGISTERS */
            case(dma0_mdec_in_madr ):
            case(dma0_mdec_in_brc  ):
            case(dma0_mdec_in_chcr ):
            case(dma1_mdec_out_madr):
            case(dma1_mdec_out_brc ):
            case(dma1_mdec_out_chcr):
            case(dma2_gpu_madr     ):
            case(dma2_gpu_brc      ):
            case(dma2_gpu_chcr     ):
            case(dma3_cdrom_madr   ):
            case(dma3_cdrom_brc    ):
            case(dma3_cdrom_chcr   ):
            case(dma4_spu_madr     ):
            case(dma4_spu_brc      ):
            case(dma4_spu_chcr     ):
            case(dma5_pio_madr     ):
            case(dma5_pio_brc      ):
            case(dma5_pio_chcr     ):
            case(dma6_otc_madr     ):
            case(dma6_otc_brc      ):
            case(dma6_otc_chcr     ):
            case(dpcr              ):
            case(dicr              ):
                *data = read_dma(address);
                break;
            /* TIMER REGISTERS */
            case(timer_0_current_counter):
            case(timer_0_mode           ):
            case(timer_0_target         ):
            case(timer_1_current_counter):
            case(timer_1_mode           ):
            case(timer_1_target         ):
            case(timer_2_current_counter):
            case(timer_2_mode           ):
            case(timer_2_target         ):
                *data = read_timers(address);
                break;
            /* GPU REGISTERS */
            case(gp0_gpu_read      ):
            case(gp1_gpu_stat      ):
                *data = read_gpu(address);
                break;
            case(cache_control     ):
                *data = memory.cache_control;
        }

        switch (size) 
        {
            case 1: *data = (uint32_t) ( uint8_t) *data; break;
            case 2: *data = (uint32_t) (uint16_t) *data; break;
            case 4: *data = (uint32_t) (uint32_t) *data; break;
        }

        return;
    }

    uint8_t *segment = NULL;

         if (physical >= 0x00000000 && physical < 0x00200000) { segment =  memory.ram; physical -= 0x00000000; }
    else if (physical >= 0x1FC00000 && physical < 0x1FC80000) { segment = memory.bios; physical -= 0x1FC00000; }
    else                                                      { return;                                        }

    assert(segment);
    assert(!pthread_rwlock_rdlock(&memory_lock));
    
    switch ( size )
    {
        case 1:
            *data |= *(segment + physical + 0) <<  0;
            break;
        case 2:
            *data |= *(segment + physical + 0) <<  0;
            *data |= *(segment + physical + 1) <<  8;
            break;
        case 4:
            *data |= *(segment + physical + 0) <<  0;
            *data |= *(segment + physical + 1) <<  8;
            *data |= *(segment + physical + 2) << 16;
            *data |= *(segment + physical + 3) << 24;
            break;
    }

    assert(!pthread_rwlock_unlock(&memory_lock));
}
