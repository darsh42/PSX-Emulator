#ifdef RENDERER_SIMD_SSE128_H_

#include <stdint.h>
#include <immintrin.h>

#define SIMD_WIDTH 4

typedef __m128  vectorf;
typedef __m128i vectori;

enum equation_component { ALPHA, BETA, GAMMA, };
struct triangle {
    vectorf r[3]; /* extracted red color values */
    vectorf g[3]; /* extracted green color values */
    vectorf b[3]; /* extracted blue color values */

    vectorf  xm[3]; /* values that multiply x component */
    vectorf  ym[3]; /* values that multiply y component */
    vectorf add[3]; /* values that add to overall computation */

    vectorf recp_area; /* recipricol of area */

    uint32_t c0, c1, c2;

    int32_t alpha_top_left,
             beta_top_left,
            gamma_top_left;
} triangle;
static inline void print_vector(const char *label, vectorf val) {
    float values[4]; _mm_storeu_ps(values, val);
    printf("%s: %f, %f, %f, %f\n", 
            label, values[0], values[1], values[2], values[3]);
}
static inline void simd_set_triangle_data(int32_t x0, int32_t y0, uint32_t c0,
                                          int32_t x1, int32_t y1, uint32_t c1,
                                          int32_t x2, int32_t y2, uint32_t c2,
                                          int32_t alpha_top_left,
                                          int32_t  beta_top_left,
                                          int32_t gamma_top_left) {
    float double_area = 
        (float) ((x1-x0)*(y2-y0)-(x2-x0)*(y1-y0));

    if (double_area == 0.0f)
        return;

    /* ensure correct triangle winding */
    if (double_area < 0.0f) {
        int32_t tmp;

        tmp = x1; x1 = x2; x2 = tmp;
        tmp = y1; y1 = y2; y2 = tmp;
        tmp = c1; c1 = c2; c2 = tmp;

        double_area = -double_area;
    }

    /* populate global triangle info */
    triangle = (struct triangle) {
        .c0 = c0, .c1 = c1, .c2 = c2,
        .r = { 
            _mm_set1_ps(R(c0)), 
            _mm_set1_ps(R(c1)), 
            _mm_set1_ps(R(c2)),
        },
        .g = { 
            _mm_set1_ps(G(c0)), 
            _mm_set1_ps(G(c1)), 
            _mm_set1_ps(G(c2)),
        },
        .b = { 
            _mm_set1_ps(B(c0)), 
            _mm_set1_ps(B(c1)), 
            _mm_set1_ps(B(c2)),
        },
        .xm = {
            _mm_set1_ps(y1 - y2),
            _mm_set1_ps(y2 - y0),
            _mm_set1_ps(y0 - y1),
        },
        .ym = {
            _mm_set1_ps(x2 - x1),
            _mm_set1_ps(x0 - x2),
            _mm_set1_ps(x1 - x0),
        },
        .add = {
            _mm_set1_ps(y2*x1 - y1*x2),
            _mm_set1_ps(y0*x2 - y2*x0),
            _mm_set1_ps(y1*x0 - y0*x1),
        },
        .recp_area =
            _mm_set1_ps(1.0f/double_area),

        .alpha_top_left = alpha_top_left,
        .beta_top_left =   beta_top_left,
        .gamma_top_left = gamma_top_left,
    };
}

static inline vectorf simd_fill_vector(int32_t value) {
    return _mm_set1_ps(value);
}
static inline vectorf simd_calculate_x_values(int32_t x) {
    return _mm_setr_ps(x, x+1, x+2, x+3);
}
static inline vectorf simd_calculate_y_values(int32_t y) {
    return _mm_set1_ps(y);
}
static inline vectorf simd_compute_equation(vectorf x, vectorf y, enum equation_component c) {
    /* (x * xm + y * ym + add) * recipricol_area */
    return _mm_mul_ps(triangle.recp_area,
                      _mm_add_ps(triangle.add[c], 
                      _mm_add_ps(_mm_mul_ps(triangle.xm[c], x),
                                 _mm_mul_ps(triangle.ym[c], y))));
}
static inline int32_t simd_compute_pixels(vectorf alphas, vectorf betas, vectorf gammas, vectorf zeros) {
    vectorf alpha_test = 
        (triangle.alpha_top_left) ? _mm_cmpge_ps(alphas, zeros): 
                                    _mm_cmpgt_ps(alphas, zeros);
    vectorf  beta_test = 
        (triangle.beta_top_left)  ? _mm_cmpge_ps( betas, zeros): 
                                    _mm_cmpgt_ps( betas, zeros);
    vectorf gamma_test = 
        (triangle.gamma_top_left) ? _mm_cmpge_ps(gammas, zeros): 
                                    _mm_cmpgt_ps(gammas, zeros);

    return _mm_movemask_ps(_mm_and_ps(alpha_test, _mm_and_ps(beta_test, gamma_test)));
}
static inline void simd_compute_colors(uint32_t *colors, vectorf alphas, vectorf betas, vectorf gammas, vectorf min, vectorf max) {
    /* compute red, blue and green components for each vertex color and add them */
    vectorf   redf = 
        _mm_add_ps(_mm_mul_ps(triangle.r[0], alphas), 
                  _mm_add_ps(_mm_mul_ps(triangle.r[1], betas), 
                             _mm_mul_ps(triangle.r[2], gammas)));
    vectorf greenf = 
        _mm_add_ps(_mm_mul_ps(triangle.g[0], alphas), 
                   _mm_add_ps(_mm_mul_ps(triangle.g[1], betas), 
                              _mm_mul_ps(triangle.g[2], gammas)));
    vectorf  bluef = 
        _mm_add_ps(_mm_mul_ps(triangle.b[0], alphas), 
                   _mm_add_ps(_mm_mul_ps(triangle.b[1], betas), 
                              _mm_mul_ps(triangle.b[2], gammas)));

    /* clamp the color ranges */
      redf = _mm_min_ps(max, _mm_max_ps(min,   redf));
    greenf = _mm_min_ps(max, _mm_max_ps(min, greenf));
     bluef = _mm_min_ps(max, _mm_max_ps(min,  bluef));

    /* convert to integers */
    vectori alpha = _mm_set1_epi32(0xff);
    vectori   red = _mm_cvtps_epi32(  redf);
    vectori green = _mm_cvtps_epi32(greenf);
    vectori  blue = _mm_cvtps_epi32( bluef);

    /* shift to position */
    alpha = _mm_slli_epi32(alpha, 24);
      red = _mm_slli_epi32(  red, 16);
    green = _mm_slli_epi32(green,  8);
     blue = _mm_slli_epi32( blue,  0);
    
    /* pack into final colors */
    _mm_storeu_si128((__m128i *) colors, _mm_or_si128(_mm_or_si128(red, green), 
                                                      _mm_or_si128(blue, alpha)));
}
#endif // RENDERER_SIMD_SSE128_INCLUDED_H_
