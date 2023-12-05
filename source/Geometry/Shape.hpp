#pragma once

#include "Geometry/AABB.hpp"
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
		template<typename T>
		requires std::is_constructible_v<std::variant<AABB, Cone, Cuboid, Cylinder, Line, LineSegment, Plane, Quad, Ray, Sphere, Triangle>, T>
		Shape(T&& p_shape) : shape{std::forward<T>(p_shape)}
		{}

		template<typename T>
		constexpr bool is() const noexcept
		{
			return std::holds_alternative<T>(shape);
		}
		template<typename T>
		constexpr T& get() noexcept
		{
			return std::get<T>(shape);
		}
		template<typename T>
		constexpr const T& get() const noexcept
		{
			return std::get<T>(shape);
		}

		std::variant<AABB, Cone, Cuboid, Cylinder, Line, LineSegment, Plane, Quad, Ray, Sphere, Triangle> shape;
	};
}