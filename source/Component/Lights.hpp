#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace Geometry
{
	class AABB;
}
namespace Component
{
	class DirectionalLight
	{
	public:
		constexpr static size_t Persistent_ID = 9;

		static inline float PCF_bias = -0.001f;

		DirectionalLight() noexcept;
		DirectionalLight(const glm::vec3& p_direction, float p_ambient_intensity, float m_diffuse_intensity) noexcept;

		glm::vec3 m_direction;
		glm::vec3 m_colour;
		float m_ambient_intensity;
		float m_diffuse_intensity;
		float m_specular_intensity;

		float m_shadow_near_plane;
		float m_shadow_far_plane;
		float m_ortho_size;

		glm::mat4 get_view_proj(const Geometry::AABB& scene_AABB);

		void draw_UI();
	};

	class PointLight
	{
	public:
		constexpr static size_t Persistent_ID = 10;

		PointLight() noexcept;
		PointLight(const glm::vec3& p_position) noexcept;

		glm::vec3 m_position;
		glm::vec3 m_colour;

		float m_ambient_intensity;
		float m_diffuse_intensity;
		float m_specular_intensity;

		float m_constant;
		float m_linear;
		float m_quadratic;

		void draw_UI();
	};

	class SpotLight
	{
	public:
		constexpr static size_t Persistent_ID = 11;

		SpotLight() noexcept;

		glm::vec3 m_position;
		glm::vec3 m_direction;
		glm::vec3 m_colour;
		float m_ambient_intensity;
		float m_diffuse_intensity;
		float m_specular_intensity;

		float m_constant;
		float m_linear;
		float m_quadratic;

		float m_cutoff;
		float m_outer_cutoff;

		void draw_UI();
	};
}