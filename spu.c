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
        case (spu_main_volume_left_right                  ): data = spu.main_volume; break;
        case (spu_reverb_output_volume_left_right         ): break;
        case (spu_voice_key_on                            ): data = spu.kon;  break;
        case (spu_voice_key_off                           ): data = spu.koff; break;
        case (spu_channel_fm                              ): break;
        case (spu_channel_noise                           ): data = spu.non; break;
        case (spu_channel_reverb                          ): data = spu.eon; break;
        case (spu_channel_status                          ): data = spu.endx; break;
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
                uint32_t voice = (address & 0x00000010) >> 8;

                case (spu_voice_volume_left_right_base   ): data = spu.voice_volume[voice];         break;
                case (spu_voice_adpcm_sample_rate_base   ): data = spu.adpcm_sample_rate[voice];    break;
                case (spu_voice_adpcm_start_address_base ): data = spu.adpcm_start_address[voice];  break;
                case (spu_voice_adsr_base                ): data = spu.adsr[voice];                 break;
                case (spu_voice_adsr_current_volume_base ): data = spu.adsr_current_volume[voice];  break;
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

#define SIGN_MSK(b) (1U << (b - 1))
#define SIGN_PRE(x, b) (x & ((1U << b) - 1))
#define SIGN_EXT(x, b) ((SIGN_PRE(x, b) ^ SIGN_MSK(b)) >> SIGN_MSK(b))

#define CLAMP(x, hi, lo) ((x < lo) ? lo: (x > hi) ? hi: x)

void xa_audio_decode_block( uint32_t  nibble, uint32_t  block, 
                            uint8_t  *source, uint16_t *destination, 
                            uint16_t *old,    uint16_t *older )
{
    /*
     * ------------------------- 
     * | Header (16 bytes)     |
     * -------------------------      ^
     *  b0 b1 b2 b3 b4 b5 b6 b7       |   
     *  _______________________       |
     * |  |  |  |  |  |  |  |  | v0   |
     * |  |  |  |  |  |  |  |  | v1   |
     * |  |  |  |  |  |  |  |  | v2   |
     * |  |  |  |  |  |  |  |  | v3   |
     * |  |  |  |  |  |  |  |  | v4   |
     * |  |  |  |  |  |  |  |  | v5   |
     * |  |  |  |  |  |  |  |  | v6   |
     * |  |  |  |  |  |  |  |  | v7   |
     * |  |  |  |  |  |  |  |  | v8   |  Each sector portion is split into
     * |  |  |  |  |  |  |  |  | v9   |  header and data sections.
     * |  |  |  |  |  |  |  |  | v10  V
     * |  |  |  |  |  |  |  |  | v11  o  The header is 16 bytes long and
     * |  | Data (112 bytes)|  | v12  i  contains the shift and filter for
     * |  |  |  |  |  |  |  |  | v13  c  each block of data contained.
     * |  |  |  |  |  |  |  |  | v14  e
     * |  |  |  |  |  |  |  |  | v15  |  The data section consists of 8 blocks
     * |  |  |  |  |  |  |  |  | v16  |  containing 4 nibbles of compressed
     * |  |  |  |  |  |  |  |  | v17  |  data for each voice, resulting in 112
     * |  |  |  |  |  |  |  |  | v18  |  bytes of data.
     * |  |  |  |  |  |  |  |  | v19  |
     * |  |  |  |  |  |  |  |  | v20  |
     * |  |  |  |  |  |  |  |  | v21  |
     * |  |  |  |  |  |  |  |  | v22  |
     * |  |  |  |  |  |  |  |  | v23  |
     * |  |  |  |  |  |  |  |  | v24  |
     * |  |  |  |  |  |  |  |  | v25  |
     * |  |  |  |  |  |  |  |  | v26  |
     * |  |  |  |  |  |  |  |  | v27  |
     * |  |  |  |  |  |  |  |  | v28  |
     * -------------------------      v
     * <---------blocks-------->
     *
     */
    static int32_t pos_xa_adpcm_table[] = {0, +60, +115, +98, +122};
    static int32_t neg_xa_adpcm_table[] = {0,   0,  -52, -55,  -60};
    
    /* retrieving shift and filter */
    uint32_t shift  = 12 - (source[4 + block * 2 + nibble] & 0xf0);
    uint32_t filter =      (source[4 + block * 2 + nibble] & 0x30) >> 4;
    
    uint32_t f0 = pos_xa_adpcm_table[filter];
    uint32_t f1 = neg_xa_adpcm_table[filter];

    for (uint32_t sample, v = 0; v < 28; v++)
    {
        /* skip the 16 byte header, voice 'v' sample */
        sample = source[16 + block + v * 4];
        /* retrive correct nibble */
        sample = (sample >> 4 * nibble) & 0x0f;
        /* sign extend the value */
        sample = SIGN_EXT(sample, 4);
        /* calculate the new sample */
        sample = (sample << shift) + ((32 + f0 * (*old) + f1 * (*older))/64);
        /* clamp the sample */
        sample = CLAMP(sample, +0x7fff, -0x8000);

    }
}

void xa_audio_decode_sector( uint32_t sector_address )
{
    struct cdrom_sector_cd_xa_2 *sector;

    memory_read_sector(sector_address, &sector);

    for (uint32_t p = 0; p < 18; p++)
    {
        for (uint32_t b = 0; b < 4; b++)
        {
            if (sector->subheader.mono_or_stereo)
            {
                /* decode stereo samples */
                uint16_t r_old, r_older;
                uint16_t l_old, l_older;
                
                xa_audio_decode_block(0, b, &sector->data + 128 * p, spu.samples[p][b], &l_old, &l_older);
                xa_audio_decode_block(1, b, &sector->data + 128 * p, spu.samples[p][b], &r_old, &r_older);
            }
            else
            {
                /* decode mono samples */
                uint16_t old, older;

                xa_audio_decode_block(0, b, &sector->data + 128 * p, spu.samples[p][b], &old, &older);
                xa_audio_decode_block(1, b, &sector->data + 128 * p, spu.samples[p][b], &old, &older);
            }
        }
    }
}
