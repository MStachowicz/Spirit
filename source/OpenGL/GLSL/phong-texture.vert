#version 430 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoord;

uniform mat4 model;
uniform mat4 light_proj_view;

layout(shared) uniform ViewProperties
{
    mat4 view;
    mat4 projection;
} viewProperties;

out VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    vec4 position_light_space;
} vs_out;

void main()
{
    vs_out.position             = vec3(model * vec4(VertexPosition, 1.0));
    vs_out.normal               = mat3(transpose(inverse(model))) * VertexNormal;
    vs_out.tex_coord            = VertexTexCoord;
    vs_out.position_light_space = light_proj_view * vec4(vs_out.position, 1.0);
    gl_Position                 = viewProperties.projection * viewProperties.view * model * vec4(VertexPosition, 1.0);
}