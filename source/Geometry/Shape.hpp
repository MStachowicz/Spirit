#pragma once

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Quad.hpp"

#include <variant>

namespace Geometry
{
	// A general shape that can be any supported geometry type.
	using Shape = std::variant<Sphere, Plane, Cuboid, Cone, Cylinder, Quad>;
}