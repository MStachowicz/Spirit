#include "Collider.hpp"

// Component
#include "Mesh.hpp"
#include "Transform.hpp"

// IMGUI
#include "imgui.h"

// STD
#include <format>

namespace Component
{
    Collider::Collider(const Component::Transform& pTransform, const Component::Mesh& pMesh)
        : mCollided(false)
        , mObjectAABB{pMesh.mModel->mCompositeMesh.mAABB}
        , mWorldAABB{Geometry::AABB::transform(mObjectAABB, pTransform.mPosition, glm::mat4_cast(pTransform.mOrientation), pTransform.mScale)}
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