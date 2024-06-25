#include "../../include/sdl.h"

struct SDL_HANDLER handler;

GLuint load_shader(const char *file);

PSX_ERROR sdl_return_handler(struct SDL_HANDLER **phandler) {
    *phandler = &handler;
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_initialize(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        print_sdl_error("sdl_initialize", "SDL could not initialize, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_INIT);
    }
    handler.window = SDL_CreateWindow(WIN_NAME, 0, 0, WIN_WIDTH, WIN_HEIGHT, WIN_FLAGS);
    if (handler.window == NULL) {
        print_sdl_error("sdl_initialize", "SDL could not create window, SDL error %s", SDL_GetError());
        return set_PSX_error(SDL_WINDOW_CREATION);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    handler.context = SDL_GL_CreateContext(handler.window);
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    renderer_init(&handler, "screen");

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_destroy(void) {
    SDL_DestroyWindow(handler.window);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_update(void) {
    renderer_end_frame(&handler);
    SDL_GL_SwapWindow(handler.window);
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // renderer_start_frame(&handler);
    return set_PSX_error(NO_ERROR);
}

static Position_t pos_from_gp0(uint32_t p) {
    Position_t pos;

    pos.x = (p >>  0) & 0xffff;
    pos.y = (p >> 16) & 0xffff;

    // printf("pos: %x\n", p);
    PRINT_POS(pos);

    return pos;
}

static Color_t col_from_gp0(uint32_t c) {
    Color_t col;

    col.r = (c >>  0) & 0xff;
    col.g = (c >>  8) & 0xff;
    col.b = (c >> 16) & 0xff;

    // printf("col: %x\n", c);
    PRINT_COL(col);

    return col;
}

static Texpos_t texpos_from_gp0(uint32_t p) {
    Texpos_t texpos;

    texpos.x = (p >> 0) & 0xff;
    texpos.y = (p >> 8) & 0xff;

    return texpos;
}

static Texpage_t texpage_from_gp0(uint32_t p) {
    Texpage_t page;
    return page;
}

// renderer functions
void RENDER_THREE_POINT_POLYGON_MONOCHROME(
    uint32_t gp0_c1, uint32_t gp0_v1, 
    uint32_t gp0_v2, 
    uint32_t gp0_v3, 
    bool semi_transparent
) 
{
    vertex_t v1, v2, v3;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {0},
        .texpage  = {0}
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {0},
        .texpage  = {0}
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {0},
        .texpage  = {0}
    };
    
    renderer_push_triangle(&handler, v1, v2, v3);
}
void RENDER_FOUR_POINT_POLYGON_MONOCHROME(
    uint32_t gp0_c1, uint32_t gp0_v1, 
                     uint32_t gp0_v2, 
                     uint32_t gp0_v3, 
                     uint32_t gp0_v4,
    bool semi_transparent
) 
{
    vertex_t v1, v2, v3, v4;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    renderer_push_triangle(&handler, v1, v2, v3);
    renderer_push_triangle(&handler, v2, v3, v4);
}
void RENDER_THREE_POINT_POLYGON_TEXTURED(
    uint32_t gp0_c1, uint32_t gp0_v1, uint32_t gp0_t1_clut,
                     uint32_t gp0_v2, uint32_t gp0_t2_page,
                     uint32_t gp0_v3, uint32_t gp0_t3,
    bool semi_transparent, bool texture_blending
) 
{
    vertex_t v1, v2, v3;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    renderer_push_triangle(&handler, v1, v2, v3);
}
void RENDER_FOUR_POINT_POLYGON_TEXTURED(
    uint32_t gp0_c1, uint32_t gp0_v1, uint32_t gp0_t1_clut,
                     uint32_t gp0_v2, uint32_t gp0_t2_page,
                     uint32_t gp0_v3, uint32_t gp0_t3,
                     uint32_t gp0_v4, uint32_t gp0_t4,
    bool semi_transparent, bool texture_blending
) 
{
    vertex_t v1, v2, v3, v4;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t4),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    renderer_push_triangle(&handler, v1, v2, v3);
    renderer_push_triangle(&handler, v2, v3, v4);
}
void RENDER_THREE_POINT_POLYGON_SHADED(
    uint32_t gp0_c1, uint32_t gp0_v1,
    uint32_t gp0_c2, uint32_t gp0_v2,
    uint32_t gp0_c3, uint32_t gp0_v3,
    bool semi_transparent
) 
{
    vertex_t v1, v2, v3;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = {},
        .texpage  = {}
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = {},
        .texpage  = {}
    };

    renderer_push_triangle(&handler, v1, v2, v3);
}
void RENDER_FOUR_POINT_POLYGON_SHADED(
    uint32_t gp0_c1, uint32_t gp0_v1,
    uint32_t gp0_c2, uint32_t gp0_v2,
    uint32_t gp0_c3, uint32_t gp0_v3,
    uint32_t gp0_c4, uint32_t gp0_v4,
    bool semi_transparent
) 
{
    vertex_t v1, v2, v3, v4;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = {},
        .texpage  = {}
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = {},
        .texpage  = {}
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = {},
        .texpage  = {}
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c4),
        .texpos   = {},
        .texpage  = {}
    };

    renderer_push_triangle(&handler, v1, v2, v3);
    renderer_push_triangle(&handler, v2, v3, v4);
}
void RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(
    uint32_t gp0_c1, uint32_t gp0_v1, uint32_t gp0_t1_clut,
    uint32_t gp0_c2, uint32_t gp0_v2, uint32_t gp0_t2_page,
    uint32_t gp0_c3, uint32_t gp0_v3, uint32_t gp0_t3,
    bool semi_transparent, bool texture_blending
) 
{
    vertex_t v1, v2, v3;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    renderer_push_triangle(&handler, v1, v2, v3);
}
void RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(
    uint32_t gp0_c1, uint32_t gp0_v1, uint32_t gp0_t1_clut,
    uint32_t gp0_c2, uint32_t gp0_v2, uint32_t gp0_t2_page,
    uint32_t gp0_c3, uint32_t gp0_v3, uint32_t gp0_t3,
    uint32_t gp0_c4, uint32_t gp0_v4, uint32_t gp0_t4,
    bool semi_transparent, bool texture_blending
) 
{
    vertex_t v1, v2, v3, v4;
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c4),
        .texpos   = texpos_from_gp0(gp0_t4),
        .texpage  = texpage_from_gp0(gp0_t2_page)
    };

    renderer_push_triangle(&handler, v1, v2, v3);
    renderer_push_triangle(&handler, v2, v3, v4);
}
