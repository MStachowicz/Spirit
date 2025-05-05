#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <iostream>

namespace Geometry
{
	// Axis alligned 3-dimensional bounding box
	class AABB
	{
	public:
		glm::vec3 m_min;
		glm::vec3 m_max;

		AABB();
		AABB(const float& p_low_X, const float& p_high_X, const float& p_low_Y, const float& p_high_Y, const float& p_low_Z, const float& p_high_Z);
		AABB(const glm::vec3& p_min, const glm::vec3& p_max);

		glm::vec3 get_size() const;
		glm::vec3 get_center() const;

		void unite(const glm::vec3& p_point);
		void unite(const AABB& p_AABB);

		bool contains(const AABB& p_AABB) const;
		void draw_UI(const char* title) const;

		// Return a bounding box encompassing both bounding boxes.
		static AABB unite(const AABB& p_AABB, const AABB& pAABB2);
		static AABB unite(const AABB& p_AABB, const glm::vec3& p_point);
		// Returns an encompassing AABB after translating and transforming p_AABB.
		static AABB transform(const AABB& p_AABB, const glm::vec3& p_position, const glm::mat4& p_rotation, const glm::vec3& p_scale);

		static void serialise(std::ostream& p_out, uint16_t p_version, const AABB& p_AABB);
		static AABB deserialise(std::istream& p_in, uint16_t p_version);
	};

	// Axis alligned 2-dimensional bounding box
	class AABB2D
	{
	public:
		glm::vec2 min;
		glm::vec2 max;

		AABB2D() : min(0.f), max(0.f) {}
		AABB2D(const glm::vec2& min, const glm::vec2& max) : min(min), max(max) {}
		bool operator==(const AABB2D& other) const = default;

		glm::vec2 size()   const { return max - min; }
		glm::vec2 center() const { return (min + max) / 2.f; }

		void unite(const glm::vec2& point);
		void unite(const AABB2D& AABB);
		bool contains(const AABB2D& AABB) const;
		bool contains(const glm::vec2& point) const;
		float distance(const glm::vec2& point) const;
		void draw_UI(const char* title) const;
	};
}// namespace Geometry