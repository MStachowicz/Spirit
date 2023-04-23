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
            ImGui::Slider("Position", mPosition, -50.f, 50.f, "%.3f m");
            ImGui::Slider("Scale", mScale, 0.1f, 10.f);

            // Editor shows the rotation as Euler Roll, Pitch, Yaw, when set these need to be converted to quaternion orientation and unit direction.
            if (ImGui::Slider("Roll Pitch Yaw", mRollPitchYaw, -179.f, 179.f, "%.3f °"))
                rotateEulerDegrees(mRollPitchYaw);

            ImGui::Separator();
            ImGui::Text("Directon",    mDirection);
            ImGui::Text("Orientation", mOrientation);

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