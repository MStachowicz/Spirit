// Takes a point representing a particle and generates a quad facing the camera
#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out GEOM_OUT
{
	vec2 tex_coord;
} geom_out;

layout(shared) uniform ViewProperties
{
	mat4 view;
	mat4 projection;
	vec4 camera_position; // w component unused
} viewProperties;

#ifdef FIXED_SIZE
	uniform float size;
#elifdef VARYING_SIZE
	uniform float start_size;
	uniform float end_size;
#endif

void main()
{
	if (gl_in[0].gl_Position.w > 0.0) // w component is lifetime
	{
		#ifdef FIXED_SIZE
			float size = size;
		#elifdef VARYING_SIZE
			// TODO calculate factor based on time left alive not just lifetime
			float factor = 0.5;
			float size = mix(start_size, end_size, factor);
		#endif

		vec4 particle_center = viewProperties.projection * viewProperties.view * vec4(gl_in[0].gl_Position.xyz, 1.0);
		vec4 camera_up       = vec4(0.0, 1.0, 0.0, 0.0);
		vec4 camera_right    = vec4(1.0, 0.0, 0.0, 0.0);

		gl_Position        = particle_center + camera_right * size + camera_up * size;
		geom_out.tex_coord = vec2(1.0, 0.0);
		EmitVertex();

		gl_Position        = particle_center - camera_right * size + camera_up * size;
		geom_out.tex_coord = vec2(0.0, 0.0);
		EmitVertex();

		gl_Position        = particle_center + camera_right * size - camera_up * size;
		geom_out.tex_coord = vec2(1.0, 1.0);
		EmitVertex();

		gl_Position        = particle_center - camera_right * size - camera_up * size;
		geom_out.tex_coord = vec2(0.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}