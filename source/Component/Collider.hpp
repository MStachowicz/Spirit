#pragma once

#include "Geometry/AABB.hpp"

namespace Component
{
	struct Transform;

	class Collider
	{
	public:
		Geometry::AABB m_world_AABB; // The world space AABB of the entity. PhysicsSystem is responsible for updating this.
		bool m_collided;
		// Constructs a collider from an object space AABB and initial world space transformation info.
		Collider();

		void draw_UI();
	};
} // namespace Component