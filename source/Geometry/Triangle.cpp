#include "Triangle.hpp"

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Geometry
{
    Triangle::Triangle() noexcept
        : mPoint1{0.0f}
        , mPoint2{0.0f}
        , mPoint3{0.0f}
    {}

    Triangle::Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3) noexcept
        : mPoint1(pPoint1)
        , mPoint2(pPoint2)
        , mPoint3(pPoint3)
    {}

    glm::vec3 Triangle::centroid() const
    {
        // calculates the arithmetic mean of the three points of the triangle, which gives the center of the triangle.
        return (mPoint1 + mPoint2 + mPoint3) / 3.0f;
    }

    void Triangle::transform(const glm::mat4& pTransformation)
    {
        // First calculates the centroid of the triangle. Then transforms each point of the triangle relative to the centroid by first subtracting the centroid
        // from the point, applying the transformation matrix to the resulting vector, and then adding the centroid back to the result.
        glm::vec3 c = centroid();

        mPoint1 = glm::vec3(pTransformation * glm::vec4(mPoint1 - c, 1.f)) + c;
        mPoint2 = glm::vec3(pTransformation * glm::vec4(mPoint2 - c, 1.f)) + c;
        mPoint3 = glm::vec3(pTransformation * glm::vec4(mPoint3 - c, 1.f)) + c;
    }

    void Triangle::translate(const glm::vec3& pTranslation)
    {
        mPoint1 += pTranslation;
        mPoint2 += pTranslation;
        mPoint3 += pTranslation;
    }
}