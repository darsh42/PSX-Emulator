#include <stdint.h>
#include <immintrin.h>

#define SIMD_WIDTH 8

typedef __m256  vectorf;
typedef __m256i vectori;

enum equation_component { ALPHA, BETA, GAMMA, };
struct component {
    /* components of the formula:
     *  x * xm + y * ym + add */
    vectorf xm, ym, add;

    /* extracted color values */
    vectorf red, green, blue;
};
struct triangle {
    struct component components[3];

    vectorf recp_area; /* recipricol of area */

    uint32_t c0, c1, c2;
    
    int32_t alpha_top_left,
             beta_top_left,
            gamma_top_left;
} triangle;

static inline void print_vector(const char *label, vectorf val) {
    float values[4]; _mm256_storeu_ps(values, val);
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
        .components = {
            {
                .xm    = _mm256_set1_ps(y1 - y2),
                .ym    = _mm256_set1_ps(x2 - x1),
                .add   = _mm256_set1_ps(y2*x1 - y1*x2),

                .red   = _mm256_set1_ps(R(c0)), 
                .green = _mm256_set1_ps(G(c0)), 
                .blue  = _mm256_set1_ps(B(c0)), 
            },
            {
                .xm    = _mm256_set1_ps(y2 - y0),
                .ym    = _mm256_set1_ps(x0 - x2),
                .add   = _mm256_set1_ps(y0*x2 - y2*x0),
                .red   = _mm256_set1_ps(R(c1)), 
                .green = _mm256_set1_ps(G(c1)), 
                .blue  = _mm256_set1_ps(B(c1)), 
            },
            {
                .xm    = _mm256_set1_ps(y0 - y1),
                .ym    = _mm256_set1_ps(x1 - x0),
                .add   = _mm256_set1_ps(y1*x0 - y0*x1),
                .red   = _mm256_set1_ps(R(c2)), 
                .green = _mm256_set1_ps(G(c2)), 
                .blue  = _mm256_set1_ps(B(c2)), 
            },
        },


        .recp_area =
            _mm256_set1_ps(1.0f/double_area),

        .alpha_top_left = alpha_top_left,
        .beta_top_left =   beta_top_left,
        .gamma_top_left = gamma_top_left,
    };
}

static inline vectorf simd_fill_vector(int32_t value) {
    return _mm256_set1_ps(value);
}
static inline vectorf simd_calculate_x_values(int32_t x) {
    return _mm256_setr_ps(x+0, x+1, x+2, x+3, 
                          x+4, x+5, x+6, x+7);
}
static inline vectorf simd_calculate_y_values(int32_t y) {
    return _mm256_set1_ps(y);
}
static inline vectorf simd_compute_equation(vectorf x, vectorf y, enum equation_component c) {
    /* (x * xm + y * ym + add) * recipricol_area */
    return _mm256_mul_ps(triangle.recp_area,
                         _mm256_add_ps(triangle.components[c].add, 
                         _mm256_add_ps(_mm256_mul_ps(triangle.components[c].xm, x),
                                       _mm256_mul_ps(triangle.components[c].ym, y))));
}
static inline int32_t simd_compute_pixels(vectorf alphas, vectorf betas, vectorf gammas, vectorf zeros) {
    /* based on drawing rules check if comparison will be greater than or equal to */
    vectorf alpha_test = 
        (triangle.alpha_top_left) ? _mm256_cmp_ps(alphas, zeros, _CMP_GE_OQ): 
                                    _mm256_cmp_ps(alphas, zeros, _CMP_GT_OQ);
    vectorf  beta_test = 
        (triangle.beta_top_left)  ? _mm256_cmp_ps( betas, zeros, _CMP_GE_OQ): 
                                    _mm256_cmp_ps( betas, zeros, _CMP_GT_OQ);
    vectorf gamma_test = 
        (triangle.gamma_top_left) ? _mm256_cmp_ps(gammas, zeros, _CMP_GE_OQ): 
                                    _mm256_cmp_ps(gammas, zeros, _CMP_GT_OQ);

    return _mm256_movemask_ps(_mm256_and_ps(alpha_test, _mm256_and_ps(beta_test, gamma_test)));
}
static inline void simd_compute_colors(uint32_t *colors, vectorf alphas, vectorf betas, vectorf gammas, vectorf min, vectorf max) {
    /* compute red, blue and green components for each vertex color and add them */
    vectorf   redf = 
        _mm256_add_ps(_mm256_mul_ps(triangle.components[ALPHA].red, alphas), 
                      _mm256_add_ps(_mm256_mul_ps(triangle.components[ BETA].red, betas), 
                                    _mm256_mul_ps(triangle.components[GAMMA].red, gammas)));
    vectorf greenf = 
        _mm256_add_ps(_mm256_mul_ps(triangle.components[ALPHA].green, alphas), 
                      _mm256_add_ps(_mm256_mul_ps(triangle.components[ BETA].green, betas), 
                                    _mm256_mul_ps(triangle.components[GAMMA].green, gammas)));
    vectorf  bluef = 
        _mm256_add_ps(_mm256_mul_ps(triangle.components[ALPHA].blue, alphas), 
                      _mm256_add_ps(_mm256_mul_ps(triangle.components[ BETA].blue, betas), 
                                    _mm256_mul_ps(triangle.components[GAMMA].blue, gammas)));

    /* clamp the color ranges */
      redf = _mm256_min_ps(max, _mm256_max_ps(min,   redf));
    greenf = _mm256_min_ps(max, _mm256_max_ps(min, greenf));
     bluef = _mm256_min_ps(max, _mm256_max_ps(min,  bluef));

    /* convert to integers */
    vectori alpha = _mm256_set1_epi32(0xff);
    vectori   red = _mm256_cvtps_epi32(  redf);
    vectori green = _mm256_cvtps_epi32(greenf);
    vectori  blue = _mm256_cvtps_epi32( bluef);

    /* shift to position */
    alpha = _mm256_slli_epi32(alpha, 24);
      red = _mm256_slli_epi32(  red, 16);
    green = _mm256_slli_epi32(green,  8);
     blue = _mm256_slli_epi32( blue,  0);
    
    /* pack into final colors */
    _mm256_storeu_si256((vectori *) colors, _mm256_or_si256(_mm256_or_si256(red, green), 
                                                            _mm256_or_si256(blue, alpha)));
}
