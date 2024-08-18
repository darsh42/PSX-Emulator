#ifndef RENDERER_H_
#define RENDERER_H_

#include "common.h"
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include "gpu.h"

#define print_renderer_error(func, format, ...) print_error("renderer.c", func, format, __VA_ARGS__)
#define FILETYPE(name, type, ret)               \
    do {                                        \
        size_t size = strlen(type);             \
        for (; *name != '.'; name++);           \
        ret = !strncmp(++name, type, size);     \
    } while (0)
// #define PRINT_POS(p) printf("pos: %f, %f\n", (float) p.x / 512.0f - 1.0, 1.0 - (float) p.y / 256.0f);
#define PRINT_POS(p) printf("pos: %d, %d\n", p.x, p.y);
#define PRINT_COL(c) printf("col: %f, %f, %f\n", c.r, c.g, c.b);
#define PRINT_VERTEX(v) printf("vertex\n" \
                               "\tdraw-textures:    %d\n" \
                               "\tpos:              %d, %d\n" \
                               "\tcol:              %f, %f, %f\n" \
                               "\ttexpos:           %d, %d\n" \
                               "\ttexpage:          %d, %d\n" \
                               "\tclutpos:          %d, %d\n" \
                               "\tdepth:            %d\n" \
                               "\tblend:            %d\n" \
                               "\tsemi-transparent: %d\n",\
                               v.draw_texture,\
                               v.position.x, v.position.y, v.color.r, v.color.g, v.color.b,\
                               v.texpos.x, v.texpos.y, v.texpage.x_base, v.texpage.y_base, \
                               v.clutpos.x, v.clutpos.y, v.depth, v.blend, v.semi_transparent)

enum OPENGL_OBJECT_TYPE {
    OPENGL_POLYGON,
    OPENGL_LINE,
    OPENGL_RECTANGLE
};

enum OPENGL_OBJECT_BLEND {
    OPENGL_NO_TEXTURE,
    OPENGL_RAW_TEXTURE,
    OPENGL_BLEND_TEXTURE
};

typedef enum OPENGL_TEXTURE_COLOR_DEPTH {
    OPENGL_4BIT,
    OPENGL_8BIT,
    OPENGL_16BIT,
} Texdepth_e;

typedef struct OPENGL_POSITION {
    int16_t x;
    int16_t y;
} Position_t;

typedef struct OPENGL_COLOR {
    float r;
    float g;
    float b;
} Color_t;

typedef struct OPENGL_TEXPOS {
    GLubyte x;
    GLubyte y;
} Texpos_t;

typedef struct OPENGL_TEXPAGE {
    GLushort x_base;
    GLushort y_base;
} Texpage_t;

typedef struct OPENGL_CLUTPOS {
    GLushort x;
    GLushort y;
} Clutpos_t;

typedef struct OPENGL_VERTEX {
    Position_t position;
    Color_t    color;
    Texpos_t   texpos;
    Texpage_t  texpage;
    Clutpos_t  clutpos;

    GLuint type;
    GLuint blend;
    GLuint depth;
    GLuint draw_texture;
    GLuint semi_transparent;
} vertex_t;

struct 
RENDERER_CONTEXT
{
    GLint  offset;
    GLuint vao;
    GLuint vbo;
    GLuint pbo;
    GLuint shader;
    GLuint texture;

    vertex_t render_vertcies[MAX_VERTICIES];
    uint32_t triangle_count;
};

/** public functions */
extern void renderer_create(const char **shaders, uint32_t shader_count);
extern void renderer_start_frame(void);
extern void renderer_end_frame(void);
extern void renderer_push_triangle(vertex_t v1, vertex_t v2, vertex_t v3);
extern void RENDER_THREE_POINT_POLYGON_MONOCHROME(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
    bool semi_transparent
);
extern void RENDER_FOUR_POINT_POLYGON_MONOCHROME(
    uint32_t c1, uint32_t v1, 
                 uint32_t v2, 
                 uint32_t v3, 
                 uint32_t v4,
    bool semi_transparent
);
extern void RENDER_THREE_POINT_POLYGON_TEXTURED(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
);
extern void RENDER_FOUR_POINT_POLYGON_TEXTURED(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
                 uint32_t v2, uint32_t t2_page,
                 uint32_t v3, uint32_t t3,
                 uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
);
extern void RENDER_THREE_POINT_POLYGON_SHADED(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    bool semi_transparent
);
extern void RENDER_FOUR_POINT_POLYGON_SHADED(
    uint32_t c1, uint32_t v1,
    uint32_t c2, uint32_t v2,
    uint32_t c3, uint32_t v3,
    uint32_t c4, uint32_t v4,
    bool semi_transparent
);
extern void RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    bool semi_transparent, bool texture_blending
);
extern void RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(
    uint32_t c1, uint32_t v1, uint32_t t1_clut,
    uint32_t c2, uint32_t v2, uint32_t t2_page,
    uint32_t c3, uint32_t v3, uint32_t t3,
    uint32_t c4, uint32_t v4, uint32_t t4,
    bool semi_transparent, bool texture_blending
);

#endif // RENDERER_H_
