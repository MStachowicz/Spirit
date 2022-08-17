#version 430 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 3) in vec2 VertexTexCoord;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

layout(shared) buffer InstancedData
{
    mat4 models[];
} instancedData;

out vec2 TexCoord;

void main()
{
    gl_Position = viewProperties.projection * viewProperties.view * instancedData.models[gl_InstanceID] * vec4(VertexPosition, 1.0);
    TexCoord = vec2(VertexTexCoord.x, VertexTexCoord.y);
}