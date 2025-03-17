#include <assert.h>
#include <stdio.h>

#define SPU_PRIVATE
#include "spu.h"
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

struct xa_audio_sector_header
{
    uint8_t file_number;

    struct 
    {
        uint8_t channel_number : 5;
        uint8_t                : 3;
    };

    struct
    {
        uint8_t eor     : 1;
        uint8_t video   : 1;
        uint8_t audio   : 1;
        uint8_t data    : 1;
        uint8_t trigger : 1;
        uint8_t form    : 1;
        uint8_t rt      : 1;
        uint8_t eof     : 1;
    };

    struct
    {
        uint8_t mono_stereo     : 2;
        uint8_t sample_rate     : 2;
        uint8_t bits_per_sample : 2;
        uint8_t emphasis        : 1;
        uint8_t                 : 1;
    };
};

#define SIGN_MSK(b) (1U << (b - 1))
#define SIGN_PRE(x, b) (x & ((1U << b) - 1))
#define SIGN_EXT(x, b) ((SIGN_PRE(x, b) ^ SIGN_MSK(b)) >> SIGN_MSK(b))

#define CLAMP(x, hi, lo) ((x < lo) ? lo: (x > hi) ? hi: x)
void xa_audio_decode_block( uint32_t *source, uint32_t block, 
                            uint32_t nibble, uint32_t *destination, 
                            uint32_t *old, uint32_t *older )
{
    static int32_t pos_xa_adpcm_table[] = {0, +60, +115, +98, +122};
    static int32_t neg_xa_adpcm_table[] = {0,   0,  -52, -55,  -60};

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
    struct xa_audio_sector_header header;
    
    /* loop over each voice */
    for (uint32_t n = 0; n < 28; n++)
    {
        

    }
}
