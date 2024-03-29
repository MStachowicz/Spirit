#version 430 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 3) in vec2 VertexTexCoord;

struct Particle
{
    vec4 position;
    vec4 velocity;
};
layout(std430, binding = 3) buffer ParticlesBuffer
{
    uint number_of_particles;
    Particle particles[];
};

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

out VS_OUT
{
    vec2 tex_coord;
} vs_out;

void main()
{
    vs_out.tex_coord = VertexTexCoord;
    gl_Position      = viewProperties.projection * viewProperties.view * vec4(VertexPosition + particles[gl_InstanceID].position.xyz, 1.0);
}