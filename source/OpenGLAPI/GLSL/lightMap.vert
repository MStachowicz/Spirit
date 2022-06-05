#version 420 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoord;

uniform float textureRepeatFactor;
uniform mat4 model;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * VertexNormal;
    TexCoords = VertexTexCoord * vec2(textureRepeatFactor);

    gl_Position = viewProperties.projection * viewProperties.view * vec4(FragPos, 1.0);
}