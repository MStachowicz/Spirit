#include "Geometry/AABB.hpp" // Assuming AABB2D is defined in this header

#include "glm/vec2.hpp"

#include <cstdint>

namespace Geometry
{
	using Key_t   = uint64_t; // Supports 32 levels of depth. 2 bits per layer.
	using Depth_t = uint8_t;

	// Hashable representation of a cell in the quad tree
	struct QuadKey
	{
		Key_t key     = 0; // Every 2 bits represent a child node. 0 = TL, 1 = TR, 2 = BL, 3 = BR.
		Depth_t depth = 0; // Depth is required for unique QuadKeys as the key alone would be the same when the leading bits are 0!

		AABB2D get_bounds(float root_size, const glm::vec2& root_center) const
		{
			glm::vec2 center = root_center;
			float size = root_size;

			for (Depth_t d = 0; d < depth; ++d)
			{
				size *= 0.5f;
				uint8_t shift = (depth - d - 1) * 2;
				uint8_t child = (key >> shift) & 0b11;

				if (child & 0b01) center.x += size * 0.5f;
				else              center.x -= size * 0.5f;

				if (child & 0b10) center.y += size * 0.5f;
				else              center.y -= size * 0.5f;
			}

			glm::vec2 half_size(size * 0.5f);
			return AABB2D(center - half_size, center + half_size);
		}
		bool operator==(const QuadKey& other) const noexcept = default;
	};

	template<typename Func>
	concept required_depth_func = requires(Func f, AABB2D bounds) {
		{ f(bounds) } -> std::convertible_to<Depth_t>;
	};

	template <required_depth_func Func>
	void generate_leaf_nodes(const glm::vec2& center, float current_size, Depth_t current_depth, Key_t current_quadkey, Depth_t max_depth, std::vector<QuadKey>& out_keys, Func&& required_depth_func)
	{
		if (max_depth > sizeof(Geometry::Key_t) * 4)
			throw std::runtime_error("Max depth is too large to be represented in a QuadKey type number of bits.");

		AABB2D bounds         = { center - glm::vec2(current_size * 0.5f), center + glm::vec2(current_size * 0.5f) };
		Depth_t desired_depth = required_depth_func(bounds);
		if (current_depth >= desired_depth || current_depth == max_depth)
		{
			out_keys.emplace_back(QuadKey{current_quadkey, current_depth});
			return;
		}

		float child_size = current_size * 0.5f;

		for (uint8_t child = 0; child < 4; ++child)
		{
			glm::vec2 offset = {
				(child & 1) ? child_size * 0.5f : -child_size * 0.5f, // East/West
				(child & 2) ? child_size * 0.5f : -child_size * 0.5f  // South/North
			};

			glm::vec2 child_center = center + offset;
			Key_t child_key        = (current_quadkey << 2) | child;

			generate_leaf_nodes(child_center, child_size, current_depth + 1, child_key, max_depth, out_keys, required_depth_func);
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