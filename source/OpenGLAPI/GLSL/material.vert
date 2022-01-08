#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragmentPosition;
out vec3 Normal;

void main()
{
    FragmentPosition = vec3(model * vec4(VertexPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * VertexNormal;

    gl_Position = projection * view * vec4(FragmentPosition, 1.0);
}