#version 330 core

in vec2 TexCoord;

out vec4 Colour;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
	Colour = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2);
}