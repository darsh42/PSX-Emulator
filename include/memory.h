#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "error.h"
#include "common.h"

#define print_memory_error(func, format, ...) print_error("cpu.c", func, format, __VA_ARGS__)

// CPU address space
typedef union MEM_MAIN                  {uint8_t mem[0X200000];}   MEM_MAIN;                  // 2048K
typedef union MEM_EXPANSION_1           {uint8_t mem[0X800000];}   MEM_EXPANSION_1;           // 8192K
typedef union MEM_SCRATCH_PAD           {uint8_t mem[0X400];}      MEM_SCRATCH_PAD;           // 1K
typedef union MEM_IO_PORTS              {
                                            uint8_t mem[0X2000];
                                            struct {
                                                // Memory control
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

                                                // Peripherals
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

                                                // Memory control 2
                                                uint32_t ram_size;

                                                uint8_t _pad_mem_cont_2_interrupt[12];
                                                
                                                // Interrupt control
                                                uint32_t i_stat;
                                                uint32_t i_mask;
                                                
                                                uint8_t _pad_interrupt_dma[8];
                                                
                                                // DMA
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

                                                // Timers (AKA root counters)
                                                uint8_t timer_0_dot_clock[16];
                                                uint8_t timer_1_horizontal_retrace[16];
                                                uint8_t timer_2_8th_system_clock[16];

                                                uint8_t _pad_timers_cdrom[1744];

                                                // TODO: CDROM registers use of bits needs attention
                                                uint8_t cd_index_status;
                                                uint8_t cd_response_fifo;
                                                uint8_t cd_data_fifo;
                                                uint8_t cd_interrupt;
                                                // uint8_t cd_index_status[1];                 //  CD Index/Status Register (Bit0-1 R/W, Bit2-7 Read Only)
                                                // uint8_t cd_response_fifo[1];                //  CD Response Fifo (R) (usually with Index1)
                                                // uint8_t cd_data_fifo[2];                    //  CD Data Fifo - 8bit/16bit (R) (usually with Index0..1)
                                                // uint8_t cd_interrupt_enable_read[1];        //  CD Interrupt Enable Register (R)
                                                // uint8_t cd_interrupt_flag[1];               //  CD Interrupt Flag Register (R/W)
                                                // uint8_t cd_interrupt_enable_read_mirror[1]; //  CD Interrupt Enable Register (R) (Mirror)
                                                // uint8_t cd_interrupt_flag_mirror[1];        //  CD Interrupt Flag Register (R/W) (Mirror)
                                                // uint8_t cd_command[1];                      //  CD Command Register (W)
                                                // uint8_t cd_parameter_fifo[1];               //  CD Parameter Fifo (W)
                                                // uint8_t cd_request[1];                      //  CD Request Register (W)
                                                // uint8_t cd_unused[1];                       //  Unknown/unused
                                                // uint8_t cd_interrupt_enable_write[1];       //  CD Interrupt Enable Register (W)
                                                // uint8_t cd_interrupt_flag_2[1];             //  CD Interrupt Flag Register (R/W)
                                                // uint8_t cd_unused2[1];                      //  Unknown/unused
                                                // uint8_t cd_volume_Lcd_Lspu[1];              //  CD Audio Volume for Left-CD-Out to Left-SPU-Input (W)
                                                // uint8_t cd_volume_Lcd_Rspu[1];              //  CD Audio Volume for Left-CD-Out to Right-SPU-Input (W)
                                                // uint8_t cd_volume_Rcd_Rspu[1];              //  CD Audio Volume for Right-CD-Out to Right-SPU-Input (W)
                                                // uint8_t cd_volume_Rcd_Lspu[1];              //  CD Audio Volume for Right-CD-Out to Left-SPU-Input (W)
                                                // uint8_t cd_volume[1];                       //  CD Audio Volume Apply Changes (by writing bit5=1)
                                                //// Verified upto this point

                                                uint8_t _pad_cdrom_gpu[12];

                                                // GPU Registers
                                                uint32_t gp0_and_gpu_read; // GP0 (Write) GPUREAD (Read) responses to GP0(C0h) and GP1(10h) commands
                                                uint32_t gp1_and_gpu_stat; // GP1 (Write) GPUSTAT (Read) GPU Status Register

                                                uint8_t _pad_gpu_mdec[8];

                                                // MDEC Registers
                                                uint32_t mdec_command_parameter;   // MDEC Command/Parameter Register (W)
                                                uint32_t mdec_data_response;       // MDEC Data/Response Register (R)
                                                uint32_t mdec_control_reset;       // MDEC Control/Reset Register (W)
                                                uint32_t mdec_status;              // MDEC Status Register (R)

                                                // TODO: SPU Voice 0..23 Registers
                                                // 
                                                //   1F801C00h+N*10h 4   Voice 0..23 Volume Left/Right
                                                //   1F801C04h+N*10h 2   Voice 0..23 ADPCM Sample Rate
                                                //   1F801C06h+N*10h 2   Voice 0..23 ADPCM Start Address
                                                //   1F801C08h+N*10h 4   Voice 0..23 ADSR Attack/Decay/Sustain/Release
                                                //   1F801C0Ch+N*10h 2   Voice 0..23 ADSR Current Volume
                                                //   1F801C0Eh+N*10h 2   Voice 0..23 ADPCM Repeat Address
                                                // 
                                                // SPU Control Registers
                                                // 
                                                //   1F801D80h 4  Main Volume Left/Right
                                                //   1F801D84h 4  Reverb Output Volume Left/Right
                                                //   1F801D88h 4  Voice 0..23 Key ON (Start Attack/Decay/Sustain) (W)
                                                //   1F801D8Ch 4  Voice 0..23 Key OFF (Start Release) (W)
                                                //   1F801D90h 4  Voice 0..23 Channel FM (pitch lfo) mode (R/W)
                                                //   1F801D94h 4  Voice 0..23 Channel Noise mode (R/W)
                                                //   1F801D98h 4  Voice 0..23 Channel Reverb mode (R/W)
                                                //   1F801D9Ch 4  Voice 0..23 Channel ON/OFF (status) (R)
                                                //   1F801DA0h 2  Unknown? (R) or (W)
                                                //   1F801DA2h 2  Sound RAM Reverb Work Area Start Address
                                                //   1F801DA4h 2  Sound RAM IRQ Address
                                                //   1F801DA6h 2  Sound RAM Data Transfer Address
                                                //   1F801DA8h 2  Sound RAM Data Transfer Fifo
                                                //   1F801DAAh 2  SPU Control Register (SPUCNT)
                                                //   1F801DACh 2  Sound RAM Data Transfer Control
                                                //   1F801DAEh 2  SPU Status Register (SPUSTAT) (R)
                                                //   1F801DB0h 4  CD Volume Left/Right
                                                //   1F801DB4h 4  Extern Volume Left/Right
                                                //   1F801DB8h 4  Current Main Volume Left/Right
                                                //   1F801DBCh 4  Unknown? (R/W)
                                                // 
                                                // SPU Reverb Configuration Area
                                                // 
                                                //   1F801DC0h 2  dAPF1  Reverb APF Offset 1
                                                //   1F801DC2h 2  dAPF2  Reverb APF Offset 2
                                                //   1F801DC4h 2  vIIR   Reverb Reflection Volume 1
                                                //   1F801DC6h 2  vCOMB1 Reverb Comb Volume 1
                                                //   1F801DC8h 2  vCOMB2 Reverb Comb Volume 2
                                                //   1F801DCAh 2  vCOMB3 Reverb Comb Volume 3
                                                //   1F801DCCh 2  vCOMB4 Reverb Comb Volume 4
                                                //   1F801DCEh 2  vWALL  Reverb Reflection Volume 2
                                                //   1F801DD0h 2  vAPF1  Reverb APF Volume 1
                                                //   1F801DD2h 2  vAPF2  Reverb APF Volume 2
                                                //   1F801DD4h 4  mSAME  Reverb Same Side Reflection Address 1 Left/Right
                                                //   1F801DD8h 4  mCOMB1 Reverb Comb Address 1 Left/Right
                                                //   1F801DDCh 4  mCOMB2 Reverb Comb Address 2 Left/Right
                                                //   1F801DE0h 4  dSAME  Reverb Same Side Reflection Address 2 Left/Right
                                                //   1F801DE4h 4  mDIFF  Reverb Different Side Reflection Address 1 Left/Right
                                                //   1F801DE8h 4  mCOMB3 Reverb Comb Address 3 Left/Right
                                                //   1F801DECh 4  mCOMB4 Reverb Comb Address 4 Left/Right
                                                //   1F801DF0h 4  dDIFF  Reverb Different Side Reflection Address 2 Left/Right
                                                //   1F801DF4h 4  mAPF1  Reverb APF Address 1 Left/Right
                                                //   1F801DF8h 4  mAPF2  Reverb APF Address 2 Left/Right
                                                //   1F801DFCh 4  vIN    Reverb Input Volume Left/Right
                                                // 
                                                // SPU Internal Registers
                                                // 
                                                //   1F801E00h+N*04h  4 Voice 0..23 Current Volume Left/Right
                                                //   1F801E60h      20h Unknown? (R/W)
                                                //   1F801E80h     180h Unknown? (Read: FFh-filled) (Unused or Write only?)

                                            };
                                        } MEM_IO_PORTS;                                       // 8K
typedef union MEM_EXPANSION_2           {uint8_t mem[0X2000];}     MEM_EXPANSION_2;           // 8K
typedef union MEM_EXPANSION_3           {uint8_t mem[0X200000];}   MEM_EXPANSION_3;           // 2048K
typedef union MEM_BIOS                  {uint8_t mem[0X80000];}    MEM_BIOS;                  // 512K
typedef union MEM_KSEG2                 {
                                            uint8_t mem[0X40000000];
                                            struct {
                                                uint8_t unused[0X3FFE0130];
                                                uint32_t cache_control;
                                            };
                                        } MEM_KSEG2;

// NON-CPU address space
typedef union MEM_VRAM                  {uint8_t mem[0X100000];} MEM_VRAM;                  // 1024K used for frame buffers, textures and CLUTs
typedef union MEM_SOUND                 {uint8_t mem[0X80000];}  MEM_SOUND;                 // 512K
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

// cpu functions
extern void cpu_exception(enum EXCEPTION_CAUSE cause);
extern bool cop0_SR_Isc(void);
extern uint8_t *GPUSTAT(void);
extern uint8_t *GPUREAD(void);
extern uint8_t *GP0(void);
extern uint8_t *GP1(void);

#endif
