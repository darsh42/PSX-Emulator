#include <assert.h>
#include "renderer.h"

static struct RENDERER_CONTEXT renderer;

static void renderer_load_shaders(GLuint *program, const char **files, uint32_t files_count);
static GLuint renderer_load_shader(const char *file);

void renderer_create(const char **shaders, uint32_t shader_count) {
    // create vertex buffers and allocate memory for verticies
    glGenBuffers(1, &renderer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICIES * sizeof(vertex_t), NULL, GL_DYNAMIC_DRAW);

    // create vertex arrays and set the attribute locations
    glGenVertexArrays(1, &renderer.vao);
    glBindVertexArray(renderer.vao);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, color));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, texpos));
    glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, texpage));
    glVertexAttribPointer(4, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, clutpos));
    glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, blend));
    glVertexAttribPointer(6, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, depth));
    glVertexAttribPointer(7, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, draw_texture));
    glVertexAttribPointer(8, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, semi_transparent));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    
    // create vram texture
    glGenTextures(1, &renderer.texture);
    glBindTexture(GL_TEXTURE_2D, renderer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 1024, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, memory_VRAM_pointer());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and compile shaders
    // renderer_load_shaders(&renderer.shader, shaders, shader_count);
    renderer.shader = renderer_load_shader("screen");
    renderer.offset = glGetUniformLocation(renderer.shader, "offset");
}

void renderer_start_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    renderer.triangle_count = 0;
}

void renderer_end_frame(void) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * renderer.triangle_count * sizeof(vertex_t), renderer.render_vertcies);

    glUseProgram(renderer.shader);
    glUniform2ui(renderer.offset, gpu_display_vram_x_start(), gpu_display_vram_y_start());
    glBindVertexArray(renderer.vao);

    glBindTexture(GL_TEXTURE_2D, renderer.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0,0,0, 512, 1024, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, memory_VRAM_pointer());

    glDrawArrays(GL_TRIANGLES, 0, renderer.triangle_count * 3);

    glDeleteTextures(1, &renderer.texture);
}

void renderer_push_triangle(vertex_t v1, vertex_t v2, vertex_t v3) {
    if (renderer.triangle_count == MAX_TRIANGLES) {
        renderer_end_frame();
        renderer_start_frame();
    }
    
    renderer.render_vertcies[renderer.triangle_count * 3 + 0] = v1;
    renderer.render_vertcies[renderer.triangle_count * 3 + 1] = v2;
    renderer.render_vertcies[renderer.triangle_count * 3 + 2] = v3;

    renderer.triangle_count++;
}

static GLuint renderer_load_shader(const char *file) {
    // create file paths for vertex and fragment shaders
    const char *folder = "./shaders/";
    const char *vertex_extension   = ".vs.glsl";
    const char *fragment_extension = ".fs.glsl";

    int vertex_shader_path_length   = (strlen(folder) + strlen(file) + strlen(vertex_extension) + 1);
    int fragment_shader_path_length = (strlen(folder) + strlen(file) + strlen(fragment_extension) + 1);
    char *vertex_shader_path   = malloc(sizeof(char) * vertex_shader_path_length);
    char *fragment_shader_path = malloc(sizeof(char) * fragment_shader_path_length);

    assert(vertex_shader_path != NULL);
    assert(fragment_shader_path != NULL);

    vertex_shader_path[0] = '\0';
    fragment_shader_path[0] = '\0';

    strcat(vertex_shader_path, folder);
    strcat(vertex_shader_path, file);
    strcat(vertex_shader_path, vertex_extension);

    strcat(fragment_shader_path, folder);
    strcat(fragment_shader_path, file);
    strcat(fragment_shader_path, fragment_extension);

    // read the vertex shader data
    FILE *vertex_fp;
    if ((vertex_fp = fopen(vertex_shader_path, "r")) == NULL) {
        fprintf(stderr, "Error: could not open vertex shader from %s\n", vertex_shader_path);
        free(vertex_shader_path);
        free(fragment_shader_path);
        exit(EXIT_FAILURE);
    }
    free(vertex_shader_path);

    fseek(vertex_fp, 0, SEEK_END);
    size_t vertex_source_length = ftell(vertex_fp);
    fseek(vertex_fp, 0, SEEK_SET);

    char *vertex_source = malloc(sizeof(char) * (vertex_source_length + 1));
    assert(vertex_source != NULL);

    if (fread(vertex_source, 1, vertex_source_length, vertex_fp) != vertex_source_length) {
        fprintf(stderr, "Error: could not read vertex shader from file\n");
        free(vertex_source);
        fclose(vertex_fp);
        free(fragment_shader_path);
        exit(EXIT_FAILURE);
    }
    vertex_source[vertex_source_length] = '\0';
    fclose(vertex_fp);

    // read the fragment shader data
    FILE *fragment_fp;
    if ((fragment_fp = fopen(fragment_shader_path, "r")) == NULL) {
        fprintf(stderr, "Error: could not open fragment shader from %s\n", fragment_shader_path);
        free(vertex_source);
        free(fragment_shader_path);
        exit(EXIT_FAILURE);
    }
    free(fragment_shader_path);

    fseek(fragment_fp, 0, SEEK_END);
    size_t fragment_source_length = ftell(fragment_fp);
    fseek(fragment_fp, 0, SEEK_SET);

    char *fragment_source = malloc(sizeof(char) * (fragment_source_length + 1));
    assert(fragment_source != NULL);

    if (fread(fragment_source, 1, fragment_source_length, fragment_fp) != fragment_source_length) {
        fprintf(stderr, "Error: could not read fragment shader from file\n");
        free(vertex_source);
        free(fragment_source);
        fclose(fragment_fp);
        exit(EXIT_FAILURE);
    }
    fragment_source[fragment_source_length] = '\0';
    fclose(fragment_fp);

    // compile and link shaders
    char info_log[512];
    GLint result = 0;
    GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // compile vertex shader
    glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_source, NULL);
    glCompileShader(vertex_shader);
    free(vertex_source);

    // check vertex shader compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        fprintf(stderr, "Error: Could not compile vertex shader %s\n", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        free(fragment_source);
        exit(EXIT_FAILURE);
    }

    // compile fragment shader
    glShaderSource(fragment_shader, 1, (const GLchar **)&fragment_source, NULL);
    glCompileShader(fragment_shader);
    free(fragment_source);

    // check fragment shader compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        fprintf(stderr, "Error: Could not compile fragment shader %s\n", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        exit(EXIT_FAILURE);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // check program link status
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Error: Could not link program %s\n", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        exit(EXIT_FAILURE);
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    printf("[LOG]: loaded shaders\n");

    return program;
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
    
    renderer_push_triangle(v1, v2, v3);
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

    renderer_push_triangle(v1, v2, v3);
    renderer_push_triangle(v2, v3, v4);
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

    renderer_push_triangle(v1, v2, v3);
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

    renderer_push_triangle(v1, v2, v3);
    renderer_push_triangle(v2, v3, v4);
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

    renderer_push_triangle(v1, v2, v3);
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

    renderer_push_triangle(v1, v2, v3);
    renderer_push_triangle(v2, v3, v4);
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

    renderer_push_triangle(v1, v2, v3);
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

    renderer_push_triangle(v1, v2, v3);
    renderer_push_triangle(v2, v3, v4);
}
