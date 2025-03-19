#include <assert.h>
#include <stdio.h>

#define SPU_PRIVATE
#include "spu.h"
#define CDROM_SECTORS
#include "cdrom.h"
#include "memory.h"

struct spu spu;

uint32_t read_spu( uint32_t address )
{
    uint32_t data = 0;

    switch (address)
    {
        case (spu_main_volume_left_right                  ): break;
        case (spu_reverb_output_volume_left_right         ): break;
        case (spu_voice_key_on                            ): data = spu.kon;  break;
        case (spu_voice_key_off                           ): data = spu.koff; break;
        case (spu_channel_fm                              ): break;
        case (spu_channel_noise                           ): break;
        case (spu_channel_reverb                          ): break;
        case (spu_channel_status                          ): break;
        case (spu_sound_ram_reverb_work_area_start_address): break;
        case (spu_sound_ram_irq_address                   ): break;
        case (spu_sound_ram_data_transfer_address         ): break;
        case (spu_sound_ram_data_transfer_fifo            ): break;
        case (spucnt                                      ): data = spu.spucnt.value; break;
        case (spu_sound_ram_data_transfer_control         ): break;
        case (spustat                                     ): data = spu.spustat.value; break;
        case (spu_cd_volume_left_right                    ): break;
        case (spu_extern_volume_left_right                ): break;
        case (spu_current_main_volume_left_right          ): break;
        default:
            /* check for voice registers */
            
            switch (address & 0xFFFFFF0F)
            {
                uint32_t voice = (address & 0x000000F0) >> 4;

                case (spu_voice_volume_left_right_base   ): break;
                case (spu_voice_adpcm_sample_rate_base   ): data = spu.adpcm_sample_rate[voice];    break;
                case (spu_voice_adpcm_start_address_base ): data = spu.adpcm_start_address[voice];  break;
                case (spu_voice_adsr_base                ): break;
                case (spu_voice_adsr_current_volume_base ): break;
                case (spu_voice_adpcm_repeat_address_base): data = spu.adpcm_repeat_address[voice]; break;
                default:
                    break;
            }

            break;
    }

    return data;
}

void write_spu( uint32_t address, uint32_t data )
{
}

#define SIGN_MSK(   b) (1U << ((b) - 1))
#define SIGN_EXT(x, b) (((x) ^ SIGN_MSK((b))) - SIGN_MSK((b)))

void spu_decompress_samples(struct spu_adpcm_sector sector
                            int16_t *decode_buffer,
                            int16_t *old, 
                            int16_t *older)
{
    static int32_t pos_adpcm_table[] = {0, +60, +115, +98, +122};
    static int32_t neg_adpcm_table[] = {0,   0,  -52, -55,  -60};

    uint32_t f0 = pos_adpcm_table[sector.filter];
    uint32_t f1 = neg_adpcm_table[sector.filter];
    
    /* shift 13-15 treated as 9 */
    if (sector.shift > 12)
        sector.shift = 9;
    
    /* decoding algorithm */
    for (uint32_t n = 0; n < 14; n++)
    {
        int32_t msb, lsb, _old, _older;
        
        /* sign extend old samples */
        _old   = SIGN_EXT(*old,   16);
        _older = SIGN_EXT(*older, 16);
        
        /* sign extend compressed samples */
        lsb = SIGN_EXT(sector.data[n] >> 0, 4);
        msb = SIGN_EXT(sector.data[n] >> 4, 4);
        
        /* apply shift mask */
        lsb <<= (12 - sector.shift);
        msb <<= (12 - sector.shift);
        
        /* calculate the sample */
        lsb = lsb + (32 + f0 * _old + f1 * _older) / 64; lsb = CLAMP(lsb, +0x7FFF, -0x8000);
        msb = msb + (32 + f0 *  lsb + f1 * _old  ) / 64; msb = CLAMP(msb, +0x7FFF, -0x8000);
        
        /* populate voice sample buffers */
        decode_buffer[n + 0] = (int16_t) lsb;
        decode_buffer[n + 1] = (int16_t) msb;
        
        /* set new old and older samples */
        *older = (int16_t) lsb;
        *old   = (int16_t) msb;
    }
}

void spu_service_voice(uint32_t voice)
{
    struct spu_adpcm_sector sector;

    uint32_t address = spu.adpcm_current_address[voice] << 3;

    memory_read_sound_ram_sector(address, &sector);

    /* sets the repeat address if current block has loop start set */
    if (sector.loop_start)
        spu.adpcm_repeat_address[voice] = address >> 3;
    
    /* jumps to adpcm repeat address if current block has loop end set */
    if (sector.loop_end)
    {
        spu.adpcm_current_address[voice] = spu.adpcm_repeat_address[voice];

        if (!sector.loop_repeat)
        {
            /* silence voice (volume 0) */
        }
    }

    /* populate voice samples buffer */
    int16_t old, older;

    spu_decompress_samples(sector, 
                           spu.voice_decode_buffer[voice],
                           &old, 
                           &older);

    spu.adpcm_current_address[voice] += 16;
}
