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

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.FragPos   = vec3(model * vec4(VertexPosition, 1.0));
    vs_out.Normal    = mat3(transpose(inverse(model))) * VertexNormal;
    vs_out.TexCoords = VertexTexCoord * vec2(textureRepeatFactor);

    gl_Position      = viewProperties.projection * viewProperties.view * vec4(vs_out.FragPos, 1.0);
}