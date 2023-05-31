#pragma once

#include "glm/glm.hpp"

#include "Transform.hpp"

namespace Component
{
    class RigidBody;

    // Free moving FPS camera. Functions using Pitch and Yaw to determine the View transformation.
    // Depends on an external source for its position e.g. Component::Transform::m_position.
    class Camera
    {
        static inline const auto Starting_Direction = glm::vec3(0.0f, 0.0f, -1.0f); // Forward direction when Camera::Pitch and Camera::Yaw are equal to 0.
        static inline const auto Pitch_Limit        = glm::radians(89.f);
        static inline const auto Yaw_constraint     = glm::radians(180.f);

    public:
        // Get the pitch and yaw angles in radians to take Starting_Direction to p_direction.
        //@param p_direction Target direction for the angles.
        //@return vec2 where x=pitch and y=yaw in radians.
        static glm::vec2 get_pitch_yaw(const glm::vec3& p_direction);

        float m_FOV;
        float m_near;
        float m_far;

        float m_pitch; // Pitch angle of view in radians, must be in the range of [-90 - 90]°
        float m_yaw;   // Yaw angle of view in radians, must be in the range of [-180 - 180]°

        float m_look_sensitivity;
        float m_move_speed; // Movement speed (m/s)
        bool m_body_move; // If a RigidBody is available, should movement use the body as opposed to incrementing position directly.
        // If primary, this camera is used to perform the view transformation when rendering the scene.
        // If multiple Cameras are primary, the first one encountered is used.
        bool m_primary;

        // Construct a camera facing p_view_direction. By default camera faces {0,0,-1}.
        //@param p_view_direction Direction the Camera will be facing after construction.
        //@param p_make_primary Make the Camera primary making rendering use it for View transformation.
        Camera(const glm::vec3& p_view_direction = Starting_Direction, bool p_make_primary = false);

        // Process mouse scrollwheel p_offset applying a zoom on the camera.
        void scroll(float p_offset);

        // Process mouse p_offset to apply a change to the view direction of the camera.
        //@param p_offset XY offset to apply in raw input data. The offset angle to apply is calculated later.
        void mouse_look(const glm::vec2& p_offset);

        // Move the position of the camera. Because Camera doesnt store a position, the Transform or RigidBody will be updated instead.
        // If m_body_move is true, the RigidBody is used if available, otherwise the position is updated directly.
        void move(Transform::MoveDirection p_direction, Transform* p_transform, RigidBody* p_body);

        // Focus the view direction on p_point.
        //@param p_point The point in world-space the camera should focus on.
        //@param p_current_position The current position of the Camera, generally the Parent Entity Transform::m_position.
        void look_at(const glm::vec3& p_point, glm::vec3& p_current_position);

        // Get the view transformation matrix taking vertices from world-space to view-space.
        //@param p_eye_position Position of the camera.
        glm::mat4 get_view(const glm::vec3& p_eye_position) const;
        //@return Camera local normalised up direction.
        glm::vec3 up() const;
        //@return Camera local normalised right direction.
        glm::vec3 right() const;
        //@return Camera local normalised forward direction.
        glm::vec3 forward() const;

        void draw_UI(Component::Transform* p_transform = nullptr);
    };
} // namespace Component