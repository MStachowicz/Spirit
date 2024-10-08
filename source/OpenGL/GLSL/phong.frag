#version 460 core

uniform float shininess;

#ifdef UNIFORM_COLOUR
	uniform vec4 uColour;
#else
	uniform sampler2D diffuse;
	uniform sampler2D specular;
#endif

#ifdef SHADOWS
	uniform float PCF_bias;
	uniform sampler2D shadow_map;

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
#endif


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

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 tex_coord;
	vec4 camera_position;
#ifdef SHADOWS
	vec4 position_light_space;
#endif
} fs_in;
out vec4 Colour;

void main()
{
	Colour = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 frag_normal    = normalize(fs_in.normal);
	vec3 view_direction = normalize(vec3(fs_in.camera_position) - fs_in.position);

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

	#ifdef UNIFORM_COLOUR
		Colour.a = uColour.a;
	#endif
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
	#ifdef UNIFORM_COLOUR
		vec3 ambient  = p_light.ambient  *        uColour.rgb;
		vec3 diffuse  = p_light.diffuse  * diff * uColour.rgb;
		vec3 specular = p_light.specular * spec * uColour.rgb;
	#else
		vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord));
		vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord));
		vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord));
	#endif

	#ifdef SHADOWS
		float shadow = shadow_calculation(fs_in.position_light_space);
		return vec4((ambient + (shadow * (diffuse + specular))).xyz, 1.0);
	#else
		return vec4((ambient + diffuse + specular).xyz, 1.0);
	#endif
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
	#ifdef UNIFORM_COLOUR
		vec3 ambient  = p_light.ambient  *        uColour.rgb * attenuation;
		vec3 diffuse  = p_light.diffuse  * diff * uColour.rgb * attenuation;
		vec3 specular = p_light.specular * spec * uColour.rgb * attenuation;
	#else
		vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord)) * attenuation;
		vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord)) * attenuation;
		vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord)) * attenuation;
	#endif

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
	#ifdef UNIFORM_COLOUR
		vec3 ambient  = p_light.ambient  *        uColour.rgb * attenuation * intensity;
		vec3 diffuse  = p_light.diffuse  * diff * uColour.rgb * attenuation * intensity;
		vec3 specular = p_light.specular * spec * uColour.rgb * attenuation * intensity;
	#else
		vec3 ambient  = p_light.ambient  *        vec3(texture(diffuse, fs_in.tex_coord) * attenuation * intensity);
		vec3 diffuse  = p_light.diffuse  * diff * vec3(texture(diffuse, fs_in.tex_coord) * attenuation * intensity);
		vec3 specular = p_light.specular * spec * vec3(texture(specular, fs_in.tex_coord) * attenuation * intensity);
	#endif

	return vec4((ambient + diffuse + specular).xyz, 1.0);
}