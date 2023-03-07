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
        mPoint1 = glm::vec3(pTransformation * glm::vec4(mPoint1, 1.f));
        mPoint2 = glm::vec3(pTransformation * glm::vec4(mPoint2, 1.f));
        mPoint3 = glm::vec3(pTransformation * glm::vec4(mPoint3, 1.f));
    }

    void Triangle::translate(const glm::vec3& pTranslation)
    {
        mPoint1 += pTranslation;
        mPoint2 += pTranslation;
        mPoint3 += pTranslation;
    }
}