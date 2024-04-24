#version 460 core

in vec4 FragmentColour;
out vec4 Colour;

void main()
{
	Colour = FragmentColour;
}