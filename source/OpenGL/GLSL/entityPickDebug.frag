#version 460 core

in vec2 TexCoord;
out vec4 Colour;

uniform usampler2D pick_texture;

// Hash an unsigned integer to a visible RGB colour.
vec3 id_to_colour(uint id)
{
	// Multiply by large primes and take fractional part for distinct colours per entity.
	float r = fract(float(id) * 0.0013 + 0.5);
	float g = fract(float(id) * 0.0029 + 0.3);
	float b = fract(float(id) * 0.0047 + 0.7);
	return vec3(r, g, b);
}

void main()
{
	uint id = texture(pick_texture, TexCoord).r;

	if (id == 0u)
		Colour = vec4(0.0, 0.0, 0.0, 1.0); // No entity — black.
	else
		Colour = vec4(id_to_colour(id), 1.0);
}
