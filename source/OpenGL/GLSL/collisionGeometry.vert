#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 model;
uniform vec3 viewPosition;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

void main()
{
    // Move the VertexPosition towards the camera by the smallest possible offset along the z/depth axis.
	gl_Position = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition - ((VertexPosition - viewPosition) * 0.00001), 1.0);
}