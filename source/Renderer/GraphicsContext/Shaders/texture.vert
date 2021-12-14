#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColour;
layout (location = 2) in vec2 VertexTexCoord;

out vec4 FragmentColour;
out vec2 TexCoord;

void main()
{
	gl_Position = vec4(VertexPosition, 1.0);
	FragmentColour = vec4(VertexColour, 1.0);
	TexCoord = vec2(VertexTexCoord.x, VertexTexCoord.y);
}