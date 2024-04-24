#version 460 core

in VS_OUT
{
	vec4 colour;
} fs_in;

out vec4 Colour;

void main()
{
	Colour = fs_in.colour;
}