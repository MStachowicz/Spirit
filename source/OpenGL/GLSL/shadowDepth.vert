#version 460 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 light_space_mat;
uniform mat4 model;

void main()
{
	gl_Position = light_space_mat * model * vec4(VertexPosition, 1.0);
}