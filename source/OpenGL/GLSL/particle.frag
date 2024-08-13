// Colour based on any combination of texture, colour, texture and colour. Each can be varying or constant over the particle's lifetime.

// Possible defined variables combinations here are:
// CONSTANT_COLOUR || VARYING_COLOUR,
// CONSTANT_TEXTURE || VARYING_TEXTURE
#version 460 core

#ifdef CONSTANT_COLOUR
	uniform vec4 colour;
#elifdef VARYING_COLOUR
	uniform vec4 start_colour;
	uniform vec4 end_colour;
#endif
#ifdef CONSTANT_TEXTURE
	uniform sampler2D diffuse;
#elifdef VARYING_TEXTURE
	uniform sampler2D start_diffuse;
	uniform sampler2D end_diffuse;
#endif

in GEOM_OUT //TODO: Wrap in #ifdef HAS_TEXTURE
{
	vec2 tex_coord;
	float lifetime_factor;
} fs_in;

out vec4 Colour;

void main()
{
	#ifdef HAS_VARYING
		float factor = fs_in.lifetime_factor;
	#endif

	#ifdef STYLE_CONSTANT_COLOUR
		Colour = colour;
	#elifdef STYLE_VARYING_COLOUR
		Colour = mix(start_colour, end_colour, factor);
	#elifdef STYLE_CONSTANT_TEXTURE
		Colour = texture(diffuse, fs_in.tex_coord);
	#elifdef STYLE_VARYING_TEXTURE
		Colour = mix(texture(start_diffuse, fs_in.tex_coord), texture(end_diffuse, fs_in.tex_coord), factor);
	#elifdef STYLE_CONSTANT_COLOUR_AND_TEXTURE
		Colour = colour * texture(diffuse, fs_in.tex_coord);
	#elifdef STYLE_VARYING_COLOUR_CONSTANT_TEXTURE
		Colour = mix(start_colour, end_colour, factor) * texture(diffuse, fs_in.tex_coord);
	#elifdef STYLE_CONSTANT_COLOUR_VARYING_TEXTURE
		Colour = colour * mix(texture(start_diffuse, fs_in.tex_coord), texture(end_diffuse, fs_in.tex_coord), factor);
	#elifdef STYLE_VARYING_COLOUR_AND_TEXTURE
		Colour = mix(start_colour, end_colour, factor) * mix(texture(start_diffuse, fs_in.tex_coord), texture(end_diffuse, fs_in.tex_coord), factor);
	#endif

	if (Colour.a < 0.1)
		discard;
}