#version 330 core

layout(location=0) in vec3 a_pos;
layout(location=1) in vec4 a_color;
layout(location=2) in vec2 a_texcoords;

layout(location=0) out vec4 v_color;
layout(location=1) out vec2 v_texcoords;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0) * u_model * u_view * u_proj;
    v_color = a_color;
    v_texcoords = a_texcoords;
}
