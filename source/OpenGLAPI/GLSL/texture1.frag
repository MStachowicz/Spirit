#version 330 core

in vec2 TexCoord;
out vec4 Colour;

uniform sampler2D texture0;

void main()
{
	Colour = texture(texture0, TexCoord);
}