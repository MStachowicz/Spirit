#pragma once

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <array>

namespace Geometry
{
	class Cuboid
	{
	public:
		constexpr Cuboid(const glm::vec3& p_center, const glm::vec3& p_half_extents = glm::vec3(1.f), const glm::quat& p_rotation = glm::identity<glm::quat>()) noexcept
			: m_center{p_center}
			, m_half_extents{p_half_extents}
			, m_rotation{p_rotation}
		{}

		void transform(const glm::vec3& p_translation, const glm::quat& p_rotation, const glm::vec3& p_scale) noexcept;
		void draw_UI() const;
		std::array<glm::vec3, 8> get_vertices() const noexcept;

		glm::vec3 m_center;
		glm::vec3 m_half_extents;
		glm::quat m_rotation;
	};
}