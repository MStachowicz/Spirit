#include "Transform.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"

#include "Utility.hpp"

#include <string>

namespace Component
{
    void Transform::DrawImGui()
    {
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::SliderFloat3("Position (m)", &mPosition.x, -50.f, 50.f);
            ImGui::SliderFloat3("Scale", &mScale.x, 0.1f, 10.f);

            if (ImGui::SliderFloat3("Roll Pitch Yaw (degrees)", &mRollPitchYaw.x, -180.f, 180.f))
            { // Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
                mOrientation = glm::normalize(Utility::toQuaternion(glm::radians(mRollPitchYaw)));
                mDirection = glm::normalize(mOrientation * StartingDirection);
            }

            ImGui::Separator();
            ImGui::Text(("Directon:    " + std::to_string(mDirection.x) + ", " + std::to_string(mDirection.y) + ", " + std::to_string(mDirection.z)).c_str());
            ImGui::Text(("Orientation: " + std::to_string(mOrientation.w) + ", " + std::to_string(mOrientation.x) + ", " + std::to_string(mOrientation.y) + ", " + std::to_string(mOrientation.z)).c_str());

            ImGui::TreePop();
        }
    }
} // namespace Component