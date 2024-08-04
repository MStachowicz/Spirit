#pragma once

#include "Shader.hpp"
#include "Types.hpp"

namespace ECS
{
	class Storage;
}
namespace System
{
	class Scene;
}
namespace OpenGL
{
	class PhongRenderer
	{
		Shader m_phong_texture;        // Variation of phong shader that uses specular and diffuse textures. Used to fetch the Directional, Point and spot light buffers.
		Shader m_phong_uniform_colour; // Variation of phong shader that uses a uniform colour instead of a texture.

		Buffer m_directional_lights_buffer; // The buffer used across shaders to bind DirectionalLight data.
		GLsizeiptr m_directional_light_fixed_size; // Size in bytes of the fixed portion of the directional light shader storage block (excludes any variable-sized-array variables sizes).
		GLsizeiptr m_directional_light_count_offset;
		GLint m_directional_light_array_stride;
		GLsizeiptr m_directional_light_direction_offset;
		GLsizeiptr m_directional_light_ambient_offset;
		GLsizeiptr m_directional_light_diffuse_offset;
		GLsizeiptr m_directional_light_specular_offset;

		Buffer m_point_lights_buffer; // The buffer used across shaders to bind PointLight data.
		GLsizeiptr m_point_light_fixed_size; // Size in bytes of the fixed portion of the point light shader storage block (excludes any variable-sized-array variables sizes).
		GLsizeiptr m_point_light_count_offset;
		GLint m_point_light_array_stride;
		GLsizeiptr m_point_light_position_offset;
		GLsizeiptr m_point_light_constant_offset;
		GLsizeiptr m_point_light_linear_offset;
		GLsizeiptr m_point_light_quadratic_offset;
		GLsizeiptr m_point_light_ambient_offset;
		GLsizeiptr m_point_light_diffuse_offset;
		GLsizeiptr m_point_light_specular_offset;

		Buffer m_spot_lights_buffer; // The buffer used across shaders to bind SpotLight data.
		GLsizeiptr m_spot_light_fixed_size; // Size in bytes of the fixed portion of the spot light shader storage block (excludes any variable-sized-array variables sizes).
		GLsizeiptr m_spot_light_count_offset;
		GLint m_spot_light_array_stride;
		GLsizeiptr m_spot_light_position_offset;
		GLsizeiptr m_spot_light_direction_offset;
		GLsizeiptr m_spot_light_cutoff_offset;
		GLsizeiptr m_spot_light_outer_cutoff_offset;
		GLsizeiptr m_spot_light_constant_offset;
		GLsizeiptr m_spot_light_linear_offset;
		GLsizeiptr m_spot_light_quadratic_offset;
		GLsizeiptr m_spot_light_ambient_offset;
		GLsizeiptr m_spot_light_diffuse_offset;
		GLsizeiptr m_spot_light_specular_offset;

	public:
		PhongRenderer();

		Shader& get_texture_shader()                        { return m_phong_texture; }
		Shader& get_uniform_colour_shader()                 { return m_phong_uniform_colour; }
		const Buffer& get_directional_lights_buffer() const { return m_directional_lights_buffer; }
		const Buffer& get_point_lights_buffer() const       { return m_point_lights_buffer; }
		const Buffer& get_spot_lights_buffer() const        { return m_spot_lights_buffer; }

		// Given a p_scene, updates the buffers with the light data from the scene's entities.
		// Only needs to happen once per frame or on changes to a light.
		void update_light_data(System::Scene& p_scene);
		void reload_shaders();
	};
} // namespace OpenGL