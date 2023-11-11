#pragma once

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Geometry
{
	class Cuboid
	{
	public:
		constexpr Cuboid(const glm::vec3& p_position, const glm::vec3& p_scale = glm::vec3(1.f), const glm::quat& p_rotation = glm::identity<glm::quat>()) noexcept
			: m_position{p_position}
			, m_scale{p_scale}
			, m_rotation{p_rotation}
		{}

		glm::vec3 m_position;
		glm::vec3 m_scale;
		glm::quat m_rotation;
	};
}