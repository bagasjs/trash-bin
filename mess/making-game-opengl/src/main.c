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
    Vec3 normal;
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
Camera camera = {0};

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
        glViewport(0, 0, current_window_width, current_window_height);
        window_width = current_window_width;
        window_height = current_window_height;
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cursor_disabled_mode = !cursor_disabled_mode;
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

    glEnable(GL_DEPTH_TEST);

    Renderer *ren = render_init();

    StringBuilder vert = {0};
    StringBuilder frag = {0};
    if(!read_entire_file("assets/shaders/1.color_cube.vert", &vert)) return -1;
    if(!read_entire_file("assets/shaders/1.color_cube.frag", &frag)) return -1;
    ShaderID lighting_shader = render_create_shader(ren, (ShaderDesc){
        .vert_glsl_source = vert.items,
        .frag_glsl_source = frag.items,
    });
    if(lighting_shader == INVALID_ID) return -1;
    vert.count = 0;
    frag.count = 0;

    if(!read_entire_file("assets/shaders/1.light_cube.vert", &vert)) return -1;
    if(!read_entire_file("assets/shaders/1.light_cube.frag", &frag)) return -1;
    ShaderID light_cube_shader = render_create_shader(ren, (ShaderDesc){
        .vert_glsl_source = vert.items,
        .frag_glsl_source = frag.items,
    });
    if(light_cube_shader == INVALID_ID) return -1;
    vert.count = 0;
    frag.count = 0;

    Vertex vertices[] = {
        { .pos = { -0.5f, -0.5f, -0.5f }, .normal = { 0.0f, 0.0f, -1.0f, }, }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .normal = { 0.0f, 0.0f, -1.0f, }, }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .normal = { 0.0f, 0.0f, -1.0f, }, }, 
        { .pos = { -0.5f, +0.5f, -0.5f }, .normal = { 0.0f, 0.0f, -1.0f, }, }, 

        { .pos = { -0.5f, -0.5f, +0.5f }, .normal = { 0.0f, 0.0f,  1.0f, }, }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .normal = { 0.0f, 0.0f,  1.0f, }, }, 
        { .pos = { +0.5f, +0.5f, +0.5f }, .normal = { 0.0f, 0.0f,  1.0f, }, }, 
        { .pos = { -0.5f, +0.5f, +0.5f }, .normal = { 0.0f, 0.0f,  1.0f, }, }, 

        { .pos = { -0.5f, +0.5f, +0.5f }, .normal = { -1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { -0.5f, +0.5f, -0.5f }, .normal = { -1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { -0.5f, -0.5f, -0.5f }, .normal = { -1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { -0.5f, -0.5f, +0.5f }, .normal = { -1.0f, 0.0f, 0.0f, }, }, 

        { .pos = { +0.5f, +0.5f, +0.5f }, .normal = {  1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .normal = {  1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .normal = {  1.0f, 0.0f, 0.0f, }, }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .normal = {  1.0f, 0.0f, 0.0f, }, }, 

        { .pos = { -0.5f, -0.5f, -0.5f }, .normal = { 0.0f, -1.0f, 0.0f, }, }, 
        { .pos = { +0.5f, -0.5f, -0.5f }, .normal = { 0.0f, -1.0f, 0.0f, }, }, 
        { .pos = { +0.5f, -0.5f, +0.5f }, .normal = { 0.0f, -1.0f, 0.0f, }, }, 
        { .pos = { -0.5f, -0.5f, +0.5f }, .normal = { 0.0f, -1.0f, 0.0f, }, }, 

        { .pos = { -0.5f, +0.5f, -0.5f }, .normal = { 0.0f,  1.0f, 0.0f, }, }, 
        { .pos = { +0.5f, +0.5f, -0.5f }, .normal = { 0.0f,  1.0f, 0.0f, }, }, 
        { .pos = { +0.5f, +0.5f, +0.5f }, .normal = { 0.0f,  1.0f, 0.0f, }, }, 
        { .pos = { -0.5f, +0.5f, +0.5f }, .normal = { 0.0f,  1.0f, 0.0f, }, }, 
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

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLuint color_cube_vao = 0;
    glGenVertexArrays(1, &color_cube_vao);
    glBindVertexArray(color_cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    GLuint light_cube_vao = 0;
    glGenVertexArrays(1, &light_cube_vao);
    glBindVertexArray(light_cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    camera = create_perspective_camera(vec3(0.0f, 0.0f, 3.0f), window_width, window_height, 0.1f, 100.0f, DEG2RAD(45.0f));
    Vec3 light_pos = vec3(1.2f, 1.0f, 2.0f);

    Mat4 view;
    Mat4 model;
    while(!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        process_input(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view  = camera_get_view_matrix(camera);

        shader_use(ren, lighting_shader);
        shader_set_uniform_vec3(ren, lighting_shader, "objectColor", vec3(1.0f, 0.5f, 0.31f));
        shader_set_uniform_vec3(ren, lighting_shader, "lightColor",  vec3(1.0f, 1.0f, 1.0));
        shader_set_uniform_vec3(ren, lighting_shader, "lightPos",  light_pos);
        shader_set_uniform_vec3(ren, lighting_shader, "viewPos",  camera.pos);
        model = mat4_eye(1.0f);
        model = mat4_translate(model, vec3(0.0f, 0.0f, 0.0f));
        shader_set_uniform_mat4(ren, lighting_shader, "view", view);
        shader_set_uniform_mat4(ren, lighting_shader, "projection", camera.projection);
        shader_set_uniform_mat4(ren, lighting_shader, "model", model);
        glBindVertexArray(color_cube_vao);
        glDrawElements(GL_TRIANGLES, ARRAY_LEN(indices), GL_UNSIGNED_INT, 0);

        shader_use(ren, light_cube_shader);
        shader_set_uniform_mat4(ren, light_cube_shader, "view", view);
        shader_set_uniform_mat4(ren, light_cube_shader, "projection", camera.projection);
        model = mat4_eye(1.0f);
        model = mat4_translate(model, light_pos);
        model = mat4_scale(model, vec3(0.2f, 0.2f, 0.2f));
        shader_set_uniform_mat4(ren, light_cube_shader, "model", model);
        glBindVertexArray(light_cube_vao);
        glDrawElements(GL_TRIANGLES, ARRAY_LEN(indices), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    da_free(&vert);
    da_free(&frag);
    render_close(ren);
    glfwTerminate();
    return 0;
}
