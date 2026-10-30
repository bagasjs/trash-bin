#ifndef GRAPHIC_H_
#define GRAPHIC_H_

#include "gm.h"
#include <stdint.h>

#ifndef BOOL
#define BOOL int
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef struct Renderer Renderer;

Renderer *render_init(void);
void render_close(Renderer *render);

#define INVALID_ID 0

typedef uint32_t ShaderID;
typedef struct {
    const char *vert_glsl_source;
    const char *frag_glsl_source;
} ShaderDesc;
ShaderID render_create_shader(Renderer *render, ShaderDesc desc);
void shader_use(Renderer *render, ShaderID shader);
int  shader_get_uniform_location(Renderer *render, ShaderID shader, const char *name);
BOOL shader_set_uniform_vec3(Renderer *rendere, ShaderID shader, const char *name, Vec3 vec);
BOOL shader_set_uniform_mat4(Renderer *rendere, ShaderID shader, const char *name, Mat4 mat);

typedef uint32_t TextureID;
typedef struct {
    uint8_t *pixels;
    uint32_t width;
    uint32_t height;
    uint32_t nchannels;
} TextureDesc;
TextureID render_create_texture_from_file(Renderer *render, const char *filepath);
TextureID render_create_texture(Renderer *render, TextureDesc desc);
BOOL texture_get_opengl_id(Renderer *render, TextureID texture, uint32_t *opengl_id);

typedef uint32_t MeshID;
MeshID render_create_mesh(Renderer *render);

typedef uint32_t PipelineID;
MeshID render_create_pipeline(Renderer *render);

typedef struct Camera {
    Vec3 pos;
    Vec3 direction;
    Vec3 up;
    Mat4 projection;
    float fov_radians;
    float near;
    float far;

    // Calculated
    Vec3 front;
} Camera;

Camera camera_ortho(Vec3 pos, uint32_t window_width, uint32_t window_height);
Camera create_perspective_camera(Vec3 pos, uint32_t window_width, uint32_t window_height, float near, float far, float fov_radians);
void camera_update_direction(Camera *camera, Vec3 target);
void camera_update_window_size(Camera *camera, uint32_t window_width, uint32_t window_height);
Mat4 camera_get_view_matrix(Camera camera);

#endif // GRAPHIC_H_
