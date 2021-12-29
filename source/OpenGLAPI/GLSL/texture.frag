#version 330 core

in vec4 FragmentColour;
in vec2 TexCoord;

out vec4 Colour;

uniform bool useTextures;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
	if	(useTextures)
	{
		Colour = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2);// * FragmentColour;
	}
	else
	{
		Colour = FragmentColour;
	}
}