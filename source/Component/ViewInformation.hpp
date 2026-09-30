#pragma once

#include "Geometry/AABB.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Component
{
	struct ClippingPlanes
	{
		float m_near;
		float m_far;

		static ClippingPlanes fit_to_bounds(const Geometry::AABB& p_bounds, const glm::vec3& p_eye_position, const glm::vec3& p_forward, bool p_orthographic = false)
		{
			const float center_depth = glm::dot(p_bounds.get_center() - p_eye_position, p_forward);
			const float half_depth = glm::dot(p_bounds.get_size() * 0.5f, glm::abs(p_forward));
			const float nearest_depth = center_depth - half_depth;
			const float farthest_depth = center_depth + half_depth;
			const float padding = glm::max(0.001f, glm::max(half_depth, glm::abs(center_depth)) * 0.05f);

			ClippingPlanes planes;
			planes.m_near = p_orthographic ? nearest_depth - padding : glm::max(0.0001f, nearest_depth * 0.95f);
			planes.m_far = glm::max(planes.m_near + 0.001f, farthest_depth + glm::max(padding, glm::abs(farthest_depth) * 0.05f));
			if (!p_orthographic)
				planes.m_far = glm::max(1.f, planes.m_far);
			return planes;
		}
	};

	struct ViewInformation
	{
		glm::mat4 m_view          = {glm::identity<glm::mat4>()};
		glm::mat4 m_projection    = {glm::identity<glm::mat4>()};
		glm::vec4 m_view_position = {0.f, 0.f, 0.f, 1.f}; // w must be 1.f for position to be transformed correctly.
	};
}