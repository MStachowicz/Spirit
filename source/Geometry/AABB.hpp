#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Geometry
{
    // Axis alligned bounding box
    class AABB
    {
    public:
        glm::vec3 mMin;
        glm::vec3 mMax;

        AABB();
        AABB(const float& pLowX, const float& pHighX, const float& pLowY, const float& pHighY, const float& pLowZ, const float& pHighZ);
        AABB(const glm::vec3& pLowPoint, const glm::vec3& pHighPoint);

        glm::vec3 getSize() const;
        glm::vec3 getCenter() const;
        // Returns the surface normal of the AABB at pPointOnAABBInWorldSpace.
        // This function does not handle edge points on the AABB and returns only the normal of one of the 6 faces of the cuboid.
        glm::vec3 getNormal(const glm::vec3& pPointOnAABBInWorldSpace) const;

        void unite(const glm::vec3& pPoint);
        void unite(const AABB& pAABB);

        bool contains(const AABB& pAABB) const;
        // Return a bounding box encompassing both bounding boxes.
        static AABB unite(const AABB& pAABB, const AABB& pAABB2);
        static AABB unite(const AABB& pAABB, const glm::vec3& pPoint);
        // Returns an encompassing AABB after translating and transforming pAABB.
        static AABB transform(const AABB& pAABB, const glm::vec3& pPosition, const glm::mat4& pRotation, const glm::vec3& pScale);
    };
}// namespace Geometry