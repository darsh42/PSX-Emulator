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

    assert((fp = fopen(bios, "rb")));
    assert(fread(memory.bios, 1, sizeof(memory.bios), fp));
    assert(!fclose(fp));
}

void memory_write(uint32_t address, uint32_t data, uint32_t size)
{
    assert(size == 4 || size == 2 || size == 1);

    /* segment to write to in case of non-device address */
    uint8_t *segment = NULL;

    /* virtual to physical memory lookup */
    uint32_t physical = address & segment_lookup[address >> 29];

    /* trace signals to memory */
    TRACE_MEM("memory_write", "address: %08x | data: %08x | size: %d\n", address, data, size);

    if ( ( physical >= 0x1F801000 && physical < 0x1F802000 ) || physical == 0xFFFE0130 ) 
    {
        pthread_cond_t *notify = NULL;

        enum memory_map device_address = address;

        switch (device_address)
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
            /* GPU REGISTERS */
            case(gp0_gpu_read      ):
            case(gp1_gpu_stat      ):
                notify = write_gpu(physical, data);
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
            /* MEMORY CONTROL 1 */
            case(expansion_1_base_address): segment = (uint8_t *) &memory.expansion_1_base_address; physical = 0; goto memory_registers_write;
            case(expansion_2_base_address): segment = (uint8_t *) &memory.expansion_2_base_address; physical = 0; goto memory_registers_write;
            case(expansion_1_delay_size  ): segment = (uint8_t *) &memory.expansion_1_delay_size  ; physical = 0; goto memory_registers_write;
            case(expansion_3_delay_size  ): segment = (uint8_t *) &memory.expansion_3_delay_size  ; physical = 0; goto memory_registers_write;
            case(bios_rom_delay_size     ): segment = (uint8_t *) &memory.bios_rom_delay_size     ; physical = 0; goto memory_registers_write;
            case(spu_delay_size          ): segment = (uint8_t *) &memory.spu_delay_size          ; physical = 0; goto memory_registers_write;
            case(cdrom_delay_size        ): segment = (uint8_t *) &memory.cdrom_delay_size        ; physical = 0; goto memory_registers_write;
            case(expansion_2_delay_size  ): segment = (uint8_t *) &memory.expansion_2_delay_size  ; physical = 0; goto memory_registers_write;
            case(com_delay_size          ): segment = (uint8_t *) &memory.com_delay_size          ; physical = 0; goto memory_registers_write;
            /* CACHE CONTROL / KSEG2 */
            case (cache_control          ): segment = (uint8_t *) &memory.cache_control           ; physical = 0; goto memory_registers_write;
        }
        
        /* if the write triggers a change in state send a signal */
        if (notify)
            assert(!pthread_cond_signal(notify));

        return;
    }

    if (physical >= 0x00000000 && physical < 0x00200000) 
    { 
        /* if cache is isolated do scratchpad, else do main ram */
        if ( cpu_cop0_sr_isc() )
        {
            segment = memory.scratchpad;
            physical &= 0x3FF;
        }
        else
        {
            segment =  memory.ram; 
        }
    }
    else if (physical >= 0x1FC00000 && physical < 0x1FC80000) { segment = memory.bios; physical -= 0x1FC00000; }
    else                                                      { return;                                        }

/* if the memory registers are accessed treat them as non-devices*/
memory_registers_write: 

    assert(segment);
    assert(!pthread_rwlock_wrlock(&memory_lock));
    
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
    
    /* trace signals to memory */
    TRACE_MEM("memory_read ", "address: %08x | data: %08x | size: %d\n", address, data, size);

    if ( ( physical >= 0x1F801000 && physical < 0x1F802000 ) || physical == 0xFFFE0130 ) 
    {
        switch ((enum memory_map) address)
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
            /* GPU REGISTERS */
            case(gp0_gpu_read      ):
            case(gp1_gpu_stat      ):
                *data = read_gpu(address);
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
            /* MEMORY CONTROL 1 */
            case(expansion_1_base_address): *data = memory.expansion_1_base_address; break;
            case(expansion_2_base_address): *data = memory.expansion_2_base_address; break;
            case(expansion_1_delay_size  ): *data = memory.expansion_1_delay_size;   break;
            case(expansion_3_delay_size  ): *data = memory.expansion_3_delay_size;   break;
            case(bios_rom_delay_size     ): *data = memory.bios_rom_delay_size ;     break;
            case(spu_delay_size          ): *data = memory.spu_delay_size;           break;
            case(cdrom_delay_size        ): *data = memory.cdrom_delay_size;         break;
            case(expansion_2_delay_size  ): *data = memory.expansion_2_delay_size;   break;
            case(com_delay_size          ): *data = memory.com_delay_size;           break;
            /* CACHE CONTROL / KSEG2 */
            case(cache_control           ): *data = memory.cache_control;            break;
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

    if (physical >= 0x00000000 && physical < 0x00200000) 
    { 
        /* if cache is isolated do scratchpad, else do main ram */
        if ( cpu_cop0_sr_isc() )
        {
            segment = memory.scratchpad;
            physical &= 0x3FF;
        }
        else
        {
            segment =  memory.ram; 
        }
    }
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
