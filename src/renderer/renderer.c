#include "sdl.h"

static GLuint renderer_load_shader(const char *file);

void renderer_init(struct SDL_HANDLER *r, const char *shader_file) {
    // create vertex buffers and allocate memory for verticies
    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICIES * sizeof(vertex_t), NULL, GL_DYNAMIC_DRAW);

    // create vertex arrays and set the attribute locations
    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);
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


    // /* The Idea behind the three pixel buffers is to create an array that is linked cpu to gpu,  *
    //  * such that when the pixle buffers are updated the textures exposed to the gpu will reflect *
    //  * those changes                                                                             */
    // GLuint tex16, tex8, tex4;
    // GLuint pbo16, pbo8, pbo4;

    // /* 16bit vram buffer */
    // glGenBuffers(GL_PIXEL_UNPACK_BUFFER_BINDING, &pbo16);
    // glBindBuffer(GL_PIXEL_UNPACK_BUFFER_BINDING,  pbo16);

    // glBufferStorage(GL_PIXEL_UNPACK_BUFFER_BINDING, 1024 * 512, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    // glBindBuffer(GL_PIXEL_UNPACK_BUFFER_BINDING, 0);
    // 
    // glGenTextures(GL_TEXTURE_2D, &tex16);
    // glBindTexture(GL_TEXTURE_2D,  tex16);

    // /* Set the texture parameters */
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // 
    // /* allocate space on the gpu */
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, 1024, 512, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    // /* select created texture and created pbo */
    // glBindTexture(GL_TEXTURE_2D, tex16);
    // glBindBuffer(GL_PIXEL_UNPACK_BUFFER_BINDING, pbo16);

    // uint16_t *ptr16 = (uint16_t *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER_BINDING, 0, 1024*512, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    


    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("[ERROR]: GL %d\n", err);
        exit(-1);
    }


    // load and compile shaders
    r->shader = renderer_load_shader(shader_file);
    r->offset = glGetUniformLocation(r->shader, "offset");
}

void renderer_start_frame(struct SDL_HANDLER *r) {
    glClear(GL_COLOR_BUFFER_BIT);
    r->triangle_count = 0;
}

void renderer_end_frame(struct SDL_HANDLER *r) {
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * r->triangle_count * sizeof(vertex_t), r->render_vertcies);

    glUseProgram(r->shader);
    glUniform2ui(r->offset, gpu_display_vram_x_start(), gpu_display_vram_y_start());
    glBindVertexArray(r->vao);

    glGenTextures(1, &r->texture);
    glBindTexture(GL_TEXTURE_2D, r->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, memory_VRAM_pointer());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawArrays(GL_TRIANGLES, 0, r->triangle_count * 3);

    glDeleteTextures(1, &r->texture);
}

void renderer_push_triangle(struct SDL_HANDLER *r, vertex_t v1, vertex_t v2, vertex_t v3) {
    if (r->triangle_count == MAX_TRIANGLES) {
        renderer_end_frame(r);
        renderer_start_frame(r);
    }
    
    r->render_vertcies[r->triangle_count * 3 + 0] = v1;
    r->render_vertcies[r->triangle_count * 3 + 1] = v2;
    r->render_vertcies[r->triangle_count * 3 + 2] = v3;

    r->triangle_count++;
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
