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
		Shader m_particle_draw_shader;
		Shader m_particle_update_shader;
		VAO m_quad_VAO;
		Buffer m_quad_VBO;
		Buffer m_quad_EBO;

	public:
		ParticleRenderer();

		void update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position, const Buffer& p_view_properties, const FBO& p_target_FBO);
	};
}