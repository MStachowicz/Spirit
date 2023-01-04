#pragma once

#include "glm/vec3.hpp"

namespace Geometry
{
    // A 2-dimensional surface extending indefinitely.
    // The equation of a plane in 3D: ax + by + cz + d = 0
    // where 'a' 'b' 'c' and 'd' are constants defining the plane and  'x' 'y' and 'z' are coordinates of a point on the plane.
    // e.g. a plane with normal [1,0,0] and d = 5 can be represented as 'x + 5 = 0' representing a plane that is 5 units to the left of the origin along the x-axis.
    struct Plane
    {
        glm::vec3 mNormal; // Unit length normal of the plane.
        float mDistance;   // The distance of the plane from the origin along mNormal. Represents the 'd' in the plane equation.
    };
}