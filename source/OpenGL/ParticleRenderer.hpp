#pragma once

#include "Shader.hpp"
#include "Types.hpp"

#include "Component/Mesh.hpp"
#include "Component/ParticleEmitter.hpp"
#include "Utility/Config.hpp"

#include "glm/fwd.hpp"

#include <array>

namespace System
{
	class Scene;
}
namespace OpenGL
{
	class ParticleRenderer
	{
		static constexpr std::array<float, 20> quad_vertices = {
			// Positions           // Texture coords
			1.0f,  1.0f, 0.0f,    1.0f, 0.0f, // Top right
			1.0f, -1.0f, 0.0f,    1.0f, 1.0f, // Bottom right
			-1.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom left
			-1.0f,  1.0f, 0.0f,   0.0f, 0.0f  // Top left
		};
		static constexpr std::array<unsigned int, 6> quad_indices = {
			3, 2, 1, // Second triangle
			3, 1, 0  // First triangle
		};

		Shader m_particle_shader;
		VAO m_quad_VAO;
		Buffer m_quad_VBO;
		Buffer m_quad_EBO;
		Buffer m_particle_buffer;
	public:
		ParticleRenderer();

		void update(const DeltaTime& p_delta_time, System::Scene& p_scene, const glm::vec3& p_camera_position, const Buffer& p_view_properties, const FBO& p_target_FBO);
	};
}