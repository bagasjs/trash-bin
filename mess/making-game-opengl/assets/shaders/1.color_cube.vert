#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main()
{
    Normal = aNormal;
    FragPos = vec3(vec4(aPos, 1.0) * model);

	gl_Position = vec4(FragPos, 1.0) * view * projection;
}
