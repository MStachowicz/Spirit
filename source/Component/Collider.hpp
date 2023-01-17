#pragma once

#include "AABB.hpp"

#include "glm/fwd.hpp"
#include "glm/mat4x4.hpp"

namespace Component
{
    struct Transform;
    class Mesh;

    struct Collider
    {
        // Constructs a collider from an object space AABB and initial world space transformation info.
        Collider(const Geometry::AABB pObjectAABB, const glm::vec3& pPosition, const glm::mat4& pRotation, const glm::vec3& pScale);
        Collider(const Component::Transform& pTransform, const Component::Mesh& pMesh);

        glm::mat4 getWorldAABBModel() const;
        Geometry::AABB mObjectAABB; // AABB used for broad phase collision checking with other colliders.
        Geometry::AABB mWorldAABB; // mObjectAABB but translated into world space position. Usually using the Component::Transform.
        bool mCollided;

        void DrawImGui();
    };
} // namespace Component