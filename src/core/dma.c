#include "dma.h"

static struct DMA dma;

PSX_ERROR dma_reset(void) {
    dma.DMA0_MDEC_IN.MADR = (union D_MADR*) memory_pointer(0X1F801080);
    dma.DMA0_MDEC_IN.BRC  = (union D_BRC*)  memory_pointer(0X1F801084);
    dma.DMA0_MDEC_IN.CHCR = (union D_CHCR*) memory_pointer(0X1F801088);

    dma.DMA1_MDEC_OUT.MADR = (union D_MADR*) memory_pointer(0X1F801090);
    dma.DMA1_MDEC_OUT.BRC  = (union D_BRC*)  memory_pointer(0X1F801094);
    dma.DMA1_MDEC_OUT.CHCR = (union D_CHCR*) memory_pointer(0X1F801098);

    dma.DMA2_GPU.MADR = (union D_MADR*) memory_pointer(0X1F8010A0);
    dma.DMA2_GPU.BRC  = (union D_BRC*)  memory_pointer(0X1F8010A4);
    dma.DMA2_GPU.CHCR = (union D_CHCR*) memory_pointer(0X1F8010A8);

    dma.DMA3_CDROM.MADR = (union D_MADR*) memory_pointer(0X1F8010B0);
    dma.DMA3_CDROM.BRC  = (union D_BRC*)  memory_pointer(0X1F8010B4);
    dma.DMA3_CDROM.CHCR = (union D_CHCR*) memory_pointer(0X1F8010B8);

    dma.DMA4_SPU.MADR = (union D_MADR*) memory_pointer(0X1F8010C0);
    dma.DMA4_SPU.BRC  = (union D_BRC*)  memory_pointer(0X1F8010C4);
    dma.DMA4_SPU.CHCR = (union D_CHCR*) memory_pointer(0X1F8010C8);

    dma.DMA5_PIO.MADR = (union D_MADR*) memory_pointer(0X1F8010D0);
    dma.DMA5_PIO.BRC  = (union D_BRC*)  memory_pointer(0X1F8010D4);
    dma.DMA5_PIO.CHCR = (union D_CHCR*) memory_pointer(0X1F8010D8);

    dma.DMA6_OTC.MADR = (union D_MADR*) memory_pointer(0X1F8010E0);
    dma.DMA6_OTC.BRC  = (union D_BRC*)  memory_pointer(0X1F8010E4);
    dma.DMA6_OTC.CHCR = (union D_CHCR*) memory_pointer(0X1F8010E8);

    dma.DPRC = (union DPRC*) memory_pointer(0X1F8010F0);

    dma.DIRC = (union DIRC*) memory_pointer(0X1F8010F4);

    return set_PSX_error(NO_ERROR);
}
