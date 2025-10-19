#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec4 VertexColour;

layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

out vec4 FragmentColour;

void main()
{
	// Snap camera position to grid units to create bone grid effect
	// This makes the grid follow the camera while maintaining grid alignment
	vec3 camera_pos = viewProperties.camera_position.xyz;
	vec3 snapped_camera_pos = floor(camera_pos);
	
	// Offset the grid position by the snapped camera position
	vec3 world_position = VertexPosition + snapped_camera_pos;
	
	gl_Position    = viewProperties.projection * viewProperties.view * vec4(world_position, 1.0);
	FragmentColour = VertexColour;
}