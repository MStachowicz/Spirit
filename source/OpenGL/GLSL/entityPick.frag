#version 460 core

layout(location = 0) out uint FragColor;

uniform uint entity_id;

void main()
{
	FragColor = entity_id;
}
