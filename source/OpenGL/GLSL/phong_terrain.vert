#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoord;

uniform mat4 model;

layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec4 camera_position;
	vec2 tex_coord;
} vs_out;

void main()
{
	vs_out.position        = vec3(model * vec4(VertexPosition, 1.0));
	vs_out.camera_position = viewProperties.camera_position;
	vs_out.tex_coord       = VertexTexCoord;
	vs_out.normal          = mat3(transpose(inverse(model))) * VertexNormal;
	gl_Position            = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}