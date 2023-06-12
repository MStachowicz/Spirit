#version 420 core

layout (location = 0) in vec3 VertexPosition;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

uniform vec4 colour;

out vec4 FragmentColour;

void main()
{
    gl_Position    = viewProperties.projection * viewProperties.view * vec4(VertexPosition, 1.0);
	FragmentColour = colour;
}