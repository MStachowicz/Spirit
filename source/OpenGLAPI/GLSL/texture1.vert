#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 3) in vec2 VertexTexCoord;

uniform mat4 model;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

out vec2 TexCoord;

void main()
{
    gl_Position = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
	TexCoord = vec2(VertexTexCoord.x, VertexTexCoord.y);
}