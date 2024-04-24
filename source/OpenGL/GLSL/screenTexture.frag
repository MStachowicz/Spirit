#version 460 core

in vec2 TexCoord;
out vec4 Colour;

uniform sampler2D screen_texture;

uniform bool invertColours = false;
uniform bool grayScale = false;
uniform bool sharpen = false;
uniform bool blur = false;
uniform bool edgeDetection = false;

uniform float offset = 1.0 / 300.0;

void main()
{
	Colour = texture(screen_texture, TexCoord);

	if (invertColours)
		Colour = vec4(vec3(1.0 - texture(screen_texture, TexCoord)), 1.0);
	else if (grayScale)
	{
		Colour = texture(screen_texture, TexCoord);
		float average = 0.2126 * Colour.r + 0.7152 * Colour.g + 0.0722 * Colour.b;
		Colour = vec4(average, average, average, 1.0);
	}
	else if (sharpen)
	{
		vec2 offsets[9] = vec2[](
			vec2(-offset, offset),  vec2(0.0f,  offset), vec2(offset,  offset), // Top
			vec2(-offset, 0.0f),    vec2(0.0f,  0.0f),   vec2(offset,  0.0f),   // Center
			vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)  // Bottom
		);

		float kernel[9] = float[](
			-1, -1, -1,
			-1,  9, -1,
			-1, -1, -1
		);

		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
			sampleTex[i] = vec3(texture(screen_texture, TexCoord.st + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
			col += sampleTex[i] * kernel[i];

		Colour = vec4(col, 1.0);
	}
	else if (blur)
	{
		vec2 offsets[9] = vec2[](
			vec2(-offset, offset),  vec2(0.0f,  offset), vec2(offset,  offset), // Top
			vec2(-offset, 0.0f),    vec2(0.0f,  0.0f),   vec2(offset,  0.0f),   // Center
			vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)  // Bottom
		);

		float kernel[9] = float[](
			1.0 / 16,  2.0 / 16,  1.0 / 16,
			2.0 / 16,  4.0 / 16,  2.0 / 16,
			1.0 / 16,  2.0 / 16,  1.0 / 16
		);

		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
			sampleTex[i] = vec3(texture(screen_texture, TexCoord.st + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
			col += sampleTex[i] * kernel[i];

		Colour = vec4(col, 1.0);
	}
	else if (edgeDetection)
	{
		vec2 offsets[9] = vec2[](
			vec2(-offset, offset),  vec2(0.0f,  offset), vec2(offset,  offset), // Top
			vec2(-offset, 0.0f),    vec2(0.0f,  0.0f),   vec2(offset,  0.0f),   // Center
			vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)  // Bottom
		);

		float kernel[9] = float[](
			1.0 ,  1.0,  1.0,
			2.0 , -8.0,  1.0,
			1.0 ,  1.0,  1.0
		);

		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
			sampleTex[i] = vec3(texture(screen_texture, TexCoord.st + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
			col += sampleTex[i] * kernel[i];

		Colour = vec4(col, 1.0);
	}
}