#include "Transform.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"

#include "Utility.hpp"

namespace Component
{
    void Transform::rotateEulerDegrees(const glm::vec3& pRollPitchYawDegrees)
    {
        mRollPitchYaw = pRollPitchYawDegrees;
        mOrientation  = glm::normalize(Utility::toQuaternion(glm::radians(mRollPitchYaw)));
        mDirection    = glm::normalize(mOrientation * Starting_Forward_Direction);
    }
    void Transform::look_at(const glm::vec3& p_point)
    {
        if (p_point != mPosition)
        {
            mDirection    = glm::normalize(p_point - mPosition);
            mOrientation  = Utility::getRotation(Starting_Forward_Direction, mDirection);
            mRollPitchYaw = Utility::toRollPitchYaw(mOrientation);
        }
    }

    void Transform::DrawImGui()
    {
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::Slider("Position", mPosition, -50.f, 50.f, "%.3f m");
            ImGui::Slider("Scale", mScale, 0.1f, 10.f);

            // Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
            if (ImGui::Slider("Roll Pitch Yaw", mRollPitchYaw, -179.f, 179.f, "%.3f °"))
                rotateEulerDegrees(mRollPitchYaw);

            ImGui::Separator();
            ImGui::Text("Directon",    mDirection);
            ImGui::Text("Orientation", mOrientation);

            ImGui::SeparatorText("Actions");
            if (ImGui::Button("Focus on origin"))
                look_at(glm::vec3(0.f));
            ImGui::SameLine();

            // https://glm.g-truc.net/0.9.2/api/a00259.html#
            if (ImGui::Button("Reset"))
            {
                mPosition     = glm::vec3(0.0f, 0.0f, 0.0f);
                mRollPitchYaw = glm::vec3(0.0f);
                mScale        = glm::vec3(1.0f);
                mDirection    = Starting_Forward_Direction;
                mOrientation  = glm::identity<glm::quat>();
            }
            ImGui::TreePop();
        }
    }
} // namespace Component