#include "Collider.hpp"

#include "imgui.h"

#include <format>

namespace Component
{
    Collider::Collider(const Geometry::AABB pAABB, const glm::vec3& pPosition, const glm::mat4& pRotation, const glm::vec3& pScale)
        : mCollided(false)
        , mObjectAABB(pAABB)
        , mWorldAABB(Geometry::AABB::transform(mObjectAABB, pPosition, pRotation, pScale))
    {}

    void Collider::DrawImGui()
    {
        if (ImGui::TreeNode("Collider"))
        {
            ImGui::Checkbox("Colliding", &mCollided);
            ImGui::Text(std::format("AABB min: {}, {}, {}", mObjectAABB.mMin.x, mObjectAABB.mMin.y, mObjectAABB.mMin.z).c_str());
            ImGui::Text(std::format("AABB max: {}, {}, {}", mObjectAABB.mMax.x, mObjectAABB.mMax.y, mObjectAABB.mMax.z).c_str());
            ImGui::Text(std::format("World AABB min: {}, {}, {}", mWorldAABB.mMin.x, mWorldAABB.mMin.y, mWorldAABB.mMin.z).c_str());
            ImGui::Text(std::format("World AABB max: {}, {}, {}", mWorldAABB.mMax.x, mWorldAABB.mMax.y, mWorldAABB.mMax.z).c_str());
            ImGui::TreePop();
        }
    }
}