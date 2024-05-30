#ifndef GPU_H_INCLUDED
#define GPU_H_INCLUDED

#include "common.h"

enum GPUSTAT_ENABLE                   { DISABLE = false,        ENABLE  = true };
enum GPUSTAT_READY                    { NOT_READY = false,      READY = true };
enum GPUSTAT_DRAW_PIXEL               { ALWAYS = false,         NOT_TO_MASKED = true };
enum GPUSTAT_REVERSEFIELD             { UNDISTORTED = false,    DISTORTED = true };
enum GPUSTAT_TEXTURE_DISABLE          { ENABLED  = false,       DISABLED = true };
enum GPUSTAT_VIDEO_MODE               { NTSC60HZ = false,       PAL50HZ  = true };
enum GPUSTAT_DISPLAY_AREA_COLOR_DEPTH { _15BIT = false,         _24BIT = true };
enum GPUSTAT_EVEN_ODD_INTERLACE       { EVEN_OR_VBLANK = false, ODD = true };
enum GPUSTAT_DMA_DIRECTION {
    OFF = 0,
    UNKNOWN = 1,
    CPU_TO_GP0 = 2,
    GPUREAD_TO_CPU = 3
};

enum COMMAND_PACKET_PRIMATIVE_IIP  { FLAT_SHADING = false,                    GOURAD_SHADING = true };
enum COMMAND_PACKET_PRIMATIVE_VTX  { VERTEX_3_POLYGON = false,                VERTEX_4_POLYGON = true };
enum COMMAND_PACKET_PRIMATIVE_TME  { TEXTURE_MAPPING_OFF = false,             TEXTURE_MAPPING_ON = true}i ;
enum COMMAND_PACKET_PRIMATIVE_ABE  { SEMI_TRANSPARENCY_OFF = false,           SEMI_TRANSPARENCY_ON = true };
enum COMMAND_PACKET_PRIMATIVE_TGE  { BRIGHTNESS_ON_TEXURE_MAPPING_ON = false, BRIGHTNESS_ON_TEXURE_MAPPING_OFF = true };
enum COMMAND_PACKET_PRIMATIVE_PLL  { SINGLE_LINE = false,                     POLY_LINE = true };
enum COMMAND_PACKET_PRIMATIVE_SIZE { 
    FREE   = 0,
    _1X1   = 1,
    _8X8   = 2,
    _16X16 = 3
};

union GPUSTAT {
    uint32_t value;
    struct {
        uint32_t texture_page_x_base: 4;
        uint32_t texture_page_y_base: 1;
        uint32_t semi_transparency: 2;
        uint32_t texture_page_colors: 2;
        enum GPUSTAT_ENABLE dither: 1;
        enum GPUSTAT_ENABLE draw_to_display_area: 1;
        enum GPUSTAT_ENABLE set_mask_when_drawing: 1;
        enum GPUSTAT_DRAW_PIXEL draw_pixels: 1;
        uint32_t interlace_field: 1;
        enum GPUSTAT_REVERSEFIELD reverse_flag: 1;
        enum GPUSTAT_TEXTURE_DISABLE texture_disable: 1;
        uint32_t horizontal_resolution_2: 1;
        uint32_t horizontal_resolution_1: 2;
        uint32_t vertical_resolution: 1;
        enum GPUSTAT_VIDEO_MODE video_mode: 1;
        enum GPUSTAT_DISPLAY_AREA_COLOR_DEPTH display_area_color_depth: 1;
        enum GPUSTAT_ENABLE vertical_interlace: 1;
        enum GPUSTAT_ENABLE display_enable: 1;
        enum GPUSTAT_ENABLE interrupt_request: 1;
        uint32_t DMA_data_request: 1;
        enum GPUSTAT_READY ready_recieve_cmd_word: 1;
        enum GPUSTAT_READY ready_send_vram_cpu: 1;
        enum GPUSTAT_READY ready_recieve_dma_block: 1;
        enum GPUSTAT_DMA_DIRECTION dma_direction: 2;
        enum GPUSTAT_EVEN_ODD_INTERLACE drawing_even_odd_interlace: 1;
    };
};

union GPUREAD {
    uint32_t value;
    struct {
        uint32_t read;
    };
};

// possible issues due to incorrect enum packing
union COMMAND_PACKET {
    uint8_t value;
    struct {
        enum COMMAND_PACKET_PRIMATIVE_TGE tge: 1;
        enum COMMAND_PACKET_PRIMATIVE_ABE abe: 1;
        enum COMMAND_PACKET_PRIMATIVE_TME tme: 1;
        enum COMMAND_PACKET_PRIMATIVE_VTX vtx: 1;
        enum COMMAND_PACKET_PRIMATIVE_IIP iip: 1;
        uint8_t type: 3;
    };
    struct {
        uint8_t : 3;
        enum COMMAND_PACKET_PRIMATIVE_SIZE size: 2;
    };
};

union GP0 {
    uint32_t value;
};

union GP1 {
    uint32_t value;
    struct {
        uint32_t command: 16;
        uint32_t parameter: 16;
    };
};

struct GPU {
    uint32_t tmp;
    
    union COMMAND_PACKET cmd_pkt;

    union GP0 *gp0;
    union GP1 *gp1;
    union GPUREAD *gpuread;
    union GPUSTAT *gpustat;

    uint32_t CMD_FIFO_GP0[16];
    uint32_t CMD_FIFO_GP1[16];
};

// memory functions
uint8_t *memory_pointer(uint32_t address);

#endif // GPU_H_INCLUDED
