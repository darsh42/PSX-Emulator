#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "error.h"
#include "common.h"
#include "cpu.h"
#include "gpu.h"

#define print_memory_error(func, format, ...) print_error("cpu.c", func, format, __VA_ARGS__)

// CPU address space
typedef union MEM_MAIN                  {                                                     // 2048K
                                            uint8_t mem[0X200000];
                                            struct {
                                                // BIOS RAM (reserved first 64Kbytes
                                                uint32_t _garbage[4];
                                                uint32_t _unused_1[12];
                                                uint32_t cop0_debug_break_vector[8];
                                                uint32_t ram_size;
                                                uint32_t _unknown_1;
                                                uint32_t _unknown_2;
                                                uint32_t _unused_2[5];
                                                uint32_t exception_vector[4];
                                                uint32_t _unused_3[4];
                                                uint32_t function_vector_Ann[4];
                                                uint32_t function_vector_Bnn[4];
                                                uint32_t function_vector_Cnn[4];
                                                uint32_t _unused_4[12];
                                                uint32_t table_of_tables[22];
                                                uint32_t _unused_5[10];
                                                uint32_t cli_argument[32];
                                                uint32_t jumptable_Ann[192];
                                            };
                                        }   MEM_MAIN;
typedef union MEM_EXPANSION_1           {uint8_t mem[0X800000];}   MEM_EXPANSION_1;           // 8192K
typedef union MEM_SCRATCH_PAD           {uint8_t mem[0X400];}      MEM_SCRATCH_PAD;           // 1K
typedef union MEM_IO_PORTS              {
                                            uint8_t mem[0X2000];
                                            struct {
                                                // Memory control 1             : 1F801000 - 1F801020
                                                uint32_t expansion_1_base_address;
                                                uint32_t expansion_2_base_address;
                                                uint32_t expansion_1;
                                                uint32_t expansion_3;
                                                uint32_t bios_rom;
                                                uint32_t spu_delay;
                                                uint32_t cdrom_delay;
                                                uint32_t expansion_2;
                                                uint32_t com_delay;
                                                
                                                uint8_t _pad_mem_cont_periph[28];

                                                // Peripherals                  : 1F801040 - 1F80105E
                                                uint32_t joy_data;
                                                uint32_t joy_stat;
                                                uint16_t joy_mode;
                                                uint16_t joy_ctrl;
                                                uint16_t _pad_joy;
                                                uint16_t joy_baud;
                                                uint32_t sio_data;
                                                uint32_t sio_stat;
                                                uint16_t sio_mode;
                                                uint16_t sio_ctrl;
                                                uint16_t sio_misc;
                                                uint16_t sio_baud;

                                                // Memory control 2             :  1F801060
                                                uint32_t ram_size;

                                                uint8_t _pad_mem_cont_2_interrupt[12];
                                                
                                                // Interrupt control            : 1F801070 - 1F801074
                                                uint32_t i_stat;
                                                uint32_t i_mask;
                                                
                                                uint8_t _pad_interrupt_dma[8];
                                                
                                                // DMA                          : 1F801080 - 1F8010FC
                                                uint32_t dma0_mdec_in[4];
                                                uint32_t dma1_mdec_out[4];
                                                uint32_t dma2_gpu[4];
                                                uint32_t dma3_cdrom[4];
                                                uint32_t dma4_spu[4];
                                                uint32_t dma5_pio[4];
                                                uint32_t dma6_otc[4];
                                                uint32_t dpcr;
                                                uint32_t dicr;

                                                uint8_t _pad_dma_timer[8];

                                                // Timers (AKA root counters)   : 1F801100 - 1F801130
                                                uint8_t timer_0_dot_clock[16];
                                                uint8_t timer_1_horizontal_retrace[16];
                                                uint8_t timer_2_8th_system_clock[16];

                                                uint8_t _pad_timers_cdrom[1744];

                                                // CDROM Registers              : 1F801800 - 1F801803
                                                uint8_t cd_index_status;
                                                uint8_t cd_response_fifo;
                                                uint8_t cd_data_fifo;
                                                uint8_t cd_interrupt;

                                                uint8_t _pad_cdrom_gpu[12];

                                                // GPU Registers                : 1F801810 - 1F801814
                                                uint32_t gp0_and_gpu_read; // GP0 (Write) GPUREAD (Read) responses to GP0(C0h) and GP1(10h) commands
                                                uint32_t gp1_and_gpu_stat; // GP1 (Write) GPUSTAT (Read) GPU Status Register

                                                uint8_t _pad_gpu_mdec[8];

                                                // MDEC Registers               : 1F801820 - 1F801824
                                                uint32_t mdec_command_parameter;   // MDEC Command/Parameter Register (W)
                                                uint32_t mdec_data_response;       // MDEC Data/Response Register (R)
                                                uint32_t mdec_control_reset;       // MDEC Control/Reset Register (W)
                                                uint32_t mdec_status;              // MDEC Status Register (R)
                                                
                                                uint8_t _pad_mdec_spu[976];

                                                // SPU Registers                : 1F801C00 - 1F801C0E
                                                
                                                // SPU Voice Registers 
                                                uint32_t voice_0_volume;
                                                uint16_t voice_0_adpcm_sample_rate;
                                                uint16_t voice_0_adpcm_start_address;
                                                uint32_t voice_0_adsr_attack_decay_sustain_release;
                                                uint16_t voice_0_adsr_current_volume;
                                                
                                                uint32_t voice_1_volume;
                                                uint16_t voice_1_adpcm_sample_rate;
                                                uint16_t voice_1_adpcm_start_address;
                                                uint32_t voice_1_adsr_attack_decay_sustain_release;
                                                uint16_t voice_1_adsr_current_volume;
                                                uint16_t voice_1_adsr_repeat_address;
                                                
                                                uint32_t voice_2_volume;
                                                uint16_t voice_2_adpcm_sample_rate;
                                                uint16_t voice_2_adpcm_start_address;
                                                uint32_t voice_2_adsr_attack_decay_sustain_release;
                                                uint16_t voice_2_adsr_current_volume;
                                                uint16_t voice_2_adsr_repeat_address;
                                                
                                                uint32_t voice_3_volume;
                                                uint16_t voice_3_adpcm_sample_rate;
                                                uint16_t voice_3_adpcm_start_address;
                                                uint32_t voice_3_adsr_attack_decay_sustain_release;
                                                uint16_t voice_3_adsr_current_volume;
                                                uint16_t voice_3_adsr_repeat_address;
                                                
                                                uint32_t voice_4_volume;
                                                uint16_t voice_4_adpcm_sample_rate;
                                                uint16_t voice_4_adpcm_start_address;
                                                uint32_t voice_4_adsr_attack_decay_sustain_release;
                                                uint16_t voice_4_adsr_current_volume;
                                                uint16_t voice_4_adsr_repeat_address;
                                                
                                                uint32_t voice_5_volume;
                                                uint16_t voice_5_adpcm_sample_rate;
                                                uint16_t voice_5_adpcm_start_address;
                                                uint32_t voice_5_adsr_attack_decay_sustain_release;
                                                uint16_t voice_5_adsr_current_volume;
                                                uint16_t voice_5_adsr_repeat_address;
                                                
                                                uint32_t voice_6_volume;
                                                uint16_t voice_6_adpcm_sample_rate;
                                                uint16_t voice_6_adpcm_start_address;
                                                uint32_t voice_6_adsr_attack_decay_sustain_release;
                                                uint16_t voice_6_adsr_current_volume;
                                                uint16_t voice_6_adsr_repeat_address;
                                                
                                                uint32_t voice_7_volume;
                                                uint16_t voice_7_adpcm_sample_rate;
                                                uint16_t voice_7_adpcm_start_address;
                                                uint32_t voice_7_adsr_attack_decay_sustain_release;
                                                uint16_t voice_7_adsr_current_volume;
                                                uint16_t voice_7_adsr_repeat_address;
                                                
                                                uint32_t voice_8_volume;
                                                uint16_t voice_8_adpcm_sample_rate;
                                                uint16_t voice_8_adpcm_start_address;
                                                uint32_t voice_8_adsr_attack_decay_sustain_release;
                                                uint16_t voice_8_adsr_current_volume;
                                                uint16_t voice_8_adsr_repeat_address;
                                                
                                                uint32_t voice_9_volume;
                                                uint16_t voice_9_adpcm_sample_rate;
                                                uint16_t voice_9_adpcm_start_address;
                                                uint32_t voice_9_adsr_attack_decay_sustain_release;
                                                uint16_t voice_9_adsr_current_volume;
                                                uint16_t voice_9_adsr_repeat_address;
                                                
                                                uint32_t voice_10_volume;
                                                uint16_t voice_10_adpcm_sample_rate;
                                                uint16_t voice_10_adpcm_start_address;
                                                uint32_t voice_10_adsr_attack_decay_sustain_release;
                                                uint16_t voice_10_adsr_current_volume;
                                                uint16_t voice_10_adsr_repeat_address;
                                                
                                                uint32_t voice_11_volume;
                                                uint16_t voice_11_adpcm_sample_rate;
                                                uint16_t voice_11_adpcm_start_address;
                                                uint32_t voice_11_adsr_attack_decay_sustain_release;
                                                uint16_t voice_11_adsr_current_volume;
                                                uint16_t voice_11_adsr_repeat_address;
                                                
                                                uint32_t voice_12_volume;
                                                uint16_t voice_12_adpcm_sample_rate;
                                                uint16_t voice_12_adpcm_start_address;
                                                uint32_t voice_12_adsr_attack_decay_sustain_release;
                                                uint16_t voice_12_adsr_current_volume;
                                                uint16_t voice_12_adsr_repeat_address;
                                                
                                                uint32_t voice_13_volume;
                                                uint16_t voice_13_adpcm_sample_rate;
                                                uint16_t voice_13_adpcm_start_address;
                                                uint32_t voice_13_adsr_attack_decay_sustain_release;
                                                uint16_t voice_13_adsr_current_volume;
                                                uint16_t voice_13_adsr_repeat_address;
                                                
                                                uint32_t voice_14_volume;
                                                uint16_t voice_14_adpcm_sample_rate;
                                                uint16_t voice_14_adpcm_start_address;
                                                uint32_t voice_14_adsr_attack_decay_sustain_release;
                                                uint16_t voice_14_adsr_current_volume;
                                                uint16_t voice_14_adsr_repeat_address;
                                                
                                                uint32_t voice_15_volume;
                                                uint16_t voice_15_adpcm_sample_rate;
                                                uint16_t voice_15_adpcm_start_address;
                                                uint32_t voice_15_adsr_attack_decay_sustain_release;
                                                uint16_t voice_15_adsr_current_volume;
                                                uint16_t voice_15_adsr_repeat_address;
                                                
                                                uint32_t voice_16_volume;
                                                uint16_t voice_16_adpcm_sample_rate;
                                                uint16_t voice_16_adpcm_start_address;
                                                uint32_t voice_16_adsr_attack_decay_sustain_release;
                                                uint16_t voice_16_adsr_current_volume;
                                                uint16_t voice_16_adsr_repeat_address;
                                                
                                                uint32_t voice_17_volume;
                                                uint16_t voice_17_adpcm_sample_rate;
                                                uint16_t voice_17_adpcm_start_address;
                                                uint32_t voice_17_adsr_attack_decay_sustain_release;
                                                uint16_t voice_17_adsr_current_volume;
                                                uint16_t voice_17_adsr_repeat_address;
                                                
                                                uint32_t voice_18_volume;
                                                uint16_t voice_18_adpcm_sample_rate;
                                                uint16_t voice_18_adpcm_start_address;
                                                uint32_t voice_18_adsr_attack_decay_sustain_release;
                                                uint16_t voice_18_adsr_current_volume;
                                                uint16_t voice_18_adsr_repeat_address;
                                                
                                                uint32_t voice_19_volume;
                                                uint16_t voice_19_adpcm_sample_rate;
                                                uint16_t voice_19_adpcm_start_address;
                                                uint32_t voice_19_adsr_attack_decay_sustain_release;
                                                uint16_t voice_19_adsr_current_volume;
                                                uint16_t voice_19_adsr_repeat_address;
                                                
                                                uint32_t voice_20_volume;
                                                uint16_t voice_20_adpcm_sample_rate;
                                                uint16_t voice_20_adpcm_start_address;
                                                uint32_t voice_20_adsr_attack_decay_sustain_release;
                                                uint16_t voice_20_adsr_current_volume;
                                                uint16_t voice_20_adsr_repeat_address;
                                                
                                                uint32_t voice_21_volume;
                                                uint16_t voice_21_adpcm_sample_rate;
                                                uint16_t voice_21_adpcm_start_address;
                                                uint32_t voice_21_adsr_attack_decay_sustain_release;
                                                uint16_t voice_21_adsr_current_volume;
                                                uint16_t voice_21_adsr_repeat_address;
                                                
                                                uint32_t voice_22_volume;
                                                uint16_t voice_22_adpcm_sample_rate;
                                                uint16_t voice_22_adpcm_start_address;
                                                uint32_t voice_22_adsr_attack_decay_sustain_release;
                                                uint16_t voice_22_adsr_current_volume;
                                                uint16_t voice_22_adsr_repeat_address;
                                                
                                                uint32_t voice_23_volume;
                                                uint16_t voice_23_adpcm_sample_rate;
                                                uint16_t voice_23_adpcm_start_address;
                                                uint32_t voice_23_adsr_attack_decay_sustain_release;
                                                uint16_t voice_23_adsr_current_volume;
                                                uint16_t voice_23_adsr_repeat_address;
                                                
                                                // SPU Control Registers
                                                uint32_t main_volume;
                                                uint32_t reverbe_output_volume;
                                                uint32_t voice_0_23_key_on;
                                                uint32_t voice_0_23_key_off;
                                                uint32_t voice_0_23_channel_fm;
                                                uint32_t voice_0_23_channel_noise_mode;
                                                uint32_t voice_0_23_channel_reverb_mode;
                                                uint32_t voice_0_23_channle_on_off;
                                                uint16_t _pad_spu_1;
                                                uint16_t sound_ram_reverb_work_area_start_address;
                                                uint16_t sound_ram_data_transfer_address;
                                                uint16_t sound_ram_data_transfer_fifo;
                                                uint16_t spu_control_register;
                                                uint32_t cd_volume;
                                                uint32_t extern_volume;
                                                uint32_t current_main_volume;
                                                uint32_t _pad_spu_2;

                                                // SPU Reverb Configuration Area
                                                uint16_t dAPF1_reverb_apf_offset_1;
                                                uint16_t dAPF2_reverb_apf_offset_2;
                                                uint16_t vIIR_reverb_reflection_volume_1;
                                                uint16_t vCOMB1_reverb_comb_volume_1;
                                                uint16_t vCOMB2_reverb_comb_volume_2;
                                                uint16_t vCOMB3_reverb_comb_volume_3;
                                                uint16_t vCOMB4_reverb_comb_volume_4;
                                                uint16_t vWALL_reverb_reflection_volume_2;
                                                uint16_t vAPF1_reverb_apf_volume_1;
                                                uint16_t vAPF2_reverb_apf_volume_2;
                                                uint32_t mSAME__reverb_same_side_reflection_address_1;
                                                uint32_t mCOMB1_reverb_comb_address_1;
                                                uint32_t mCOMB2_reverb_comb_address_2;
                                                uint32_t dSAME_reverb_same_side_reflection_address_2;
                                                uint32_t mDIFF_reverb_different_side_reflection_address_1;
                                                uint32_t mCOMB3_reverb_comb_address_3;
                                                uint32_t mCOMB4_reverb_comb_address_4;
                                                uint32_t dDIFF_reverb_different_side_reflection_address_2;
                                                uint32_t mAPF1_reverb_apf_address_1;
                                                uint32_t mAPF2_reverb_apf_address_2;
                                                uint32_t vIN_reverb_input_volume;
                                                
                                                // SPU Internal Registers
                                                uint32_t voice_0_current_volume;
                                                uint32_t voice_1_current_volume;
                                                uint32_t voice_2_current_volume;
                                                uint32_t voice_3_current_volume;
                                                uint32_t voice_4_current_volume;
                                                uint32_t voice_5_current_volume;
                                                uint32_t voice_6_current_volume;
                                                uint32_t voice_7_current_volume;
                                                uint32_t voice_8_current_volume;
                                                uint32_t voice_9_current_volume;
                                                uint32_t voice_10_current_volume;
                                                uint32_t voice_11_current_volume;
                                                uint32_t voice_12_current_volume;
                                                uint32_t voice_13_current_volume;
                                                uint32_t voice_14_current_volume;
                                                uint32_t voice_15_current_volume;
                                                uint32_t voice_16_current_volume;
                                                uint32_t voice_17_current_volume;
                                                uint32_t voice_18_current_volume;
                                                uint32_t voice_19_current_volume;
                                                uint32_t voice_20_current_volume;
                                                uint32_t voice_21_current_volume;
                                                uint32_t voice_22_current_volume;
                                                uint32_t voice_23_current_volume;
                                            };
                                        } MEM_IO_PORTS;                                       // 8K
typedef union MEM_EXPANSION_2           {uint8_t mem[0X2000];}     MEM_EXPANSION_2;           // 8K
typedef union MEM_EXPANSION_3           {uint8_t mem[0X200000];}   MEM_EXPANSION_3;           // 2048K
typedef union MEM_BIOS                  {                                                     // 512K
                                            uint8_t mem[0X80000];
                                            struct {
                                                // Kernal Part 1  BFC00000 - BFC10000
                                                uint32_t _pad_start_kernel_date[0x100];
                                                uint32_t kernel_date;
                                                uint32_t console_type;
                                                uint32_t kernel_maker;
                                                uint32_t gui_version;
                                                // Kernal Part 2  BFC10000 - BFC18000
                                                // Intro/Bootmenu BFC10000 - BFC18000
                                                // Character Sets BFC64000 - BFC80000
                                            };
                                        }    MEM_BIOS;
typedef union MEM_KSEG2                 {
                                            uint8_t mem[0X40000000];
                                            struct {
                                                uint8_t unused[0X3FFE0130];
                                                uint32_t cache_control;
                                            };
                                        } MEM_KSEG2;

// NON-CPU address space
typedef union MEM_VRAM                  {uint8_t mem[0X100000];} MEM_VRAM;                  // 1024K used for frame buffers, textures and CLUTs
typedef union MEM_SOUND                 {
                                            uint8_t mem[0X80000];
                                            struct {
                                                uint32_t cd_audio_left[256];
                                                uint32_t cd_audio_right[256];
                                                uint32_t voice_mono_1[256];
                                                uint32_t voice_mono_2[256];
                                            };
                                        }  MEM_SOUND;                 // 512K
typedef union MEM_CDROM_CONTROLLER_RAM  {uint8_t mem[0X200];}    MEM_CDROM_CONTROLLER_RAM;  // 0.5K
typedef union MEM_CDROM_CONTROLLER_ROM  {uint8_t mem[0X4200];}   MEM_CDROM_CONTROLLER_ROM;  // 16.5K
typedef union MEM_CDROM_BUFFER          {uint8_t mem[0X8400];}   MEM_CDROM_BUFFER;          // 32K
typedef union MEM_EXTERNAL_MEMORY_CARDS {uint8_t mem[0X20000];}  MEM_EXTERNAL_MEMORY_CARDS; // 128K

struct MEMORY {
    // CPU BUS
    MEM_MAIN MAIN;
    MEM_EXPANSION_1 EXPANSION_1;
    MEM_SCRATCH_PAD SCRATCH_PAD;
    MEM_IO_PORTS IO_PORTS;
    MEM_EXPANSION_2 EXPANSION_2;
    MEM_EXPANSION_3 EXPANSION_3;
    MEM_BIOS BIOS;
    MEM_KSEG2 KSEG2;

    // NON-CPU BUS
    MEM_VRAM  VRAM;
    MEM_SOUND SOUND;
    MEM_CDROM_CONTROLLER_RAM CDROM_CONTROLLER_RAM;
    MEM_CDROM_CONTROLLER_RAM CDROM_CONTROLLER_ROM;
    MEM_CDROM_BUFFER CDROM_BUFFER;
    MEM_EXTERNAL_MEMORY_CARDS EXTERNAL_MEMORY_CARDS;

    uint32_t address_accessed; // used for debugging
};


// external API function
extern struct MEMORY *get_memory( void );
extern PSX_ERROR memory_load_bios(const char *filebios);
extern uint8_t *memory_VRAM_pointer(void);
extern uint8_t *memory_pointer(uint32_t address);

// cpu address space memory functions
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_8bit(uint32_t address, uint32_t data);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_16bit(uint32_t address, uint32_t data);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_32bit(uint32_t address, uint32_t data);

// gpu address space memory functions 
extern void memory_gpu_load_4bit(uint32_t address, uint8_t *data);
extern void memory_gpu_load_8bit(uint32_t address, uint32_t *data);
extern void memory_gpu_load_16bit(uint32_t address, uint32_t *data);
extern void memory_gpu_load_24bit(uint32_t address, uint32_t *data);
extern void memory_gpu_store_4bit(uint32_t address, uint8_t data);
extern void memory_gpu_store_8bit(uint32_t address, uint32_t data);
extern void memory_gpu_store_16bit(uint32_t address, uint32_t data);
extern void memory_gpu_store_24bit(uint32_t address, uint32_t data);

#endif
