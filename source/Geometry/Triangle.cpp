#include "Triangle.hpp"

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

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

    void Triangle::transform(const glm::mat4& pTransformMatrix)
    {
        mPoint1 = glm::vec3(pTransformMatrix * glm::vec4(mPoint1, 1.f));
        mPoint2 = glm::vec3(pTransformMatrix * glm::vec4(mPoint2, 1.f));
        mPoint3 = glm::vec3(pTransformMatrix * glm::vec4(mPoint3, 1.f));
    }
    void Triangle::translate(const glm::vec3& pTranslation)
    {
        mPoint1 += pTranslation;
        mPoint2 += pTranslation;
        mPoint3 += pTranslation;
    }
}