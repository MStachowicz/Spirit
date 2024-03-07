#include "TwoAxisCamera.hpp"
#include "Utility/Logger.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

namespace Component
{
	constexpr float Yaw_Constraint       = glm::radians(180.f);
	constexpr float Pitch_Constraint     = glm::radians(89.f);
	constexpr float Zoom_Near_Constraint = 0.1f;

	TwoAxisCamera::TwoAxisCamera()
		: m_FOV{90.f}
		, m_near{0.001f}
		, m_far{100.f}
		, m_mouse_move_sensitivity{0.035f}
		, m_zoom_sensitivity{1.f}
		, m_distance{10.f}
		, m_pitch{0.f}
		, m_yaw{0.f}
	{}

	glm::vec3 TwoAxisCamera::get_right() const
	{
		// The right vector is the cross product of the world up vector and the camera's forward vector
		return glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), get_forward()));
	}

	glm::vec3 TwoAxisCamera::get_up() const
	{
		// The up vector is the cross product of the camera's forward vector and its right vector
		return glm::normalize(glm::cross(get_forward(), get_right()));
	}
	glm::vec3 TwoAxisCamera::get_forward() const
	{
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
		// Rotate the forward vector by the pitch and yaw angles
		forward = glm::rotateY(forward, m_yaw);
		forward = glm::rotateX(forward, m_pitch);
		return forward;
	}
	glm::vec3 TwoAxisCamera::get_position() const
	{
		// The camera's position is its distance from the origin along the negative z-axis,
		// rotated by the pitch and yaw angles
		glm::vec3 position = glm::vec3(0.0f, 0.0f, -m_distance);
		position = glm::rotateY(position, m_yaw);
		position = glm::rotateX(position, m_pitch);
		return position;
	}

	ViewInformation TwoAxisCamera::get_view_information(const float& p_aspect_ratio) const
	{
		ViewInformation view_info;
		view_info.m_view_position = get_position();
		view_info.m_view          = get_view();
		view_info.m_projection    = glm::perspective(glm::radians(m_FOV), p_aspect_ratio, m_near, m_far);
		return view_info;
	}

	glm::mat4 TwoAxisCamera::get_view() const
	{
		glm::mat4 view      = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, m_distance);

		// Rotate the camera position by the pitch and yaw angles
		cameraPos = glm::rotateY(cameraPos, m_yaw);
		cameraPos = glm::rotateX(cameraPos, m_pitch);

		// Create the view matrix
		view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		return view;
	}

	// Function to update the camera's rotation based on mouse movement
	void TwoAxisCamera::mouse_look(const glm::vec2& p_offset)
	{
		m_yaw += glm::radians(-p_offset.x * m_mouse_move_sensitivity);
		if (m_yaw > Yaw_Constraint)
			m_yaw -= glm::radians(360.f);
		else if (m_yaw < -Yaw_Constraint)
			m_yaw += glm::radians(360.f);

		m_pitch += glm::radians(p_offset.y * m_mouse_move_sensitivity);
		if (m_pitch > Pitch_Constraint)
			m_pitch = Pitch_Constraint;
		if (m_pitch < -Pitch_Constraint)
			m_pitch = -Pitch_Constraint;
	}

	void TwoAxisCamera::mouse_scroll(float p_offset)
	{
		m_distance -= p_offset * m_zoom_sensitivity;
		if (m_distance < Zoom_Near_Constraint)
			m_distance = Zoom_Near_Constraint;
	}

	void TwoAxisCamera::draw_UI()
	{
		ImGui::Begin("Camera options");
		ImGui::Slider("FOV",  m_FOV, 1.f, 90.f);
		ImGui::Slider("Near", m_near, 0.001f, 10.f);
		ImGui::Slider("Far",  m_far, 1.f, 1000.f);
		ImGui::Slider("Look sensitivity", m_mouse_move_sensitivity, 0.01f, 10.f);
		ImGui::Slider("Move speed", m_zoom_sensitivity, 0.1f, 10.f);
		ImGui::Text("View", get_view());
		ImGui::End();
	}
}