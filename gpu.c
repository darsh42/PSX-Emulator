#include <assert.h>
#include <stdio.h>

#define GPU_PRIVATE
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "system.h"

static struct gpu gpu;

uint32_t read_gpu( uint32_t address ) 
{ 
    uint32_t data;
    switch ( address )
    {
        case( gp0_gpu_read ): data = gpu.gpuread;       break;
        case( gp1_gpu_stat ): data = gpu.gpustat.value; break;
    }

    TRACE_DEVMEM("gpu.c", "read_gpu ", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_gpu( uint32_t address, uint32_t data )
{
    switch ( address )
    {
        case( gp0_gpu_read ): fifo_push(&gpu.gp0, data); break;
        case( gp1_gpu_stat ): gpu.gp1 = data;            gpu.state = GPU_PROCESS_GP1; break;
    }

    // TRACE_DEVMEM("gpu.c", "write_gpu ", "address: %08x | data: %08x\n", address, data);
}

/* if gpu is transferring data to or from vram it computes next address */
uint32_t gpu_get_vram_address( void )
{
    // #error BUG: computing correct vram memory addresses
    assert(gpu.gpustat.ready_send_vram_cpu);
    
    /* compute vram address */
    uint32_t address = 2 * ((gpu.vram_direct_access_y + gpu.vram_direct_access_cy) * VRAM_WIDTH +
                            (gpu.vram_direct_access_x + gpu.vram_direct_access_cx));
    
    /* increment count by number of pixels */
    gpu.vram_direct_access_cx += 2;

    if (gpu.vram_direct_access_cx == gpu.vram_direct_access_w)
    {
        gpu.vram_direct_access_cy++;
        gpu.vram_direct_access_cx = 0;
        
        if (gpu.vram_direct_access_cy == gpu.vram_direct_access_h)
        {
            /* if the address equal to the max coordinate end the transfer */
            gpu.gpustat.ready_send_vram_cpu = 0;
        }
    }
    
    return address;
}

/* external status functions */
void gpu_notify_dma_block_end( void ) { gpu.gpustat.ready_recieve_dma_block = 0; }

bool gpu_gpustat_dma_data_request       ( void ) { return (gpu.gpustat.dma_data_request);       }
bool gpu_gpustat_ready_send_vram_cpu    ( void ) { return (gpu.gpustat.ready_send_vram_cpu);    }
bool gpu_gpustat_dma_ready_recieve_block( void ) { return (gpu.gpustat.ready_recieve_dma_block); }

bool gpu_hblank( void ) { return (gpu.hblank); }
bool gpu_vblank( void ) { return (gpu.vblank); }

// gp0 instructions
static void gp0_nop( void ) { TRACE_GPU("gp0_nop", "command: %08x\n", fifo_pop(&gpu.gp0)); }
static void gp0_direct_vram_access( void ) 
{
    /* BUG: possible issue when transferring using non-dma */
    switch (COMMAND(fifo_peek(&gpu.gp0)))
    {
        case 0x01: /* vram clear vram */
        case 0x02: /* fill rectangle in vram */
        case 0x80: /* copy vram to vram */
            /* pop command */
            TRACE_GPU("gp0_direct_vram_access", "command: %08x\n", fifo_pop(&gpu.gp0));
            break;
        case 0xa0: /* copy cpu to vram */
        {
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            
            /* pop command */
            TRACE_GPU("gp0_direct_vram_access", "command: %08x\n", fifo_pop(&gpu.gp0));

            uint32_t destination = fifo_pop(&gpu.gp0);
            uint32_t dimensions  = fifo_pop(&gpu.gp0);
            
            /* set the conditions */ 
            gpu.vram_direct_access_x = (destination >>  0) & 0xffff;
            gpu.vram_direct_access_y = (destination >> 16) & 0xffff;
            gpu.vram_direct_access_w = (dimensions  >>  0) & 0xffff;
            gpu.vram_direct_access_h = (dimensions  >> 16) & 0xffff;
            
            /* set the counter to 0 */
            gpu.vram_direct_access_cx = 0;
            gpu.vram_direct_access_cy = 0;

            /* set the direction to cpu to vram */
            gpu.vram_direct_access_d = 0;

            /* set the gpu state */
            gpu.gpustat.ready_send_vram_cpu = 1;
            break;
        }
        case 0xc0: /* copy vram to cpu */
        {
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            
            /* pop command */
            TRACE_GPU("gp0_direct_vram_access", "command: %08x\n", fifo_pop(&gpu.gp0));

            uint32_t destination = fifo_pop(&gpu.gp0);
            uint32_t dimensions  = fifo_pop(&gpu.gp0);
            
            /* set the conditions */ 
            gpu.vram_direct_access_x = (destination >>  0) & 0xffff;
            gpu.vram_direct_access_y = (destination >> 16) & 0xffff;
            gpu.vram_direct_access_w = (dimensions  >>  0) & 0xffff;
            gpu.vram_direct_access_h = (dimensions  >> 16) & 0xffff;
            
            /* set the counter to 0 */
            gpu.vram_direct_access_cx = 0;
            gpu.vram_direct_access_cy = 0;

            /* set the direction to vram to cpu */
            gpu.vram_direct_access_d = 1;

            /* set the gpu state */
            gpu.gpustat.ready_send_vram_cpu = 1;
            break;
        }
    }
}
static void gp0_interrupt_request( void ) {}
static void gp0_render_polygons( void ) 
{
    uint32_t c1,      c2,      c3, c4;
    uint32_t v1,      v2,      v3, v4;
    uint32_t t1_clut, t2_page, t3, t4;

    switch (COMMAND(fifo_peek(&gpu.gp0))) {

        /* 
         * Monochrome Polygon
         *    1st  Color+Command     (CcBbGgRrh)
         *    2nd  Vertex1           (YyyyXxxxh)
         *    3rd  Vertex2           (YyyyXxxxh)
         *    4th  Vertex3           (YyyyXxxxh)
         *   (5th) Vertex4           (YyyyXxxxh) (if any)
         */

        case 0X20: // GP0(20h) - Monochrome three-point polygon, opaque
            if (!fifo_has_length(&gpu.gp0, 4)) return;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            v3 = fifo_pop(&gpu.gp0);

            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);

            render_three_point_polygon_monochrome(c1, v1,
                                                      v2,
                                                      v3,
                                                  false);
            break;
        case 0X22: // GP0(22h) - Monochrome three-point polygon, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 4)) return;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            v3 = fifo_pop(&gpu.gp0);

            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);

            render_three_point_polygon_monochrome(c1, v1,
                                                      v2,
                                                      v3,
                                                  true);
            break;
        case 0X28: // GP0(28h) - Monochrome four-point polygon, opaque
            if (!fifo_has_length(&gpu.gp0, 5)) return;

            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            v3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);

            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);

            render_four_point_polygon_monochrome(c1, v1, 
                                                     v2, 
                                                     v3, 
                                                     v4, 
                                                 false);
            break;
        case 0X2A: // GP0(2Ah) - Monochrome four-point polygon, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 5))
                return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            v3 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);

            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);

            render_four_point_polygon_monochrome(c1, v1, 
                                                     v2, 
                                                     v3, 
                                                     v4, 
                                                 true);
            break;

        /* 
         * Textured Polygon
         *   1st  Color+Command     (CcBbGgRrh) (color is ignored for raw-textures)
         *   2nd  Vertex1           (YyyyXxxxh)
         *   3rd  Texcoord1+Palette (ClutYyXxh)
         *   4th  Vertex2           (YyyyXxxxh)
         *   5th  Texcoord2+Texpage (PageYyXxh)
         *   6th  Vertex3           (YyyyXxxxh)
         *   7th  Texcoord3         (0000YyXxh)
         *  (8th) Vertex4           (YyyyXxxxh) (if any)
         *  (9th) Texcoord4         (0000YyXxh) (if any)
         */

        case 0X24: // GP0(24h) - Textured three-point polygon, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 7)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_textured(c1, v1, t1_clut,
                                                    v2, t2_page,
                                                    v3, t3,
                                                false, true);
            break;
        case 0X25: // GP0(25h) - Textured three-point polygon, opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 7)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_textured(c1, v1, t1_clut,
                                                    v2, t2_page,
                                                    v3, t3,
                                                false, false);
            break;
        case 0X26: // GP0(26h) - Textured three-point polygon, semi-transparent, texture-blending
            if (!fifo_has_length(&gpu.gp0, 7)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_textured(c1, v1, t1_clut,
                                                    v2, t2_page,
                                                    v3, t3,
                                                true, true);
            break;
        case 0X27: // GP0(27h) - Textured three-point polygon, semi-transparent, raw-texture
            if (!fifo_has_length(&gpu.gp0, 7)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_textured(c1, v1, t1_clut,
                                                    v2, t2_page,
                                                    v3, t3,
                                                true, false);
            break;
        case 0X2C: // GP0(2Ch) - Textured four-point polygon, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
                                     v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_textured(c1, v1, t1_clut,
                                                   v2, t2_page,
                                                   v3, t3,
                                                   v4, t4,
                                                false, true);
            break;
        case 0X2D: // GP0(2Dh) - Textured four-point polygon, opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
                                     v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_textured(c1, v1, t1_clut,
                                                   v2, t2_page,
                                                   v3, t3,
                                                   v4, t4,
                                                false, false);
            break;
        case 0X2E: // GP0(2Eh) - Textured four-point polygon, semi-transparent, texture-blending
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
                                     v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_textured(c1, v1, t1_clut,
                                                   v2, t2_page,
                                                   v3, t3,
                                                   v4, t4,
                                                true, true);
            break;
        case 0X2F: // GP0(2Fh) - Textured four-point polygon, semi-transparent, raw-texture
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
                                     v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0); 
                                     v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
                                     v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_textured(c1, v1, t1_clut,
                                                   v2, t2_page,
                                                   v3, t3,
                                                   v4, t4,
                                                true, false);
            break;

        /* 
         * Shaded Polygon
         *   1st  Color1+Command    (CcBbGgRrh)
         *   2nd  Vertex1           (YyyyXxxxh)
         *   3rd  Color2            (00BbGgRrh)
         *   4th  Vertex2           (YyyyXxxxh)
         *   5th  Color3            (00BbGgRrh)
         *   6th  Vertex3           (YyyyXxxxh)
         *  (7th) Color4            (00BbGgRrh) (if any)
         *  (8th) Vertex4           (YyyyXxxxh) (if any)
         */

        case 0X30: // GP0(30h) - Shaded three-point polygon, opaque
            if (!fifo_has_length(&gpu.gp0, 6)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_shaded(c1, v1,
                                              c2, v2,
                                              c3, v3,
                                              false);
            break;
        case 0X32: // GP0(32h) - Shaded three-point polygon, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 6)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_shaded(c1, v1,
                                              c2, v2,
                                              c3, v3,
                                              true);
            break;
        case 0X38: // GP0(38h) - Shaded four-point polygon, opaque
            if (!fifo_has_length(&gpu.gp0, 8)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_shaded(c1, v1,
                                             c2, v2,
                                             c3, v3,
                                             c4, v4,
                                             false);
            break;
        case 0X3A: // GP0(3Ah) - Shaded four-point polygon, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 8)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0);
            c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_shaded(c1, v1,
                                             c2, v2,
                                             c3, v3,
                                             c4, v4,
                                             true);
            break;

        /* 
         * Shaded Textured Polygon 
         *   1st  Color1+Command    (CcBbGgRrh)
         *   2nd  Vertex1           (YyyyXxxxh)
         *   3rd  Texcoord1+Palette (ClutYyXxh)
         *   4th  Color2            (00BbGgRrh)
         *   5th  Vertex2           (YyyyXxxxh)
         *   6th  Texcoord2+Texpage (PageYyXxh)
         *   7th  Color3            (00BbGgRrh)
         *   8th  Vertex3           (YyyyXxxxh)
         *   9th  Texcoord3         (0000YyXxh)
         *  (10th) Color4           (00BbGgRrh) (if any)
         *  (11th) Vertex4          (YyyyXxxxh) (if any)
         *  (12th) Texcoord4        (0000YyXxh) (if any)
         */

        case 0X34: // GP0(34h) - Shaded Textured three-point polygon, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_shaded_textured(c1, v1, t1_clut,
                                                       c2, v2, t2_page,
                                                       c3, v3, t3,
                                                       false, true);
            break;
        case 0X36: // GP0(36h) - Shaded Textured three-point polygon, semi-transparent, tex-blend
            if (!fifo_has_length(&gpu.gp0, 9)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_three_point_polygon_shaded_textured(c1, v1, t1_clut,
                                                       c2, v2, t2_page,
                                                       c3, v3, t3,
                                                       true, true);
            break;
        case 0X3C: // GP0(3Ch) - Shaded Textured four-point polygon, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 12)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_shaded_textured(c1, v1, t1_clut,
                                                      c2, v2, t2_page,
                                                      c3, v3, t3,
                                                      c4, v4, t4,
                                                      false, true);
            break;
        case 0X3E: // GP0(3Eh) - Shaded Textured four-point polygon, semi-transparent, tex-blend
            if (!fifo_has_length(&gpu.gp0, 12)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); t1_clut = fifo_pop(&gpu.gp0);
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0); t2_page = fifo_pop(&gpu.gp0);
            c3 = fifo_pop(&gpu.gp0); v3 = fifo_pop(&gpu.gp0); t3      = fifo_pop(&gpu.gp0);
            c4 = fifo_pop(&gpu.gp0); v4 = fifo_pop(&gpu.gp0); t4      = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_polygon", "command: %08x\n", c1);
            render_four_point_polygon_shaded_textured(c1, v1, t1_clut,
                                                      c2, v2, t2_page,
                                                      c3, v3, t3,
                                                      c4, v4, t4,
                                                      true, true);
            break;
    }
}
static void gp0_render_lines( void ) 
{
    uint32_t c1, c2;
    uint32_t v1, v2;
    switch(COMMAND(fifo_peek(&gpu.gp0)))
    {
        // Monochrome Line
        //   1st   Color+Command     (CcBbGgRrh)
        //   2nd   Vertex1           (YyyyXxxxh)
        //   3rd   Vertex2           (YyyyXxxxh)
        //  (...)  VertexN           (YyyyXxxxh) (poly-line only)
        //  (Last) Termination Code  (55555555h) (poly-line only)

        case 0x40: // GP0(40h) - Monochrome line, opaque
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_line", "command: %08x\n", c1);
            render_line_monochrome(c1, v1, v2, false);
            break;
        case 0x42: // GP0(42h) - Monochrome line, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_line", "command: %08x\n", c1);
            render_line_monochrome(c1, v1, v2, true);
            break;
        case 0x48: // GP0(48h) - Monochrome Poly-line, opaque
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            TRACE_GPU("gpu_render_line", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_polyline_monochrome(gpu.gp0, false);
            break;
        case 0x4A: // GP0(4Ah) - Monochrome Poly-line, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            TRACE_GPU("gpu_render_line", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_polyline_monochrome(gpu.gp0, true);
            break;

        // Shaded Line
        //   1st   Color1+Command    (CcBbGgRrh)
        //   2nd   Vertex1           (YyyyXxxxh)
        //   3rd   Color2            (00BbGgRrh)
        //   4th   Vertex2           (YyyyXxxxh)
        //  (...)  ColorN            (00BbGgRrh) (poly-line only)
        //  (...)  VertexN           (YyyyXxxxh) (poly-line only)
        //  (Last) Termination Code  (55555555h) (poly-line only)

        case 0x50: // GP0(50h) - Shaded line, opaque
            if (!fifo_has_length(&gpu.gp0, 4)) return;
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); 
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_line", "command: %08x\n", c1);
            render_line_monochrome(c1, v1, 
                                   c2, v2, 
                                   false);
            break;
        case 0x52: // GP0(52h) - Shaded line, semi-transparent
            c1 = fifo_pop(&gpu.gp0); v1 = fifo_pop(&gpu.gp0); 
            c2 = fifo_pop(&gpu.gp0); v2 = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_line", "command: %08x\n", c1);
            render_line_monochrome(c1, v1, 
                                   c2, v2, 
                                   true);
            break;
        case 0x58: // GP0(58h) - Shaded Poly-line, opaque
            if (!fifo_has_length(&gpu.gp0, 4)) return;
            TRACE_GPU("gpu_render_line", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_polyline_shaded(gpu.gp0, false);
            break;
        case 0x5A: // GP0(5Ah) - Shaded Poly-line, semi-transparent
            if (!fifo_has_length(&gpu.gp0, 4)) return;
            TRACE_GPU("gpu_render_line", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_polyline_shaded(gpu.gp0, true);
            break;
    }
}
static void gp0_render_rectangles( void ) 
{
    /* 
     * FIXME: rendering parameters need to be stored into variables and then
     * can be passed to rendering functions 
     */
    uint32_t c, v, t_clut, s;
    switch (COMMAND(fifo_peek(&gpu.gp0)))
    {
        /* Monochrome 
         *   1st  Color+Command     (CcBbGgRrh)
         *   2nd  Vertex            (YyyyXxxxh)
         *  (3rd) Width+Height      (YsizXsizh) (variable size only) (max 1023x511) */

        case 0x60: // GP0(60h) - Monochrome Rectangle (variable size) (opaque)
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, false);
            break;
        case 0x62: // GP0(62h) - Monochrome Rectangle (variable size) (semi-transparent)
            if (!fifo_has_length(&gpu.gp0, 3)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = fifo_pop(&gpu.gp0);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, true);
            break;
        case 0x68: // GP0(68h) - Monochrome Rectangle (1x1) (Dot) (opaque)
            if (!fifo_has_length(&gpu.gp0, 2)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = PACK_RECT_SIZE(1, 1);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, false);
            break;
        case 0x6A: // GP0(6Ah) - Monochrome Rectangle (1x1) (Dot) (semi-transparent)
            if (!fifo_has_length(&gpu.gp0, 2)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = PACK_RECT_SIZE(1, 1);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, true);
            break;
        case 0x70: // GP0(70h) - Monochrome Rectangle (8x8) (opaque)
            if (!fifo_has_length(&gpu.gp0, 2)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = PACK_RECT_SIZE(8, 8);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, false);
            break;
        case 0x72: // GP0(72h) - Monochrome Rectangle (8x8) (semi-transparent)
            if (!fifo_has_length(&gpu.gp0, 2)) return;
            c = fifo_pop(&gpu.gp0); v = fifo_pop(&gpu.gp0); s = PACK_RECT_SIZE(8, 8);
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", c);
            render_rectangle_monochrome(c, v, s, true);
            break;
        case 0x78: // GP0(78h) - Monochrome Rectangle (16x16) (opaque)
            if (!fifo_has_length(&gpu.gp0, 2))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_monochrome(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    PACK_RECT_SIZE(16, 16), false);
            break;
        case 0x7A: // GP0(7Ah) - Monochrome Rectangle (16x16) (semi-transparent)
            if (!fifo_has_length(&gpu.gp0, 2))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_monochrome(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    PACK_RECT_SIZE(16, 16), true);
            break;

        /* Textured
         *  1st  Color+Command     (CcBbGgRrh) (color is ignored for raw-textures)
         *  2nd  Vertex            (YyyyXxxxh) (upper-left edge of the rectangle)
         *  3rd  Texcoord+Palette  (ClutYyXxh) (for 4bpp Textures Xxh must be even!)
         * (4th) Width+Height      (YsizXsizh) (variable size only) (max 1023x511) */

        case 0x64: // GP0(64h) - Textured Rectangle, variable size, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 4))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0),
                    false, true);
            break;
        case 0x65: // GP0(65h) - Textured Rectangle, variable size, opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 4))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0),
                    false, false);
            break;
        case 0x66: // GP0(66h) - Textured Rectangle, variable size, semi-transp, texture-blending
            if (!fifo_has_length(&gpu.gp0, 4))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0),
                    true, true);
            break;
        case 0x67: // GP0(67h) - Textured Rectangle, variable size, semi-transp, raw-texture
            if (!fifo_has_length(&gpu.gp0, 4))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0),
                    true, false);
            break;
        case 0x6C: // GP0(6Ch) - Textured Rectangle, 1x1 (nonsense), opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(1, 1),
                    false, true);
            break;
        case 0x6D: // GP0(6Dh) - Textured Rectangle, 1x1 (nonsense), opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(1, 1),
                    false, false);
            break;
        case 0x6E: // GP0(6Eh) - Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(1, 1),
                    true, true);
            break;
        case 0x6F: // GP0(6Fh) - Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(1, 1),
                    true, false);
            break;
        case 0x74: // GP0(74h) - Textured Rectangle, 8x8, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(8, 8),
                    false, true);
            break;
        case 0x75: // GP0(75h) - Textured Rectangle, 8x8, opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(8, 8),
                    false, false);
            break;
        case 0x76: // GP0(76h) - Textured Rectangle, 8x8, semi-transparent, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(8, 8),
                    true, true);
            break;
        case 0x77: // GP0(77h) - Textured Rectangle, 8x8, semi-transparent, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(8, 8),
                    true, false);
            break;
        case 0x7C: // GP0(7Ch) - Textured Rectangle, 16x16, opaque, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(16, 16),
                    false, true);
            break;
        case 0x7D: // GP0(7Dh) - Textured Rectangle, 16x16, opaque, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(16, 16),
                    false, false);
            break;
        case 0x7E: // GP0(7Eh) - Textured Rectangle, 16x16, semi-transparent, texture-blending
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(16, 16),
                    true, true);
            break;
        case 0x7F: // GP0(7Fh) - Textured Rectangle, 16x16, semi-transparent, raw-texture
            if (!fifo_has_length(&gpu.gp0, 3))
                return;
            TRACE_GPU("gpu_render_rectangle", "command: %08x\n", fifo_peek(&gpu.gp0));
            render_rectangle_textured(
                    fifo_pop(&gpu.gp0), fifo_pop(&gpu.gp0), 
                    fifo_pop(&gpu.gp0), PACK_RECT_SIZE(16, 16),
                    true, false);
            break;
    }
}
static void gp0_rendering_attributes( void ) 
{   
    TRACE_GPU("gp0_rendering_attributes", "command: %08x\n", fifo_peek(&gpu.gp0));

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
    TRACE_GPU("gp1_reset", "command: %08x\n", gpu.gp1);
    
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
    TRACE_GPU("gp1_reset_command_buffer", "command: %08x\n", gpu.gp1);
    fifo_reset(&gpu.gp0);
}
static inline void gp1_acknowledge_interrupt( void ) 
{
    TRACE_GPU("gp1_acknowledge_interrupt", "command: %08x\n", gpu.gp1);
}
static inline void gp1_display_enable( void ) 
{
    // 0     Display On/Off   (0=On, 1=Off)                         ;GPUSTAT.23
    // 1-23  Not used (zero)
    TRACE_GPU("gp1_display_enable", "command: %08x\n", gpu.gp1);
    gpu.gpustat.display_enable = PARAMETER(gpu.gp1) & 0x1;
}
static inline void gp1_dma_direction_or_data_request( void ) 
{
    // 0-1  DMA Direction (0=Off, 1=FIFO, 2=CPUtoGP0, 3=GPUREADtoCPU) ;GPUSTAT.29-30
    // 2-23 Not used (zero)
    TRACE_GPU("gp1_dma_direction_or_data_request", "command: %08x\n", gpu.gp1);
    gpu.gpustat.dma_direction = PARAMETER(gpu.gp1) & 0b11;
    switch (PARAMETER(gpu.gp1) & 0b11) 
    {
        case 0: gpu.gpustat.dma_data_request = 0; break;
        case 1: gpu.gpustat.dma_data_request = !fifo_full(&gpu.gp0); break;
        case 2: gpu.gpustat.dma_data_request = gpu.gpustat.ready_recieve_dma_block; break;
        case 3: gpu.gpustat.dma_data_request = gpu.gpustat.ready_send_vram_cpu; break;
    }
}
static inline void gp1_start_of_display_area_in_vram( void ) 
{
    // 0-9   X (0-1023)    (halfword address in VRAM)  (relative to begin of VRAM)
    // 10-18 Y (0-511)     (scanline number in VRAM)   (relative to begin of VRAM)
    // 19-23 Not used (zero)
    //
    // Upper/left Display source address in VRAM. The size and target position on 
    // screen is set via Display Range registers; target=X1,Y2; size=(X2-X1/cycles_per_pix), (Y2-Y1).
    TRACE_GPU("gp1_start_of_display_area_in_vram", "command: %08x\n", gpu.gp1);
    gpu.display_vram_x_start = (PARAMETER(gpu.gp1) >>  0) & 0x3ff;
    gpu.display_vram_y_start = (PARAMETER(gpu.gp1) >> 10) & 0x1ff;
}
static inline void gp1_horiontal_display_range( void ) 
{
    TRACE_GPU("gp1_horiontal_display_range", "command: %08x\n", gpu.gp1);
    // 0-11   X1 (260h+0)       ;12bit       ;\counted in 53.222400MHz units,
    // 12-23  X2 (260h+320*8)   ;12bit       ;/relative to HSYNC
    gpu.display_horizontal_start = (PARAMETER(gpu.gp1) >>  0) & 0xfff;
    gpu.display_horizontal_end   = (PARAMETER(gpu.gp1) >> 12) & 0xfff;
}
static inline void gp1_vertical_display_range( void ) 
{
    TRACE_GPU("gp1_vertical_display_range", "command: %08x\n", gpu.gp1);
    // 0-9   Y1 (NTSC=88h-(224/2), (PAL=A3h-(264/2))  ;\scanline numbers on screen,
    // 10-19 Y2 (NTSC=88h+(224/2), (PAL=A3h+(264/2))  ;/relative to VSYNC
    // 20-23 Not used (zero)
    gpu.display_vertical_start = (PARAMETER(gpu.gp1) >>  0) & 0x3ff;
    gpu.display_vertical_end   = (PARAMETER(gpu.gp1) >> 10) & 0x3ff;
}
static inline void gp1_display_mode( void ) 
{
    TRACE_GPU("gp1_display_mode", "command: %08x\n", gpu.gp1);
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

static void gpu_process_gp0( void )
{
    if (gpu.gpustat.dma_data_request && fifo_empty(&gpu.gp0))
    {
        /* if a request dma transfer is initiated and the block has been consumed, request another block */
        gpu.gpustat.ready_recieve_dma_block = 1;
    }
    else if (gpu.gpustat.ready_send_vram_cpu)
    {
        /* if the vram to cpu or cpu to vram transfer is non-dma */
        if (gpu.vram_direct_access_d) 
        {
            /* vram to cpu, read contents of vram and load into gpu read */
            uint32_t data;

            memory_read_vram(gpu_get_vram_address(), &data, 4);

            gpu.gpuread = data;
        }
        else
        {
            /* cpu to vram, if data in fifo, write to next vram address */
            if (!fifo_empty(&gpu.gp0))
                memory_write_vram(gpu_get_vram_address(), fifo_pop(&gpu.gp0), 4);
        }
    }
    else if (!fifo_empty(&gpu.gp0))
    {
        /* otherwise treat as a basic gp0 command */
        switch ( COMMAND(fifo_peek(&gpu.gp0)) )
        {
            case 0X00: gp0_nop(); break;
            case 0X01: 
            case 0X02: 
            case 0X80: 
            case 0XA0: 
            case 0XC0: gp0_direct_vram_access(); break;
            case 0X1F: gp0_interrupt_request();  break;
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
}

static void gpu_process_gp1( void )
{
    switch ( COMMAND(gpu.gp1) ) 
    {
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

    gpu.state = GPU_PROCESS_GP0;
}

static void gpu_tick( void )
{

    /* calculate dots and scanlines, interlace for bi31 in gpustat */
    static uint32_t cycles_since_last_dot = 0, interlace = 0;

    gpu.cycles++; cycles_since_last_dot++;
    
    /* increment the dot counter depending on the horizontal resolution */
    if (cycles_since_last_dot >= ((gpu.gpustat.horizontal_resolution_2 == 1) ? CYCLES_PER_DOT_368PIX:
                                  (gpu.gpustat.horizontal_resolution_1 == 0) ? CYCLES_PER_DOT_256PIX:
                                  (gpu.gpustat.horizontal_resolution_1 == 1) ? CYCLES_PER_DOT_320PIX:
                                  (gpu.gpustat.horizontal_resolution_1 == 2) ? CYCLES_PER_DOT_512PIX:
                                                                               CYCLES_PER_DOT_640PIX))
    {
        gpu.dots++; cycles_since_last_dot = 0;
    }

    /* set HBLANK when outside horizontal drawing region */
    if (gpu.hblank == 0 && gpu.dots >= ((gpu.gpustat.horizontal_resolution_2 == 1) ? 368:
                                        (gpu.gpustat.horizontal_resolution_1 == 0) ? 256:
                                        (gpu.gpustat.horizontal_resolution_1 == 1) ? 320:
                                        (gpu.gpustat.horizontal_resolution_1 == 2) ? 512:
                                                                                    640))
    {
        gpu.hblank = 1;
    }
    
    switch (gpu.gpustat.video_mode)
    {
        case 0:
            if (gpu.cycles >= NTSC_CYCLES_PER_SCANLINE)
                gpu.cycles = 0;

            /* if the video mode is NTSC, increment the scanline counter depending on *
             * the horizontal resolution                                              */ 
            if (gpu.dots >= ((gpu.gpustat.horizontal_resolution_2 == 1) ? NTSC_DOTS_PER_SCANLINE_368PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 0) ? NTSC_DOTS_PER_SCANLINE_256PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 1) ? NTSC_DOTS_PER_SCANLINE_320PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 2) ? NTSC_DOTS_PER_SCANLINE_512PIX:
                                                                          NTSC_DOTS_PER_SCANLINE_640PIX))
            {
                gpu.scanlines++;

                gpu.hblank = 0;
                gpu.dots   = 0;
            
                /* when in 240pix vertical resolution, even_odd_interlace (b31) changes *
                 * per scanline.                                                        */
                if (gpu.gpustat.vertical_interlace && !gpu.gpustat.vertical_resolution && !gpu.vblank)
                {
                    gpu.gpustat.drawing_even_odd_interlace = interlace;
                    interlace = ~interlace;
                }

                /* set VBLANK when outside vertical drawing region */
                if (gpu.vblank == 0 && gpu.scanlines >=  240)
                {
                    gpu.vblank = 1;
                    
                    /* even odd interlace is always 0 during vblank */
                    if (gpu.gpustat.vertical_interlace)
                        gpu.gpustat.drawing_even_odd_interlace = 0;
                }

                /* check for scanline max */
                if (gpu.scanlines >= NTSC_SCANLINES_PER_FRAME)
                {
                    gpu.scanlines = 0; 
                    gpu.vblank    = 0;

                    system_render();

                    if (gpu.gpustat.vertical_interlace && gpu.gpustat.vertical_resolution)
                    {
                        gpu.gpustat.drawing_even_odd_interlace = interlace;
                        interlace = ~interlace;
                    }
                }
            }
            break;

        case 1:
            if (gpu.cycles >=  PAL_CYCLES_PER_SCANLINE)
                gpu.cycles = 0;

            /* if the video mode is PAL, increment the scanline counter depending on  *
             * the horizontal resolution                                              */ 
            if (gpu.dots >= ((gpu.gpustat.horizontal_resolution_2 == 1) ?  PAL_DOTS_PER_SCANLINE_368PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 0) ?  PAL_DOTS_PER_SCANLINE_256PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 1) ?  PAL_DOTS_PER_SCANLINE_320PIX:
                             (gpu.gpustat.horizontal_resolution_1 == 2) ?  PAL_DOTS_PER_SCANLINE_512PIX:
                                                                           PAL_DOTS_PER_SCANLINE_640PIX))
            {
                gpu.scanlines++;

                gpu.hblank = 0;
                gpu.dots   = 0;

                /* when in 240pix vertical resolution, even_odd_interlace (b31) changes *
                 * per scanline.                                                        */
                if (gpu.gpustat.vertical_interlace && !gpu.gpustat.vertical_resolution && !gpu.vblank)
                {
                    gpu.gpustat.drawing_even_odd_interlace = interlace;
                    interlace = ~interlace;
                }

                /* set VBLANK when outside vertical drawing region */
                if (gpu.vblank == 0 && gpu.scanlines >= 240)
                {
                    gpu.vblank = 1;
                    
                    /* even odd interlace is always 0 during vblank */
                    if (gpu.gpustat.vertical_interlace)
                        gpu.gpustat.drawing_even_odd_interlace = 0;
                }

                /* check for scanline max */
                if (gpu.scanlines >=  PAL_SCANLINES_PER_FRAME)
                {
                    gpu.scanlines = 0; 
                    gpu.vblank    = 0;
                    
                    system_render();

                    if (gpu.gpustat.vertical_interlace && gpu.gpustat.vertical_resolution)
                    {
                        gpu.gpustat.drawing_even_odd_interlace = interlace;
                        interlace = ~interlace;
                    }
                }
            }
            break;
    }

}

static void gpu_render_frame( void )
{
}

void init_gpu( void )
{
    /* destroy gp0 fifo */
    // fifo_destroy( &gpu.gp0 );

    /* create gp0 fifo */
    fifo_create( &gpu.gp0, 256 );
    
    /* clear stat register */
    gpu.gpustat.value = 0;
    
    /* set default values */
    gpu.gpustat.display_enable             = 1;
    gpu.gpustat.ready_recieve_cmd_word     = 1;
    gpu.gpustat.ready_recieve_dma_block    = 1;
    gpu.gpustat.drawing_even_odd_interlace = 0;

    gpu.state = GPU_PROCESS_GP0;
}

void task_gpu( void )
{
    /* tick the gpu internal clock */
    gpu_tick();

    /* process gpu commands */
    switch (gpu.state)
    {
        case GPU_RENDERING:   gpu_render_frame(); break;
        case GPU_PROCESS_GP0: gpu_process_gp0();  break;
        case GPU_PROCESS_GP1: gpu_process_gp1();  break;
        default:
            break;
    }
}
