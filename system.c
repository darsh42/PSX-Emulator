#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRIVATE_SYSTEM
#include "system.h"
#include "dma.h"
#include "gpu.h"
#include "memory.h"

#define VRAM_WIDTH  2048
#define VRAM_HEIGHT  512

/* c: color, s: size (bytes) */
#define PUT_PIX(x, y, c, s) \
    memory_write_vram((y) * VRAM_WIDTH + (x), (c), (s))

#define ABS(a)    (a > 0) ? a : -a
#define MAX(a, b) (a > b) ? a :  b
#define MIN(a, b) (a < b) ? a :  b

/* externed from whatever os interface used */
extern struct system sys;

/* function to place a horizontal line of pixels */
void system_draw_horizontalline(uint16_t x0, 
                                uint16_t x1, 
                                uint16_t y,
                                uint16_t c)
{ for (uint16_t x = MIN(x0, x1); x < MAX(x0, x1); x++) PUT_PIX(x, y, c, 2); }

/* function to place a vertical line of pixels */
void system_draw_verticalline(uint16_t x,
                              uint16_t y0, 
                              uint16_t y1, 
                              uint16_t c)
{ for (uint16_t y = MIN(y0, y1); y < MAX(y0, y1); y++) PUT_PIX(x, y, c, 2); }

void system_draw_toptriangle(uint16_t x0, uint16_t x1, uint16_t x2,
                             uint16_t y0, uint16_t y1, uint16_t y2)
{

}

void system_draw_bottriangle(uint16_t x0, uint16_t x1, uint16_t x2,
                             uint16_t y0, uint16_t y1, uint16_t y2)
{

}

inline int32_t abs(int32_t a)            { return (a > 0) ? a : -a; }
inline int32_t max(int32_t a, int32_t b) { return (a > b) ? a :  b; }
inline int32_t min(int32_t a, int32_t b) { return (a < b) ? a :  b; }
void system_draw_triangle_bresenham(uint16_t x0, uint16_t x1, uint16_t x2,
                                    uint16_t y0, uint16_t y1, uint16_t y2,
                                    uint16_t c0, uint16_t c1, uint16_t c2)
{
    /* 
     * GIVEN
     *           (x0, y0)
     *              /\
     *             /  \
     *            /    \
     *           /      \
     *          /        \
     *         ------------   
     *      (x1, y1)    (x2, y2)
     *               OR
     *      (x1, y1)    (x2, y2)
     *         ------------   
     *          \        /
     *           \      / 
     *            \    /  
     *             \  /   
     *              \/    
     *           (x0, y0)
     */

    /* check if the triangle is flat bottom/top */
    assert(y0 == y1 || y1 == y2 || y2 == y0);
    
    /* line 0 */
    int32_t l0dx =  abs(x1 - x0), l0sx = (x0 < x1) ? 1: -1;
    int32_t l0dy = -abs(y1 - y0), l0sy = (y0 < y1) ? 1: -1;
    /* line 1*/
    int32_t l1dx =  abs(x2 - x0), l1sx = (x0 < x2) ? 1: -1;
    int32_t l1dy = -abs(y2 - y0), l1sy = (y0 < y2) ? 1: -1;
    
    /* make sure they are both heading in
     * the same direction as each other */
    assert(l0sy == l1sy);

    /* error for each line */
    int32_t l0e = l0dx + l0dy;
    int32_t l1e = l1dx + l1dy;

    /* cursors for each line (y0 is common y-axis cursor) */
    int32_t l0x = x0, l1x = x0;
    
    for (;;)
    {
        int32_t e2 = l0e * 2;
        if (e2 >= l0dy) {
            if (l0x == x1) { goto end; } /* reached end */
            l0e += l0dy; l0x += l0sx;    /* increment x */
        }
        if (e2 <= l0dx) {
            /* increment other line x until *
             * increment in y reached       */
            for (;;) {
                e2 = l1e * 2;
                if (e2 >= l1dy) {
                    if (l1x == x2) { goto end; }
                    l1e += l1dy; l1x += l1sx;
                }
                
                /* if increment in y needed break out
                 * of loop to increment as per previous
                 * request                              */
                if (e2 <= l1dx) { break; }
            }
            
            /* draw horizontal line between (l0x, y0) to (l1x, y0) */
            system_draw_horizontalline(l0x, l1x, y0, c0);
            
            /* increment y0 */
            if (y0 == y1) { goto end; } /* reached end */
            l0e += l0dx; l1e += l1dx; y0 += l0sy;
        }
    }
end:
    return;
}

void system_draw_triangle(uint16_t x0, uint16_t x1, uint16_t x2,
                          uint16_t y0, uint16_t y1, uint16_t y2,
                          uint16_t c0, uint16_t c1, uint16_t c2)
{
    if (y0 == y1 || y1 == y2 || y2 == y0) 
    { 
        // system_draw_triangle_bresenham(x0, x1, x2,
        //                                y0, y1, y2,
        //                                c0, c1, c2); 
    } 
    else
    {
        /* sort and split triangle to have one flat bottom */
    }
}

void render_line_monochrome(
        uint32_t c, uint32_t v1,
                    uint32_t v2,
        bool semi_transparent
)
{
}
void render_line_shaded(
        uint32_t c1, uint32_t v1,
        uint32_t c2, uint32_t v2,
        bool semi_transparent
)
{
}
void render_polyline_monochrome(
        struct fifo gp0,
        bool semi_transparent
)
{
}
void render_polyline_shaded(
        struct fifo gp0,
        bool semi_transparent
)
{
}

void render_rectangle_monochrome(
        uint32_t c, uint32_t v, uint32_t s,
        bool semi_transparent
)
{
}
void render_rectangle_textured(
        uint32_t c, uint32_t v, uint32_t t_clut, uint32_t s,
        bool semi_transparent, bool texture_blending
)
{
}
void render_three_point_polygon_monochrome(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
    bool semi_transparent
) 
{
    TRACE_SYS("render_three_point_polygon_monochrome", 
            "\n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             semi_transparent);
}
void render_four_point_polygon_monochrome(uint32_t c1, uint32_t v1, 
                                                       uint32_t v2, 
                                                       uint32_t v3, 
                                                       uint32_t v4, 
                                          bool semi_transparent
) 
{
    TRACE_SYS("render_four_point_polygon_monochrome", 
            "\n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tx: %03d | y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             X(v4), Y(v4), R(c1), G(c1), B(c1),
             semi_transparent);
    system_draw_triangle(X(v1), X(v2), X(v3),
                         Y(v1), Y(v2), Y(v3),
                         c1 ,   c1 ,   c1 );
    system_draw_triangle(X(v1), X(v2), X(v4),
                         Y(v1), Y(v2), Y(v4),
                           c1 ,   c1 ,   c1 );
}
void render_three_point_polygon_textured(uint32_t c1, uint32_t v1, uint32_t t1_clut,
                                                      uint32_t v2, uint32_t t2_page,
                                                      uint32_t v3, uint32_t t3,
                                         bool semi_transparent, bool texture_blending) 
{
    TRACE_SYS("render_three_point_polygon_textured", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             semi_transparent);
}
void render_four_point_polygon_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
                 uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_four_point_polygon_textured", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c1), G(c1), B(c1),
             X(v3), Y(v3), R(c1), G(c1), B(c1),
             X(v4), Y(v4), R(c1), G(c1), B(c1),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             TX(t4     ), TY(t4     ),
             semi_transparent);
}
void render_three_point_polygon_shaded(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    bool semi_transparent
) 
{
    TRACE_SYS("render_three_point_polygon_shaded", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             semi_transparent);
}
void render_four_point_polygon_shaded(uint32_t c1, uint32_t v1,
                                      uint32_t c2, uint32_t v2,
                                      uint32_t c3, uint32_t v3,
                                      uint32_t c4, uint32_t v4,
                                      bool semi_transparent) 
{
    TRACE_SYS("render_four_point_polygon_shaded", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\tsemitransparent: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             X(v4), Y(v4), R(c4), G(c4), B(c4),
             semi_transparent);
}
void render_three_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_three_point_polygon_shaded_textured", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n \
             \n\ttextureblending: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             semi_transparent,
             texture_blending);
}
void render_four_point_polygon_shaded_textured(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    uint32_t c4, uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
) 
{
    TRACE_SYS("render_four_point_polygon_shaded_textured", 
            "\n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\t x: %03d |  y: %03d | r: %03d | g: %03d | b: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\ttx: %03d | ty: %03d \
             \n\tsemitransparent: %d\n \
             \n\ttextureblending: %d\n",
             X(v1), Y(v1), R(c1), G(c1), B(c1),
             X(v2), Y(v2), R(c2), G(c2), B(c2),
             X(v3), Y(v3), R(c3), G(c3), B(c3),
             X(v4), Y(v4), R(c4), G(c4), B(c4),
             TX(t1_clut), TY(t1_clut),
             TX(t2_page), TY(t2_page),
             TX(t3     ), TY(t3     ),
             TX(t4     ), TY(t4     ),
             semi_transparent,
             texture_blending);
}
