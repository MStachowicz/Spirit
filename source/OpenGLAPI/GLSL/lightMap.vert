#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * VertexNormal;
    TexCoords = VertexTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}