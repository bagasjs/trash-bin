#include "graphic.h"
#include <stdlib.h>

#include "cutils.h"
#include "vendors/glad.h"
#include "vendors/stb_image.h"

#ifndef NDEBUG
#include <stdio.h>
#define _DEBUG_ERROR(FMT, ...) fprintf(stderr, "ERROR: "FMT"\n", __VA_ARGS__)
#define DEBUG_ERROR(...) _DEBUG_ERROR(__VA_ARGS__)
#define _DEBUG_INFO(FMT, ...) printf("INFO: "FMT"\n", __VA_ARGS__)
#define DEBUG_INFO(...) _DEBUG_INFO(__VA_ARGS__)
#else
#define DEBUG_ERROR(...) 
#define DEBUG_INFO(...)
#endif

typedef struct Shader {
    int init;
    GLuint program;
} Shader;

typedef struct Texture {
    int init;
    GLuint texture;
} Texture;

#define MAX_SHADER 32

typedef struct Renderer {
    // TODO: This must be a table instead of a plain dynamic array
    struct {
        Shader *items;
        size_t count;
        size_t capacity;
    } shaders;
    struct {
        Texture *items;
        size_t count;
        size_t capacity;
    } textures;
} Renderer;

Renderer *render_init(void)
{
    Renderer *ren;
    ren = malloc(sizeof(*ren));
    memset(ren, 0, sizeof(*ren));
    // stubs
    da_append(&ren->shaders,  ((Shader){.init=1}));
    da_append(&ren->textures, ((Texture){.init=1}));
    return ren;
}

void render_close(Renderer *ren)
{
    if(ren) free(ren);
}

ShaderID render_create_shader(Renderer *ren, ShaderDesc desc)
{
    int  success;
    char info_log[512];

    GLuint vsmod = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsmod, 1, (const char**)&desc.vert_glsl_source, NULL);
    glCompileShader(vsmod);
    glGetShaderiv(vsmod, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vsmod, sizeof(info_log), NULL, info_log);
        DEBUG_ERROR("vertex shader compilation failed: %s\n", info_log);
        return INVALID_ID;
    }

    GLuint fsmod = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsmod, 1, (const char**)&desc.frag_glsl_source, NULL);
    glCompileShader(fsmod);
    glGetShaderiv(fsmod, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fsmod, sizeof(info_log), NULL, info_log);
        DEBUG_ERROR("fragment shader compilation failed: %s\n", info_log);
        return INVALID_ID;
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vsmod);
    glAttachShader(shader_program, fsmod);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        DEBUG_ERROR("shader program linking failed: %s\n", info_log);
        return INVALID_ID;
    }
    glDeleteShader(vsmod);
    glDeleteShader(fsmod);

    ShaderID id = ren->shaders.count;
    da_append(&ren->shaders, ((Shader){
        .init = 1,
        .program = shader_program,
    }));
    return id;
}

void shader_use(Renderer *render, ShaderID id)
{
    Shader shader = render->shaders.items[id];
    if(shader.init) {
        glUseProgram(shader.program);
    } else {
        glUseProgram(0);
    }
}

int shader_get_uniform_location(Renderer *render, ShaderID id, const char *name)
{
    Shader shader = render->shaders.items[id];
    if(shader.init) {
        int loc = glGetUniformLocation(shader.program, name);
        if(loc < 0) {
            DEBUG_ERROR("Failed to get uniform with name: %s\n", name);
            return -1;
        }
        return loc;
    }
    return -1;
}

BOOL shader_set_uniform_vec3(Renderer *ren, ShaderID shader, const char *name, Vec3 vec)
{
    int loc = shader_get_uniform_location(ren, shader, name);
    if(loc < 0) return FALSE;
    glUniform3fv(loc, 1, (GLfloat*)&vec);
    return TRUE;
}

BOOL shader_set_uniform_mat4(Renderer *ren, ShaderID shader, const char *name, Mat4 mat)
{
    int loc = shader_get_uniform_location(ren, shader, name);
    if(loc < 0) return FALSE;
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data);
    return TRUE;
}

TextureID render_create_texture_from_file(Renderer *render, const char *filepath)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nchannels;
    uint8_t *pixels = stbi_load(filepath, &width, &height, &nchannels, 0);
    if(!pixels) {
        DEBUG_ERROR("Failed to load file: %s", filepath);
        return INVALID_ID;
    }
    TextureID texture = render_create_texture(render, (TextureDesc) {
        .width = width,
        .height = height,
        .pixels = pixels,
        .nchannels = nchannels,
    });
    stbi_image_free(pixels);
    return texture;
}

TextureID render_create_texture(Renderer *ren, TextureDesc desc)
{
    GLenum internal_format = desc.nchannels == 4 ? GL_RGBA  : GL_RGB;
    GLenum source_format   = desc.nchannels == 4 ? GL_RGBA  : GL_RGB;
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, desc.width, desc.height, 0, source_format, GL_UNSIGNED_BYTE, desc.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    TextureID id = ren->textures.count;
    da_append(&ren->textures, ((Texture){
        .init = 1,
        .texture = texture,
    }));
    return id;
}

BOOL texture_get_opengl_id(Renderer *render, TextureID id, uint32_t *opengl_id)
{
    if(!opengl_id) {
        DEBUG_ERROR("Invalid argument opengl_id: %p\n", opengl_id);
        return FALSE;
    }
    Texture texture = render->textures.items[id];
    if(!texture.init) {
        DEBUG_ERROR("Invalid texture id: %u\n", id);
        return FALSE;
    }
    *opengl_id = texture.texture;
    return TRUE;
}

Camera create_perspective_camera(Vec3 pos, uint32_t window_width, uint32_t window_height, float near, float far, float fov_radians)
{
    Camera cam = {0};
    cam.fov_radians = fov_radians;
    cam.near = near;
    cam.far  = far;
    cam.pos = pos;
    cam.up = vec3(0.0f, 1.0f, 0.0f);
    camera_update_window_size(&cam, window_width, window_height);
    camera_update_direction(&cam, vec3(0.0f, 0.0f, 0.0f));
    return cam;
}

void camera_update_window_size(Camera *camera, uint32_t window_width, uint32_t window_height)
{
    camera->projection = mat4_perspective(camera->fov_radians, (float)window_width/window_height, camera->near, camera->far);
}

void camera_update_direction(Camera *camera, Vec3 target)
{
    camera->direction = target;
    camera->front = vec3_normalize(camera->direction);
}

Mat4 camera_get_view_matrix(Camera camera)
{
    Mat4 view  = mat4_look_at(camera.pos, vec3_add(camera.pos, camera.front), camera.up);
    return view;
}
