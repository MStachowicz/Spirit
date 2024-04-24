#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube texture0;

void main()
{
	FragColor = texture(texture0, TexCoords);
}