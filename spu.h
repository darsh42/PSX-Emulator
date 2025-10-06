#ifndef SPU_H_INCLUDED
#define SPU_H_INCLUDED


#ifdef SPU_PRIVATE

#include "trace.h"

#define TRACE_SPU(function, format, ...) \
    trace(TRACE_SPU_EN, "spu.c", function, format, __VA_ARGS__)

#include <stdint.h>

union spucnt {
    uint16_t value;

    struct {
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

union spustat {
    uint16_t value;

    struct {
        uint16_t spu_mode                                      : 6;
        uint16_t irq9_request                                  : 1;
        uint16_t data_transfer_dma_request_read_write          : 1;
        uint16_t data_transfer_dma_request_write               : 1;
        uint16_t data_transfer_dma_request_read                : 1;
        uint16_t data_transfer_busy                            : 1;
        uint16_t write_to_first_second_half_of_capture_buffers : 1;
    };
};

struct adsr {
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

union volume {
    uint16_t value;

    struct {
        uint16_t voice_vol_half: 15;
    };

    struct {
        uint32_t sweep_step      : 2;
        uint32_t sweep_shift     : 5;
        uint32_t                 : 1;
        uint32_t sweep_phase     : 1;
        uint32_t sweep_direction : 1;
        uint32_t sweep_mode      : 1;
    };
};

struct spu_voice {
    /* volume */
    uint32_t volume;

    /* adpcm */
    uint32_t current_address;
    uint16_t repeat_address;
    uint16_t start_address;
    uint16_t sample_rate;

    /* adsr */
    uint32_t adsr;
    uint16_t adsr_volume;

    /* pitch */
    uint16_t pitch_counter;

    /* samples */
    int16_t index;
    int16_t decoded[28];
    int16_t old, older;
};

struct spu {
    /* voices */
    struct spu_voice voices[24];

    /* pitch */
    uint32_t pmon;
    
    /* volume */
    uint32_t main_volume;

    /* voice flags */
    uint32_t kon;
    uint32_t koff;
    uint32_t endx;

    /* spu noise generator */
    uint32_t non;

    /* spu reverb */
    uint32_t eon;

    /* spu control and status */
    union spucnt  spucnt;
    union spustat spustat;

    /* spu memory control registers */
    uint32_t sram_current;
    uint16_t sram_address;
    uint16_t sram_control;

    /*

   1F801D84h spu   vLOUT   volume  Reverb Output Volume Left
   1F801D86h spu   vROUT   volume  Reverb Output Volume Right
   1F801DA2h spu   mBASE   base    Reverb Work Area Start Address in Sound RAM
   1F801DC0h rev00 dAPF1   disp    Reverb APF Offset 1
   1F801DC2h rev01 dAPF2   disp    Reverb APF Offset 2
   1F801DC4h rev02 vIIR    volume  Reverb Reflection Volume 1
   1F801DC6h rev03 vCOMB1  volume  Reverb Comb Volume 1
   1F801DC8h rev04 vCOMB2  volume  Reverb Comb Volume 2
   1F801DCAh rev05 vCOMB3  volume  Reverb Comb Volume 3
   1F801DCCh rev06 vCOMB4  volume  Reverb Comb Volume 4
   1F801DCEh rev07 vWALL   volume  Reverb Reflection Volume 2
   1F801DD0h rev08 vAPF1   volume  Reverb APF Volume 1
   1F801DD2h rev09 vAPF2   volume  Reverb APF Volume 2
   1F801DD4h rev0A mLSAME  src/dst Reverb Same Side Reflection Address 1 Left
   1F801DD6h rev0B mRSAME  src/dst Reverb Same Side Reflection Address 1 Right
   1F801DD8h rev0C mLCOMB1 src     Reverb Comb Address 1 Left
   1F801DDAh rev0D mRCOMB1 src     Reverb Comb Address 1 Right
   1F801DDCh rev0E mLCOMB2 src     Reverb Comb Address 2 Left
   1F801DDEh rev0F mRCOMB2 src     Reverb Comb Address 2 Right
   1F801DE0h rev10 dLSAME  src     Reverb Same Side Reflection Address 2 Left
   1F801DE2h rev11 dRSAME  src     Reverb Same Side Reflection Address 2 Right
   1F801DE4h rev12 mLDIFF  src/dst Reverb Different Side Reflect Address 1 Left
   1F801DE6h rev13 mRDIFF  src/dst Reverb Different Side Reflect Address 1 Right
   1F801DE8h rev14 mLCOMB3 src     Reverb Comb Address 3 Left
   1F801DEAh rev15 mRCOMB3 src     Reverb Comb Address 3 Right
   1F801DECh rev16 mLCOMB4 src     Reverb Comb Address 4 Left
   1F801DEEh rev17 mRCOMB4 src     Reverb Comb Address 4 Right
   1F801DF0h rev18 dLDIFF  src     Reverb Different Side Reflect Address 2 Left
   1F801DF2h rev19 dRDIFF  src     Reverb Different Side Reflect Address 2 Right
   1F801DF4h rev1A mLAPF1  src/dst Reverb APF Address 1 Left
   1F801DF6h rev1B mRAPF1  src/dst Reverb APF Address 1 Right
   1F801DF8h rev1C mLAPF2  src/dst Reverb APF Address 2 Left
   1F801DFAh rev1D mRAPF2  src/dst Reverb APF Address 2 Right
   1F801DFCh rev1E vLIN    volume  Reverb Input Volume Left
   1F801DFEh rev1F vRIN    volume  Reverb Input Volume Right  

   */

    // uint16_t spu_vLOUT     // volume  Reverb Output Volume Left
    // uint16_t spu_vROUT     // volume  Reverb Output Volume Right
    // uint16_t spu_mBASE     // base    Reverb Work Area Start Address in Sound RAM
    // uint16_t rev00_dAPF1   // disp    Reverb APF Offset 1
    // uint16_t rev01_dAPF2   // disp    Reverb APF Offset 2
    // uint16_t rev02_vIIR    // volume  Reverb Reflection Volume 1
    // uint16_t rev03_vCOMB1  // volume  Reverb Comb Volume 1
    // uint16_t rev04_vCOMB2  // volume  Reverb Comb Volume 2
    // uint16_t rev05_vCOMB3  // volume  Reverb Comb Volume 3
    // uint16_t rev06_vCOMB4  // volume  Reverb Comb Volume 4
    // uint16_t rev07_vWALL   // volume  Reverb Reflection Volume 2
    // uint16_t rev08_vAPF1   // volume  Reverb APF Volume 1
    // uint16_t rev09_vAPF2   // volume  Reverb APF Volume 2
    // uint16_t rev0A_mLSAME  // src/dst Reverb Same Side Reflection Address 1 Left
    // uint16_t rev0B_mRSAME  // src/dst Reverb Same Side Reflection Address 1 Right
    // uint16_t rev0C_mLCOMB1 // src     Reverb Comb Address 1 Left
    // uint16_t rev0D_mRCOMB1 // src     Reverb Comb Address 1 Right
    // uint16_t rev0E_mLCOMB2 // src     Reverb Comb Address 2 Left
    // uint16_t rev0F_mRCOMB2 // src     Reverb Comb Address 2 Right
    // uint16_t rev10_dLSAME  // src     Reverb Same Side Reflection Address 2 Left
    // uint16_t rev11_dRSAME  // src     Reverb Same Side Reflection Address 2 Right
    // uint16_t rev12_mLDIFF  // src/dst Reverb Different Side Reflect Address 1 Left
    // uint16_t rev13_mRDIFF  // src/dst Reverb Different Side Reflect Address 1 Right
    // uint16_t rev14_mLCOMB3 // src     Reverb Comb Address 3 Left
    // uint16_t rev15_mRCOMB3 // src     Reverb Comb Address 3 Right
    // uint16_t rev16_mLCOMB4 // src     Reverb Comb Address 4 Left
    // uint16_t rev17_mRCOMB4 // src     Reverb Comb Address 4 Right
    // uint16_t rev18_dLDIFF  // src     Reverb Different Side Reflect Address 2 Left
    // uint16_t rev19_dRDIFF  // src     Reverb Different Side Reflect Address 2 Right
    // uint16_t rev1A_mLAPF1  // src/dst Reverb APF Address 1 Left
    // uint16_t rev1B_mRAPF1  // src/dst Reverb APF Address 1 Right
    // uint16_t rev1C_mLAPF2  // src/dst Reverb APF Address 2 Left
    // uint16_t rev1D_mRAPF2  // src/dst Reverb APF Address 2 Right
    // uint16_t rev1E_vLIN    // volume  Reverb Input Volume Left
    // uint16_t rev1F_vRIN    // volume  Reverb Input Volume Right  
};

#endif // SPU_PRIVATE

#ifdef SPU_SECTORS
struct spu_adpcm_sector
{
    uint8_t shift : 4;
    uint8_t filter: 3;
    uint8_t       : 1;

    uint8_t loop_start : 1;
    uint8_t loop_repeat: 1;
    uint8_t loop_end   : 1;
    uint8_t            : 5;
    
    /* layout:                            *
     *  lsb (sample  1), msb (sample  2), *
     *  lsb (sample  3), msb (sample  4), *
     *  ...                               *
     *  lsb (sample 27), msb (sample 28), */
    uint8_t data[14];
};
#endif

extern uint32_t  read_spu( uint32_t address );
extern void     write_spu( uint32_t address, uint32_t data );
extern uint32_t  read_spu_voice( uint32_t address );
extern void     write_spu_voice( uint32_t address, uint32_t data );

extern void task_spu( void );

#endif // SPU_H_INCLUDED
