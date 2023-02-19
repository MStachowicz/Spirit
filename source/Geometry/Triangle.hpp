#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Geometry
{
    struct Triangle
    {
        glm::vec3 mPoint1;
        glm::vec3 mPoint2;
        glm::vec3 mPoint3;

        Triangle() noexcept;
        Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3) noexcept;

        // Transform all the points in the triangle by the trasnformation matrix applying around the centroid of the triangle.
        void transform(const glm::mat4& pTransformation);
        void translate(const glm::vec3& pTranslation);

        // Returns the current world-space centroid of the triangle.
        glm::vec3 centroid() const;

        bool operator==(const Triangle& pOther) const { return mPoint1 == pOther.mPoint1 && mPoint2 == pOther.mPoint2 && mPoint3 == pOther.mPoint3; }
    };
} // namespace Geometry