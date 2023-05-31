#include "Camera.hpp"
#include "RigidBody.hpp"

#include "imgui.h"

namespace Component
{
    glm::vec2 Camera::get_pitch_yaw(const glm::vec3& p_direction)
    {
        // https://math.stackexchange.com/questions/470112/calculate-camera-pitch-yaw-to-face-point
        float yaw   = -std::atan2(p_direction.z, p_direction.x) - glm::radians(90.f);
        float pitch = std::atan2(p_direction.y, std::sqrt(std::pow(p_direction.x, 2) + std::pow(p_direction.z, 2)));

        if (yaw > Yaw_constraint)
            yaw -= glm::radians(360.f);
        else if (yaw < -Yaw_constraint)
            yaw += glm::radians(360.f);

        if (pitch > Pitch_Limit)
            pitch = Pitch_Limit;
        if (pitch < -Pitch_Limit)
            pitch = -Pitch_Limit;

        return {pitch, yaw};
        // return {-glm::asin(p_direction.y), glm::asin(p_direction.x / glm::sqrt(1.f - std::pow(p_direction.y, 2)))};
    }

    Camera::Camera(const glm::vec3& p_view_direction /* = glm::vec3(0.f, 0.f, -1.f)*/, bool p_make_primary /* = false*/)
        : m_FOV{45.f}
        , m_near{0.01f}
        , m_far{150.f}
        , m_pitch{get_pitch_yaw(p_view_direction).x}
        , m_yaw{get_pitch_yaw(p_view_direction).y}
        , m_look_sensitivity{0.1f}
        , m_move_speed{1.f}
        , m_body_move{false}
        , m_primary{p_make_primary}
    {}

    void Camera::scroll(float p_offset)
    {
        m_FOV -= p_offset;
        if (m_FOV < 1.0f)
            m_FOV = 1.0f;
        if (m_FOV > 45.0f)
            m_FOV = 45.0f;
    }

    void Camera::mouse_look(const glm::vec2& p_offset)
    {
        m_yaw += glm::radians(-p_offset.x * m_look_sensitivity);
        if (m_yaw > Yaw_constraint)
            m_yaw -= glm::radians(360.f);
        else if (m_yaw < -Yaw_constraint)
            m_yaw += glm::radians(360.f);

        m_pitch += glm::radians(p_offset.y * m_look_sensitivity);
        if (m_pitch > Pitch_Limit)
            m_pitch = Pitch_Limit;
        if (m_pitch < -Pitch_Limit)
            m_pitch = -Pitch_Limit;
    }

    void Camera::move(Transform::MoveDirection p_direction, Transform* p_transform, RigidBody* p_body)
    {
        // #TODO add Shift modifier = half speed.
        float adjusted_speed = m_move_speed * 0.005f; // #TODO fix this hack, InputSystem updates poll without limit, m_move_speed is not actually m/s

        if (p_body && m_body_move)
        {
            if (p_direction == Transform::MoveDirection::Forward)  p_body->apply_linear_force(forward()  * adjusted_speed);
            if (p_direction == Transform::MoveDirection::Backward) p_body->apply_linear_force(-forward() * adjusted_speed);
            if (p_direction == Transform::MoveDirection::Right)    p_body->apply_linear_force(right()    * adjusted_speed);
            if (p_direction == Transform::MoveDirection::Left)     p_body->apply_linear_force(-right()   * adjusted_speed);
            if (p_direction == Transform::MoveDirection::Up)       p_body->apply_linear_force(up()       * adjusted_speed);
            if (p_direction == Transform::MoveDirection::Down)     p_body->apply_linear_force(-up()      * adjusted_speed);
        }
        else if (p_transform)
        {
            if (p_direction == Transform::MoveDirection::Forward)  p_transform->mPosition += (forward()  * adjusted_speed * 0.05f);
            if (p_direction == Transform::MoveDirection::Backward) p_transform->mPosition += (-forward() * adjusted_speed * 0.05f);
            if (p_direction == Transform::MoveDirection::Right)    p_transform->mPosition += (right()    * adjusted_speed * 0.05f);
            if (p_direction == Transform::MoveDirection::Left)     p_transform->mPosition += (-right()   * adjusted_speed * 0.05f);
            if (p_direction == Transform::MoveDirection::Up)       p_transform->mPosition += (up()       * adjusted_speed * 0.05f);
            if (p_direction == Transform::MoveDirection::Down)     p_transform->mPosition += (-up()      * adjusted_speed * 0.05f);
        }
    }

    void Camera::look_at(const glm::vec3& p_point, glm::vec3& p_current_position)
    {
        if (p_point != p_current_position)
        {
            auto direction = glm::normalize(p_point - p_current_position);
            auto pitch_yaw = get_pitch_yaw(direction);
            m_pitch        = pitch_yaw.x;
            m_yaw          = pitch_yaw.y;
        }
    }

    glm::vec3 Camera::up() const
    {
        const float cos_pitch = glm::cos(m_pitch);
        const float sin_pitch = glm::sin(m_pitch);
        const float cos_yaw   = glm::cos(m_yaw);
        const float sin_yaw   = glm::sin(m_yaw);
        return {sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch};
    }
    glm::vec3 Camera::right() const
    {
        const float cos_yaw = glm::cos(m_yaw);
        const float sin_yaw = glm::sin(m_yaw);
        return {cos_yaw, 0, -sin_yaw}; // No roll in FPS camera, right.y is always 0
    }
    glm::vec3 Camera::forward() const
    {
        const float cos_pitch = glm::cos(m_pitch);
        const float sin_pitch = glm::sin(m_pitch);
        const float cos_yaw   = glm::cos(m_yaw);
        const float sin_yaw   = glm::sin(m_yaw);
        return glm::normalize(glm::vec3{-sin_yaw * cos_pitch, sin_pitch, -cos_pitch * cos_yaw});
    }

    glm::mat4 Camera::get_view(const glm::vec3& p_eye_position) const
    {
        // Compute the axes of the view matrix. This is derived from the concatenation of a rotation about the X axis followed by a rotation
        // about the Y axis. Then we build the view matrix by taking advantage of the fact that the final column of the matrix is just the
        // dot product of the basis vectors with the eye position of the camera.

        const float cos_pitch = glm::cos(m_pitch);
        const float sin_pitch = glm::sin(m_pitch);
        const float cos_yaw   = glm::cos(m_yaw);
        const float sin_yaw   = glm::sin(m_yaw);

        const glm::vec3 xaxis = {cos_yaw, 0, -sin_yaw};
        const glm::vec3 yaxis = {sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch};
        const glm::vec3 zaxis = {sin_yaw * cos_pitch, -sin_pitch, cos_pitch * cos_yaw};

        // Create a 4x4 view matrix from the right, up, forward and eye position vectors
        return {
            glm::vec4(xaxis.x, yaxis.x, zaxis.x, 0),
            glm::vec4(xaxis.y, yaxis.y, zaxis.y, 0),
            glm::vec4(xaxis.z, yaxis.z, zaxis.z, 0),
            glm::vec4(-glm::dot(xaxis, p_eye_position), -glm::dot(yaxis, p_eye_position), -glm::dot(zaxis, p_eye_position), 1)};
    }

    void Camera::draw_UI(Component::Transform* p_transform/*= nullptr*/)
    {
        if (ImGui::TreeNode("FPS Camera"))
        {
            ImGui::SeparatorText("Projection");
            ImGui::Slider("FOV", m_FOV, -1.f, 30.);
            ImGui::Slider("Near", m_near, 0.01f, 10.f);
            ImGui::Slider("Far", m_far, 10.f, 300.f);

            ImGui::SeparatorText("View");
            auto pitch_degrees = glm::degrees(m_pitch);
            if (ImGui::Slider("Pitch", pitch_degrees, -90.f, 90.f, "%.3f °"))
                m_pitch = glm::radians(pitch_degrees);
            auto yaw_degrees = glm::degrees(m_yaw);
            ImGui::Slider("Yaw", yaw_degrees, -100.f, 100.f, "%.3f °");
            m_yaw = glm::radians(yaw_degrees);

            ImGui::SeparatorText("Controls");
            ImGui::Slider("Look sensitivity", m_look_sensitivity, 0.01f, 1.f);
            ImGui::Slider("Move speed", m_move_speed, 0.01f, 10.f);
            ImGui::Checkbox("Body move", &m_body_move);

            ImGui::SeparatorText("Info");
            ImGui::Text("Right", right());
            ImGui::Text("Up", up());
            ImGui::Text("Forward", forward());

            ImGui::SeparatorText("Actions");
            if (p_transform)
            {
                if (ImGui::Button("Focus on origin"))
                    look_at(glm::vec3(0.f), p_transform->mPosition);
                ImGui::SameLine();
            }
            if (ImGui::Button("Reset"))
            {
                m_pitch            = 0.f;
                m_yaw              = 0.f;
                m_FOV              = 45.f;
                m_look_sensitivity = 0.1f;
            }

            ImGui::TreePop();
        }
    }
} // namespace Component