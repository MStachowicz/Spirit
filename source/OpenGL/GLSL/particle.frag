#version 460 core

uniform sampler2D diffuse;

in GEOM_OUT
{
	vec2 tex_coord;
} fs_in;

out vec4 Colour;

void main()
{
	Colour = texture(diffuse, fs_in.tex_coord);

	if (Colour.a < 0.1)
		discard;
}