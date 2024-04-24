#version 460 core

uniform vec3 view_position;
uniform float shininess;

uniform sampler2D shadow_map;
uniform float PCF_bias;
uniform vec4 uColour;

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
layout(std430) buffer DirectionalLightsBuffer
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
layout(std430) buffer PointLightsBuffer
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
layout(std430) buffer SpotLightsBuffer
{
	uint number_of_spot_lights;
	SpotLight spot_lights[];
};
vec4 spot_light_contribution(SpotLight p_light, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction);


float shadow_calculation(vec4 frag_position_light_space);

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec4 position_light_space;
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

	Colour.a = uColour.a;
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
	vec3 ambient  = p_light.ambient  *        vec3(uColour.rgb);
	vec3 diffuse  = p_light.diffuse  * diff * vec3(uColour.rgb);
	vec3 specular = p_light.specular * spec * vec3(uColour.rgb);

	float shadow = shadow_calculation(fs_in.position_light_space);
	//shadow = 0.0;
	return vec4((ambient + (shadow * (diffuse + specular))).xyz, 1.0);
	//return vec4((ambient + diffuse + specular).xyz, 1.0);
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
	vec3 ambient  = p_light.ambient  *        vec3(uColour.rgb) * attenuation;
	vec3 diffuse  = p_light.diffuse  * diff * vec3(uColour.rgb) * attenuation;
	vec3 specular = p_light.specular * spec * vec3(uColour.rgb) * attenuation;

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
	vec3 ambient  = p_light.ambient  *        vec3(uColour.rgb) * attenuation * intensity;
	vec3 diffuse  = p_light.diffuse  * diff * vec3(uColour.rgb) * attenuation * intensity;
	vec3 specular = p_light.specular * spec * vec3(uColour.rgb) * attenuation * intensity;

	return vec4((ambient + diffuse + specular).xyz, 1.0);
}

float shadow_calculation(vec4 frag_position_light_space)
{
	// If the depth of the frag_position_light_space is larger than the closest depth from the light perspective.
	// The fragment being rendered is ocluded.

	// perform perspective divide + transfrom to [0,1] texture space
	vec3 projected_coords = ((frag_position_light_space.xyz / frag_position_light_space.w) * 0.5) + 0.5;
	// get depth of current fragment from light's perspective
	float frag_depth_light_space = projected_coords.z;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closest_depth = texture(shadow_map, projected_coords.xy).r;
	// check whether current frag pos is in shadow
	return frag_depth_light_space + PCF_bias > closest_depth ? 0.5 : 1.0;
}