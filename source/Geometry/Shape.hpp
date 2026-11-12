#pragma once

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Cylinder.hpp"

#include <variant>

namespace Geometry
{
	// A shape in its owner's own space, placed by e.g. the owner's Transform. A Plane here is relative to that placement.
	// Quad is not one: it is four world-space corners, a polygon like Triangle, so a quad collider is a thin Cuboid.
	using Shape = std::variant<Sphere, Plane, Cuboid, Cone, Cylinder>;
}