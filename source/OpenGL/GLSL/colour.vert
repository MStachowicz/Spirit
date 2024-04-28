#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec4 VertexColour;

uniform mat4 model;
layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

out vec4 FragmentColour;

void main()
{
	gl_Position    = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
	FragmentColour = VertexColour;
}