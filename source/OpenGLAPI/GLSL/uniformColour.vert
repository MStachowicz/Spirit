#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 model;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

void main()
{
	gl_Position = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}