#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace Component
{
    struct Transform;

    // Camera operating using Euler angles to calculate the corresponding Vectors and Matrices to define a view in 3D space.
    class Camera
    {
        static inline const glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    public:

        glm::vec3 m_position;       // eye position
        glm::vec3 m_velocity;
        glm::vec3 m_up;             // Camera space up
        glm::vec3 m_right;          // Camera space right
        glm::vec3 m_view_direction; // Camera view/front direction
        glm::mat4 m_view;
        float m_look_sensitivity;
        float m_move_speed;
        // Represents the proportion m_velocity that remains after each second. Must be between 0-1.
        // e.g. a dampening factor of 0.9 means that the velocity is reduced to 90% of its previous value after each second.
        float m_move_dampening; // per second, how much speed the camera is losing;
        float m_zoom;
    public:
        enum class move_direction {Forward, Backward, Left, Right, Up, Down};
        bool m_primary_camera; // Only one Component::Camera can be a primary Camera, this camera is used when rendering the scene.

        Camera(bool p_make_primary_camera = false);

        // Set the position and update the dependents.
        void set_position(const glm::vec3& p_new_position);
        // Focus the camera on p_point. m_position is unchanged.
        void look_at(const glm::vec3& p_point);
        // Relative to the m_view_direction, move in p_move_direction.
        void move(const move_direction& p_move_direction);
        // Apply p_x_offset and p_y_offset to the m_view_direction.
        void look(const float& p_x_offset, const float& p_y_offset, const bool& p_constrain_pitch = true);

        // Process mouse scrollwheel events. Applies a zoom on the camera.
        void scroll(const float& p_offset);
        // Returns the view transform matrix of the current state of the camera.
        const glm::mat4& get_view()     const { return m_view; };
        const glm::vec3& get_position() const { return m_position; };

        void draw_UI();
    };
} // namespace Component