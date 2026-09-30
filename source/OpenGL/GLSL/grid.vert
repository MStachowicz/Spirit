#version 460 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 invViewProj;

out vec3 nearPoint;
out vec3 rayPoint;

vec3 unproject(vec2 position, float depth)
{
	vec4 point = invViewProj * vec4(position, depth, 1.0);
	return point.xyz / point.w;
}

void main()
{
	gl_Position = vec4(VertexPosition, 1.0);
	nearPoint = unproject(VertexPosition.xy, -1.0);
	rayPoint = unproject(VertexPosition.xy, 0.0);
}