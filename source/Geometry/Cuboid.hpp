#pragma once

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <array>

namespace Geometry
{
	// A box centred on the origin of its own space and aligned with its axes. Where it sits and how it is turned are passed
	// separately, e.g. from a Transform, so a shape never holds a second copy of an entity's pose that could disagree with it.
	class Cuboid
	{
	public:
		void draw_UI() const;
		// The corners of the cuboid placed at p_position and turned by p_orientation.
		std::array<glm::vec3, 8> get_vertices(const glm::vec3& p_position, const glm::quat& p_orientation) const noexcept;

		glm::vec3 m_half_extents;
	};
}