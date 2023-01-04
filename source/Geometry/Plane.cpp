#include "Plane.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
    Plane::Plane(const glm::vec3& pPoint, const glm::vec3& pDirection)
      : mNormal{glm::normalize(pDirection)}
      , mDistance{-glm::dot(mNormal, pPoint)}
    {}
}