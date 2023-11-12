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
		inline static const glm::vec3 Starting_Forward_Direction = glm::vec3(0.f, 0.f, -1.f);

		constexpr Transform(const glm::vec3& p_position = glm::vec3(0.f)) noexcept
			: mPosition{p_position}
			, mRollPitchYaw{glm::vec3(0.0f)}
			, mScale{glm::vec3(1.0f)}
			, mDirection{Starting_Forward_Direction}
			, mOrientation{glm::identity<glm::quat>()}
			, mModel{glm::identity<glm::mat4>()}
		{}

		glm::vec3 mPosition;   // World-space position.
		glm::vec3 mRollPitchYaw;   // Roll, Pitch, Yaw rotation represented in Euler degree angles. Range [-180 - 180].
		glm::vec3 mScale;
		glm::vec3 mDirection; // World-space direction vector the entity is facing.
		glm::quat mOrientation; // Unit quaternion taking the Starting_Forward_Direction to the current orientation.

		glm::mat4 mModel;

		// Rotate the object to roll pitch and yaw euler angles in the order XYZ. Angles suppled are in degrees.
		void rotateEulerDegrees(const glm::vec3& pRollPitchYawDegrees);
		// Focus the view direction/orientation on p_point.
		void look_at(const glm::vec3& p_point);

		glm::vec3 forward() const { return mDirection; };
		glm::vec3 right()   const { return glm::normalize(mOrientation * glm::vec3(1.f,0.f,0.f)); };
		glm::vec3 up()      const { return glm::normalize(mOrientation * glm::vec3(0.f,1.f,0.f)); };
		// Get the local space XYZ vectors (XYZ = right, up, forward).
		std::array<glm::vec3, 3> get_local_axes() const { return {right(), up(), forward()}; };

		void draw_UI();
	};
}