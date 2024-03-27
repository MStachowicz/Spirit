#pragma once

#include <cstddef>

using EntityID = size_t;

namespace ECS
{
	class Entity
	{
	public:
		EntityID ID;
		Entity(size_t i) : ID(i) {}
		operator EntityID() const { return ID; } // Implicitly convert an Entity to an EntityID.
	};
}