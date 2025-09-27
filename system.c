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
#define PUT_PIX(x, y, c) \
    sys.frame_buffer[y][x] = c

#define ABS(a)    (a > 0) ? a : -a
#define MAX(a, b) (a > b) ? a :  b
#define MIN(a, b) (a < b) ? a :  b

/* externed from whatever os interface used */
extern struct system sys;

/* function to place a horizontal line of pixels */
static void system_draw_horizontalline(uint16_t x0,
                                       uint16_t x1,
                                       uint16_t y,
                                       uint16_t c)
{ assert(x0 < x1); for (uint16_t x = x0; x < x1; x++) PUT_PIX(x, y, c); }

/* function to place a vertical line of pixels */
static void system_draw_verticalline(uint16_t x,
                                     uint16_t y0,
                                     uint16_t y1,
                                     uint16_t c)
{ for (uint16_t y = MIN(y0, y1); y < MAX(y0, y1); y++) PUT_PIX(x, y, c); }

static struct triangle_info {
    int32_t x0, x1, x2;
    int32_t y0, y1, y2;
    int32_t c0, c1, c2;

    int32_t maxx, minx;
    int32_t maxy, miny;

    double total_area;
} info;

static double signed_triangle_area(int32_t x0, int32_t x1, int32_t x2,
                                   int32_t y0, int32_t y1, int32_t y2) {
    return 0.5*((y1-y0)*(x1+x0)+(y2-y0)*(x2+x0)+(y2-y1)*(x2-x1));

}
static void *pthread_draw_triangle_segment(void *arg) {
    int32_t y = *(int32_t *) arg;

    for (int32_t x = info.minx; x<info.maxx; x++) {
        double alpha = signed_triangle_area(x, info.x1, info.x2, y, info.y1, info.y2) / info.total_area;
        double beta  = signed_triangle_area(x, info.x2, info.x0, y, info.y2, info.y0) / info.total_area;
        double gamma = signed_triangle_area(x, info.x0, info.x1, y, info.y0, info.y1) / info.total_area;

        if (alpha < 0 || beta < 0 || gamma < 0) 
            continue;

        uint32_t color = 
            ((uint32_t)(alpha*B(info.c0) + beta*B(info.c1) + gamma*B(info.c2)) << 16) |
            ((uint32_t)(alpha*G(info.c0) + beta*G(info.c1) + gamma*G(info.c2)) <<  8) |
            ((uint32_t)(alpha*R(info.c0) + beta*R(info.c1) + gamma*R(info.c2)) <<  0);

        PUT_PIX(x, y, color);
    }
}
void system_draw_triangle(uint16_t x0, uint16_t x1, uint16_t x2,
                          uint16_t y0, uint16_t y1, uint16_t y2,
                          uint16_t c0, uint16_t c1, uint16_t c2) {
    /* populate the general read only data for all threads */
    info.x0=x0; info.x1=x1; info.x2=x2;
    info.y0=y0; info.y1=y1; info.y2=y2;
    info.c0=c0; info.c1=c1; info.c2=c2;

    info.minx = MIN(x0, x1); info.minx = MIN(info.minx, x2);
    info.miny = MIN(y0, y1); info.miny = MIN(info.miny, y2);
    info.maxx = MAX(x0, x1); info.maxx = MAX(info.maxx, x2);
    info.maxy = MAX(y0, y1); info.maxy = MAX(info.maxy, y2);

    info.total_area = signed_triangle_area(info.x0, info.x1, info.x2, 
                                           info.y0, info.y1, info.y2);

    /* create a thread for each row in the bounding box */
    int32_t thread_count = info.maxy - info.miny;
    pthread_t *threads = 
        malloc(thread_count*sizeof(*threads));
    int32_t *args = 
        malloc(thread_count*sizeof(*args));

    /* spawn each thread */
    for (int32_t t = 0; t < thread_count; t++) {
        /* create and populate arg */
        int32_t *arg = &args[t]; *arg = t+info.miny;
        /* dispatch to thread */
        pthread_create(&threads[t], NULL, 
                pthread_draw_triangle_segment, (void *) arg);
    }

    /* wait for threads */
    for (int32_t t = 0; t < thread_count; t++) {
        pthread_join(threads[t], NULL);
    }

    free(threads);
    free(args);
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
    system_draw_triangle(X(v1), X(v2), X(v3),
                         Y(v1), Y(v2), Y(v3),
                           c1 ,   c2 ,   c3 );
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
