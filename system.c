#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRIVATE_SYSTEM
#include "system.h"
#include "dma.h"
#include "gpu.h"
#include "memory.h"

#include "simd.h"

#define VRAM_WIDTH  2048
#define VRAM_HEIGHT  512

/* c: color, s: size (bytes) */
#define PUT_PIX(x, y, c) \
    sys.frame_buffer[y][x] = c

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
{ assert(y0 < y1); for (uint16_t y = y0; y < y1; y++) PUT_PIX(x, y, c); }

#define KERNEL_SIZE 256
void init_threads(void) {
    sys.thread_count = 0;
    sys.allocations  = 0;
    sys.threads      = NULL;
    sys.tiles        = NULL;
}
void free_threads(void) {
    free(sys.threads);
    free(sys.tiles);
}
static void allocate_threads(size_t thread_count) {
    /* TODO: implement a pooling technique,     *
     *       allocate threads only when there   *
     *       arent enough threads, otherwise    *
     *       reuse the currently allocated ones.*
     *       After each frame de-allocate the   *
     *       threads and arguments              */
    if (thread_count > sys.thread_count) {
        /* set new thread count */
        sys.allocations++;
        sys.thread_count = thread_count;
        /* reallocate */
        sys.threads = 
            realloc(sys.threads, 
                    sys.thread_count*sizeof(*sys.threads));
        sys.tiles = 
            realloc(sys.tiles, 
                    sys.thread_count*sizeof(*sys.tiles));
    }
}

static float compute_equations(int32_t x, int32_t y, enum equation_component c) {
    return triangle.recp_area[0]* (x * triangle.components[c].xm[0] + 
                                   y * triangle.components[c].ym[0] + 
                                       triangle.components[c].add[0]);
}
static int32_t compute_pixel(float alpha, float beta, float gamma) {
    int32_t alpha_test = (triangle.alpha_top_left) ? (alpha >= 0) : (alpha > 0);
    int32_t  beta_test = (triangle.beta_top_left)  ? ( beta >= 0) : ( beta > 0);
    int32_t gamma_test = (triangle.gamma_top_left) ? (gamma >= 0) : (gamma > 0);

    return (alpha_test & beta_test & gamma_test);
}
static uint32_t compute_color(float alpha, float beta, float gamma) {
    return ((uint32_t) (alpha*triangle.components[ALPHA].red[0]   + 
                         beta*triangle.components[ BETA].red[0]   + 
                        gamma*triangle.components[GAMMA].red[0])   << 16) |
           ((uint32_t) (alpha*triangle.components[ALPHA].green[0] + 
                         beta*triangle.components[ BETA].green[0] + 
                        gamma*triangle.components[GAMMA].green[0]) <<  8) |
           ((uint32_t) (alpha*triangle.components[ALPHA].blue[0]  + 
                         beta*triangle.components[ BETA].blue[0]  + 
                        gamma*triangle.components[GAMMA].blue[0]  ) <<  0);
}

static void *fill_tile(void *arg) {
    /* get tile */
    struct box *t = 
        (struct box *) arg;

    vectorf zeros           = simd_fill_vector(  0);
    vectorf min_color_value = simd_fill_vector(  0);
    vectorf max_color_value = simd_fill_vector(255);
    
    uint32_t colors[SIMD_WIDTH];

    for (int32_t cy = t->miny; cy < t->maxy; cy++) {
        int32_t aligned_end = 
            t->maxx - (t->maxx - t->minx) % SIMD_WIDTH;

        for (int32_t cx = t->minx; cx < t->maxx; cx += SIMD_WIDTH) {
            vectorf x = simd_calculate_x_values(cx);
            vectorf y = simd_calculate_y_values(cy);

            vectorf alpha = simd_compute_equation(x, y, ALPHA);
            vectorf  beta = simd_compute_equation(x, y,  BETA);
            vectorf gamma = simd_compute_equation(x, y, GAMMA);

            /* compute what pixels are set */
            int32_t pixels =
                simd_compute_pixels(alpha, beta, gamma, zeros);

            if (!pixels)
                continue;

            /* compute the pixel colors */
            simd_compute_colors(colors, alpha, beta, gamma, 
                                min_color_value, max_color_value);
            
            /* get the base location of the framebuffer */
            uint32_t *location = &sys.frame_buffer[cy][cx];
            for (int32_t pix = 0; pix < SIMD_WIDTH; pix++) {
                /* if pixel set place color *
                 * else replace old pixel   */
                location[pix] = (pixels &(1 << pix)) ?
                    colors[pix]: location[pix];
            }
        }

        for (int32_t cx = aligned_end; cx < t->maxx; cx++) {
            float alpha = compute_equations(cx, cy, ALPHA);
            float  beta = compute_equations(cx, cy,  BETA);
            float gamma = compute_equations(cx, cy, GAMMA);

            int32_t pixel = compute_pixel(alpha, beta, gamma);

            if (!pixel) 
                continue;

            int32_t color = compute_color(alpha, beta, gamma);

            PUT_PIX(cx, cy, color);
        }
    }
}

#define MIN3(a, b, c) min(min(a, b), c)
#define MAX3(a, b, c) max(max(a, b), c)
static inline int32_t min(int32_t a, int32_t b) { return (a < b) ? a: b; }
static inline int32_t max(int32_t a, int32_t b) { return (a > b) ? a: b; }
static inline int32_t clamp(int32_t a, int32_t low, int32_t high) {
    return max(low, min(high, a));
}

static inline int32_t is_top_left_edge(int32_t x0, int32_t x1, int32_t y0, int32_t y1) {
    return (y0 < y1) || (y0 == y1 && x0 < x1);
}
void fill_triangle( int32_t x0,  int32_t x1,  int32_t x2, 
                    int32_t y0,  int32_t y1,  int32_t y2, 
                   uint32_t c0, uint32_t c1, uint32_t c2) {
    /* set the global triangle data state */
    simd_set_triangle_data(x0, y0, c0, x1, y1, c1, x2, y2, c2,
                           is_top_left_edge(x1, y1, x2, y2),
                           is_top_left_edge(x2, y2, x0, y0),
                           is_top_left_edge(x0, y0, x1, y1));

    /* calculate bounding box */
    struct box bb = {
        .minx = min(x0, min(x1, x2)),
        .maxx = max(x0, max(x1, x2)),
        .miny = min(y0, min(y1, y2)),
        .maxy = max(y0, max(y1, y2))
    };

    /* find bounding box length and width */
    bb.width  = bb.maxx - bb.minx;
    bb.height = bb.maxy - bb.miny;

    /* find number of tiles in bounding box */
    int32_t xtiles = 
        (bb.width +KERNEL_SIZE-1)/KERNEL_SIZE;
    int32_t ytiles =
        (bb.height+KERNEL_SIZE-1)/KERNEL_SIZE;

    /* allocate thread and arg for each tile    */
    int32_t thread_count = xtiles * ytiles;
    allocate_threads(thread_count);


    /* dispatch thread to render tile */
    for (int32_t t = 0; t < thread_count; t++) {
        /* calculate next minimum and current maximum */
        sys.tiles[t].minx = bb.minx+(t%xtiles)*KERNEL_SIZE,
        sys.tiles[t].miny = bb.miny+(t/xtiles)*KERNEL_SIZE,
        sys.tiles[t].maxx = min(sys.tiles[t].minx+KERNEL_SIZE, WIN_W);
        sys.tiles[t].maxy = min(sys.tiles[t].miny+KERNEL_SIZE, WIN_H);

        /* dispatch to thread */
        pthread_create(&sys.threads[t], NULL, 
                fill_tile, (void *) &sys.tiles[t]);
    }

    /* wait for threads */
    for (int32_t t = 0; t < thread_count; t++) {
        pthread_join(sys.threads[t], NULL);
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c1 ,   c1 );
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c1 ,   c1 );
    fill_triangle(X(v2), X(v3), X(v4),
                  Y(v2), Y(v3), Y(v4),
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c1 ,   c1 );
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c1 ,   c1 );
    fill_triangle(X(v2), X(v3), X(v4),
                  Y(v2), Y(v3), Y(v4),
                    c1 ,   c1 ,   c1 );
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
    fill_triangle(X(v1), X(v2), X(v3),
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c2 ,   c3 );
    fill_triangle(X(v2), X(v3), X(v4),
                  Y(v2), Y(v3), Y(v4),
                    c2 ,   c3 ,   c4 );
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c2 ,   c3 );
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
    fill_triangle(X(v1), X(v2), X(v3),
                  Y(v1), Y(v2), Y(v3),
                    c1 ,   c2 ,   c3 );
    fill_triangle(X(v2), X(v3), X(v4),
                  Y(v2), Y(v3), Y(v4),
                    c2 ,   c3 ,   c4 );
}
