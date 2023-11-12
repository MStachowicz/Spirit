#version 330 core

out vec4 FragColor;

uniform vec4 colour;

void main()
{
    FragColor = colour;
}