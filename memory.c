#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define MEMORY_PRIVATE
#include "memory.h"

#define PSX_EXE_FORMAT
#include "fileformats.h"

// devices
#include "cpu.h"
#include "gpu.h"
#include "spu.h"
#include "dma.h"
#include "timer.h"
#include "interrupts.h"

static struct memory memory;

/* for sdl texture streaming */
uint32_t *get_vram_pointer( void ) { return memory.vram; }

/* virtual to physical memory lookup table */
static uint32_t segment_lookup[] = {
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, // KUSEG
    (uint32_t) 0X7FFFFFFF,                                     // KSEG0
    (uint32_t) 0X1FFFFFFF,                                     // KSEG1
    (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF               // KSEG2
};

void memory_load_exe( const char *exe )
{
    FILE *fp;

    struct psx_exe_header header;
    
    /* load the header */
    assert((fp = fopen(exe, "rb")));
    assert(fread((void *) &header, 1, sizeof(header), fp) == sizeof(header));

    /* set the cpu registers */
    cpu_load_initial_exe_registers(header.initial_pc,
                                   header.initial_gp,
                                   header.initial_sp_fp_base,
                                   header.initial_sp_fp_offset);

    /* seek to first data section */
    assert(!fseek(fp, 0x800, SEEK_SET));
    /* load the exe contents into ram based on header information */
    assert(fread(memory.ram + (header.destination_address & 0x1fffffff),
                1, header.filesize, fp) == header.filesize);
    /* close the file */
    assert(!fclose(fp));
}

void memory_load_bios( const char *bios )
{
    FILE *fp;

    assert((fp = fopen(bios, "rb")));
    assert(fread(memory.bios, 1, sizeof(memory.bios), fp) == sizeof(memory.bios));
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
        enum memory_map device_address = address;

        switch (device_address)
        {
            /* INTERRUPT REGISTERS */
            case (i_stat):
            case (i_mask):
                write_interrupts(address, data);
                break;
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
                write_dma(address, data);
                break;
            /* GPU REGISTERS */
            case(gp0_gpu_read      ):
            case(gp1_gpu_stat      ):
                write_gpu(address, data);
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
                write_timers(address, data);
                break;
            /* SPU */
            case(spu_voice_volume_left_right_base            ):
            case(spu_voice_adpcm_sample_rate_base            ):
            case(spu_voice_adpcm_start_address_base          ):
            case(spu_voice_adsr_base                         ):
            case(spu_voice_adsr_current_volume_base          ):
            case(spu_voice_adpcm_repeat_address_base         ):
            case(spu_main_volume_left_right                  ):
            case(spu_reverb_output_volume_left_right         ):
            case(spu_voice_key_on                            ):
            case(spu_voice_key_off                           ):
            case(spu_channel_fm                              ):
            case(spu_channel_noise                           ):
            case(spu_channel_reverb                          ):
            case(spu_channel_status                          ):
            case(spu_sound_ram_reverb_work_area_start_address):
            case(spu_sound_ram_irq_address                   ):
            case(spu_sound_ram_data_transfer_address         ):
            case(spu_sound_ram_data_transfer_fifo            ):
            case(spucnt                                      ):
            case(spu_sound_ram_data_transfer_control         ):
            case(spustat                                     ):
            case(spu_cd_volume_left_right                    ):
            case(spu_extern_volume_left_right                ):
            case(spu_current_main_volume_left_right          ):
                write_spu(address, data);
                break;
            /* MEMORY CONTROL 1 */
            case(expansion_1_base_address): 
            case(expansion_2_base_address): 
            case(expansion_1_delay_size  ): 
            case(expansion_3_delay_size  ): 
            case(bios_rom_delay_size     ): 
            case(spu_delay_size          ): 
            case(cdrom_delay_size        ): 
            case(expansion_2_delay_size  ): 
            case(com_delay_size          ): 
            /* CACHE CONTROL / KSEG2 */
            case(cache_control           ):
			   	goto memory_registers_write;
        }

        return;
    }

/* if the memory registers are accessed treat them as non-devices*/
memory_registers_write: 
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
	/* internal memory registers */
	else if (physical == expansion_1_base_address ) {segment = (uint8_t *) &memory.expansion_1_base_address; physical = 0; }
	else if (physical == expansion_2_base_address ) {segment = (uint8_t *) &memory.expansion_2_base_address; physical = 0; }
	else if (physical == expansion_1_delay_size   ) {segment = (uint8_t *) &memory.expansion_1_delay_size  ; physical = 0; }
	else if (physical == expansion_3_delay_size   ) {segment = (uint8_t *) &memory.expansion_3_delay_size  ; physical = 0; }
	else if (physical == bios_rom_delay_size      ) {segment = (uint8_t *) &memory.bios_rom_delay_size     ; physical = 0; }
	else if (physical == spu_delay_size           ) {segment = (uint8_t *) &memory.spu_delay_size          ; physical = 0; }
	else if (physical == cdrom_delay_size         ) {segment = (uint8_t *) &memory.cdrom_delay_size        ; physical = 0; }
	else if (physical == expansion_2_delay_size   ) {segment = (uint8_t *) &memory.expansion_2_delay_size  ; physical = 0; }
	else if (physical == com_delay_size           ) {segment = (uint8_t *) &memory.com_delay_size          ; physical = 0; }
	else if (physical == cache_control            ) {segment = (uint8_t *) &memory.cache_control           ; physical = 0; }
    else 
	{
			// assert(0 && "Unhandled memory address");
            return;
	}

    assert(segment);

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
}

void memory_read(uint32_t address, uint32_t *data, uint32_t size)
{
    assert(data);
    assert(size == 4 || size == 2 || size == 1);
    
	/* memory segment pointer */
    uint8_t *segment = NULL;

    /* virtual to physical memory lookup */
    uint32_t physical = address & segment_lookup[address >> 29];

    /* clear data pointer */
    *data = 0;
    
    if ( (physical >= 0x1F801000 && physical < 0x1F802000) || physical == 0xFFFE0130 ) 
    {
        switch ((enum memory_map) address)
        {
            /* INTERRUPT REGISTERS */
            case (i_stat):
            case (i_mask):
                *data = read_interrupts(address);
                break;
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
            /* SPU */
            case(spu_voice_volume_left_right_base            ):
            case(spu_voice_adpcm_sample_rate_base            ):
            case(spu_voice_adpcm_start_address_base          ):
            case(spu_voice_adsr_base                         ):
            case(spu_voice_adsr_current_volume_base          ):
            case(spu_voice_adpcm_repeat_address_base         ):
            case(spu_main_volume_left_right                  ):
            case(spu_reverb_output_volume_left_right         ):
            case(spu_voice_key_on                            ):
            case(spu_voice_key_off                           ):
            case(spu_channel_fm                              ):
            case(spu_channel_noise                           ):
            case(spu_channel_reverb                          ):
            case(spu_channel_status                          ):
            case(spu_sound_ram_reverb_work_area_start_address):
            case(spu_sound_ram_irq_address                   ):
            case(spu_sound_ram_data_transfer_address         ):
            case(spu_sound_ram_data_transfer_fifo            ):
            case(spucnt                                      ):
            case(spu_sound_ram_data_transfer_control         ):
            case(spustat                                     ):
            case(spu_cd_volume_left_right                    ):
            case(spu_extern_volume_left_right                ):
            case(spu_current_main_volume_left_right          ):
                *data = read_spu(address);
                break;
            /* MEMORY CONTROL 1 */
            case(expansion_1_base_address): 
            case(expansion_2_base_address): 
            case(expansion_1_delay_size  ): 
            case(expansion_3_delay_size  ): 
            case(bios_rom_delay_size     ): 
            case(spu_delay_size          ): 
            case(cdrom_delay_size        ): 
            case(expansion_2_delay_size  ): 
            case(com_delay_size          ): 
            /* CACHE CONTROL / KSEG2 */
            case(cache_control           ): 
				goto memory_registers_read;
        }

        switch (size) 
        {
            case 1: *data = (uint32_t) ( uint8_t) *data; break;
            case 2: *data = (uint32_t) (uint16_t) *data; break;
            case 4: *data = (uint32_t) (uint32_t) *data; break;
        }

        return;
    }

/* if the memory registers are accessed treat them as non-devices*/
memory_registers_read:

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
	/* internal memory registers */
	else if (physical == expansion_1_base_address ) {segment = (uint8_t *) &memory.expansion_1_base_address; physical = 0; }
	else if (physical == expansion_2_base_address ) {segment = (uint8_t *) &memory.expansion_2_base_address; physical = 0; }
	else if (physical == expansion_1_delay_size   ) {segment = (uint8_t *) &memory.expansion_1_delay_size  ; physical = 0; }
	else if (physical == expansion_3_delay_size   ) {segment = (uint8_t *) &memory.expansion_3_delay_size  ; physical = 0; }
	else if (physical == bios_rom_delay_size      ) {segment = (uint8_t *) &memory.bios_rom_delay_size     ; physical = 0; }
	else if (physical == spu_delay_size           ) {segment = (uint8_t *) &memory.spu_delay_size          ; physical = 0; }
	else if (physical == cdrom_delay_size         ) {segment = (uint8_t *) &memory.cdrom_delay_size        ; physical = 0; }
	else if (physical == expansion_2_delay_size   ) {segment = (uint8_t *) &memory.expansion_2_delay_size  ; physical = 0; }
	else if (physical == com_delay_size           ) {segment = (uint8_t *) &memory.com_delay_size          ; physical = 0; }
	else if (physical == cache_control            ) {segment = (uint8_t *) &memory.cache_control           ; physical = 0; }
    else 
	{
			// assert(0 && "Unhandled memory address");
            return;
	}

    assert(segment);
    
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


    /* trace signals to memory */
    TRACE_MEM("memory_read ", "address: %08x | data: %08x | size: %d\n", address, *data, size);
}

void memory_write_vram( uint32_t address, uint32_t data, uint32_t size )
{
    assert(address < VRAM_SIZE);
    assert(size == 1 || size == 2 || size == 4);

    /* trace signals to memory */
    TRACE_MEM("memory_write_vram", "address: %08x | data: %08x | size: %d\n", address, data, size);

    switch ( size )
    {
        case 1:
            memory.vram[address + 0][0]  = (data >>  0);
            memory.vram[address + 0][1]  = (data >>  0);
            memory.vram[address + 0][2]  = (data >>  0);
            break;
        case 2:
            memory.vram[address + 0][0]  = (data >>  0);
            memory.vram[address + 0][1]  = (data >>  0);
            memory.vram[address + 0][2]  = (data >>  0);

            memory.vram[address + 1][0]  = (data >>  8);
            memory.vram[address + 1][1]  = (data >>  8);
            memory.vram[address + 1][2]  = (data >>  8);
            break;
        case 4:
            memory.vram[address + 0][0]  = (data >>  0);
            memory.vram[address + 0][1]  = (data >>  0);
            memory.vram[address + 0][2]  = (data >>  0);

            memory.vram[address + 1][0]  = (data >>  8);
            memory.vram[address + 1][1]  = (data >>  8);
            memory.vram[address + 1][2]  = (data >>  8);

            memory.vram[address + 2][0]  = (data >> 16);
            memory.vram[address + 2][1]  = (data >> 16);
            memory.vram[address + 2][2]  = (data >> 16);

            memory.vram[address + 3][0]  = (data >> 24);
            memory.vram[address + 3][1]  = (data >> 24);
            memory.vram[address + 3][2]  = (data >> 24);
            break;
    }
}

void memory_read_vram( uint32_t address, uint32_t *data, uint32_t size )
{
    assert(data);
    assert(address < VRAM_SIZE);
    assert(size == 1 || size == 2 || size == 4);
    
    /* clear data pointer */
    *data = 0;

    switch ( size )
    {
        case 1:
            *data |= memory.vram[address + 0][0] <<  0;
            break;
        case 2:
            *data |= memory.vram[address + 0][0] <<  0;
            *data |= memory.vram[address + 1][0] <<  8;
            break;
        case 4:
            *data |= memory.vram[address + 0][0] <<  0;
            *data |= memory.vram[address + 1][0] <<  8;
            *data |= memory.vram[address + 2][0] << 16;
            *data |= memory.vram[address + 3][0] << 24;
            break;
    }

    /* trace signals to memory */
    TRACE_MEM("memory_read_vram", "address: %08x | data: %08x | size: %d\n", address, *data, size);
}

void memory_write_sound_ram( uint32_t address, uint32_t data, uint32_t size )
{
    assert(address < SOUND_RAM_SIZE);
    assert(size == 1 || size == 2 || size == 4);

    /* trace signals to memory */
    TRACE_MEM("memory_write_sound_ram", "address: %08x | data: %08x | size: %d\n", address, data, size);

    switch ( size )
    {
        case 1:
            *(memory.sound_ram + address + 0)  = (data >>  0);
            break;
        case 2:
            *(memory.sound_ram + address + 0)  = (data >>  0);
            *(memory.sound_ram + address + 1)  = (data >>  8);
            break;
        case 4:
            *(memory.sound_ram + address + 0)  = (data >>  0);
            *(memory.sound_ram + address + 1)  = (data >>  8);
            *(memory.sound_ram + address + 2)  = (data >> 16);
            *(memory.sound_ram + address + 3)  = (data >> 24);
            break;
    }
}

void memory_read_sound_ram( uint32_t address, uint32_t *data, uint32_t size )
{
    assert(data);
    assert(address < SOUND_RAM_SIZE);
    assert(size == 1 || size == 2 || size == 4);
    
    /* clear data pointer */
    *data = 0;

    switch ( size )
    {
        case 1:
            *data |= *(memory.sound_ram + address + 0) <<  0;
            break;
        case 2:
            *data |= *(memory.sound_ram + address + 0) <<  0;
            *data |= *(memory.sound_ram + address + 1) <<  8;
            break;
        case 4:
            *data |= *(memory.sound_ram + address + 0) <<  0;
            *data |= *(memory.sound_ram + address + 1) <<  8;
            *data |= *(memory.sound_ram + address + 2) << 16;
            *data |= *(memory.sound_ram + address + 3) << 24;
            break;
    }

    /* trace signals to memory */
    TRACE_MEM("memory_read_sound_ram", "address: %08x | data: %08x | size: %d\n", address, *data, size);
}
