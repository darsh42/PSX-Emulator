#include "memory.h"

static struct MEMORY memory;

PSX_ERROR memory_load_bios(const char *filebios) {
  FILE *fp;
  if ((fp = fopen(filebios, "rb")) == NULL) {
    return BIOS_FILE_NOT_FOUND;
  }
  if (fread(fp, sizeof(memory.BIOS), 1, fp) == 0) {
    return BIOS_FILE_UNREADABLE;
  }
  fclose(fp);
  return NO_ERROR;
}

PSX_ERROR memory_KUSEG_load_32bit(uint32_t address, uint32_t result) {
  uint8_t *segment;
  if (address >= 0X00000000 && address < 0X00200000) {segment = memory.MAIN;}
  else if (address >= 0X1F000000 && address < 0X1F800000) {segment = memory.EXPANSION_1;}
  else if (address >= 0X1F800000 && address < 0X1F801000) {segment = memory.SCRATCH_PAD;}
  else if (address >= 0X1F801000 && address < 0X1F802000) {segment = memory.IO_PORTS;}
  else if (address >= 0X1F802000 && address < 0X1FA00000) {segment = memory.EXPANSION_2;}
  else if (address >= 0X1F8A0000 && address < 0X1FC00000) {segment = memory.EXPANSION_3;}
  else if (address >= 0X1FC00000 && address < 0X1FC80000) {segment = memory.BIOS;}
  else return MEMORY_KUSEG_UNMAPPED_ADDRESS;

  uint8_t b0 = *(segment + 0);
  uint8_t b1 = *(segment + 1);
  uint8_t b2 = *(segment + 2);
  uint8_t b3 = *(segment + 3);

  result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
  return NO_ERROR;
}

PSX_ERROR memory_KSEG0_load_32bit(uint32_t address, uint32_t result) {
  uint8_t *segment;
  if (address >= 0X8000000 && address < 0X8200000) {segment = memory.MAIN;}
  else if (address >= 0X9F000000 && address < 0X9F800000) {segment = memory.EXPANSION_1;}
  else if (address >= 0X9F800000 && address < 0X9F801000) {segment = memory.SCRATCH_PAD;}
  else if (address >= 0X9F801000 && address < 0X9F802000) {segment = memory.IO_PORTS;}
  else if (address >= 0X9F802000 && address < 0X9FA00000) {segment = memory.EXPANSION_2;}
  else if (address >= 0X9F8A0000 && address < 0X9FC00000) {segment = memory.EXPANSION_3;}
  else if (address >= 0X9FC00000 && address < 0X9FC80000) {segment = memory.BIOS;}
  else return MEMORY_KSEG0_UNMAPPED_ADDRESS;

  uint8_t b0 = *(segment + 0);
  uint8_t b1 = *(segment + 1);
  uint8_t b2 = *(segment + 2);
  uint8_t b3 = *(segment + 3);

  result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
  return NO_ERROR;
}

PSX_ERROR memory_KSEG1_load_32bit(uint32_t address, uint32_t result) {
  uint8_t *segment;
  if (address >= 0XA000000 && address < 0XA200000) {segment = memory.MAIN;}
  else if (address >= 0XBF000000 && address < 0XBF800000) {segment = memory.EXPANSION_1;}
  else if (address >= 0XBF800000 && address < 0XBF801000) {segment = memory.SCRATCH_PAD;}
  else if (address >= 0XBF801000 && address < 0XBF802000) {segment = memory.IO_PORTS;}
  else if (address >= 0XBF802000 && address < 0XBFA00000) {segment = memory.EXPANSION_2;}
  else if (address >= 0XBF8A0000 && address < 0XBFC00000) {segment = memory.EXPANSION_3;}
  else if (address >= 0XBFC00000 && address < 0XBFC80000) {segment = memory.BIOS;}
  else return MEMORY_KSEG1_UNMAPPED_ADDRESS;

  uint8_t b0 = *(segment + 0);
  uint8_t b1 = *(segment + 1);
  uint8_t b2 = *(segment + 2);
  uint8_t b3 = *(segment + 3);

  result = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
  return NO_ERROR;
}
