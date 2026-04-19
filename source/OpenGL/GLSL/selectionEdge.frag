#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D mask;
uniform sampler2D mask_depth;
uniform sampler2D scene_depth;
uniform vec4 colour;
uniform vec2 texel_size;
uniform int radius;

void main()
{
	float centre = texture(mask, TexCoord).r;

	// If the centre pixel is inside the selected entity, skip (no outline on interior).
	if (centre > 0.5)
		discard;

	// Find the closest visible entity depth across the neighbourhood.
	// A neighbour is "visible" if its mask is white and its entity depth <= scene depth at that position.
	float min_entity_depth = 1.0;
	bool found_visible = false;

	for (int y = -radius; y <= radius; ++y)
	{
		for (int x = -radius; x <= radius; ++x)
		{
			vec2 sample_uv = TexCoord + vec2(float(x), float(y)) * texel_size;
			float mask_val = texture(mask, sample_uv).r;

			if (mask_val > 0.5)
			{
				float entity_depth = texture(mask_depth, sample_uv).r;
				float scene_val    = texture(scene_depth, sample_uv).r;

				if (entity_depth <= scene_val + 0.0001)
				{
					min_entity_depth = min(min_entity_depth, entity_depth);
					found_visible = true;
				}
			}
		}
	}

	if (!found_visible)
		discard;

	// Reject outline pixels where closer geometry exists at the centre position.
	float centre_scene = texture(scene_depth, TexCoord).r;
	if (centre_scene < min_entity_depth - 0.0001)
		discard;

	FragColor = colour;
}
