#version 330 core

out vec4 Colour;

in vec4 FragmentColour;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
	// linearly interpolate between both textures then combine with the frag colours
	Colour = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2) * FragmentColour;
}