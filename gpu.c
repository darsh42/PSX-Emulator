#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#define GPU_PRIVATE
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "system.h"

static struct gpu gpu;

static pthread_cond_t  gpu_dma_notify = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gpu_dma_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t  gpu_notify = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gpu_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t read_gpu( uint32_t address ) 
{ 
    uint32_t data;
    switch ( address )
    {
        case( gp0_gpu_read ): data = gpu.gpuread;       break;
        case( gp1_gpu_stat ): data = gpu.gpustat.value; break;
    }
    return data;
}

pthread_cond_t *write_gpu( uint32_t address, uint32_t data )
{
    switch ( address )
    {
        case( gp0_gpu_read ): fifo_push(&gpu.gp0, data); gpu.state = GPU_PROCESS_GP0; break;
        case( gp1_gpu_stat ): gpu.gp1 = data;            gpu.state = GPU_PROCESS_GP1; break;
    }
    return &gpu_notify;
}

void wait_gpu_gpustat_dma_data_request( void )
{
    /* whenever the flag is changed, a signal must be sent to the pthread_cond_t to notify dma */
    if ( !gpu.gpustat.dma_data_request )
    {
        assert(!pthread_cond_wait(&gpu_dma_notify, &gpu_dma_mutex));
    }
}

void wait_gpu_gpustat_dma_ready_recieve_block( void )
{
    /* whenever the flag is changed, a signal must be sent to the pthread_cond_t to notify dma */
    if ( !gpu.gpustat.ready_recieve_dma_block )
    {
        assert(!pthread_cond_wait(&gpu_dma_notify, &gpu_dma_mutex));
    }
}

// gp0 instructions
static void gp0_nop( void ) 
{
    if (!fifo_has_length(&gpu.gp0, 1))
        return;

    fifo_pop(&gpu.gp0);
}

static void vram_clear_cache( void ) {}
static void vram_fill_rectangle( void ) {}
static void vram_to_vram_copy_rectangle( void ) {}
static void cpu_to_vram_copy_rectangle( void ) {}
static void vram_to_cpu_copy_rectangle( void ) {}

static void gp0_direct_vram_access( void ) 
{
    switch (fifo_peek(&gpu.gp0))
    {
        case 0x01: vram_clear_cache(); break;
        case 0x02: vram_fill_rectangle(); break;
        case 0x80: vram_to_vram_copy_rectangle(); break;
        case 0xa0: cpu_to_vram_copy_rectangle(); break;
        case 0xc0: vram_to_cpu_copy_rectangle(); break;
    }
}
static void gp0_interrupt_request( void ) {}
static void gp0_render_polygons( void ) 
{
    switch (COMMAND(fifo_peek(&gpu.gp0))) {
        case 0X20: {
            if (!fifo_has_length(&gpu.gp0, 4))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_MONOCHROME\n");
            #endif

            uint32_t c, v1, v2, v3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 

            render_three_point_polygon_monochrome(c, v1, v2, v3, false);

            break;
        }
        case 0X22: {
            if (!fifo_has_length(&gpu.gp0, 4))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_MONOCHROME\n");
            #endif

            uint32_t c, v1, v2, v3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 

            render_three_point_polygon_monochrome(c, v1, v2, v3, true);

            break;
        }
        case 0X28: {
            if (!fifo_has_length(&gpu.gp0, 5))
                return;
            
            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_MONOCHROME\n");
            #endif

            uint32_t c, v1, v2, v3, v4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 
            v4 = fifo_pop(&gpu.gp0); 
            
            render_four_point_polygon_monochrome(c, v1, v2, v3, v4, false);

            break;
        }
        case 0X2A: {
            if (!fifo_has_length(&gpu.gp0, 5))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_MONOCHROME\n");
            #endif

            uint32_t c, v1, v2, v3, v4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 
            v4 = fifo_pop(&gpu.gp0); 

            render_four_point_polygon_monochrome(c, v1, v2, v3, v4, true);

            break;
        }
        case 0X24: {
            if (!fifo_has_length(&gpu.gp0, 7))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0);
                                            
            render_three_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, false, true);

            break;
        }
        case 0X25: {
            if (!fifo_has_length(&gpu.gp0, 7))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0);

            render_three_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, false, false);

            break;
        }
        case 0X26: {
            if (!fifo_has_length(&gpu.gp0, 7))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0);

            render_three_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, true, true);

            break;
        }
        case 0X27: {
            if (!fifo_has_length(&gpu.gp0, 7))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0);

            render_three_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, true, false);

            break;
        }
        case 0X2C: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            t4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, v4, t4, false, true);

            break;
        }
        case 0X2D: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            t4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, v4, t4, false, false);

            break;
        }
        case 0X2E: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            t4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, v4, t4, true, true);

            break;
        }
        case 0X2F: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;
            
            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            #endif
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            t2 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            t4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_textured(c, v1, t1, v2, t2, v3, t3, v4, t4, true, false);

            break;
        }
        case 0X30: {
            if (!fifo_has_length(&gpu.gp0, 6))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_SHADED\n");
            #endif

            uint32_t c1, v1, v2, c2, v3, c3;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); 
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 

            render_three_point_polygon_shaded(c1, v1, c2, v2, c3, v3, false);

            break;
        }
        case 0X32: {
            if (!fifo_has_length(&gpu.gp0, 6))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_SHADED\n");
            #endif

            uint32_t c1, v1, v2, c2, v3, c3;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); 
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); 

            render_three_point_polygon_shaded(c1, v1, c2, v2, c3, v3, true);

            break;
        }
        case 0X38: {
            if (!fifo_has_length(&gpu.gp0, 8))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_SHADED\n");
            #endif

            uint32_t c1, c2, c3, c4, v1, v2, v3, v4;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_shaded(c1, v1, c2, v2, c3, v3, c4, v4, false);

            break;
        }
        case 0X3A: {
            if (!fifo_has_length(&gpu.gp0, 8))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_SHADED\n");
            #endif

            uint32_t c1, c2, c3, c4, v1, v2, v3, v4;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);

            render_four_point_polygon_shaded(c1, v1, c2, v2, c3, v3, c4, v4, true);

            break;
        }
        case 0X34: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED\n");
            #endif
            
            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0);
            v2 = fifo_pop(&gpu.gp0); t2 = fifo_pop(&gpu.gp0); c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            t3 = fifo_pop(&gpu.gp0);

            render_three_point_polygon_shaded_textured(c1, v1, t1, c2, v2, t2, c3, v3, t3, false, true);

            break;
        }
        case 0X36: {
            if (!fifo_has_length(&gpu.gp0, 9))
                return;

            #ifdef DEBUG
            printf("RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED\n");
            #endif
            
            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0);
            v2 = fifo_pop(&gpu.gp0); t2 = fifo_pop(&gpu.gp0); c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            t3 = fifo_pop(&gpu.gp0);

            render_three_point_polygon_shaded_textured(c1, v1, t1, c2, v2, t2, c3, v3, t3, true, true);

            break;
        }
        case 0X3C: {
            if (!fifo_has_length(&gpu.gp0, 12))
                return;
            
            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED\n");
            #endif

            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3, c4, v4, t4;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0);
            v2 = fifo_pop(&gpu.gp0); t2 = fifo_pop(&gpu.gp0); c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            t3 = fifo_pop(&gpu.gp0); c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0); t4 = fifo_pop(&gpu.gp0);
            

            render_four_point_polygon_shaded_textured(c1, v1, t1, c2, v2, t2, c3, v3, t3, c4, v4, t4, false, true);

            break;
        }
        case 0X3E: {
            if (!fifo_has_length(&gpu.gp0, 12))
                return;

            #ifdef DEBUG
            printf("RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED\n");
            #endif

            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3, c4, v4, t4;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1 = fifo_pop(&gpu.gp0); c2 = fifo_pop(&gpu.gp0);
            v2 = fifo_pop(&gpu.gp0); t2 = fifo_pop(&gpu.gp0); c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            t3 = fifo_pop(&gpu.gp0); c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0); t4 = fifo_pop(&gpu.gp0);
            

            render_four_point_polygon_shaded_textured(c1, v1, t1, c2, v2, t2, c3, v3, t3, c4, v4, t4, true, true);

            break;
        }
    }
}
static void gp0_render_lines( void ) {}
static void gp0_render_rectangles( void ) {}
static void gp0_rendering_attributes( void ) 
{   
    uint32_t value = fifo_pop(&gpu.gp0);
    // pop current command as it doesnt need more arguments
    switch (COMMAND(value) & 0b1111) {
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
            gpu.gpustat.texture_page_x_base  = (PARAMETER(value) >>  0) & 0xf;
            gpu.gpustat.texture_page_y_base  = (PARAMETER(value) >>  4) & 0x1;
            gpu.gpustat.semi_transparency    = (PARAMETER(value) >>  5) & 0x3;
            gpu.gpustat.texture_page_colors  = (PARAMETER(value) >>  7) & 0x3;
            gpu.gpustat.dither               = (PARAMETER(value) >>  9) & 0x1;
            gpu.gpustat.draw_to_display_area = (PARAMETER(value) >> 10) & 0x1;
            gpu.gpustat.texture_disable      = (PARAMETER(value) >> 11) & 0x1;
            gpu.texture_rectangle_x_flip     = (PARAMETER(value) >> 12) & 0x1;
            gpu.texture_rectangle_y_flip     = (PARAMETER(value) >> 13) & 0x1;
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
            gpu.texture_window_mask_x   = (PARAMETER(value) >>  0) & 0x1f;
            gpu.texture_window_mask_y   = (PARAMETER(value) >>  5) & 0x1f;
            gpu.texture_window_offset_x = (PARAMETER(value) >> 10) & 0x1f;
            gpu.texture_window_offset_y = (PARAMETER(value) >> 15) & 0x1f;
            break;
        }
        case 0X03: { 
            /* SET DRAWING AREA TOP LEFT */
            //  0-9    X-coordinate (0..1023)
            //  10-19  Y-coordinate (0..1023)  
            //  20-23  Not used (zero)        
            //  24-31  Command  (Exh)
            //
            //  Sets the drawing area corners. The Render commands GP0(20h..7Fh) are automatically clipping any pixels that are outside of this region.
            gpu.drawing_area_left = (PARAMETER(value) >>  0) & 0x3ff;
            gpu.drawing_area_top  = (PARAMETER(value) >> 10) & 0x3ff;
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
            gpu.drawing_area_right   = (PARAMETER(value) >>  0) & 0x3ff;
            gpu.drawing_area_bottom  = (PARAMETER(value) >> 10) & 0x1ff;
            break;
        }
        case 0X05: {
            // 0-10   X-offset (-1024..+1023) (usually within X1,X2 of Drawing Area)
            // 11-21  Y-offset (-1024..+1023) (usually within Y1,Y2 of Drawing Area)
            // 22-23  Not used (zero)
            // 24-31  Command  (E5h)
            int16_t x = ((PARAMETER(value) >>  0) & 0x7ff) << 5; // forcing sign extension
            int16_t y = ((PARAMETER(value) >> 11) & 0x7ff) << 5; // forcing sign extension

            gpu.drawing_offset_x = x >> 5;
            gpu.drawing_offset_y = y >> 5;
            break;
        }
        case 0X06: {
            /* MASK BIT SETTING */
            // 0     Set mask while drawing (0=TextureBit15, 1=ForceBit15=1)   ;GPUSTAT.11
            // 1     Check mask before draw (0=Draw Always, 1=Draw if Bit15=0) ;GPUSTAT.12
            // 2-23  Not used (zero)
            // 24-31 Command  (E6h)
            gpu.gpustat.set_mask_when_drawing = (PARAMETER(value) >> 0) & 0x1;
            gpu.gpustat.draw_pixels           = (PARAMETER(value) >> 1) & 0x1;
            break;
        }
    }
}

// gp1 instructions
static void gp1_reset( void ) 
{
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
    fifo_reset(&gpu.gp0);

    gpu.gpustat.value = 0X14802000;
    
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
static void gp1_reset_command_buffer( void ) 
{
    // 0-23  Not used (zero)
    // Clears the command FIFO, and aborts the current rendering command 
    // (eg. this may end up with an incompletely drawn triangle).
    
}
static inline void gp1_acknowledge_interrupt( void ) {}
static inline void gp1_display_enable( void ) 
{
    // 0     Display On/Off   (0=On, 1=Off)                         ;GPUSTAT.23
    // 1-23  Not used (zero)
    gpu.gpustat.display_enable = PARAMETER(gpu.gp1) & 0x1;
}
static inline void gp1_dma_direction_or_data_request( void ) 
{
    // 0-1  DMA Direction (0=Off, 1=FIFO, 2=CPUtoGP0, 3=GPUREADtoCPU) ;GPUSTAT.29-30
    // 2-23 Not used (zero)
    gpu.gpustat.dma_direction = PARAMETER(gpu.gp1) & 0b11;
    switch (PARAMETER(gpu.gp1) & 0b11) {
        case 0: gpu.gpustat.dma_data_request = 0; break;
        case 1: gpu.gpustat.dma_data_request = !fifo_full(&gpu.gp0); break;
        case 2: gpu.gpustat.dma_data_request = gpu.gpustat.ready_recieve_dma_block; break;
        case 3: gpu.gpustat.dma_data_request = gpu.gpustat.ready_send_vram_cpu; break;
    }

    assert(!pthread_cond_signal(&gpu_dma_notify));
}
static inline void gp1_start_of_display_area_in_vram( void ) 
{
    // 0-9   X (0-1023)    (halfword address in VRAM)  (relative to begin of VRAM)
    // 10-18 Y (0-511)     (scanline number in VRAM)   (relative to begin of VRAM)
    // 19-23 Not used (zero)
    //
    // Upper/left Display source address in VRAM. The size and target position on 
    // screen is set via Display Range registers; target=X1,Y2; size=(X2-X1/cycles_per_pix), (Y2-Y1).
    gpu.display_vram_x_start = (PARAMETER(gpu.gp1) >>  0) & 0x3ff;
    gpu.display_vram_y_start = (PARAMETER(gpu.gp1) >> 10) & 0x1ff;
}
static inline void gp1_horiontal_display_range( void ) 
{
    // 0-11   X1 (260h+0)       ;12bit       ;\counted in 53.222400MHz units,
    // 12-23  X2 (260h+320*8)   ;12bit       ;/relative to HSYNC
    gpu.display_horizontal_start = (PARAMETER(gpu.gp1) >>  0) & 0xfff;
    gpu.display_horizontal_end   = (PARAMETER(gpu.gp1) >> 12) & 0xfff;
}
static inline void gp1_vertical_display_range( void ) 
{
    // 0-9   Y1 (NTSC=88h-(224/2), (PAL=A3h-(264/2))  ;\scanline numbers on screen,
    // 10-19 Y2 (NTSC=88h+(224/2), (PAL=A3h+(264/2))  ;/relative to VSYNC
    // 20-23 Not used (zero)
    gpu.display_vertical_start = (PARAMETER(gpu.gp1) >>  0) & 0x3ff;
    gpu.display_vertical_end   = (PARAMETER(gpu.gp1) >> 10) & 0x3ff;
}
static inline void gp1_display_mode( void ) 
{
    // 0-1   Horizontal Resolution 1     (0=256, 1=320, 2=512, 3=640) ;GPUSTAT.17-18
    // 2     Vertical Resolution         (0=240, 1=480, when Bit5=1)  ;GPUSTAT.19
    // 3     Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)    ;GPUSTAT.20
    // 4     Display Area Color Depth    (0=15bit, 1=24bit)           ;GPUSTAT.21
    // 5     Vertical Interlace          (0=Off, 1=On)                ;GPUSTAT.22
    // 6     Horizontal Resolution 2     (0=256/320/512/640, 1=368)   ;GPUSTAT.16
    // 7     "Reverseflag"               (0=Normal, 1=Distorted)      ;GPUSTAT.14
    // 8-23  Not used (zero)
    gpu.gpustat.horizontal_resolution_1  = (PARAMETER(gpu.gp1) >> 0) & 0x3;
    gpu.gpustat.vertical_resolution      = (PARAMETER(gpu.gp1) >> 2) & 0x1;
    gpu.gpustat.video_mode               = (PARAMETER(gpu.gp1) >> 3) & 0x1;
    gpu.gpustat.display_area_color_depth = (PARAMETER(gpu.gp1) >> 4) & 0x1;
    gpu.gpustat.vertical_interlace       = (PARAMETER(gpu.gp1) >> 5) & 0x1;
    gpu.gpustat.horizontal_resolution_2  = (PARAMETER(gpu.gp1) >> 6) & 0x1;
    gpu.gpustat.reverse_flag             = (PARAMETER(gpu.gp1) >> 7) & 0x1;
}
static inline void gp1_new_texture_disable( void ) {}
static inline void gp1_special_or_prototype_texture_disable( void ) {}
static inline void gp1_display_info( void ) {}

void gpu_process_gp0( void )
{
    switch ( COMMAND(fifo_peek(&gpu.gp0)) )
    {
        case 0X00: gp0_nop(); break;
        case 0X01: 
        case 0X02: 
        case 0X80: 
        case 0XA0: 
        case 0XC0: gp0_direct_vram_access(); break;
        case 0X1F: gp0_interrupt_request(); break;
        case 0X03: break;
        default:
            switch ( COMMAND(fifo_peek(&gpu.gp0)) >> 4 ) 
            {
                case 0x02: case 0x03: gp0_render_polygons(); break;
                case 0x04: case 0x05: gp0_render_lines(); break;
                case 0x06: case 0x07: gp0_render_rectangles(); break;
                case 0x0e:            gp0_rendering_attributes(); break;
                default:
                    break;
            }
            break;
    }
}

void gpu_process_gp1( void )
{
    switch ( COMMAND(gpu.gp1) ) {
        case 0x00: gp1_reset(); break;
        case 0x01: gp1_reset_command_buffer(); break;
        case 0x02: gp1_acknowledge_interrupt(); break;
        case 0x03: gp1_display_enable(); break;
        case 0x04: gp1_dma_direction_or_data_request(); break;
        case 0x05: gp1_start_of_display_area_in_vram(); break;
        case 0x06: gp1_horiontal_display_range(); break;
        case 0x07: gp1_vertical_display_range(); break;
        case 0x08: gp1_display_mode(); break;
        case 0x09: gp1_new_texture_disable(); break;
        case 0x20: break;
        default:
            if (COMMAND(gpu.gp1) >> 4 == 0x01) 
                gp1_display_info();

            break;
    }
}

void gpu_render_frame( void )
{
}

/* only used if external devices need to notify gpu without writing to it */
void unlock_gpu( void )
{
    assert(!pthread_cond_signal(&gpu_notify));
}

void *task_gpu( void *ignore )
{
    printf("GPU DEVICE: %ld\n", pthread_self());
    
    fifo_create( &gpu.gp0, 12 );

    while (running)
    {
        switch (gpu.state)
        {
            case GPU_IDLE:        
                assert(!pthread_cond_wait(&gpu_notify, &gpu_mutex)); 
                break;
            case GPU_RENDERING:   gpu_render_frame(); break;
            case GPU_PROCESS_GP0: gpu_process_gp0();  break;
            case GPU_PROCESS_GP1: gpu_process_gp1();  break;
        }
    }
}
