#include "Utility.hpp"
#include "Logger.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <numbers>

namespace Utility
{
	glm::mat4 make_model_matrix(const glm::vec3& p_position, const glm::vec3& p_rotation, const glm::vec3& p_scale)
	{
		glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), p_position);
		model = glm::rotate(model, glm::radians(p_rotation.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(p_rotation.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(p_rotation.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, p_scale);
		return model;
	}

	glm::vec3 to_roll_pitch_yaw(const glm::quat p_orientation)
	{
		glm::dvec3 angles;

		// Roll (x-axis rotation)
		double sinr_cosp = 2.0 * (p_orientation.w * p_orientation.x + p_orientation.y * p_orientation.z);
		double cosr_cosp = 1.0 - 2.0 * (p_orientation.x * p_orientation.x + p_orientation.y * p_orientation.y);
		angles.x = std::atan2(sinr_cosp, cosr_cosp);

		// Pitch (y-axis rotation)
		double sinp = 2 * (p_orientation.w * p_orientation.y - p_orientation.z * p_orientation.x);
		if (std::abs(sinp) >= 1.0)
			angles.y = std::copysign(std::numbers::pi / 2.0, sinp); // use 90 degrees if out of range
		else
			angles.y = std::asin(sinp);

		// Yaw (z-axis rotation)
		double siny_cosp = 2.0 * (p_orientation.w * p_orientation.z + p_orientation.x * p_orientation.y);
		double cosy_cosp = 1.0 - 2.0 * (p_orientation.y * p_orientation.y + p_orientation.z * p_orientation.z);
		angles.z       = std::atan2(siny_cosp, cosy_cosp);

		return glm::vec3(static_cast<float>(angles.x), static_cast<float>(angles.y), static_cast<float>(angles.z));
	}

	glm::quat to_quaternion(const float& p_roll, const float& p_pitch, const float& p_yaw)
	{
		// Abbreviations for the various angular functions
		// Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

		const float cr = glm::cos(p_roll * 0.5f);
		const float sr = glm::sin(p_roll * 0.5f);
		const float cp = glm::cos(p_pitch * 0.5f);
		const float sp = glm::sin(p_pitch * 0.5f);
		const float cy = glm::cos(p_yaw * 0.5f);
		const float sy = glm::sin(p_yaw * 0.5f);

		glm::quat q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;
		return q;
	}

	glm::quat get_rotation(const glm::vec3& p_start, const glm::vec3& p_destination)
	{
		ASSERT(glm::length2(p_start) == 1.f,       "[UTILITY] p_start is not normalized");
		ASSERT(glm::length2(p_destination) == 1.f, "[UTILITY] p_destination is not normalized");

		auto norm_u_norm_v = std::sqrt(glm::dot(p_start, p_start) * glm::dot(p_destination, p_destination));
		auto real_part     = norm_u_norm_v + glm::dot(p_start, p_destination);
		glm::vec3 w;

		if (real_part < 1.e-6f * norm_u_norm_v)
		{
			// If p_start and p_destination are exactly opposite, rotate 180 degrees round an arbitrary orthogonal axis.
			// Axis normalisation can happen later, when we normalise the quaternion.
			real_part = 0.0f;
			w         = std::abs(p_start.x) > std::abs(p_start.z) ? glm::vec3(-p_start.y, p_start.x, 0.f) : glm::vec3(0.f, -p_start.z, p_start.y);
		}
		else// Otherwise, build quaternion the standard way.
			w = glm::cross(p_start, p_destination);

		return glm::normalize(glm::quat(real_part, w.x, w.y, w.z));
	}

	glm::vec3 get_direction_from_cursor(const glm::vec2& p_cursor_pos, const glm::ivec2& p_window_size, const glm::mat4& p_projection, const glm::mat4& p_view)
	{
		ASSERT(p_cursor_pos.x >= 0.f && p_cursor_pos.y >= 0.f, "[UTILITY] Mouse coordinates cannot be negative, did you miss a Input::cursor_captured() check before calling");

		// VIEWPORT [0 - WINDOWSIZE] to OpenGL NDC [-1 - 1]
		const glm::vec2 normalizedDisplayCoords = glm::vec2((2.f * p_cursor_pos.x) / p_window_size.x - 1.f, (2.f * p_cursor_pos.y) / p_window_size.y - 1.f);

		// NDC to CLIPSPACE - Reversing normalizedDisplayCoords.y -> OpenGL windowSpace is relative to bottom left, get_cursor_position returns screen coordinates relative to top-left
		const glm::vec4 clipSpaceRay = glm::vec4(normalizedDisplayCoords.x, -normalizedDisplayCoords.y, -1.f, 1.f);

		// CLIPSPACE to EYE SPACE
		auto eyeSpaceRay = glm::inverse(p_projection) * clipSpaceRay;
		eyeSpaceRay      = glm::vec4(eyeSpaceRay.x, eyeSpaceRay.y, -1.f, 0.f); // Set the direction into the screen -1.f

		// EYE SPACE to WORLD SPACE
		const glm::vec3 worldSpaceRay = glm::normalize(glm::vec3(glm::inverse(p_view) * eyeSpaceRay));
		return worldSpaceRay;
	}
	Geometry::Ray get_cursor_ray(const glm::vec2& p_cursor_pos, const glm::ivec2& p_window_size, const glm::vec3& p_view_position, const glm::mat4& p_projection, const glm::mat4& p_view)
	{
		return Geometry::Ray(p_view_position, get_direction_from_cursor(p_cursor_pos, p_window_size, p_projection, p_view));
	}
}