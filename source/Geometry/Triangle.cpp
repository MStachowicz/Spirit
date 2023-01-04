#include "Triangle.hpp"

namespace Geometry
{
    Triangle::Triangle(const glm::vec3& pPoint1, const glm::vec3& pPoint2, const glm::vec3& pPoint3)
        : mPoint1(pPoint1)
        , mPoint2(pPoint2)
        , mPoint3(pPoint3)
    {}
}