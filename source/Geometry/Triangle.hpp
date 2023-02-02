#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Geometry
{
    struct Triangle
    {
        Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3);

        // Transform all the points in the triangle by the matrix.
        void transform(const glm::mat4& pTransformMatrix);
        void translate(const glm::vec3& pTranslation);

        glm::vec3 mPoint1;
        glm::vec3 mPoint2;
        glm::vec3 mPoint3;
    };
} // namespace Geometry