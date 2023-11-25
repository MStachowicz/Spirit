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
		Shape(T&& p_shape) : shape{std::forward<T>(p_shape)}
		{
			using shape_type = std::decay_t<T>;
			static_assert(std::disjunction<
			std::is_same<shape_type, AABB>,
			std::is_same<shape_type, Cone>,
			std::is_same<shape_type, Cuboid>,
			std::is_same<shape_type, Cylinder>,
			std::is_same<shape_type, Line>,
			std::is_same<shape_type, LineSegment>,
			std::is_same<shape_type, Plane>,
			std::is_same<shape_type, Quad>,
			std::is_same<shape_type, Ray>,
			std::is_same<shape_type, Sphere>,
			std::is_same<shape_type, Triangle>>::value,
			"Type T must be one of the types in the variant");
		}

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