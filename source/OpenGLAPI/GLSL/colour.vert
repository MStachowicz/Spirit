#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec3 VertexColour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 FragmentColour;

void main()
{
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
	FragmentColour = vec4(VertexColour, 1.0);
}