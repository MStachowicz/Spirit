#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 3) in vec2 VertexTexCoord;

struct Particle
{
	vec4 position; // w component is life time
	vec4 velocity; // w component unused
};
layout(std430, binding = 0) buffer ParticlesBuffer
{
	Particle particles[];
};
layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

out VS_OUT
{
	vec2 tex_coord;
} vs_out;

void main()
{
	if (particles[gl_InstanceID].position.w > 0.0)
	{
		vs_out.tex_coord = VertexTexCoord;
		gl_Position      = viewProperties.projection * viewProperties.view * vec4(VertexPosition + particles[gl_InstanceID].position.xyz, 1.0);
	}
}