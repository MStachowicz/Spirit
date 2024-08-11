#version 460 core

layout (location = 0) in vec4 particle_position;

void main()
{
	gl_Position = particle_position;
}