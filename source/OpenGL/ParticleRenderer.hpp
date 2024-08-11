#pragma once

#include "Shader.hpp"
#include "Types.hpp"
#include "Utility/Config.hpp"

#include "glm/fwd.hpp"

namespace System
{
	class Scene;
}
namespace OpenGL
{
	class ParticleRenderer
	{
		Shader m_particle_draw_constant_colour_fixed_size;
		Shader m_particle_draw_constant_texture_fixed_size;
		Shader m_particle_draw_constant_colour_and_texture_fixed_size;
		Shader m_particle_draw_varying_colour_fixed_size;
		Shader m_particle_draw_varying_texture_fixed_size;
		Shader m_particle_draw_varying_colour_constant_texture_fixed_size;
		Shader m_particle_draw_constant_colour_varying_texture_fixed_size;
		Shader m_particle_draw_varying_colour_and_texture_fixed_size;
		Shader m_particle_draw_constant_colour_varying_size;
		Shader m_particle_draw_constant_texture_varying_size;
		Shader m_particle_draw_constant_colour_and_texture_varying_size;
		Shader m_particle_draw_varying_colour_varying_size;
		Shader m_particle_draw_varying_texture_varying_size;
		Shader m_particle_draw_varying_colour_constant_texture_varying_size;
		Shader m_particle_draw_constant_colour_varying_texture_varying_size;
		Shader m_particle_draw_varying_colour_and_texture_varying_size;

		Shader m_particle_update;
		VAO m_particle_VAO;

	public:
		ParticleRenderer();

		void reload_shaders();
		void update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position, const Buffer& p_view_properties, const FBO& p_target_FBO);
	};
}