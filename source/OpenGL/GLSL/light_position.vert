#version 430 core

uniform float scale;

layout (location = 0) in vec3 VertexPosition;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
buffer PointLightsBuffer
{
    uint number_of_point_lights;
    PointLight point_lights[];
};

out VS_OUT
{
    vec4 colour;
} vs_out;

void main()
{
    vs_out.colour = vec4(point_lights[gl_InstanceID].diffuse.xyz, 1.0);
    mat4 model    = mat4(
        vec4( scale, 0.0,   0.0,   0.0),
        vec4( 0.0,   scale, 0.0,   0.0),
        vec4( 0.0,   0.0,   scale, 0.0),
        vec4( point_lights[gl_InstanceID].position.xyz, 1.0));

    gl_Position = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}