#include "QuadTreeTester.hpp"
#include "Geometry/QuadTree.hpp"

#include "Test/MemoryCorrectnessItem.hpp"

#include <array>

namespace Test
{
	void QuadTreeTester::run_unit_tests()
	{
		SCOPE_SECTION("QuadTree")
		{
			constexpr float min = 0.f;
			constexpr float max = 100.f;
			constexpr float mid = (min + max) / 2.f;

			using QuadTree = Geometry::QuadTree;
			using Node     = QuadTree::Node;

			{
				QuadTree quad_tree;
				{SCOPE_SECTION("Empty")
					CHECK_TRUE(quad_tree.empty(), "Empty");
					CHECK_EQUAL(quad_tree.depth(), 0, "Depth");
					CHECK_EQUAL(quad_tree.size(), 0, "Size");
				}
				{SCOPE_SECTION("Add root node")
					quad_tree.add_root_node(Geometry::AABB2D{glm::vec2{min}, glm::vec2{max}});
					CHECK_TRUE(!quad_tree.empty(), "Not empty");
					CHECK_EQUAL(quad_tree.depth(), 0, "Depth");
					CHECK_EQUAL(quad_tree.size(), 1, "Size");
				}
				{SCOPE_SECTION("Subdivide root")
					quad_tree.subdivide(quad_tree.root_node());
					CHECK_TRUE(!quad_tree.empty(), "Not empty");
					CHECK_EQUAL(quad_tree.depth(), 1, "Depth after subdivision");
					CHECK_EQUAL(quad_tree.size(), 5, "Size after subdivision");

					std::array<Geometry::AABB2D, 4> expected_bounds = { // Clockwise from top-left
						Geometry::AABB2D{glm::vec2{min, mid}, glm::vec2{mid, max}},
						Geometry::AABB2D{glm::vec2{mid, mid}, glm::vec2{max, max}},
						Geometry::AABB2D{glm::vec2{min, min}, glm::vec2{mid, mid}},
						Geometry::AABB2D{glm::vec2{mid, min}, glm::vec2{max, mid}}
					};
					size_t index = 0;
					quad_tree.for_each_child(quad_tree.root_node(), [&](size_t node)
					{
						CHECK_EQUAL(quad_tree[node].bounds, expected_bounds[index], "Root child bounds");
						++index;
					});
				}
				{SCOPE_SECTION("Merge root")
					quad_tree.merge(quad_tree.root_node());
					CHECK_TRUE(!quad_tree.empty(), "Not empty");
					CHECK_EQUAL(quad_tree.depth(), 0, "Depth after merge");
					CHECK_EQUAL(quad_tree.size(), 1, "Size after merge");
				}
			}
		}
	}
} // namespace Test