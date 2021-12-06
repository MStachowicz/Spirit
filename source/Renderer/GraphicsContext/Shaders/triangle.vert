#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColour;

out vec4 FragmentColour;

void main()
{
    gl_Position = vec4(VertexPosition.xyz, 1.0);
    FragmentColour = vec4(VertexColour.xyz, 1.0);
}