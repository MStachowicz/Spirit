#version 430 core

uniform vec3 view_position;
uniform float shininess;
uniform sampler2D diffuse; // Diffuse texture
uniform sampler2D specular; // Specular texture



struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
buffer DirectionalLightsBuffer
{
    uint number_of_directional_lights;
    DirectionalLight directional_lights[];
};
vec4 directional_light_contribution(DirectionalLight p_light, vec3 p_frag_normal, vec3 p_view_direction);


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
vec4 point_light_contribution(PointLight p_light, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction);


struct SpotLight
{
    vec3 position;

    vec3 direction;
    float cutoff;
    float outer_cutoff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
buffer SpotLightsBuffer
{
    uint number_of_spot_lights;
    SpotLight spot_lights[];
};
vec4 spot_light_contribution(SpotLight p_light, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction);



in VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
} fs_in;
out vec4 Colour;

void main()
{
    Colour = vec4(0.0, 0.0, 0.0, 1.0);

    vec3 frag_normal    = normalize(fs_in.normal);
    vec3 view_direction = normalize(view_position - fs_in.position);

    for (uint i = 0; i < number_of_point_lights; i++)
    {
        Colour += point_light_contribution(point_lights[i], frag_normal, fs_in.position, view_direction);
    }
    for (uint i = 0; i < number_of_spot_lights; i++)
    {
        Colour += spot_light_contribution(spot_lights[i], frag_normal, fs_in.position, view_direction);
    }
    for (uint i = 0; i < number_of_directional_lights; i++)
    {
        Colour += directional_light_contribution(directional_lights[i], frag_normal, view_direction);
    }
}

vec4 directional_light_contribution(DirectionalLight p_light, vec3 p_frag_normal, vec3 p_view_direction)
{
    vec3 light_direction = normalize(-p_light.direction);

    // diffuse shading
    float diff = max(dot(p_frag_normal, light_direction), 0.0);

    // specular shading
    vec3 reflect_direction = reflect(-light_direction, p_frag_normal);
    float spec             = pow(max(dot(p_view_direction, reflect_direction), 0.0), shininess);

    // combine results
    vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord));
    vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord));
    vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord));

    return vec4((ambient + diffuse + specular).xyz, 1.0);
}
vec4 point_light_contribution(PointLight p_light, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction)
{
    vec3 light_direction = normalize(p_light.position - p_frag_pos);

    // diffuse shading
    float diff = max(dot(p_frag_normal, light_direction), 0.0);

    // specular shading
    vec3 reflect_direction = reflect(-light_direction, p_frag_normal);
    float spec             = pow(max(dot(p_view_direction, reflect_direction), 0.0), shininess);

    // attenuation
    float distance    = length(p_light.position - p_frag_pos);
    float attenuation = 1.0 / (p_light.constant + p_light.linear * distance + p_light.quadratic * (distance * distance));

    // combine results
    vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord)) * attenuation;
    vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord)) * attenuation;
    vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord)) * attenuation;

    return vec4((ambient + diffuse + specular).xyz, 1.0);
}
vec4 spot_light_contribution(SpotLight p_light, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction)
{
    vec3 light_direction = normalize(p_light.position - p_frag_pos);

    // diffuse shading
    float diff = max(dot(p_frag_normal, light_direction), 0.0);

    // specular shading
    vec3 reflect_direction = reflect(-light_direction, p_frag_normal);
    float spec             = pow(max(dot(p_view_direction, reflect_direction), 0.0), shininess);

    // attenuation
    float distance    = length(p_light.position - p_frag_pos);
    float attenuation = 1.0 / (p_light.constant + p_light.linear * distance + p_light.quadratic * (distance * distance));

    // spotlight intensity
    float theta     = dot(light_direction, normalize(-p_light.direction));
    float epsilon   = p_light.cutoff - p_light.outer_cutoff;
    float intensity = clamp((theta - p_light.outer_cutoff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord) * attenuation * intensity);
    vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord) * attenuation * intensity);
    vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord) * attenuation * intensity);

    return vec4((ambient + diffuse + specular).xyz, 1.0);
}