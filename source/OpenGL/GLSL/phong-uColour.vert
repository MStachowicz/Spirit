#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 model;
uniform mat4 light_proj_view;

layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec4 position_light_space;
	vec4 camera_position;
} vs_out;

void main()
{
	vs_out.position             = vec3(model * vec4(VertexPosition, 1.0));
	vs_out.normal               = mat3(transpose(inverse(model))) * VertexNormal;
	vs_out.position_light_space = light_proj_view * vec4(vs_out.position, 1.0);
	vs_out.camera_position      = viewProperties.camera_position;
	gl_Position                 = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}