#include "Collider.hpp"

// Component
#include "Mesh.hpp"
#include "Transform.hpp"

// IMGUI
#include "imgui.h"

// GLM
#include "glm/glm.hpp"

// STD
#include <format>

namespace Component
{
    Collider::Collider(const Component::Transform& pTransform, const Component::Mesh& pMesh)
        : mCollided(false)
        , mObjectAABB{}
        , mWorldAABB{Geometry::AABB::transform(mObjectAABB, pTransform.mPosition, glm::mat4_cast(pTransform.mOrientation), pTransform.mScale)}
    {}

    glm::mat4 Collider::getWorldAABBModel() const
    {
        glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), mWorldAABB.getCenter());
        glm::vec3 scale = mWorldAABB.getSize() / mObjectAABB.getSize();
        model = glm::scale(model, scale);
        return model;
    }

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