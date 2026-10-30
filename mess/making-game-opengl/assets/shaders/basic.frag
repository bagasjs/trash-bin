#version 330 core

layout(location=0) out vec4 frag_color;

layout(location=0) in vec4 v_color;
layout(location=1) in vec2 v_texcoords;

uniform sampler2D u_tex;

void main() {
    frag_color = texture(u_tex, v_texcoords);
}

