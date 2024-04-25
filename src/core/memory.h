#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "error.h"
#include "common.h"

#define print_memory_error(func) print_error("memory.c", funct)

typedef struct MEM_MAIN                  {uint8_t mem[0X200000];} MEM_MAIN;                  // 2048K
typedef struct MEM_EXPANSION_1           {uint8_t mem[0X800000];} MEM_EXPANSION_1;           // 8192K
typedef struct MEM_SCRATCH_PAD           {uint8_t mem[0X400];}    MEM_SCRATCH_PAD;           // 1K
typedef struct MEM_IO_PORTS              {uint8_t mem[0X2000];}   MEM_IO_PORTS;              // 8K
typedef struct MEM_EXPANSION_2           {uint8_t mem[0X2000];}   MEM_EXPANSION_2;           // 8K
typedef struct MEM_EXPANSION_3           {uint8_t mem[0X800000];} MEM_EXPANSION_3;           // 2048K
typedef struct MEM_BIOS                  {uint8_t mem[0X100000];} MEM_BIOS;                  // 512K
typedef struct MEM_VRAM                  {uint8_t mem[0X200000];} MEM_VRAM;                  // 1024K
typedef struct MEM_SOUND                 {uint8_t mem[0X100000];} MEM_SOUND;                 // 512K
typedef struct MEM_CDROM_CONTROLLER_RAM  {uint8_t mem[0X200];}    MEM_CDROM_CONTROLLER_RAM;  // 0.5K
typedef struct MEM_CDROM_CONTROLLER_ROM  {uint8_t mem[0X4200];}   MEM_CDROM_CONTROLLER_ROM;  // 16.5K
typedef struct MEM_CDROM_BUFFER          {uint8_t mem[0X8400];}   MEM_CDROM_BUFFER;          // 32K
typedef struct MEM_EXTERNAL_MEMORY_CARDS {uint8_t mem[0X20000];}  MEM_EXTERNAL_MEMORY_CARDS; // 128K

struct MEMORY {
  // CPU BUS
  MEM_MAIN MAIN;
  MEM_EXPANSION_1 EXPANSION_1;
  MEM_SCRATCH_PAD SCRATCH_PAD;
  MEM_IO_PORTS IO_PORTS;
  MEM_EXPANSION_2 EXPANSION_2;
  MEM_EXPANSION_3 EXPANSION_3;
  MEM_BIOS BIOS;

  // NON-CPU BUS
  MEM_VRAM VRAM;
  MEM_SOUND SOUND;
  MEM_CDROM_CONTROLLER_RAM CDROM_CONTROLLER_RAM;
  MEM_CDROM_CONTROLLER_RAM CDROM_CONTROLLER_ROM;
  MEM_CDROM_BUFFER CDROM_BUFFER;
  MEM_EXTERNAL_MEMORY_CARDS EXTERNAL_MEMORY_CARDS;
};

#endif
