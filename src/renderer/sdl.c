#include "sdl.h"

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

    if ((handler.context = SDL_GL_CreateContext(handler.window)) == NULL) {
        exit(-1);
    }
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    // renderer_init(&handler, "screen");

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_destroy(void) {
    SDL_DestroyWindow(handler.window);
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR sdl_update(void) {
    // renderer_end_frame(&handler);
    SDL_GL_SwapWindow(handler.window);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            exit(0);
        }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // renderer_start_frame(&handler);

    return set_PSX_error(NO_ERROR);
}

static Position_t pos_from_gp0(uint32_t p) {
    Position_t pos;

    pos.x = ((p >>  0) & 0xffff);
    pos.y = ((p >> 16) & 0xffff);

    // pos.x -= gpu_display_vram_x_start();

    // pos.x = (pos.x / 512.0f) - 1.0f;
    // pos.y = 1.0f - (pos.y / 256.0f);

    // PRINT_POS(pos);

    return pos;
}

static Color_t col_from_gp0(uint32_t c) {
    Color_t col;

    col.r = (c >>  0) & 0xff;
    col.g = (c >>  8) & 0xff;
    col.b = (c >> 16) & 0xff;

    col.r = col.r / 255.0f;
    col.g = col.g / 255.0f;
    col.b = col.b / 255.0f;

    // PRINT_COL(col);

    return col;
}

static Texpos_t texpos_from_gp0(uint32_t p) {
    Texpos_t texpos;

    texpos.x = (p >> 0) & 0xff;
    texpos.y = (p >> 4) & 0xff;

    return texpos;
}

static Texpage_t texpage_from_gp0(uint32_t p) {
    Texpage_t page;
    
    p >>= 16;

    page.x_base = ((p >> 0) & 0xf) * 64;
    page.y_base = ((p >> 4) & 0x1) *256;

    return page;
}

static Texdepth_e texdepth_from_gp0(uint32_t p) {
    Texdepth_e depth;

    // p >>= 16;

    depth = (p >> 7) & 0x3;

    return depth;
}

static Clutpos_t clutpos_from_gp0(uint32_t c) {
    Clutpos_t clut;

    c >>= 16;
    
    clut.x = ((c >> 0) & 0x3f) * 16;
    clut.y = ((c >> 6) & 0x1ff) * 1;
    
    return clut;
}

// renderer functions
void RENDER_THREE_POINT_POLYGON_MONOCHROME(
    uint32_t gp0_c1, uint32_t gp0_v1, 
    uint32_t gp0_v2, 
    uint32_t gp0_v3, 
    bool semi_transparent
) 
{
    vertex_t v1 = {}, v2 = {}, v3 = {};
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
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
    vertex_t v1 = {}, v2 = {}, v3 = {}, v4 = {};
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
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
    vertex_t v1 = {}, v2 = {}, v3 = {};
    
    v1 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
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
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v4 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t4),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    PRINT_VERTEX(v1);
    PRINT_VERTEX(v2);
    PRINT_VERTEX(v3);
    PRINT_VERTEX(v4);

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
    vertex_t v1 = {}, v2 = {}, v3 = {};
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .semi_transparent = semi_transparent
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
    vertex_t v1 = {}, v2 = {}, v3 = {}, v4 = {};
    
    v1 = (vertex_t) {
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .semi_transparent = semi_transparent
    };

    v4 = (vertex_t) {
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c4),
        .semi_transparent = semi_transparent
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
    vertex_t v1 = {}, v2 = {}, v3 = {};
    
    v1 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
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
    vertex_t v1 = {}, v2 = {}, v3 = {}, v4 = {};
    
    v1 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v1),
        .color    = col_from_gp0(gp0_c1),
        .texpos   = texpos_from_gp0(gp0_t1_clut),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v2 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v2),
        .color    = col_from_gp0(gp0_c2),
        .texpos   = texpos_from_gp0(gp0_t2_page),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v3 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v3),
        .color    = col_from_gp0(gp0_c3),
        .texpos   = texpos_from_gp0(gp0_t3),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    v4 = (vertex_t) {
        .draw_texture = true,
        .position = pos_from_gp0(gp0_v4),
        .color    = col_from_gp0(gp0_c4),
        .texpos   = texpos_from_gp0(gp0_t4),
        .texpage  = texpage_from_gp0(gp0_t2_page),
        .depth    = texdepth_from_gp0(gp0_t2_page),
        .clutpos  = clutpos_from_gp0(gp0_t1_clut),
        .blend    = texture_blending + 1,
        .semi_transparent = semi_transparent
    };

    renderer_push_triangle(&handler, v1, v2, v3);
    renderer_push_triangle(&handler, v2, v3, v4);
}
