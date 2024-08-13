#version 460 core

layout (location = 0) in vec4 particle_position; // w component is remaining lifetime
layout (location = 1) in vec4 particle_velocity; // w component is starting lifetime

out VERT_OUT
{
	float lifetime_factor;
} vert_out;

void main()
{
	vert_out.lifetime_factor = 1.0 - particle_position.w / particle_velocity.w;
	gl_Position              = particle_position;
}