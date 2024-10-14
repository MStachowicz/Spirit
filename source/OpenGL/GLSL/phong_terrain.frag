#version 460 core

uniform float shininess;
uniform float min_height;
uniform float max_height;

uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D snow;
// uniform sampler2D sand;

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
vec4 directional_light_contribution(DirectionalLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_view_direction);


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
vec4 point_light_contribution(PointLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction);


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
vec4 spot_light_contribution(SpotLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction);

// Function to calculate contribution based on slope (steepness)
//@returns a value between 0.0 and 1.0 where 0.0 is flat and 1.0 is steep
float slope_factor(vec3 normal)
{
	float slope = dot(normal, vec3(0.0, 1.0, 0.0)); // Dot product with the "up" vector
	return 1.0 - clamp(slope, 0.0, 1.0);
}
// Function to calculate contribution based on height (elevation)
//@returns a value between 0.0 and 1.0 where 0.0 is the minimum height and 1.0 is the maximum height
float height_factor(float height)
{
	return smoothstep(min_height, max_height, height);  // Smooth transition between min_height and max_height
}
// Function to map a value from one range to another
float map(float value, float current_min, float current_max, float output_min, float output_max)
{
	return clamp(output_min + (value - current_min) * (output_max - output_min) / (current_max - current_min), min(output_min, output_max), max(output_min, output_max));
}

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec4 camera_position;
	vec2 tex_coord;

} fs_in;
out vec4 Colour;


void main()
{
	Colour = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 view_direction = normalize(vec3(fs_in.camera_position) - fs_in.position);
	vec3 frag_normal    = normalize(fs_in.normal);

	float height = height_factor(fs_in.position.y);
	float slope  = slope_factor(frag_normal);

	float grass_factor = map(slope, 0.0, 0.3, 1.0, 0.0) * map(height, 0.6, 0.8, 1.0, 0.0);
	float snow_factor  = map(height, 0.85, 1.0, 0.0, 1.0);
	snow_factor = clamp(snow_factor * 1.5, 0.0, 1.0); // Boost snow factor contribution
	float rock_factor  = (1.0 - grass_factor) * (1.0 - snow_factor);

	// Sample the textures
	vec4 grass_col = texture(grass, fs_in.tex_coord) * grass_factor;
	vec4 rock_col  = texture(rock,  fs_in.tex_coord) * rock_factor;
	vec4 snow_col  = texture(snow,  fs_in.tex_coord) * snow_factor;

	// Blend the textures based on the calculated contributions
	vec3 frag_col = grass_col.xyz + rock_col.xyz + snow_col.xyz;

	for (uint i = 0; i < number_of_point_lights; i++)
	{
		Colour += point_light_contribution(point_lights[i], frag_col, frag_normal, fs_in.position, view_direction);
	}
	for (uint i = 0; i < number_of_spot_lights; i++)
	{
		Colour += spot_light_contribution(spot_lights[i], frag_col, frag_normal, fs_in.position, view_direction);
	}
	for (uint i = 0; i < number_of_directional_lights; i++)
	{
		Colour += directional_light_contribution(directional_lights[i], frag_col, frag_normal, view_direction);
	}
}

vec4 directional_light_contribution(DirectionalLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_view_direction)
{
	vec3 light_direction = normalize(-p_light.direction);

	// diffuse shading
	float diff = max(dot(p_frag_normal, light_direction), 0.0);

	// specular shading
	vec3 reflect_direction = reflect(-light_direction, p_frag_normal);
	float spec             = pow(max(dot(p_view_direction, reflect_direction), 0.0), shininess);

	vec3 ambient  = p_light.ambient         * p_frag_col;
	vec3 diffuse  = p_light.diffuse  * diff * p_frag_col;
	vec3 specular = p_light.specular * spec * p_frag_col;

	return vec4((ambient + diffuse + specular).xyz, 1.0);
}
vec4 point_light_contribution(PointLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction)
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

	vec3 ambient  = p_light.ambient  *        p_frag_col * attenuation;
	vec3 diffuse  = p_light.diffuse  * diff * p_frag_col * attenuation;
	vec3 specular = p_light.specular * spec * p_frag_col * attenuation;

	return vec4((ambient + diffuse + specular).xyz, 1.0);
}
vec4 spot_light_contribution(SpotLight p_light, vec3 p_frag_col, vec3 p_frag_normal, vec3 p_frag_pos, vec3 p_view_direction)
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
	vec3 ambient  = p_light.ambient  *        p_frag_col * attenuation * intensity;
	vec3 diffuse  = p_light.diffuse  * diff * p_frag_col * attenuation * intensity;
	vec3 specular = p_light.specular * spec * p_frag_col * attenuation * intensity;

	return vec4((ambient + diffuse + specular).xyz, 1.0);
}
