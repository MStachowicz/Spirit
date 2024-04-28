#version 460 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 model;

layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

void main()
{
	gl_Position = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}