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
	template <typename T>
	concept Has_On_Subdivide = requires(T t) { t.on_subdivide(); };
	template <typename T>
	concept Has_On_Merge = requires(T t) { t.on_merge(); };

	// Quad tree data structure for 2D space partitioning.
	// Each node has a T, a 2D axis-aligned bounding box (AABB) and can have 0 or 4 child nodes.
	// The tree is stored in a vector and uses lazy deletion on node removal, leaving holes in the vector.
	//@tparam T The type of data stored in each node alongside the AABB. If T has on_subdivide and on_merge methods, they will be called when the parent node is subdivided or merged.
	template <typename T>
	requires std::is_object_v<T>
	class QuadTree
	{
		bool is_free(size_t index) const { return std::find(free_indices.begin(), free_indices.end(), index) != free_indices.end(); }
		// Add a node to the tree. If there are no free indices, a new node is added to the end of the nodes vector.
		// Otherwise, the node at a free index is replaced with the new node.
		// add_node invalidates any iterators or pointers to Node objects.
		// @returns The index of the new node.
		template <typename U>
		requires std::is_constructible_v<T, U&&>
		size_t add_node(const AABB2D& bounds, U&& data, size_t depth)
		{
			if (free_indices.empty())
			{
				nodes.emplace_back(std::in_place, bounds, std::forward<U>(data), depth);
				return nodes.size() - 1;
			}
			else
			{
				nodes[free_indices.back()].emplace(bounds, std::forward<U>(data), depth);
				size_t index = free_indices.back();
				free_indices.pop_back();
				return index;
			}
		}
	public:
		// A node in the quad tree.
		// Node references are invalidated when the tree is modified.
		struct Node
		{
			template <typename U>
			requires std::is_constructible_v<T, U&&>
			Node(const AABB2D& bounds, U&& data, size_t depth) : bounds{bounds}, data{std::forward<U>(data)}, children_indices{std::nullopt}, depth{depth} {}

			bool leaf()           const { return !children_indices.has_value(); }
			size_t top_left()     const { return (*children_indices)[0]; }
			size_t top_right()    const { return (*children_indices)[1]; }
			size_t bottom_left()  const { return (*children_indices)[2]; }
			size_t bottom_right() const { return (*children_indices)[3]; }

			T* operator->()             { return &data; }
			const T* operator->() const { return &data; }
			T& operator*()              { return data; }
			const T& operator*() const  { return data; }

			AABB2D bounds;
			T data;
			std::optional<std::array<size_t, 4>> children_indices; // top-left, top-right, bottom-right, bottom-left
			size_t depth;
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
		//@returns The index of the node in the nodes vector.
		size_t node_index(const Node& node) const
		{
			// Search the nodes vector for the node.
			auto it = std::find_if(nodes.begin(), nodes.end(), [&](const auto& optional_node) { return &(*optional_node) == &node; });
			ASSERT_THROW(it != nodes.end(), "Node not found in tree.");

			return it - nodes.begin();
		}
		// Add a root node to the tree. Throws if a root node already exists.
		//@param bounds The bounds of the root node.
		//@param data The data to store in the root node.
		template <typename U>
		requires std::is_constructible_v<T, U&&>
		size_t add_root_node(const AABB2D& bounds, U&& data)
		{
			if (!empty()) throw std::runtime_error("Root node already exists.");
			return add_node(bounds, std::forward<U>(data), 0);
		}
		Node& root_node()
		{
			if (empty()) throw std::runtime_error("No root node exists.");
			return *nodes[0];
		}
		const Node& root_node() const
		{
			if (empty()) throw std::runtime_error("No root node exists.");
			return *nodes[0];
		}

		// Divide this node into 4 children nodes. May cause a reallocation invalidating any iterators or pointers to the nodes.
		//@param node The node to subdivide.
		//@param data The data to store in the new nodes, ordered top-left, top-right, bottom-right, bottom-left.
		//@returns The index of the first child node (top-left).
		template <typename... Args>
		requires (sizeof...(Args) == 4 && (std::is_constructible_v<T, Args&&>&& ...))
		size_t subdivide(Node& node, Args&&... child_data)
		{
			if (node.children_indices.has_value())
				throw std::runtime_error("Node already subdivided");

			const auto& bounds = node.bounds;
			const auto center  = bounds.center();

			// Define child bounds in clockwise order: top-left, top-right, bottom-right, bottom-left
			const std::array<AABB2D, 4> child_bounds = {
				AABB2D{glm::vec2{bounds.min.x, center.y}, glm::vec2{center.x, bounds.max.y}},
				AABB2D{center, bounds.max},
				AABB2D{bounds.min, center},
				AABB2D{glm::vec2{center.x, bounds.min.y}, glm::vec2{bounds.max.x, center.y}}
			};

			size_t index     = node_index(node); // Get the index of the node before add_node may invalidate the reference
			size_t new_depth = node.depth + 1; // Grab node.depth before it is invalidated by add_node

			std::array<size_t, 4> indices;
			size_t i = 0;
			((indices[i] = add_node(child_bounds[i], std::forward<Args>(child_data), new_depth), ++i), ...);

			Node& parent_node_after = *nodes[index];
			parent_node_after.children_indices = indices;

			// If the T has a on_subdivide function call it
			if constexpr (Has_On_Subdivide<T>)
				parent_node_after.data.on_subdivide();

			return indices[0]; // Return top-left child index
		}
		// Merge the children of this node into the node. Deletes the children nodes
		// Does not invalidate any iterators or pointers to the nodes
		void merge(Node& node)
		{
			if (node.leaf())
				throw std::runtime_error("Cannot merge a leaf node.");

			auto& children = *node.children_indices;

			// Delete the children and store their indices in the free_indices vector
			for (size_t i = 0; i < 4; ++i)
			{
				free_indices.push_back(children[i]);
				nodes[children[i]].reset();
			}

			node.children_indices.reset();

			if constexpr (Has_On_Merge<T>)
				node.data.on_merge();
		}

		template <typename Func>
		requires std::invocable<Func, Node&>
		void for_each_child(Node& node, Func&& func)
		{
			if (node.leaf())
				return;

			for (size_t i = 0; i < 4; ++i)
				func(*nodes[(*node.children_indices)[i]]);
		}

		// Depth-first traversal (pre-order) starting from the specified node.
		template <typename Func>
		requires std::invocable<Func, Node&>
		void depth_first_traversal(Node& start_node, Func&& func)
		{
			std::vector<size_t> stack;
			stack.push_back(node_index(start_node));

			while (!stack.empty())
			{
				size_t index = stack.back();
				stack.pop_back();

				if (is_free(index))
					continue;

				Node& node = *nodes[index];

				if constexpr (std::is_same_v<std::invoke_result_t<Func, Node&>, bool>)
				{
					if (func(node))
						return;
				}
				else
					func(node);

				if (!node.leaf())
					for (size_t i = 4; i-- > 0;)
						stack.push_back((*node.children_indices)[i]);
			}
		}
		// Depth-first traversal (pre-order) starting from the root node.
		template <typename Func>
		void depth_first_traversal(Func&& func) { depth_first_traversal(root_node(), func); }

		// Breadth-first traversal starting from the root node.
		template <typename Func>
		void breadth_first_traversal(Func&& func) { breadth_first_traversal(root_node(), func); }
		// Breadth-first traversal starting from the specified node.
		template <typename Func>
		requires std::invocable<Func, Node&>
		void breadth_first_traversal(Node& start_node, Func&& func)
		{
			std::vector<size_t> queue;
			queue.push_back(node_index(start_node));

			while (!queue.empty())
			{
				size_t index = queue.front();
				queue.erase(queue.begin());

				if (is_free(index))
					continue;

				Node& node = *nodes[index];
				func(node);

				if (!node.leaf())
					for (size_t i = 0; i < 4; ++i)
						queue.push_back((*node.children_indices)[i]);
			}
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
	};
}// namespace Geometry