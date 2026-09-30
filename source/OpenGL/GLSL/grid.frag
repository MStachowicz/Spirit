#version 460 core

in vec3 nearPoint;
in vec3 rayPoint;

uniform mat4 viewProjection;
uniform vec3 cameraPosition;
uniform float farClip;

out vec4 Colour;

float grid_line(vec2 position, vec2 footprint, float spacing)
{
	vec2 distance_to_line = abs(fract(position / spacing + 0.5) - 0.5) * spacing;
	vec2 coverage = 1.0 - smoothstep(vec2(0.0), vec2(1.0), distance_to_line / footprint);
	return max(coverage.x, coverage.y);
}

void main()
{
	vec3 ray_direction = normalize(rayPoint - nearPoint);
	float denominator = abs(ray_direction.y) > 0.000001 ? ray_direction.y : 0.000001;
	float plane_distance = -(nearPoint.y + cameraPosition.y) / denominator;
	vec3 relative_position = nearPoint + plane_distance * ray_direction;
	vec2 position = relative_position.xz + cameraPosition.xz;
	vec2 footprint = max(fwidth(relative_position.xz), vec2(0.000001));

	vec4 clip_position = viewProjection * vec4(relative_position, 1.0);
	float depth = clip_position.z / clip_position.w;
	if (abs(ray_direction.y) <= 0.000001 || plane_distance <= 0.0
		|| clip_position.w <= 0.0 || depth < -1.0 || depth > 1.0)
		discard;

	float level = log(max(footprint.x, footprint.y) * 12.0) / log(10.0);
	float spacing = pow(10.0, floor(level));
	float transition = smoothstep(0.0, 1.0, fract(level));
	float fine = grid_line(position, footprint, spacing);
	float medium = grid_line(position, footprint, spacing * 10.0);
	float coarse = grid_line(position, footprint, spacing * 100.0);
	float alpha = max(fine * 0.35 * (1.0 - transition),
		max(medium * mix(0.6, 0.35, transition), coarse * 0.6));
	vec3 colour = vec3(0.55);

	vec2 axes = 1.0 - smoothstep(vec2(0.5), vec2(1.5), abs(position) / footprint);
	colour = mix(colour, vec3(0.2, 0.45, 0.95), axes.x);
	colour = mix(colour, vec3(0.9, 0.2, 0.2), axes.y);
	alpha = max(alpha, max(axes.x, axes.y) * 0.85);
	alpha *= smoothstep(0.015, 0.1, abs(ray_direction.y));
	alpha *= 1.0 - smoothstep(farClip * 0.65, farClip * 0.95, plane_distance);

	Colour = vec4(colour, alpha);
	gl_FragDepth = depth * 0.5 + 0.5;
}