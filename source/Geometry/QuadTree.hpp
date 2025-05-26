#pragma once

#include "AABB.hpp"

#include "Utility/Logger.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Geometry
{
	// Quad tree data structure for 2D space partitioning.
	// Each node has2D axis-aligned bounding box (AABB) and can have 0 or 4 child nodes.
	// The tree is stored in a vector and uses lazy deletion on node removal, leaving holes in the vector.
	// On Node adds, the first free index is used. If no free indices are available, a new node is added to the end of the vector.
	class QuadTree
	{
		bool is_free(size_t index) const { return std::find(free_indices.begin(), free_indices.end(), index) != free_indices.end(); }
		// Add a node to the tree. If there are no free indices, a new node is added to the end of the nodes vector.
		// Otherwise, the node at a free index is replaced with the new node.
		// add_node invalidates any iterators or pointers to Node objects.
		// @returns The index of the new node.
		template <typename AddNodeFunc>
		size_t add_node(const AABB2D& bounds, size_t depth, std::optional<size_t> parent_index = std::nullopt, AddNodeFunc&& add_node_func = {})
		{
			size_t index = 0;
			if (free_indices.empty())
			{
				nodes.emplace_back(std::in_place, bounds, depth, parent_index);
				index = nodes.size() - 1;
			}
			else
			{
				nodes[free_indices.back()].emplace(bounds, depth, parent_index);
				index = free_indices.back();
				free_indices.pop_back();
			}

			add_node_func(index);
			return index;
		}
	public:
		// A node in the quad tree.
		// Node references are invalidated when the tree is modified.
		struct Node
		{
			Node(const AABB2D& bounds, size_t depth, std::optional<size_t> parent_index = std::nullopt)
				: bounds{bounds}, children_indices{std::nullopt}, depth{depth}, parent_index{parent_index} {}

			bool leaf()           const { return !children_indices.has_value(); }
			size_t top_left()     const { return (*children_indices)[0]; }
			size_t top_right()    const { return (*children_indices)[1]; }
			size_t bottom_left()  const { return (*children_indices)[2]; }
			size_t bottom_right() const { return (*children_indices)[3]; }

			AABB2D bounds;
			std::optional<std::array<size_t, 4>> children_indices; // top-left, top-right, bottom-left, bottom-right
			size_t depth;
			std::optional<size_t> parent_index; // Index of the parent node or nullopt if it's the root.
		};

		QuadTree() : nodes{}, free_indices{} {}
		// @returns The number of nodes in the tree.
		size_t size() const { return nodes.size() - free_indices.size(); }
		// @returns True if the tree has no nodes.
		bool empty()  const { return size() == 0; }
		// @returns The maximum depth of the tree. Root node has depth 0.
		size_t depth() const
		{
			size_t max_depth = 0;
			for (auto it = begin(); it != end(); ++it)
				max_depth = std::max(max_depth, it->depth);
			return max_depth;
		}
		// Reserve space for the specified number of nodes.
		void reserve(size_t size) { nodes.reserve(size); }

		bool is_root(size_t node_index) const
		{
			if (empty()) throw std::runtime_error("No root node exists.");
			return node_index == 0;
		}

		std::optional<std::array<size_t, 3>> get_siblings(size_t node_index) const
		{
			if (empty()) throw std::runtime_error("No root node exists.");

			auto& node   = nodes[node_index].value();
			if (node.parent_index.has_value())
			{
				auto& parent = nodes[node.parent_index.value()].value();

				std::array<size_t, 3> siblings;
				size_t sibling_count = 0;
				for (size_t i = 0; i < 4; ++i)
				{
					if (parent.children_indices->at(i) != node_index)
						siblings[sibling_count++] = parent.children_indices->at(i);
				}
				return siblings;
			}

			return std::nullopt;
		}
		// Given a point, find the node that contains it. O(log n) complexity.
		// @param point The point to search for.
		// @returns The node that contains the point.
		size_t get_current_node(const glm::vec2& point)
		{
			// Recursively search the tree for the node that contains the point.
			// At each node we split the search space into 4 quadrants and search the appropriate one.
			// The complexity of this algorithm is O(log n) where n is the number of nodes in the tree.
			size_t current_node_index = root_node();
			Node* current_node = &this->operator[](current_node_index);

			if (!current_node->bounds.contains(point))
				LOG_ERROR(false, "[QUADTREE] Player position is outside the bounds of the quad tree. Player position: ({:.2f}, {:.2f}), Quad tree bounds: ({:.2f}, {:.2f}) -> ({:.2f}, {:.2f})",
					point.x, point.y, current_node->bounds.min.x, current_node->bounds.min.y, current_node->bounds.max.x, current_node->bounds.max.y);

			while (!current_node->leaf())
			{
				bool high_x = point.x > this->operator[](current_node->bottom_left()).bounds.max.x;
				bool high_y = point.y > this->operator[](current_node->bottom_left()).bounds.max.y;

				if (high_x)
					if (high_y)
						current_node_index = current_node->top_right();
					else
						current_node_index = current_node->bottom_right();
				else
					if (high_y)
						current_node_index = current_node->top_left();
					else
						current_node_index = current_node->bottom_left();

				current_node = &this->operator[](current_node_index);
			}

			return current_node_index;
		}
		// Find the neighbor of a node in the specified offset direction.
		// @param node_index The index of the node to find the neighbor of.
		// @param offset The offset direction to search in in units of the node's size from the bottom left corner of the node.
		// @param target_depth The depth of the node to search for.
		// @returns The index of the neighbor node of the same size or nullopt if no neighbor was found. The returned node could have children nodes, of which multiple could be valid neighbors.
		std::optional<size_t> get_neighbour_with_offset(size_t node_index, const glm::vec2& offset, size_t target_depth) const
		{
			const auto& node = nodes[node_index].value();
			auto node_depth  = node.depth;

			if (offset.x == 0.f && offset.y == 0.f && target_depth == node_depth)
				return node_index;
			else if (offset.x >= 0.f && offset.y >= 0.f && offset.x < 1.f && offset.y < 1.f && target_depth > node_depth)
			{
				// We are searching inside the current node for a finer node but we have no children, return the current node.
				if (target_depth > node_depth && !node.children_indices.has_value())
					return node_index;

				// Knowing we are a child of current node, we can check the offset to find the correct child node
				// then translate and scale the offset into the child node's space and recurse.
				bool high_x = offset.x >= 0.5f;
				bool high_y = offset.y >= 0.5f;
				glm::vec2 new_offset = offset;

				if (high_x)
				{
					if (high_y)
					{
						new_offset = (offset - glm::vec2{0.5f}) * 2.f;      // Top right 0.5, 0.5 -> 1.0, 1.0
						return get_neighbour_with_offset(node.top_right(), new_offset, target_depth);
					}
					else
					{
						new_offset = (offset - glm::vec2{0.5f, 0.f}) * 2.f; // Bottom right 0.5, 0.0 -> 1.0, 0.5
						return get_neighbour_with_offset(node.bottom_right(), new_offset, target_depth);
					}
				}
				else // Low x
				{
					if (high_y)
					{
						new_offset = (offset - glm::vec2{0.f, 0.5f}) * 2.f; // Top left 0.0, 0.5 -> 0.5, 1.0
						return get_neighbour_with_offset(node.top_left(), new_offset, target_depth);
					}
					else // low_y
					{
						new_offset = (offset - glm::vec2{0.f}) * 2.f; // Bottom left 0.0, 0.0 -> 0.5, 0.5
						return get_neighbour_with_offset(node.bottom_left(), new_offset, target_depth);
					}
				}

			}
			else// if (offset.x < 0.f || offset.y < 0.f || offset.x > 1.f || offset.y > 1.f || target_depth > node_depth)
			{
				// We are searching outside the current node, so we need to find the neighbour node by climbing up the tree and searching for the neighbour node.

				if (!node.parent_index.has_value()) // Root node cannot have a neighbor
					return std::nullopt;

				auto parent_index  = node.parent_index;
				const auto& parent = nodes[parent_index.value()];
				glm::vec2 new_offset = offset;

				if (node_index == parent->top_left())
					new_offset = (new_offset + glm::vec2{0.f, 1.0f}) * 0.5f;
				else if (node_index == parent->top_right())
					new_offset = (new_offset + glm::vec2{1.0f, 1.0f}) * 0.5f;
				else if (node_index == parent->bottom_left())
					new_offset = (new_offset + glm::vec2{0.f, 0.f}) * 0.5f;
				else if (node_index == parent->bottom_right())
					new_offset = (new_offset + glm::vec2{1.0f, 0.f}) * 0.5f;

				return get_neighbour_with_offset(parent_index.value(), new_offset, target_depth);
			}
			// else
			// 	return std::nullopt; // No neighbor found
		}

		enum class Direction : uint8_t
		{
			Top,
			Bottom,
			Left,
			Right
		};
		std::optional<size_t> find_neighbor(size_t node_index, Direction direction) const
		{
			glm::vec2 offset{0.f, 0.f};
			switch (direction)
			{
				case Direction::Top:    offset.y = 1.f;  break;
				case Direction::Bottom: offset.y = -1.f; break;
				case Direction::Left:   offset.x = -1.f; break;
				case Direction::Right:  offset.x = 1.f;  break;
			}

			return get_neighbour_with_offset(node_index, offset, nodes[node_index].value().depth);
		}


		// Add a root node to the tree. Throws if a root node already exists.
		template <typename Func>
		size_t add_root_node(const AABB2D& bounds, Func&& add_node_func = {})
		{
			if (!empty()) throw std::runtime_error("Root node already exists.");
			return add_node(bounds, 0, std::nullopt, add_node_func);
		}
		size_t add_root_node(const AABB2D& bounds)
		{
			return add_root_node(bounds, [](size_t) {});
		}
		size_t root_node() const
		{
			if (empty()) throw std::runtime_error("No root node exists.");
			return 0;
		}

		// Divide this node into 4 children nodes. May cause a reallocation invalidating any iterators or pointers to the nodes.
		// The subdivision could cascade to the neighbors of this node if the difference in depth is greater than 1.
		// @param node_index The index of the node to subdivide.
		// @param add_node_func A function to call when a node is added. The function takes the index of the new node as an argument.
		// @param on_subdivided_func A function to call when the node is subdivided. The function takes the index of the node as an argument.
		template <typename AddNodeFunc, typename OnSubdivideFunc>
		void subdivide(size_t node_index, AddNodeFunc&& add_node_func = {}, OnSubdivideFunc&& on_subdivided_func = {})
		{
			Node* node = &nodes[node_index].value();

			if (node->children_indices.has_value())
				throw std::runtime_error("Node already subdivided");

			const auto bounds = node->bounds;
			const auto center  = bounds.center();

			// Define child bounds in order of top-left, top-right, bottom-left, bottom-right
			const std::array<AABB2D, 4> child_bounds = {
				AABB2D{glm::vec2{bounds.min.x, center.y}, glm::vec2{center.x, bounds.max.y}},
				AABB2D{center, bounds.max},
				AABB2D{bounds.min, center},
				AABB2D{glm::vec2{center.x, bounds.min.y}, glm::vec2{bounds.max.x, center.y}}
			};

			size_t new_depth = node->depth + 1; // Grab node.depth before it's invalidated by add_node
			std::array<size_t, 4> indices;

			for (size_t i = 0; i < 4; ++i)
				indices[i] = add_node(child_bounds[i], new_depth, node_index, add_node_func);

			node = &(nodes[node_index].value()); // Reassign node* to the original node after it was invalidated by add_node
			node->children_indices = indices;
			on_subdivided_func(node_index);

			// Get the neighbours of this node and subdivide them if they are not already subdivided
			for (auto dir : {Direction::Top, Direction::Bottom, Direction::Left, Direction::Right})
			{
				if (auto neighbor = find_neighbor(node_index, dir))
				{
					auto& neighbor_node = nodes[neighbor.value()];
					if (neighbor_node->depth + 1 < new_depth)
						subdivide(neighbor.value(), add_node_func, on_subdivided_func);
				}
			}
		}
		void subdivide(size_t node_index)
		{
			subdivide(node_index, [](size_t) {}, [](size_t) {});
		}

		// Prune the tree by merging nodes that are deeper than the specified max depth.
		// Recursively merges the children depth-first until the depth is no greater than max_depth.
		template <typename ClearNodeFunc, typename PostMergeFunc>
		void ensure_depth_no_greater_than(size_t node_index, size_t max_depth, ClearNodeFunc&& clear_node_func = {}, PostMergeFunc&& post_merge_func = {})
		{
			auto& node = nodes[node_index].value();

			if (node.children_indices.has_value())
				ensure_depth_no_greater_than(node.children_indices->front(), max_depth, clear_node_func, post_merge_func);

			if (node.depth > max_depth)
			{
				if (auto parent_index = node.parent_index)
				{
					merge(parent_index.value(), clear_node_func, post_merge_func);
				}
			}
		}

		// Merge the children of this node into the node. Deletes the children nodes
		// Does not invalidate any iterators or pointers to the nodes
		template <typename ClearNodeFunc, typename PostMergeFunc>
		void merge(size_t node_index, ClearNodeFunc&& clear_node_func = {}, PostMergeFunc&& post_merge_func = {})
		{
			Node& node = nodes[node_index].value();

			if (node.leaf())
				throw std::runtime_error("Cannot merge a leaf node.");

			auto& children = *node.children_indices;

			// Delete the children and store their indices in the free_indices vector
			for (size_t i = 0; i < 4; ++i)
			{
				clear_node_func(children[i]);
				free_indices.push_back(children[i]);
				nodes[children[i]].reset();
			}

			node.children_indices.reset();
			post_merge_func(node_index);

			for (auto dir : {Direction::Top, Direction::Bottom, Direction::Left, Direction::Right})
			{
				if (auto neighbor = find_neighbor(node_index, dir))
					ensure_depth_no_greater_than(neighbor.value(), node.depth, clear_node_func, post_merge_func);
			}
		}
		void merge(size_t node_index)
		{
			merge(node_index, [](size_t) {}, [](size_t) {});
		}

		template <typename Func>
		requires std::invocable<Func, size_t>
		void for_each_child(size_t node, Func&& func)
		{
			if (this->operator[](node).leaf())
				return;

			auto children = this->operator[](node).children_indices.value();
			for (auto child : children)
				func(child);
		}

		struct QuadTreeIterator
		{
			QuadTree& quad_tree;
			size_t index;

			QuadTreeIterator(QuadTree& quad_tree, size_t index) : quad_tree{quad_tree}, index{index} {}
			QuadTreeIterator& operator++()
			{
				do { ++index; }
				while (index < quad_tree.nodes.size() && quad_tree.is_free(index));
				return *this;
			}
			Node& operator*() { return quad_tree[index]; }
			Node* operator->() { return &quad_tree[index]; }
			bool operator!=(const QuadTreeIterator& other) const { return index != other.index; }
		};
		struct ConstQuadTreeIterator
		{
			const QuadTree& quad_tree;
			size_t index;

			ConstQuadTreeIterator(const QuadTree& quad_tree, size_t index) : quad_tree{quad_tree}, index{index} {}
			ConstQuadTreeIterator& operator++()
			{
				do { ++index; }
				while (index < quad_tree.nodes.size() && quad_tree.is_free(index));
				return *this;
			}
			const Node& operator*() { return quad_tree[index]; }
			const Node* operator->() { return &quad_tree[index]; }
			bool operator!=(const ConstQuadTreeIterator& other) const { return index != other.index; }
		};
		QuadTreeIterator begin()             { return QuadTreeIterator(*this, 0); }
		QuadTreeIterator end()               { return QuadTreeIterator(*this, nodes.size()); }
		ConstQuadTreeIterator begin()  const { return ConstQuadTreeIterator(*this, 0); }
		ConstQuadTreeIterator end()    const { return ConstQuadTreeIterator(*this, nodes.size()); }
		ConstQuadTreeIterator cbegin() const noexcept { return begin(); }
		ConstQuadTreeIterator cend()   const noexcept { return end(); }

		Node& operator[](size_t index)
		{
			ASSERT(!is_free(index), "Cannot access a freed index.");
			ASSERT(index < nodes.size(), "Index out of bounds.");
			return *nodes[index];
		}
		const Node& operator[](size_t index) const
		{
			ASSERT(!is_free(index), "Cannot access a freed index.");
			ASSERT(index < nodes.size(), "Index out of bounds.");
			return *nodes[index];
		}
	private:
		std::vector<std::optional<Node>> nodes; // Stores all the freed and active nodes.
		std::vector<size_t> free_indices;       // Indices into nodes vector that are free.
	}; // class QuadTree
}// namespace Geometry