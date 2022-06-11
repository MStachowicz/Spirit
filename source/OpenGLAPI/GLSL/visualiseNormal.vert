#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out VS_OUT {
    vec3 normal;
    mat4 projection;
} vs_out;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

uniform mat4 model;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(viewProperties.view * model)));
    vs_out.normal     = vec3(vec4(normalMatrix * VertexNormal, 0.0));
    vs_out.projection = viewProperties.projection;
    gl_Position       = viewProperties.view * model * vec4(VertexPosition, 1.0);
}