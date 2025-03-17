#ifndef SPU_H_INCLUDED
#define SPU_H_INCLUDED

#ifdef SPU_PRIVATE

#include <stdint.h>

union SPUCNT
{
    uint16_t value;

    struct 
    {
        uint16_t cd_audio_enable         : 1;
        uint16_t external_audio_enable   : 1;
        uint16_t cd_audio_reverb         : 1;
        uint16_t external_audio_reverb   : 1;
        uint16_t sound_ram_transfer_mode : 2;
        uint16_t irq9_enable             : 1;
        uint16_t reverb_master           : 1;
        uint16_t noise_frequency_step    : 2;
        uint16_t noise_frequency_shift   : 4;
        uint16_t mute_spu                : 1;
        uint16_t spu_enable              : 1;
    };
};

union SPUSTAT
{
    uint16_t value;

    struct
    {
        uint16_t spu_mode                                      : 6;
        uint16_t irq9_request                                  : 1;
        uint16_t data_transfer_dma_request_read_write          : 1;
        uint16_t data_transfer_dma_request_write               : 1;
        uint16_t data_transfer_dma_request_read                : 1;
        uint16_t data_transfer_busy                            : 1;
        uint16_t write_to_first_second_half_of_capture_buffers : 1;
    };
};

struct ADSR
{
    uint32_t sustain_level     : 4;
    uint32_t decay_shift       : 4;
    uint32_t attack_step       : 2;
    uint32_t attack_shift      : 5;
    uint32_t attack_mode       : 1;

    uint32_t release_shift     : 5;
    uint32_t release_mode      : 1;
    uint32_t sustain_step      : 2;
    uint32_t sustain_shift     : 5;
    uint32_t                   : 1;
    uint32_t sustain_direction : 1;
    uint32_t sustain_mode      : 1;
};

union VOLUME
{
    uint16_t value;

    struct
    {
        uint16_t voice_vol_half: 15;
    };

    struct
    {
        uint32_t sweep_step      : 2;
        uint32_t sweep_shift     : 5;
        uint32_t                 : 1;
        uint32_t sweep_phase     : 1;
        uint32_t sweep_direction : 1;
        uint32_t sweep_mode      : 1;
    };
};

struct spu
{
    /* adpcm samples */
    uint16_t adpcm_start_address[24];
    uint16_t adpcm_repeat_address[24];

    /* adpcm pitch */
    uint16_t adpcm_sample_rate[24];
    uint32_t pitch_modulation_enable;

    /* volume and adsr generator */
    uint32_t adsr[24];
    uint32_t main_volume;
    uint32_t voice_volume[24];
    uint16_t adsr_current_volume[24];

    /* voice flags */
    uint32_t kon;
    uint32_t koff;
    uint32_t endx;

    /* spu noise generator */
    uint32_t non;

    /* spu reverb */
    uint32_t eon;

    /* spu control and status */
    union SPUCNT  spucnt;
    union SPUSTAT spustat;
    
};

#endif // SPU_PRIVATE

extern uint32_t  read_spu( uint32_t address );
extern void     write_spu( uint32_t address, uint32_t data );

#endif // SPU_H_INCLUDED
