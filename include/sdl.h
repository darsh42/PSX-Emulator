#ifndef PSXSDL_H_INCLUDED
#define PSXSDL_H_INCLUDED

#include "common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "glad/glad.h"

#define print_sdl_error(func, format, ...) print_error("sdl.c", func, format, __VA_ARGS__)

#define MAX_TRIANGLES 1024
#define MAX_VERTICIES 3 * MAX_TRIANGLES

#define WIN_NAME "PSX-Emulator"
#define WIN_WIDTH  1024
#define WIN_HEIGHT 512
#define WIN_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL

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

struct SDL_HANDLER {
    SDL_Window   *window;
    SDL_GLContext context;

    GLuint vao;
    GLuint vbo;
    GLuint shader;
    GLuint texture;

    GLint  offset;
    vertex_t render_vertcies[MAX_VERTICIES];
    uint32_t triangle_count;
};

extern void renderer_init(struct SDL_HANDLER *r, const char *shader_file);
extern void renderer_start_frame(struct SDL_HANDLER *r);
extern void renderer_end_frame(struct SDL_HANDLER *r);
extern void renderer_push_triangle(struct SDL_HANDLER *r, vertex_t v1, vertex_t v2, vertex_t v3);

// retrieves the screen data
extern bool gpu_vram_write(void);
extern uint32_t gpu_display_vram_x_start(void);
extern uint32_t gpu_display_vram_y_start(void);

// memory
extern uint8_t *memory_VRAM_pointer(void);

#endif // PSXSDL_H_INCLUDED
