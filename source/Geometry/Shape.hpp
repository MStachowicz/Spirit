#pragma once

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Quad.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

#include <variant>

namespace Geometry
{
	template <typename T>
	concept is_valid_shape_type = std::disjunction_v<
		std::is_same<T, Cone>,
		std::is_same<T, Cuboid>,
		std::is_same<T, Cylinder>,
		std::is_same<T, Quad>,
		std::is_same<T, Sphere>,
		std::is_same<T, Triangle>>;

	// An alternative to a base class for derived shapes. Represents a union of all possible shapes.
	class Shape
	{
	public:
		template<typename T>
		requires std::is_constructible_v<std::variant<Cone, Cuboid, Cylinder, Quad, Sphere, Triangle>, T>
		Shape(T&& p_shape) : shape{std::forward<T>(p_shape)}
		{}

		template<typename T>
		requires is_valid_shape_type<T>
		constexpr bool is() const noexcept
		{
			return std::holds_alternative<T>(shape);
		}
		template<typename T>
		requires is_valid_shape_type<T>
		constexpr T& get() noexcept
		{
			return std::get<T>(shape);
		}
		template<typename T>
		requires is_valid_shape_type<T>
		constexpr const T& get() const noexcept
		{
			return std::get<T>(shape);
		}

		std::variant<Cone, Cuboid, Cylinder, Quad, Sphere, Triangle> shape;
	};
}