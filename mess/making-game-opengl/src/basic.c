#include "vendors/glad.h"
#include <GLFW/glfw3.h>
#include "cutils.h"
#include "vendors/stb_image.h"
#include "graphic.h"

#define GM_IMPLEMENTATION
#include "gm.h"
#undef GM_IMPLEMENTATION

typedef struct Vertex {
    Vec3 pos;
    Vec4 color;
    Vec2 tex_coords;
} Vertex;

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;
float last_x = (float)WINDOW_WIDTH/2, last_y = (float)WINDOW_HEIGHT/2;
float yaw = -90.0f, pitch = 0.0f;
float delta_time = 0.0f;
float last_frame = 0.0f;
bool cursor_disabled_mode = true;
Camera camera = {0};;

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos;
    last_x = xpos;
    last_y = ypos;
    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if(pitch > 89.0f)
        pitch =  89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;
}

void process_input(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    int current_window_width, current_window_height;
    glfwGetWindowSize(window, &current_window_width, &current_window_height);
    if(current_window_width != window_width || current_window_height != window_height) {
        window_width = current_window_width;
        window_height = current_window_height;
        glViewport(0, 0, window_width, window_height);
        camera_update_window_size(&camera, window_width, window_height);
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if(cursor_disabled_mode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPosCallback(window, NULL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPosCallback(window, cursor_pos_callback);
        }
    }

    camera_update_direction(&camera, vec3(
        cos(DEG2RAD(yaw)) * cos(DEG2RAD(pitch)),
        sin(DEG2RAD(pitch)),
        sin(DEG2RAD(yaw)) * cos(DEG2RAD(pitch))
    ));

    const float camera_speed = 2.5f * delta_time;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camera.pos = vec3_add(camera.pos, vec3_mul_scalar(camera.front, camera_speed));
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
        camera.pos = vec3_sub(camera.pos, vec3_mul_scalar(camera.front, camera_speed));
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
        camera.pos = vec3_sub(camera.pos, vec3_mul_scalar(vec3_normalize(vec3_cross(camera.front, camera.up)), camera_speed));
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
        camera.pos = vec3_add(camera.pos, vec3_mul_scalar(vec3_normalize(vec3_cross(camera.front, camera.up)), camera_speed));
}

int main(void)
{
    if(!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Isometric Minecraft", NULL, NULL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    if(!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Vertex vertices[] = {
        { .pos = { -0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { -0.5f, +0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 

        { .pos = { -0.5f, -0.5f, +0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { +0.5f, +0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { -0.5f, +0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 

        { .pos = { -0.5f, +0.5f, +0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { -0.5f, +0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { -0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { -0.5f, -0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 

        { .pos = { +0.5f, +0.5f, +0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 

        { .pos = { -0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { -0.5f, -0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 

        { .pos = { -0.5f, +0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f, } }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .tex_coords = { 1.0f, 0.0f, } }, 
        { .pos = { +0.5f, +0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 1.0f, 1.0f, } }, 
        { .pos = { -0.5f, +0.5f, +0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 1.0f, } }, 
    };

    // Indices are part of vao
    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    glEnable(GL_DEPTH_TEST);

    Renderer *ren = render_init();

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

    GLuint ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    StringBuilder vert = {0};
    StringBuilder frag = {0};
    if(!read_entire_file("assets/shaders/basic.vert", &vert)) return -1;
    if(!read_entire_file("assets/shaders/basic.frag", &frag)) return -1;
    ShaderID shader = render_create_shader(ren, (ShaderDesc){
        .vert_glsl_source = vert.items,
        .frag_glsl_source = frag.items,
    });
    if(shader == INVALID_ID) return -1;
    shader_use(ren, shader);
    vert.count = 0;
    frag.count = 0;

    TextureID textureID = render_create_texture_from_file(ren, "assets/images/brick-wall.jpg");
    if(textureID == INVALID_ID) return -1;
    GLuint texture;
    if(!texture_get_opengl_id(ren, textureID, &texture)) return -1;

    int loc = 0;
    loc = shader_get_uniform_location(ren, shader, "u_tex");
    if(loc < 0) return -1;
    glUniform1i(loc, 0);

    Vec3 cube_positions[] = {
        vec3( 0.0f,  0.0f,  0.0f), 
        vec3( 2.0f,  5.0f, -15.0f), 
        vec3(-1.5f, -2.2f, -2.5f),  
        vec3(-3.8f, -2.0f, -12.3f),  
        vec3( 2.4f, -0.4f, -3.5f),  
        vec3(-1.7f,  3.0f, -7.5f),  
        vec3( 1.3f, -2.0f, -2.5f),  
        vec3( 1.5f,  2.0f, -2.5f), 
        vec3( 1.5f,  0.2f, -1.5f), 
        vec3(-1.3f,  1.0f, -1.5f)  
    };

    camera = create_perspective_camera(vec3(0.0f, 0.0f, 3.0f), window_width, window_height, 0.1f, 100.0f, DEG2RAD(45.0f));

    while(!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        process_input(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 view  = camera_get_view_matrix(camera);

        shader_use(ren, shader);
        shader_set_uniform_mat4(ren, shader, "u_view", view);
        shader_set_uniform_mat4(ren, shader, "u_proj", camera.projection);

        for(int i = 0; i < ARRAY_LEN(cube_positions); ++i) {
            Mat4 model = mat4_eye(1.0f);
            model = mat4_translate(model, cube_positions[i]);
            shader_set_uniform_mat4(ren, shader, "u_model", model);
            glDrawElements(GL_TRIANGLES, ARRAY_LEN(indices), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    da_free(&vert);
    da_free(&frag);
    render_close(ren);
    glfwTerminate();
    return 0;
}
