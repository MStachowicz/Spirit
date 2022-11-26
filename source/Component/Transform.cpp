#include "Transform.hpp"

#include "imgui.h"
#include "glm/gtx/euler_angles.hpp"

#include <string>

namespace Component
{
    void Transform::DrawImGui()
    {
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::SliderFloat3("Position (m)", &mPosition.x, -50.f, 50.f);
            ImGui::SliderFloat3("Scale", &mScale.x, 0.1f, 10.f);

            if (ImGui::SliderFloat3("Rotation (degrees)", &mRotation.x, -360.f, 360.f))
            {
                const auto rotationMat = glm::inverse(glm::eulerAngleXYZ(glm::radians(mRotation.x), glm::radians(mRotation.y), glm::radians(mRotation.z)));
                mDirection = glm::vec3(glm::vec4(0.f, 0.f, 1.f, 0.f) * rotationMat);
                mOrientation = glm::normalize(glm::toQuat(rotationMat));
            }

            ImGui::Separator();
            ImGui::Text(("Directon:    " + std::to_string(mDirection.x) + ", " + std::to_string(mDirection.y) + ", " + std::to_string(mDirection.z)).c_str());
            ImGui::Text(("Orientation: " + std::to_string(mOrientation.w) + ", " + std::to_string(mOrientation.x) + ", " + std::to_string(mOrientation.y) + ", " + std::to_string(mOrientation.z)).c_str());

            ImGui::TreePop();
        }
    }
} // namespace Component