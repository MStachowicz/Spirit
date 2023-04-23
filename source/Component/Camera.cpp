#include "Camera.hpp"
#include "Transform.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"

namespace Component
{
    Camera::Camera(bool p_make_primary_camera)
        : m_position{0.f,0.f,30.f}
        , m_velocity{0.f}
        , m_up{0.f, 1.0f, 0.0f}
        , m_right{1.f, 0.0f, 0.0f}
        , m_view_direction{0.f, 0.f, -1.f}
        , m_view{glm::lookAt(m_position, m_position + m_view_direction, m_up)}
        , m_look_sensitivity{0.001f}
        , m_move_speed{0.002f}
        , m_move_dampening{0.99f}
        , m_zoom{45.0f}
        , m_primary_camera{p_make_primary_camera}
    {}

    void Camera::set_position(const glm::vec3& p_new_position)
    {
        m_position = p_new_position;
        m_view     = glm::lookAt(m_position, m_position + m_view_direction, m_up);
    }

    void Camera::look_at(const glm::vec3& p_point)
    {
        if (p_point != m_position)
        {
            m_view_direction = glm::normalize(p_point - m_position);
            m_right          = glm::normalize(glm::cross(m_view_direction, WorldUp));
            m_up             = glm::normalize(glm::cross(m_right, m_view_direction));
            m_view           = glm::lookAt(m_position, m_position + m_view_direction, m_up);
        }
    }

    void Camera::move(const Camera::move_direction& p_move_direction)
    {
        // Doesn't apply the movement to m_position deferring this to PhysicsSystem which will
        // apply a deltatime and provide smoother motion.
        switch (p_move_direction)
        {
            case Camera::move_direction::Forward  : m_velocity += m_view_direction * m_move_speed; break;
            case Camera::move_direction::Backward : m_velocity -= m_view_direction * m_move_speed; break;
            case Camera::move_direction::Left     : m_velocity -= m_right * m_move_speed;          break;
            case Camera::move_direction::Right    : m_velocity += m_right * m_move_speed;          break;
            case Camera::move_direction::Up       : m_velocity += m_up * m_move_speed;             break;
            case Camera::move_direction::Down     : m_velocity -= m_up * m_move_speed;             break;
        }
    }

    void Camera::look(const float& p_x_offset, const float& p_y_offset, const bool& p_constrain_pitch)
    {
        m_view_direction.x += p_x_offset * m_look_sensitivity;
        m_view_direction.y += p_y_offset * m_look_sensitivity;
        m_view_direction = glm::normalize(m_view_direction);
        m_right          = glm::normalize(glm::cross(m_view_direction, WorldUp));
        m_up             = glm::normalize(glm::cross(m_right, m_view_direction));
        m_view           = glm::lookAt(m_position, m_position + m_view_direction, m_up);
    }


    void Camera::scroll(const float& p_offset)
    {
        m_zoom -= p_offset;
        if (m_zoom < 1.0f)
            m_zoom = 1.0f;
        if (m_zoom > 45.0f)
            m_zoom = 45.0f;
    }

    void Camera::draw_UI()
    {
        if (ImGui::TreeNode("Camera"))
        {
            ImGui::SeparatorText("State");
            ImGui::Slider("Position      ", m_position, -30.f, 30., "%.3f m");
            ImGui::Slider("Velocity      ", m_velocity, -1.f, 1.f, "%.3f m/s");
            ImGui::Slider("Up            ", m_up, -1.f, 1.f);
            ImGui::Slider("Right         ", m_right, -1.f, 1.f);
            ImGui::Slider("View direction", m_view_direction, -1.f, 1.f);
            ImGui::Text("View matrix", m_view);

            ImGui::SeparatorText("Controls");
            ImGui::Slider("Look sensitivity", m_look_sensitivity, 0.f, 1.f);
            ImGui::Slider("Move speed      ", m_move_speed, 0.f, 1.f);
            ImGui::Slider("Move dampening  ", m_move_dampening, 0.f, 1.f);
            ImGui::Slider("Zoom            ", m_zoom, 1.f, 45.f);

            ImGui::SeparatorText("Quick options");
            if (ImGui::Button("Look at 0,0,0"))
                look_at(glm::vec3(0.f, 0.f, 0.f));
            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                m_position       = {0.f, 0.f, 30.f};
                m_velocity       = glm::vec3{0.f};
                m_up             = {0.f, 1.0f, 0.0f};
                m_right          = {1.f, 0.0f, 0.0f};
                m_view_direction = {0.f, 0.f, -1.f};
                m_view           = {glm::lookAt(m_position, m_position + m_view_direction, m_up)};
            }

            ImGui::TreePop();
        }
    }
} // namespace Component