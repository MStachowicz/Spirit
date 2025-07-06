#pragma once

#include "Geometry/AABB.hpp" // Assuming AABB2D is defined in this header

#include "glm/vec2.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace Geometry
{
	using Key_t   = uint64_t; // Supports 32 levels of depth. 2 bits per layer.
	using Depth_t = uint8_t;

	// Hashable representation of a cell in the quad tree
	struct QuadKey
	{
		Key_t key     = 0; // Every 2 bits represent a child node. 0 = TL, 1 = TR, 2 = BL, 3 = BR.
		Depth_t depth = 0; // Depth is required for unique QuadKeys as the key alone would be the same when the leading bits are 0!

		enum class Quadrant : uint8_t
		{
			TopLeft     = 0,
			TopRight    = 1,
			BottomLeft  = 2,
			BottomRight = 3
		};

		constexpr QuadKey(Key_t key, Depth_t depth) : key(key), depth(depth) {}
		constexpr QuadKey(std::initializer_list<Quadrant> quadrants)
		{
			key = 0;
			depth = static_cast<Depth_t>(quadrants.size());

			for (auto q : quadrants)
			{
				key = (key << 2) | static_cast<Key_t>(q);
			}
		}

		std::string to_string() const
		{
			std::string result;
			for (Depth_t d = 0; d < depth; ++d)
			{
				uint8_t shift = (depth - d - 1) * 2;
				uint8_t child = (key >> shift) & 0b11;

				switch (child)
				{
					case 0: result += "TL"; break; // TopLeft
					case 1: result += "TR"; break; // TopRight
					case 2: result += "BL"; break; // BottomLeft
					case 3: result += "BR"; break; // BottomRight
				}
				if (d < depth - 1) result += " -> "; // Separator between quadrants
			}
			return result;
		}

		AABB2D get_bounds(float root_half_size, const glm::vec2& root_center) const
		{
			glm::vec2 center = root_center;

			for (Depth_t d = 0; d < depth; ++d)
			{
				root_half_size *= 0.5f;
				uint8_t shift = (depth - d - 1) * 2;
				uint8_t child = (key >> shift) & 0b11;

				if (child & 0b01) center.x += root_half_size; // Right = +X
				else              center.x -= root_half_size; // Left  = -X

				if (child & 0b10) center.y -= root_half_size; // Down = -Y
				else              center.y += root_half_size; // Up   = +Y
			}

			return AABB2D(center - root_half_size, center + root_half_size);
		}
		bool contains(QuadKey other) const
		{
			if (depth >= other.depth) return false;
			Key_t shifted = other.key >> ((other.depth - depth) * 2);
			return shifted == key;
		}
		bool isContainedBy(QuadKey other) const
		{
			if (depth <= other.depth) return false;
			Key_t shifted = key >> ((depth - other.depth) * 2);
			return shifted == other.key;
		}
		QuadKey remap_root_quadrant(Quadrant new_root) const
		{
			if (depth == 0)
				throw std::invalid_argument("Cannot remap root quadrant of depth 0 QuadKey.");

			// Compute mask to clear the top 2 bits
			Key_t mask = (1ULL << ((depth - 1) * 2)) - 1;

			// Clear top 2 bits, and set new top quadrant
			Key_t new_key = (static_cast<Key_t>(new_root) << ((depth - 1) * 2)) | (key & mask);
			return QuadKey{new_key, depth};
		}

		bool operator==(const QuadKey& other) const noexcept = default;
	};

	template<typename Func>
	concept required_depth_func = requires(Func f, AABB2D bounds) {
		{ f(bounds) } -> std::convertible_to<Depth_t>;
	};

	template <required_depth_func Func>
	void generate_leaf_nodes(const glm::vec2& center, float current_half_size, Depth_t current_depth, Key_t current_quadkey, Depth_t max_depth, std::vector<QuadKey>& out_keys, Func&& required_depth_func)
	{
		if (max_depth > sizeof(Geometry::Key_t) * 4)
			throw std::runtime_error("Max depth is too large to be represented in a QuadKey type number of bits.");

		AABB2D bounds         = { center - glm::vec2(current_half_size), center + glm::vec2(current_half_size) };
		Depth_t desired_depth = required_depth_func(bounds);
		if (current_depth >= desired_depth || current_depth == max_depth)
		{
			out_keys.emplace_back(QuadKey{current_quadkey, current_depth});
			return;
		}

		float child_half_size = current_half_size * 0.5f;

		for (uint8_t child = 0; child < 4; ++child)
		{
			glm::vec2 offset = {
				(child & 1) ? child_half_size : -child_half_size, // Left = -X, Right = +X
				(child & 2) ? -child_half_size : child_half_size  // Down = -Y, Up = +Y
			};

			glm::vec2 child_center = center + offset;
			Key_t child_key        = (current_quadkey << 2) | child;

			generate_leaf_nodes(child_center, child_half_size, current_depth + 1, child_key, max_depth, out_keys, required_depth_func);
		}
	}
}

template<>
struct std::hash<Geometry::QuadKey>
{
	std::size_t operator()(const Geometry::QuadKey& quad_key) const noexcept
	{
		return std::hash<Geometry::Key_t>()(quad_key.key) ^ (std::hash<Geometry::Depth_t>()(quad_key.depth) << 1);
	}
};