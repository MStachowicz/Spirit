#version 420 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec3 VertexColour;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

out vec4 FragmentColour;

void main()
{
    gl_Position    = viewProperties.projection * viewProperties.view * vec4(VertexPosition, 1.0);
	FragmentColour = vec4(VertexColour, 1.0);
}