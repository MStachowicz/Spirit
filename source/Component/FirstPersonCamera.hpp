#pragma once

#include "Component/Transform.hpp"
#include "Component/ViewInformation.hpp"
#include "Geometry/Frustrum.hpp"
#include "Utility/Config.hpp"

#include "glm/glm.hpp"

#include <iostream>

namespace Component
{
	// Free moving FPS camera. Functions using Pitch and Yaw to determine the View transformation.
	// Depends on an external source for its position e.g. Component::Transform::m_position.
	class FirstPersonCamera
	{
		static inline const auto Starting_Direction = glm::vec3(0.0f, 0.0f, -1.0f); // Forward direction when  Pitch and Yaw are equal to 0.
		static inline const auto Pitch_Limit        = glm::radians(89.f);
		static inline const auto Yaw_constraint     = glm::radians(180.f);

	public:
		constexpr static size_t Persistent_ID = 5;

		// Get the pitch and yaw angles in radians to take Starting_Direction to p_direction.
		//@param p_direction Target direction for the angles.
		//@return vec2 where x=pitch and y=yaw in radians.
		static glm::vec2 get_pitch_yaw(const glm::vec3& p_direction);

		float m_vertical_FOV; // Vertical field of view in radians
		float m_near;
		float m_far;

		float m_pitch; // Pitch angle of view in radians, must be in the range of [-90 - 90]°
		float m_yaw;   // Yaw angle of view in radians, must be in the range of [-180 - 180]°

		float m_look_sensitivity;
		float m_move_speed; // Movement speed (m/s)
		// If primary, this camera is used to perform the view transformation when rendering the scene.
		// If multiple Cameras are primary, the first one encountered is used.
		bool m_primary;

		// Construct a FirstPersonCamera facing p_view_direction. By default camera faces {0,0,-1}.
		//@param p_view_direction Direction the Camera will be facing after construction.
		//@param p_make_primary Make the camera primary making rendering use it for View transformation.
		FirstPersonCamera(const glm::vec3& p_view_direction = Starting_Direction, bool p_make_primary = false);

		// Process mouse scrollwheel p_offset applying a zoom on the camera.
		void scroll(float p_offset);

		// Process mouse p_offset to apply a change to the view direction of the camera.
		//@param p_offset XY offset to apply in raw input data. The offset angle to apply is calculated later.
		void mouse_look(const glm::vec2& p_offset);

		// Focus the view direction on p_point.
		//@param p_point The point in world-space the camera should focus on.
		//@param p_current_position The current position of the Camera, generally the Parent Entity Transform::m_position.
		void look_at(const glm::vec3& p_point, glm::vec3& p_current_position);

		// Get the view transformation matrix taking vertices from world-space to view-space.
		//@param p_eye_position Position of the camera.
		glm::mat4 view(const glm::vec3& p_eye_position) const;
		// Get the ViewInformation representing the state of the camera.
		//@param p_eye_position Position of the camera.
		//@param p_aspect_ratio Aspect ratio of the parent window (width / height).
		//@return ViewInformation representing the state of the camera.
		ViewInformation view_information(const glm::vec3& p_eye_position, const float& p_aspect_ratio) const;

		//@param p_aspect_ratio Aspect ratio of the parent window (width / height).
		//@return The projection matrix. Used to transform points into clip-space.
		glm::mat4 projection(const float p_aspect_ratio) const;
		//@return Camera local normalised up direction.
		glm::vec3 up() const;
		//@return Camera local normalised right direction.
		glm::vec3 right() const;
		//@return Camera local normalised forward direction.
		glm::vec3 forward() const;
		//@return The view frustrum of the camera.
		Geometry::Frustrum frustrum(const float p_aspect_ratio, const glm::vec3& p_eye_position) const;
		//@return The horizontal field of view in radians.
		float get_horizontal_FOV(const float p_aspect_ratio) const;
		//@return The maximum distance the camera can see. Equivalent to the radius of the sphere that encompasses the view frustum.
		float get_maximum_view_distance(float aspect_ratio) const;

		void draw_UI(Component::Transform* p_transform = nullptr);
		static void serialise(std::ostream& p_out, uint16_t p_version, const FirstPersonCamera& p_first_person_camera);
		static FirstPersonCamera deserialise(std::istream& p_in, uint16_t p_version);
	};
} // namespace Component