#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // Get the moment of inertia (I) for a solid cuboid in SI units (kg m²)
    static glm::vec3 CuboidInertia(float pMass, float pHeight, float pWidth, float pDepth)
    {
        const float InertiaX = (1.f/12.f) * pMass * (std::pow(pDepth, 2.f) + std::pow(pHeight, 2.f));
        const float InertiaY = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pDepth, 2.f));
        const float InertiaZ = (1.f/12.f) * pMass * (std::pow(pWidth, 2.f) + std::pow(pHeight, 2.f));
        return glm::vec3(InertiaX, InertiaY, InertiaZ);
    }
}