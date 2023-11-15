#pragma once

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Quad.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

#include <variant>

namespace Geometry
{
	// An alternative to a base class for derived shapes. Represents a union of all possible shapes.
	class Shape
	{
	public:
		std::variant<Cone, Cuboid, Cylinder, Line, LineSegment, Plane, Quad, Ray, Sphere, Triangle> shape;
	};
}