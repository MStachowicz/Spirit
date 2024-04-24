#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out VS_OUT {
	vec4 normal;
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
	vs_out.projection = viewProperties.projection;
	vs_out.normal     = normalize(viewProperties.view * model * vec4(VertexNormal, 0.0));
	gl_Position       = viewProperties.view * model * vec4(VertexPosition, 1.0);
}