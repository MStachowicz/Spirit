#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 3) in vec2 VertexTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0, 1.0);
	TexCoord = vec2(VertexTexCoord.x, VertexTexCoord.y);
}