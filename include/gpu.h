#ifndef GPU_H_INCLUDED
#define GPU_H_INCLUDED

#include "common.h"
#include "memory.h"
#include "renderer.h"

#define print_gpu_error(func, format, ...) print_error("gpu.c", func, format, __VA_ARGS__)

#define VRAM_WIDTH 1024
#define VRAM_ADDRESS(x, y) y * VRAM_WIDTH + x

enum GPU_RENDER_PHASE {
    RENDER,     // main renderering mode
    HBLANK,     // when dots > horizontal resolution
    VBLANK      // when scanlines > vertical resolution
};

enum GPU_COPY_DIRECTION {
    VRAM_TO_VRAM = 0,
    CPU_TO_VRAM  = 1,
    VRAM_TO_CPU  = 2
};

enum GPU_MODE {
    IDLE    = 0,
    GP0  = 1,
    GP1  = 2,
    COPY = 3
};

enum GPUSTAT_READY                    { NOT_READY = false,      READY = true };
enum GPUSTAT_DRAW_PIXEL               { ALWAYS = false,         NOT_TO_MASKED = true };
enum GPUSTAT_REVERSEFIELD             { UNDISTORTED = false,    DISTORTED = true };
enum GPUSTAT_TEXTURE_DISABLE          { ENABLED  = false,       DISABLED = true };
enum GPUSTAT_VIDEO_MODE               { NTSC60HZ = false,       PAL50HZ  = true };
enum GPUSTAT_DISPLAY_AREA_COLOR_DEPTH { _15BIT = false,         _24BIT = true };
enum GPUSTAT_EVEN_ODD_INTERLACE       { EVEN_OR_VBLANK = false, ODD = true };
enum GPUSTAT_TEXTURE_DEPTH {
    DEPTH_4BIT,
    DEPTH_8BIT,
    DEPTH_15BIT,
    DEPTH_RESERVED
};
enum GPUSTAT_DMA_OR_DATA_REQUEST {
    FIFO_FULL = false,
    FIFO_NOT_FULL = true,
};
enum GPUSTAT_DMA_DIRECTION {
    OFF = 0,
    UNKNOWN = 1,
    CPU_TO_GP0 = 2,
    GPUREAD_TO_CPU = 3
};

enum COMMAND_PACKET_PRIMATIVE_IIP  { FLAT_SHADING = false,                    GOURAD_SHADING = true };
enum COMMAND_PACKET_PRIMATIVE_VTX  { VERTEX_3_POLYGON = false,                VERTEX_4_POLYGON = true };
enum COMMAND_PACKET_PRIMATIVE_TME  { TEXTURE_MAPPING_OFF = false,             TEXTURE_MAPPING_ON = true};
enum COMMAND_PACKET_PRIMATIVE_ABE  { SEMI_TRANSPARENCY_OFF = false,           SEMI_TRANSPARENCY_ON = true };
enum COMMAND_PACKET_PRIMATIVE_TGE  { BRIGHTNESS_ON_TEXURE_MAPPING_ON = false, BRIGHTNESS_ON_TEXURE_MAPPING_OFF = true };
enum COMMAND_PACKET_PRIMATIVE_PLL  { SINGLE_LINE = false,                     POLY_LINE = true };
enum COMMAND_PACKET_PRIMATIVE_SIZE {
    FREE   = 0,
    _1X1   = 1,
    _8X8   = 2,
    _16X16 = 3
};

// possible issues due to incorrect enum packing
union COMMAND_PACKET {
    uint32_t value;
    struct {
        uint32_t parameters: 24;
        uint32_t number: 8;
    };
};

// 64 byte fifos' and can queue upto 3 commands
#define FIFO_SIZE 12
#define MAX_COMMANDS 3

typedef struct FIFO fifo_t;

struct COMMAND_FIFO {
    union COMMAND_PACKET commands[FIFO_SIZE];

    int head, tail, size, len;
};

union VERTEX {
    uint32_t value;
    struct {
        uint32_t x: 10;
        uint32_t  :  6;
        uint32_t y: 10;
        uint32_t  :  6;
    };
};

union COLOR_24BIT {
    uint32_t value;
    struct {
        uint32_t red:   8;
        uint32_t green: 8;
        uint32_t blue:  8;
    };
};

union COLOR_15BIT {
    uint32_t value;
    struct {
        uint32_t red:   5;
        uint32_t green: 5;
        uint32_t blue:  5;
        uint32_t mask:  1;
    };
};

#define VERTEX_AND_COLOR_TO_DATA(v, c)    \
        ((float) (int) v.x / 512) - 1.0f, \
        1.0f - ((float) (int) v.y / 256), \
        0.0f,                             \
        (c.red   / (float) 0X1f),         \
        (c.green / (float) 0X1f),         \
        (c.blue  / (float) 0X1f)

union GPUSTAT {
    uint32_t value;
    struct {
        uint32_t texture_page_x_base: 4;
        uint32_t texture_page_y_base: 1;
        uint32_t semi_transparency: 2;
        enum GPUSTAT_TEXTURE_DEPTH texture_page_colors: 2;
        enum PSX_ENABLE dither: 1;
        enum PSX_ENABLE draw_to_display_area: 1;
        enum PSX_ENABLE set_mask_when_drawing: 1;
        enum GPUSTAT_DRAW_PIXEL draw_pixels: 1;
        uint32_t interlace_field: 1;
        enum GPUSTAT_REVERSEFIELD reverse_flag: 1;
        enum GPUSTAT_TEXTURE_DISABLE texture_disable: 1;
        uint32_t horizontal_resolution_2: 1;
        uint32_t horizontal_resolution_1: 2;
        uint32_t vertical_resolution: 1;
        enum GPUSTAT_VIDEO_MODE video_mode: 1;
        enum GPUSTAT_DISPLAY_AREA_COLOR_DEPTH display_area_color_depth: 1;
        enum PSX_ENABLE vertical_interlace: 1;
        enum PSX_ENABLE display_enable: 1;
        enum PSX_ENABLE interrupt_request: 1;
        uint32_t dma_data_request: 1;
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

struct GP0 {
    struct COMMAND_FIFO fifo;
    bool write_occured;
};

struct GP1 {
    union COMMAND_PACKET command;
    bool write_occured;
};

struct GPU_COPY {
    uint32_t d_x, d_y, d_w, d_h; // destination
    uint32_t s_x, s_y, s_w, s_h; // source

    bool copying;

    enum GPU_COPY_DIRECTION direction;
};

struct GPU {
    struct GP0 gp0;
    struct GP1 gp1;
    union GPUREAD gpuread;
    union GPUSTAT gpustat;

    struct GPU_COPY copy;

    enum GPU_MODE current_mode;
    enum GPU_MODE previous_mode;
    bool vram_write;

    uint32_t cycles;
    uint32_t scanlines;

    enum GPU_RENDER_PHASE render_phase;

    uint8_t texture_window_mask_x;   // texture window x mask (8 bit steps)
    uint8_t texture_window_mask_y;   // texture window y mask (8 bit steps)
    uint8_t texture_window_offset_x; // texture window x offset (8 bit steps)
    uint8_t texture_window_offset_y; // texture window y offset (8 bit steps)

    uint16_t drawing_area_top;    // drawing area top most line
    uint16_t drawing_area_left;   // drawing area left most column
    uint16_t drawing_area_right;  // drawing area right most column
    uint16_t drawing_area_bottom; // drawing area bottom most line

    int16_t drawing_offset_x; // horizontal offset applied too all verticies
    int16_t drawing_offset_y; // vertical offset applied too all verticies

    uint16_t display_vram_x_start;     // first column of the display in vram
    uint16_t display_vram_y_start;     // first line of the display in vram
    uint16_t display_horizontal_start; // display output start relative to hsync
    uint16_t display_horizontal_end;   // display output end relative to hsync
    uint16_t display_vertical_start;   // display output start relative to vsync
    uint16_t display_vertical_end;     // display output end relative to vsync

    bool texture_rectangle_x_flip; // if texture is flipped in x direction
    bool texture_rectangle_y_flip; // if texture is flipped in y direction
};

/* public functions */
extern struct GPU *get_gpu(void);
extern void gpu_reset(void);
extern void gpu_step(void);
extern uint8_t *write_GP0(void);
extern uint8_t *write_GP1(void);
extern uint8_t *read_GPUSTAT(void);
extern uint8_t *read_GPUREAD(void);
extern bool gpu_vram_write(void);
extern bool gpustat_display_enable(void);
extern bool gpustat_interrupt_request(void);
extern bool gpustat_dma_data_request(void);
extern bool gpustat_ready_recieve_cmd_word(void);
extern bool gpustat_ready_send_vram_cpu(void);
extern bool gpustat_ready_recieve_dma_block(void);
extern uint32_t gpu_display_vram_x_start(void);
extern uint32_t gpu_display_vram_y_start(void);

#endif // GPU_H_INCLUDED
