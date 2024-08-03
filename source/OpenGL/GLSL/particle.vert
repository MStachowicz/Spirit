#version 460 core

layout (location = 0) in vec4 particle_position;
layout (location = 1) in vec4 particle_velocity;

void main()
{
	gl_Position = particle_position;
}