#include "AABB.hpp"

#include "Utility/Serialise.hpp"

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtx/transform.hpp"

#include "imgui.h"

#include <algorithm>

namespace Geometry
{
	AABB::AABB()
		: m_min{0.f, 0.f, 0.f}
		, m_max{0.f, 0.f, 0.f}
	{}
	AABB::AABB(const float& p_low_X, const float& p_high_X, const float& p_low_Y, const float& p_high_Y, const float& p_low_Z, const float& p_high_Z)
		: m_min{p_low_X, p_low_Y, p_low_Z}
		, m_max{p_high_X, p_high_Y, p_high_Z}
	{}
	AABB::AABB(const glm::vec3& p_min, const glm::vec3& p_max)
		: m_min{p_min}
		, m_max{p_max}
	{}
	glm::vec3 AABB::get_size() const
	{
		return m_max - m_min;
	}
	glm::vec3 AABB::get_center() const
	{
		return (m_min + m_max) / 2.f;
	}
	void AABB::unite(const glm::vec3& p_point)
	{
		m_min.x = std::min(m_min.x, p_point.x);
		m_min.y = std::min(m_min.y, p_point.y);
		m_min.z = std::min(m_min.z, p_point.z);
		m_max.x = std::max(m_max.x, p_point.x);
		m_max.y = std::max(m_max.y, p_point.y);
		m_max.z = std::max(m_max.z, p_point.z);
	}
	void AABB::unite(const AABB& p_AABB)
	{
		m_min.x = std::min(m_min.x, p_AABB.m_min.x);
		m_min.y = std::min(m_min.y, p_AABB.m_min.y);
		m_min.z = std::min(m_min.z, p_AABB.m_min.z);
		m_max.x = std::max(m_max.x, p_AABB.m_max.x);
		m_max.y = std::max(m_max.y, p_AABB.m_max.y);
		m_max.z = std::max(m_max.z, p_AABB.m_max.z);
	}

	bool AABB::contains(const AABB& p_AABB) const
	{
		return
			m_min.x <= p_AABB.m_max.x &&
			m_max.x >= p_AABB.m_min.x &&
			m_min.y <= p_AABB.m_max.y &&
			m_max.y >= p_AABB.m_min.y &&
			m_min.z <= p_AABB.m_max.z &&
			m_max.z >= p_AABB.m_min.z;
	}

	AABB AABB::unite(const AABB& p_AABB, const AABB& pAABB2)
	{
		return {
			std::min(p_AABB.m_min.x, pAABB2.m_min.x),
			std::max(p_AABB.m_max.x, pAABB2.m_max.x),
			std::min(p_AABB.m_min.y, pAABB2.m_min.y),
			std::max(p_AABB.m_max.y, pAABB2.m_max.y),
			std::min(p_AABB.m_min.z, pAABB2.m_min.z),
			std::max(p_AABB.m_max.z, pAABB2.m_max.z)};
	}
	AABB AABB::unite(const AABB& p_AABB, const glm::vec3& p_point)
	{
		return {
			std::min(p_AABB.m_min.x, p_point.x),
			std::max(p_AABB.m_max.x, p_point.x),
			std::min(p_AABB.m_min.y, p_point.y),
			std::max(p_AABB.m_max.y, p_point.y),
			std::min(p_AABB.m_min.z, p_point.z),
			std::max(p_AABB.m_max.z, p_point.z)};
	}
	AABB AABB::transform(const AABB& p_AABB, const glm::vec3& p_position, const glm::mat4& p_rotation, const glm::vec3& p_scale)
	{
		// Reference: Real-Time Collision Detection (Christer Ericson)
		// Each vertex of transformedAABB is a combination of three transformed min and max values from p_AABB.
		// The minimum extent is the sum of all the smallers terms, the maximum extent is the sum of all the larger terms.
		// Translation doesn't affect the size calculation of the new AABB so can be added in.
		AABB transformedAABB;
		const auto rotateScale = glm::scale(p_rotation, p_scale);

		// For all 3 axes
		for (int i = 0; i < 3; i++)
		{
			// Apply translation
			transformedAABB.m_min[i] = transformedAABB.m_max[i] = p_position[i];

			// Form extent by summing smaller and larger terms respectively.
			for (int j = 0; j < 3; j++)
			{
				const float e = rotateScale[j][i] * p_AABB.m_min[j];
				const float f = rotateScale[j][i] * p_AABB.m_max[j];

				if (e < f)
				{
					transformedAABB.m_min[i] += e;
					transformedAABB.m_max[i] += f;
				}
				else
				{
					transformedAABB.m_min[i] += f;
					transformedAABB.m_max[i] += e;
				}
			}
		}

		return transformedAABB;
	}

	void AABB::draw_UI(const char* title) const
	{
		if (title)
			ImGui::SeparatorText(title);

		ImGui::Text("Min: (%.3f, %.3f, %.3f)", m_min.x, m_min.y, m_min.z);
		ImGui::Text("Max: (%.3f, %.3f, %.3f)", m_max.x, m_max.y, m_max.z);
	}

	void AABB::serialise(std::ostream& p_out, uint16_t p_version, const AABB& p_AABB)
	{
		Utility::write_binary(p_out, p_version, p_AABB.m_min);
		Utility::write_binary(p_out, p_version, p_AABB.m_max);
	}
	AABB AABB::deserialise(std::istream& p_in, uint16_t p_version)
	{
		AABB out_AABB;
		Utility::read_binary(p_in, p_version, out_AABB.m_min);
		Utility::read_binary(p_in, p_version, out_AABB.m_max);
		return out_AABB;
	}
	static_assert(Utility::Is_Serializable_v<AABB>, "AABB is not serializable. Check that the required functions are implemented.");

	void AABB2D::unite(const glm::vec2& point)
	{
		min.x = std::min(min.x, point.x);
		min.y = std::min(min.y, point.y);
		max.x = std::max(max.x, point.x);
		max.y = std::max(max.y, point.y);
	}
	void AABB2D::unite(const AABB2D& AABB)
	{
		min.x = std::min(min.x, AABB.min.x);
		min.y = std::min(min.y, AABB.min.y);
		max.x = std::max(max.x, AABB.max.x);
		max.y = std::max(max.y, AABB.max.y);
	}
	bool AABB2D::contains(const AABB2D& AABB) const
	{
		return
			min.x <= AABB.min.x &&
			max.x >= AABB.max.x &&
			min.y <= AABB.min.y &&
			max.y >= AABB.max.y;
	}
	bool AABB2D::contains(const glm::vec2& point) const
	{
		return
			min.x <= point.x &&
			max.x >= point.x &&
			min.y <= point.y &&
			max.y >= point.y;
	}
	void AABB2D::draw_UI(const char* title) const
	{
		if (title)
			ImGui::SeparatorText(title);

		ImGui::Text("Min: (%.3f, %.3f)", min.x, min.y);
		ImGui::Text("Max: (%.3f, %.3f)", max.x, max.y);
	}
} // namespace Geometry