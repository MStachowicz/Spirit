#include "Triangle.hpp"

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

namespace Geometry
{
    Triangle::Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3)
        : mPoint1(pPoint1)
        , mPoint2(pPoint2)
        , mPoint3(pPoint3)
    {}

    void Triangle::transform(const glm::mat4& pTransformMatrix)
    {
        mPoint1 = glm::vec4(mPoint1, 1.f) * pTransformMatrix;
        mPoint2 = glm::vec4(mPoint2, 1.f) * pTransformMatrix;
        mPoint3 = glm::vec4(mPoint3, 1.f) * pTransformMatrix;
    }
}