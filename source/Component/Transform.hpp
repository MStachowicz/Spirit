#pragma once

#include "glm/gtx/quaternion.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <array>

namespace Component
{
	struct Transform
	{
		enum class MoveDirection : uint8_t { Forward, Backward, Left, Right, Up, Down };
		static constexpr glm::vec3 Starting_Forward_Direction = glm::vec3(0.f, 0.f, -1.f);

		constexpr Transform(const glm::vec3& p_position = glm::vec3(0.f)) noexcept
			: m_position{p_position}
			, m_roll_pitch_yaw{glm::vec3(0.0f)}
			, m_scale{glm::vec3(1.0f)}
			, m_direction{Starting_Forward_Direction}
			, m_orientation{glm::identity<glm::quat>()}
			, m_model{glm::identity<glm::mat4>()}
		{}

		glm::vec3 m_position;       // World-space position.
		glm::vec3 m_roll_pitch_yaw; // Roll, Pitch, Yaw rotation represented in Euler degree angles. Range [-180 - 180].
		glm::vec3 m_scale;
		glm::vec3 m_direction;      // World-space direction vector the entity is facing.
		glm::quat m_orientation;    // Unit quaternion taking the Starting_Forward_Direction to the current orientation.
		glm::mat4 m_model;

		// Rotate the object to roll pitch and yaw euler angles in the order XYZ. Angles suppled are in degrees.
		void rotateEulerDegrees(const glm::vec3& p_roll_pitch_yawDegrees);
		// Focus the view direction/orientation on p_point.
		void look_at(const glm::vec3& p_point);

		void set_model_matrix(const glm::mat4& p_model);

		glm::vec3 forward() const { return m_direction; };
		glm::vec3 right()   const { return glm::normalize(m_orientation * glm::vec3(1.f,0.f,0.f)); };
		glm::vec3 up()      const { return glm::normalize(m_orientation * glm::vec3(0.f,1.f,0.f)); };
		// Get the local space XYZ vectors (XYZ = right, up, forward).
		std::array<glm::vec3, 3> get_local_axes() const { return {right(), up(), forward()}; };

		void draw_UI();
	};
}