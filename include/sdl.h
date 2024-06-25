#ifndef PSXSDL_H_INCLUDED
#define PSXSDL_H_INCLUDED

#include "common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <glad/glad.h>

#define print_sdl_error(func, format, ...) print_error("sdl.c", func, format, __VA_ARGS__)

#define MAX_TRIANGLES 1024
#define MAX_VERTICIES 3 * MAX_TRIANGLES

#define WIN_NAME "PSX-Emulator"
#define WIN_WIDTH  640
#define WIN_HEIGHT 480
#define WIN_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL

#define PRINT_POS(p) printf("pos: %d, %d\n", p.x, p.y);
#define PRINT_COL(c) printf("col: %d, %d, %d\n", c.r, c.g, c.b);

typedef struct OPENGL_POSITION {
    GLshort x;
    GLshort y;
} Position_t;

typedef struct OPENGL_COLOR {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} Color_t;

typedef struct OPENGL_TEXPOS {
    GLubyte x;
    GLubyte y;
} Texpos_t;

typedef struct OPENGL_TEXPAGE {
    GLushort x_base;
    GLushort y_base;
} Texpage_t;

typedef struct OPENGL_VERTEX {
    Position_t position;
    Color_t    color;
    Texpos_t   texpos;
    Texpage_t  texpage;
} vertex_t;

struct SDL_HANDLER {
    SDL_Window   *window;
    SDL_GLContext context;

    GLuint vao;
    GLuint vbo;

    GLuint shader;
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

#endif // PSXSDL_H_INCLUDED
