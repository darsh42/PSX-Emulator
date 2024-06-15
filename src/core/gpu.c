#include "../../include/gpu.h"

static struct GPU gpu;

static void gpu_execute_op(void);

// helpers
static void gpu_handle_gp0(void);
static void gpu_handle_gp1(void);
static void reset_command_fifo(struct COMMAND_FIFO *fifo);
static union COMMAND_PACKET *push_command_fifo(struct COMMAND_FIFO *fifo);
static union COMMAND_PACKET  peek_command_fifo(struct COMMAND_FIFO *fifo);
static union COMMAND_PACKET  pop_command_fifo(struct COMMAND_FIFO *fifo);

// gp0 instructions
static void GP0_NOP(union COMMAND_PACKET packet);
static void GP0_DIRECT_VRAM_ACCESS(union COMMAND_PACKET packet);
static void GP0_INTERRUPT_REQUEST(union COMMAND_PACKET packet);
static void GP0_RENDER_POLYGONS(union COMMAND_PACKET packet);
static void GP0_RENDER_LINES(union COMMAND_PACKET packet);
static void GP0_RENDER_RECTANGLES(union COMMAND_PACKET packet);
static void GP0_RENDERING_ATTRIBUTES(union COMMAND_PACKET packet);
static void GP0_DISPLAY_CONTROL(union COMMAND_PACKET packet);

// gp1 instructions
static void GP1_RESET(union COMMAND_PACKET packet);
static void GP1_RESET_COMMAND_BUFFER(union COMMAND_PACKET packet);
static void GP1_ACKNOWLEDGE_INTERRUPT(union COMMAND_PACKET packet);
static void GP1_DISPLAY_ENABLE(union COMMAND_PACKET packet);
static void GP1_DMA_DIRECTION_OR_DATA_REQUEST(union COMMAND_PACKET packet);
static void GP1_START_OF_DISPLAY_AREA_IN_VRAM(union COMMAND_PACKET packet);
static void GP1_HORIONTAL_DISPLAY_RANGE(union COMMAND_PACKET packet);
static void GP1_VERTICAL_DISPLAY_RANGE(union COMMAND_PACKET packet);
static void GP1_DISPLAY_MODE(union COMMAND_PACKET packet);
static void GP1_NEW_TEXTURE_DISABLE(union COMMAND_PACKET packet);
static void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(union COMMAND_PACKET packet);
static void GP1_DISPLAY_INFO(union COMMAND_PACKET packet);

// external interface
struct GPU *get_gpu(void) { return &gpu; }
uint8_t *GP0(void) { gpu.gp0.write_occured = true; return (uint8_t *) push_command_fifo(&gpu.gp0.fifo); }
uint8_t *GP1(void) { gpu.gp1.write_occured = true; return (uint8_t *) &gpu.gp1.command.value; }
uint8_t *GPUSTAT(void) { return (uint8_t *) &gpu.gpustat.value; }
uint8_t *GPUREAD(void) { return (uint8_t *) &gpu.gpustat.value; }

void gpu_reset(void) {
    // set gpustat starting values
    gpu.gpustat.value = 0;
    gpu.gpustat.display_enable = ENABLE;
    gpu.gpustat.ready_recieve_cmd_word = READY;
    gpu.gpustat.ready_recieve_dma_block = READY;
    gpu.gpustat.drawing_even_odd_interlace = ODD;

    // set gp0 and gp1 starting values
    reset_command_fifo(&gpu.gp0.fifo);
}

void gpu_step(void) {
    // handle all direct access to and from vram
    if (gpu.iscopying) {
        if (gpu.vram_to_vram) {
        } else if (gpu.vram_to_cpu) {
        } else if (gpu.cpu_to_vram) {
            uint32_t data = pop_command_fifo(&gpu.gp0.fifo).value;
            memory_gpu_store_16bit(gpu.copy_address++, (data >>  0) & 0XFFFF);
            memory_gpu_store_16bit(gpu.copy_address++, (data >> 16) & 0XFFFF);
            gpu.copy_size--;
            gpu.copy_size--;
        }

        if (gpu.copy_size <= 0) {
            gpu.iscopying = false;
        }
    }

    if (gpu.gp0.write_occured) {
        gpu.gp0.write_occured = false;
        gpu_handle_gp0();
    }
    
    if (gpu.gp1.write_occured) {
        gpu.gp1.write_occured = false;
        gpu_handle_gp1();
    }
}

void gpu_handle_gp0(void) {
    // GP0
    union COMMAND_PACKET command = peek_command_fifo(&gpu.gp0.fifo);

    switch (command.number) {
        case 0X00: GP0_NOP(command); break;
        case 0X01: 
        case 0X02: 
        case 0X80: 
        case 0XA0: 
        case 0XC0: GP0_DIRECT_VRAM_ACCESS(command); break;
        case 0X1F: GP0_INTERRUPT_REQUEST(command); break;
        case 0X03: break; // UNKNOWN
        default:
            switch (command.number >> 4) {
                case 0X02: case 0X03: GP0_RENDER_POLYGONS(command); break;
                case 0X04: case 0X05: GP0_RENDER_LINES(command); break;
                case 0X06: case 0X07: GP0_RENDER_RECTANGLES(command); break;
                case 0X0E:            GP0_RENDERING_ATTRIBUTES(command); break;
                default:
                    break;
            }
            break;
    };
}

void gpu_handle_gp1(void) {
    union COMMAND_PACKET command = gpu.gp1.command;

    switch (command.number) {
        case 0X00: GP1_RESET(command); break;
        case 0X01: GP1_RESET_COMMAND_BUFFER(command); break;
        case 0X02: GP1_ACKNOWLEDGE_INTERRUPT(command); break;
        case 0X03: GP1_DISPLAY_ENABLE(command); break;
        case 0X04: GP1_DMA_DIRECTION_OR_DATA_REQUEST(command); break;
        case 0X05: GP1_START_OF_DISPLAY_AREA_IN_VRAM(command); break;
        case 0X06: GP1_HORIONTAL_DISPLAY_RANGE(command); break;
        case 0X07: GP1_VERTICAL_DISPLAY_RANGE(command); break;
        case 0X08: GP1_DISPLAY_MODE(command); break;
        case 0X09: GP1_NEW_TEXTURE_DISABLE(command); break;
        case 0X20: break;
        default:
            if ((command.number >> 4)== 0X01) 
                GP1_DISPLAY_INFO(command);
            break;
    }
}

// command fifo helpers
void reset_command_fifo(struct COMMAND_FIFO *fifo) {
    fifo->isempty = true;
    fifo->isfull  = false;
    fifo->head    = 0;
    fifo->tail    = 0;
    fifo->count   = 0;
}

union COMMAND_PACKET *push_command_fifo(struct COMMAND_FIFO *fifo) {
    // get current free command
    union COMMAND_PACKET *refrence = &fifo->commands[fifo->tail];
    
    // reserve the next command space 
    // by incrementing the tail of fifo
    fifo->tail++;
    fifo->tail %= FIFO_SIZE;
    fifo->count++;
    
    if (fifo->isempty) {
        fifo->isempty = false;
    }
    
    if (fifo->count >= FIFO_SIZE && fifo->head == fifo->tail) {
        fifo->isfull = true;
        // 
        gpu.gpustat.dma_data_request = FIFO_FULL;
    }

    return refrence;
}

union COMMAND_PACKET peek_command_fifo(struct COMMAND_FIFO *fifo) {
    // get current command
    return fifo->commands[fifo->head];
}

union COMMAND_PACKET pop_command_fifo(struct COMMAND_FIFO *fifo) {
    union COMMAND_PACKET command = fifo->commands[fifo->head];

    fifo->head++;
    fifo->head %= FIFO_SIZE;
    fifo->count--;
    
    if (fifo->isfull) {
        fifo->isfull = false;
    }

    if (fifo->count == 0) {
        fifo->isempty = true;
    }
    
    return command;
}

// vram helpers
static void VRAM_CLEAR_CACHE(void);
static void VRAM_FILL_RECTANGLE(void);
static void VRAM_TO_VRAM_COPY_RECTANGLE(void);
static void CPU_TO_VRAM_COPY_RECTANGLE(void);
static void VRAM_TO_CPU_COPY_RECTANGLE(void);

// rendering helpers
static void RENDER_THREE_POINT_POLYGON_MONOCHROME(bool semi_transparent);
static void RENDER_FOUR_POINT_POLYGON_MONOCHROME(bool semi_transparent);
static void RENDER_THREE_POINT_POLYGON_TEXTURED(bool semi_transparent, bool texture_blending);
static void RENDER_FOUR_POINT_POLYGON_TEXTURED(bool semi_transparent, bool texture_blending);
static void RENDER_THREE_POINT_POLYGON_SHADED(bool semi_transparent);
static void RENDER_FOUR_POINT_POLYGON_SHADED(bool semi_transparent);
static void RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(bool semi_transparent, bool texture_blending);
static void RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(bool semi_transparent, bool texture_blending);

// gp0 functions
void GP0_NOP(union COMMAND_PACKET packet) {
    pop_command_fifo(&gpu.gp0.fifo);
}
void GP0_DIRECT_VRAM_ACCESS(union COMMAND_PACKET packet) {
    switch(packet.number) {
        case 0X01: VRAM_CLEAR_CACHE(); break;
        case 0X02: VRAM_FILL_RECTANGLE(); break;
        case 0X80: VRAM_TO_VRAM_COPY_RECTANGLE(); break;
        case 0XA0: CPU_TO_VRAM_COPY_RECTANGLE(); break;
        case 0XC0: VRAM_TO_CPU_COPY_RECTANGLE(); break;
    }
}
void GP0_INTERRUPT_REQUEST(union COMMAND_PACKET packet) {}
void GP0_RENDER_POLYGONS(union COMMAND_PACKET packet) {
    switch (packet.number) {
        case 0X20: RENDER_THREE_POINT_POLYGON_MONOCHROME(0); break;
        case 0X22: RENDER_THREE_POINT_POLYGON_MONOCHROME(1); break;
        case 0X28: RENDER_FOUR_POINT_POLYGON_MONOCHROME(0); break;
        case 0X2A: RENDER_FOUR_POINT_POLYGON_MONOCHROME(1); break;

        case 0X24: RENDER_THREE_POINT_POLYGON_TEXTURED(0, 1); break;
        case 0X25: RENDER_THREE_POINT_POLYGON_TEXTURED(0, 0); break;
        case 0X26: RENDER_THREE_POINT_POLYGON_TEXTURED(1, 1); break;
        case 0X27: RENDER_THREE_POINT_POLYGON_TEXTURED(1, 0); break;
        case 0X2C: RENDER_FOUR_POINT_POLYGON_TEXTURED(0, 1); break;
        case 0X2D: RENDER_FOUR_POINT_POLYGON_TEXTURED(0, 0); break;
        case 0X2E: RENDER_FOUR_POINT_POLYGON_TEXTURED(1, 1); break;
        case 0X2F: RENDER_FOUR_POINT_POLYGON_TEXTURED(1, 0); break;

        case 0X30: RENDER_THREE_POINT_POLYGON_SHADED(0); break;
        case 0X32: RENDER_THREE_POINT_POLYGON_SHADED(1); break;
        case 0X38: RENDER_FOUR_POINT_POLYGON_SHADED(0); break;
        case 0X3A: RENDER_FOUR_POINT_POLYGON_SHADED(1); break;

        case 0X34: RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(0, 1); break;
        case 0X36: RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(1, 1); break;
        case 0X3C: RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(0, 1); break;
        case 0X3E: RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(1, 1); break;
    }
}
void GP0_RENDER_LINES(union COMMAND_PACKET packet) {}
void GP0_RENDER_RECTANGLES(union COMMAND_PACKET packet) {}
void GP0_RENDERING_ATTRIBUTES(union COMMAND_PACKET packet) {
    // pop current command as it doesnt need more arguments
    pop_command_fifo(&gpu.gp0.fifo);
    switch (packet.number & 0b1111) {
        case 0X01: {
            /* DRAWMODE SETTING */
            //  0-3   Texture page X Base   (N*64) (ie. in 64-halfword steps)    ;GPUSTAT.0-3
            //  4     Texture page Y Base   (N*256) (ie. 0 or 256)               ;GPUSTAT.4
            //  5-6   Semi Transparency     (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)   ;GPUSTAT.5-6
            //  7-8   Texture page colors   (0=4bit, 1=8bit, 2=15bit, 3=Reserved);GPUSTAT.7-8
            //  9     Dither 24bit to 15bit (0=Off/strip LSBs, 1=Dither Enabled) ;GPUSTAT.9
            //  10    Drawing to display area (0=Prohibited, 1=Allowed)          ;GPUSTAT.10
            //  11    Texture Disable (0=Normal, 1=Disable if GP1(09h).Bit0=1)   ;GPUSTAT.15
            //          (Above might be chipselect for (absent) second VRAM chip?)
            //  12    Textured Rectangle X-Flip   (BIOS does set this bit on power-up...?)
            //  13    Textured Rectangle Y-Flip   (BIOS does set it equal to GPUSTAT.13...?)
            //  14-23 Not used (should be 0)
            //  24-31 Command  (E1h)
            gpu.gpustat.texture_page_x_base  = (packet.parameters >>  0) & 0b111;
            gpu.gpustat.texture_page_y_base  = (packet.parameters >>  4) & 0b001;
            gpu.gpustat.semi_transparency    = (packet.parameters >>  5) & 0b011;
            gpu.gpustat.texture_page_colors  = (packet.parameters >>  7) & 0b011;
            gpu.gpustat.dither               = (packet.parameters >>  9) & 0b001;
            gpu.gpustat.draw_to_display_area = (packet.parameters >> 10) & 0b001;
            gpu.gpustat.texture_disable      = (packet.parameters >> 11) & 0b001;
            gpu.texture_rectangle_x_flip     = (packet.parameters >> 12) & 0b001;
            gpu.texture_rectangle_y_flip     = (packet.parameters >> 13) & 0b001;
            break;
        }
        case 0X02: {
            /* TEXTURE WINDOW SETTING */
            //  0-4    Texture window Mask X   (in 8 pixel steps)
            //  5-9    Texture window Mask Y   (in 8 pixel steps)
            //  10-14  Texture window Offset X (in 8 pixel steps)
            //  15-19  Texture window Offset Y (in 8 pixel steps)
            //  20-23  Not used (zero)
            //   24-31  Command  (E2h)
            gpu.texture_window_mask_x   = (packet.parameters >>  0) & 0b11111;
            gpu.texture_window_mask_y   = (packet.parameters >>  5) & 0b11111;
            gpu.texture_window_offset_x = (packet.parameters >> 10) & 0b11111;
            gpu.texture_window_offset_y = (packet.parameters >> 15) & 0b11111;
            break;
        }
        case 0X03: { 
            /* SET DRAWING AREA TOP LEFT */
            //  0-9    X-coordinate (0..1023)
            //  10-18  Y-coordinate (0..511)   ;\on Old 160pin GPU (max 1MB VRAM)
            //  19-23  Not used (zero)         ;/
            //  10-19  Y-coordinate (0..1023)  ;\on New 208pin GPU (max 2MB VRAM)
            //  20-23  Not used (zero)         ;/(retail consoles have only 1MB though)
            //  24-31  Command  (Exh)
            //
            //  Sets the drawing area corners. The Render commands GP0(20h..7Fh) are automatically clipping any pixels that are outside of this region.
            gpu.drawing_area_left = (packet.parameters >>  0) & 0b1111111111;
            gpu.drawing_area_top  = (packet.parameters >> 10) & 0b1111111111;
            break;
        }
        case 0X04: {
            /* SET DRAWING AREA BOTTOM RIGHT */
            //  0-9    X-coordinate (0..1023)
            //  10-18  Y-coordinate (0..511)   ;\on Old 160pin GPU (max 1MB VRAM)
            //  19-23  Not used (zero)         ;/
            //  10-19  Y-coordinate (0..1023)  ;\on New 208pin GPU (max 2MB VRAM)
            //  20-23  Not used (zero)         ;/(retail consoles have only 1MB though)
            //  24-31  Command  (Exh)
            //
            //  Sets the drawing area corners. The Render commands GP0(20h..7Fh) are automatically clipping any pixels that are outside of this region.
            gpu.drawing_area_right   = (packet.parameters >>  0) & 0b1111111111;
            gpu.drawing_area_bottom  = (packet.parameters >> 10) & 0b1111111111;
            break;
        }
        case 0X05: break;
        case 0X06: {
            /* MASK BIT SETTING */
            // 0     Set mask while drawing (0=TextureBit15, 1=ForceBit15=1)   ;GPUSTAT.11
            // 1     Check mask before draw (0=Draw Always, 1=Draw if Bit15=0) ;GPUSTAT.12
            // 2-23  Not used (zero)
            // 24-31 Command  (E6h)
            gpu.gpustat.set_mask_when_drawing = (packet.parameters >> 0) & 0b1;
            gpu.gpustat.draw_pixels           = (packet.parameters >> 1) & 0b1;
            break;
        }
    }

}
void GP0_DISPLAY_CONTROL(union COMMAND_PACKET packet) {}

// gp1 functions
void GP1_RESET(union COMMAND_PACKET packet) {
    // Resets the GPU to the following values:
    //
    //  GP1(01h)      ;clear fifo
    //  GP1(02h)      ;ack irq (0)
    //  GP1(03h)      ;display off (1)
    //  GP1(04h)      ;dma off (0)
    //  GP1(05h)      ;display address (0)
    //  GP1(06h)      ;display x1,x2 (x1=200h, x2=200h+256*10)
    //  GP1(07h)      ;display y1,y2 (y1=010h, y2=010h+240)
    //  GP1(08h)      ;display mode 320x200 NTSC (0)
    //  GP0(E1h..E6h) ;rendering attributes (0)
    //
    // Accordingly, GPUSTAT becomes 14802000h.The x1,y1 values are too small, ie. the upper-left edge isn't visible. 
    // Note that GP1(09h) is NOT affected by the reset command.
    
    // clear the fifo
    reset_command_fifo(&gpu.gp0.fifo);

    gpu.gpustat.value = 0X00000000;
    gpu.gpustat.interlace_field = true;
    gpu.gpustat.display_enable = ENABLE;
    gpu.gpustat.ready_recieve_cmd_word  = READY;
    gpu.gpustat.ready_recieve_dma_block = READY;
    
    gpu.texture_window_mask_x = 0;
    gpu.texture_window_mask_y = 0;
    gpu.texture_window_offset_x = 0;
    gpu.texture_window_offset_y = 0;

    gpu.texture_rectangle_x_flip = false;
    gpu.texture_rectangle_y_flip = false;
    
    gpu.drawing_area_top    = 0;
    gpu.drawing_area_left   = 0;
    gpu.drawing_area_right  = 0;
    gpu.drawing_area_bottom = 0;

    gpu.display_vram_x_start = 0;
    gpu.display_vram_y_start = 0;

    gpu.display_horizontal_start = 0X200;
    gpu.display_horizontal_end   = 0XC00;

    gpu.display_vertical_start = 0X010;
    gpu.display_vertical_end   = 0X100;
}
void GP1_RESET_COMMAND_BUFFER(union COMMAND_PACKET packet) {
    // 0-23  Not used (zero)
    // Clears the command FIFO, and aborts the current rendering command 
    // (eg. this may end up with an incompletely drawn triangle).
    
}
void GP1_ACKNOWLEDGE_INTERRUPT(union COMMAND_PACKET packet) {
}
void GP1_DISPLAY_ENABLE(union COMMAND_PACKET packet) {}
void GP1_DMA_DIRECTION_OR_DATA_REQUEST(union COMMAND_PACKET packet) {
    // 0-1  DMA Direction (0=Off, 1=FIFO, 2=CPUtoGP0, 3=GPUREADtoCPU) ;GPUSTAT.29-30
    // 2-23 Not used (zero)
    gpu.gpustat.dma_direction = packet.parameters & 0b11;
}
void GP1_START_OF_DISPLAY_AREA_IN_VRAM(union COMMAND_PACKET packet) {
    // 0-9   X (0-1023)    (halfword address in VRAM)  (relative to begin of VRAM)
    // 10-18 Y (0-511)     (scanline number in VRAM)   (relative to begin of VRAM)
    // 19-23 Not used (zero)
    //
    // Upper/left Display source address in VRAM. The size and target position on 
    // screen is set via Display Range registers; target=X1,Y2; size=(X2-X1/cycles_per_pix), (Y2-Y1).
    gpu.display_vram_x_start = (packet.parameters >>  0) & 0b1111111111;
    gpu.display_vram_y_start = (packet.parameters >> 10) & 0b0111111111;
}
void GP1_HORIONTAL_DISPLAY_RANGE(union COMMAND_PACKET packet) {
    // 0-11   X1 (260h+0)       ;12bit       ;\counted in 53.222400MHz units,
    // 12-23  X2 (260h+320*8)   ;12bit       ;/relative to HSYNC
    gpu.display_horizontal_start = (packet.parameters >>  0) & 0b111111111111;
    gpu.display_horizontal_end   = (packet.parameters >> 12) & 0b111111111111;
}
void GP1_VERTICAL_DISPLAY_RANGE(union COMMAND_PACKET packet) {
    // 0-9   Y1 (NTSC=88h-(224/2), (PAL=A3h-(264/2))  ;\scanline numbers on screen,
    // 10-19 Y2 (NTSC=88h+(224/2), (PAL=A3h+(264/2))  ;/relative to VSYNC
    // 20-23 Not used (zero)
    gpu.display_vertical_start = (packet.parameters >>  0) * 0b111111111;
    gpu.display_vertical_end   = (packet.parameters >>  0) * 0b111111111;
}
void GP1_DISPLAY_MODE(union COMMAND_PACKET packet) {
    // 0-1   Horizontal Resolution 1     (0=256, 1=320, 2=512, 3=640) ;GPUSTAT.17-18
    // 2     Vertical Resolution         (0=240, 1=480, when Bit5=1)  ;GPUSTAT.19
    // 3     Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)    ;GPUSTAT.20
    // 4     Display Area Color Depth    (0=15bit, 1=24bit)           ;GPUSTAT.21
    // 5     Vertical Interlace          (0=Off, 1=On)                ;GPUSTAT.22
    // 6     Horizontal Resolution 2     (0=256/320/512/640, 1=368)   ;GPUSTAT.16
    // 7     "Reverseflag"               (0=Normal, 1=Distorted)      ;GPUSTAT.14
    // 8-23  Not used (zero)
    gpu.gpustat.horizontal_resolution_1  = (packet.parameters >> 0) & 0b11;
    gpu.gpustat.vertical_resolution      = (packet.parameters >> 2) & 0b01;
    gpu.gpustat.video_mode               = (packet.parameters >> 3) & 0b01;
    gpu.gpustat.display_area_color_depth = (packet.parameters >> 4) & 0b01;
    gpu.gpustat.vertical_interlace       = (packet.parameters >> 5) & 0b01;
    gpu.gpustat.horizontal_resolution_2  = (packet.parameters >> 6) & 0b11;
    gpu.gpustat.reverse_flag             = (packet.parameters >> 7) & 0b01;

}
void GP1_NEW_TEXTURE_DISABLE(union COMMAND_PACKET packet) {}
void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(union COMMAND_PACKET packet) {}
void GP1_DISPLAY_INFO(union COMMAND_PACKET packet) {}

void VRAM_CLEAR_CACHE(void) {
    // clear texture cache
    
}
void VRAM_FILL_RECTANGLE(void) {}
void VRAM_TO_VRAM_COPY_RECTANGLE(void) {}
void CPU_TO_VRAM_COPY_RECTANGLE(void) {
    //  1st  Command           (Cc000000h)
    //  2nd  Destination Coord (YyyyXxxxh)  ;Xpos counted in halfwords
    //  3rd  Width+Height      (YsizXsizh)  ;Xsiz counted in halfwords
    //  ...  Data              (...)      <--- usually transferred via DMA
    //
    //  Transfers data from CPU to frame buffer. If the number of halfwords to be sent is odd, an extra halfword should be sent (packets consist of 32bit units). The transfer is affected by Mask setting.
    if (gpu.gp0.fifo.count < 3) {
        return;
    }

    union COMMAND_PACKET command;
    uint32_t destination, dimensions;

    command     = pop_command_fifo(&gpu.gp0.fifo);
    destination = (uint32_t) pop_command_fifo(&gpu.gp0.fifo).value;
    dimensions  = (uint32_t) pop_command_fifo(&gpu.gp0.fifo).value;

    uint32_t x, y, w, h;
    x = (destination >>  0) & 0XFFFF;
    y = (destination >> 16) & 0XFFFF;
    w = (dimensions  >>  0) & 0XFFFF;
    h = (dimensions  >> 16) & 0XFFFF;

    x = (x % 2 == 1) ? x + 1: x; // if odd halfwords make even
    w = (w % 2 == 1) ? w + 1: w; // if odd halfwords make even

    // base address, multiplied by 2 as they are halfwords
    gpu.copy_address = y * 1024 + 2 * x; // y * vram_width + x
    gpu.copy_size    = 2 * h * w;
    gpu.iscopying = true;
}
void VRAM_TO_CPU_COPY_RECTANGLE(void) {}

void RENDER_THREE_POINT_POLYGON_MONOCHROME(bool semi_transparent) {}
void RENDER_FOUR_POINT_POLYGON_MONOCHROME(bool semi_transparent) {
    // render command needs atleast 1 + 4 words
    // for command and parameters
    if (gpu.gp0.fifo.count < 5) {
        return;
    }
    uint32_t color;
    union VERTEX v1, v2, v3, v4;

    color = pop_command_fifo(&gpu.gp0.fifo).value;
    v1.value = pop_command_fifo(&gpu.gp0.fifo).value;
    v2.value = pop_command_fifo(&gpu.gp0.fifo).value;
    v3.value = pop_command_fifo(&gpu.gp0.fifo).value;
    v4.value = pop_command_fifo(&gpu.gp0.fifo).value;
    
    printf("Rendering quad");
}
void RENDER_THREE_POINT_POLYGON_TEXTURED(bool semi_transparent, bool texture_blending) {}
void RENDER_FOUR_POINT_POLYGON_TEXTURED(bool semi_transparent, bool texture_blending) {}
void RENDER_THREE_POINT_POLYGON_SHADED(bool semi_transparent) {}
void RENDER_FOUR_POINT_POLYGON_SHADED(bool semi_transparent) {}
void RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(bool semi_transparent, bool texture_blending) {}
void RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(bool semi_transparent, bool texture_blending) {}
