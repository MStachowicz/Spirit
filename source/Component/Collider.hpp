#pragma once

#include "Geometry/AABB.hpp"

#include <iostream>

namespace Component
{
	struct Transform;

	class Collider
	{
	public:
		constexpr static size_t Persistent_ID = 4;

		Geometry::AABB m_world_AABB; // The world space AABB of the entity. PhysicsSystem is responsible for updating this.
		bool m_collided;

		// Constructs a collider from an object space AABB and initial world space transformation info.
		Collider();

		void draw_UI();

		static void serialise(std::ostream& p_out, uint16_t p_version, const Collider& p_collider);
		static Collider deserialise(std::istream& p_in, uint16_t p_version);
	};
} // namespace Component