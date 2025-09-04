#ifndef GPU_H_INCLUDED
#define GPU_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#include "fifo.h"

union gpustat {
    uint32_t value;
    struct {
        uint32_t texture_page_x_base         : 4;
        uint32_t texture_page_y_base         : 1;
        uint32_t semi_transparency           : 2;
        uint32_t texture_page_colors         : 2;
        uint32_t dither                      : 1;
        uint32_t draw_to_display_area        : 1;
        uint32_t set_mask_when_drawing       : 1;
        uint32_t draw_pixels                 : 1;
        uint32_t interlace_field             : 1;
        uint32_t reverse_flag                : 1;
        uint32_t texture_disable             : 1;
        uint32_t horizontal_resolution_2     : 1;
        uint32_t horizontal_resolution_1     : 2;
        uint32_t vertical_resolution         : 1;
        uint32_t video_mode                  : 1;
        uint32_t display_area_color_depth    : 1;
        uint32_t vertical_interlace          : 1;
        uint32_t display_enable              : 1;
        uint32_t interrupt_request           : 1;
        uint32_t dma_data_request            : 1;
        uint32_t ready_recieve_cmd_word      : 1;
        uint32_t ready_send_vram_cpu         : 1;
        uint32_t ready_recieve_dma_block     : 1;
        uint32_t dma_direction               : 2;
        uint32_t drawing_even_odd_interlace  : 1;
    };
};

enum gpu_state {
    GPU_IDLE,
    GPU_RENDERING,
    GPU_PROCESS_GP0,
    GPU_PROCESS_GP1,
    GPU_VRAM_TRANSFER,
};

enum gpu_transfer_direction
{
    TRANSFER_TO_VRAM,
    TRANSFER_TO_MAIN
};

struct gpu {
    enum gpu_state state;

    struct fifo gp0;
    uint32_t    gp1;

    union gpustat gpustat;
    uint32_t      gpuread;

    uint32_t cycles;    // gpu cycles 
    uint32_t dots;      // gpu dots (horizontal pixels)
    uint32_t scanlines; // gpu scanlines (vertical pixels)

    uint32_t vblank;    // vertical blank (outside vertical resolution)
    uint32_t hblank;    // horizontal blank (outside horizontal resolution)

    uint32_t vram_direct_access_x; // minimum x
    uint32_t vram_direct_access_y; // minimum y
    uint32_t vram_direct_access_w; // access width
    uint32_t vram_direct_access_h; // access height
    uint32_t vram_direct_access_cx; // access cursor - where in vram is accessed
    uint32_t vram_direct_access_cy; // access cursor - where in vram is accessed
    uint32_t vram_direct_access_d;  // access direction -
                                    //     0 (cpu to vram), 1 (vram to cpu)

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


bool gpu_hblank( void );
bool gpu_vblank( void );
void gpu_set_gpustat( union gpustat  stat );
void gpu_get_gpustat( union gpustat *stat );

uint32_t gpu_get_vram_address( void );

uint32_t  read_gpu( uint32_t address );
void     write_gpu( uint32_t address, uint32_t data );

void init_gpu( void );
void task_gpu( void );

#endif //  GPU_H_INCLUDED
