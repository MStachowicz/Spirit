#include "Transform.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"

#include "Utility.hpp"

#include <string>

namespace Component
{
    void Transform::rotateEulerDegrees(const glm::vec3& pRollPitchYawDegrees)
    {
        mRollPitchYaw = pRollPitchYawDegrees;
        mOrientation = glm::normalize(Utility::toQuaternion(glm::radians(mRollPitchYaw)));
        mDirection   = glm::normalize(mOrientation * StartingDirection);
    }

    void Transform::DrawImGui()
    {
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::SliderFloat3("Position (m)", &mPosition.x, -50.f, 50.f);
            ImGui::SliderFloat3("Scale", &mScale.x, 0.1f, 10.f);

            // Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
            if (ImGui::SliderFloat3("Roll Pitch Yaw (degrees)", &mRollPitchYaw.x, -179.999f, 179.999f))
                rotateEulerDegrees(mRollPitchYaw);

            ImGui::Separator();
            ImGui::Text(("Directon:    " + std::to_string(mDirection.x) + ", " + std::to_string(mDirection.y) + ", " + std::to_string(mDirection.z)).c_str());
            ImGui::Text(("Orientation: " + std::to_string(mOrientation.w) + ", " + std::to_string(mOrientation.x) + ", " + std::to_string(mOrientation.y) + ", " + std::to_string(mOrientation.z)).c_str());

            // https://glm.g-truc.net/0.9.2/api/a00259.html#
            if (ImGui::Button("Reset"))
            {
                mPosition     = glm::vec3(0.0f, 0.0f, 0.0f);
                mRollPitchYaw = glm::vec3(0.0f);
                mScale        = glm::vec3(1.0f);
                mDirection    = StartingDirection;
                mOrientation  = glm::identity<glm::quat>();
            }
            ImGui::TreePop();
        }
    }
} // namespace Component