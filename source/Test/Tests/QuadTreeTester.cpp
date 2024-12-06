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

			using QuadTree = Geometry::QuadTree<char>;
			using Node     = QuadTree::Node;

			{
				QuadTree quad_tree;
				{SCOPE_SECTION("Empty")
					CHECK_TRUE(quad_tree.empty(), "Empty");
					CHECK_EQUAL(quad_tree.depth(), 0, "Depth");
					CHECK_EQUAL(quad_tree.size(), 0, "Size");
				}
				{SCOPE_SECTION("Add root node")
					quad_tree.add_root_node(Geometry::AABB2D{glm::vec2{min}, glm::vec2{max}}, 'A');
					CHECK_TRUE(!quad_tree.empty(), "Not empty");
					CHECK_EQUAL(quad_tree.depth(), 0, "Depth");
					CHECK_EQUAL(quad_tree.size(), 1, "Size");
				}
				{SCOPE_SECTION("Subdivide root")
					std::array<char, 4> expected_data = {'B', 'C', 'D', 'E'};

					quad_tree.subdivide(quad_tree.root_node(), expected_data[0], expected_data[1], expected_data[2], expected_data[3]);
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
					quad_tree.for_each_child(quad_tree.root_node(), [&](Node& node)
					{
						CHECK_EQUAL(node.bounds, expected_bounds[index], "Root child bounds");
						CHECK_EQUAL(node.data,   expected_data[index], "Root child data");
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
			// Build a tree with a depth of 2 and test traversal
			//       A
			//    __/|\__
			//    B C D E
			// __/|\__
			// F G H I
			{SCOPE_SECTION("Traversal")

				QuadTree quad_tree;
				size_t A = quad_tree.add_root_node(Geometry::AABB2D{glm::vec2{min}, glm::vec2{max}}, 'A');
				size_t B = quad_tree.subdivide(quad_tree[A], 'B', 'C', 'D', 'E');
				size_t F = quad_tree.subdivide(quad_tree[B], 'F', 'G', 'H', 'I');

				CHECK_EQUAL(quad_tree.size(), 9, "Size after subdivision");
				CHECK_EQUAL(quad_tree.depth(), 2, "Depth after subdivision");

				{SCOPE_SECTION("Breadth-first")
					{SCOPE_SECTION("From root")
						std::array expected_BFS_order = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
						size_t index = 0;
						quad_tree.breadth_first_traversal([&](Node& node)
						{
							CHECK_EQUAL(node.data, expected_BFS_order[index], "BFS order " + std::to_string(index));
							++index;
						});
					}
					{SCOPE_SECTION("From node")
						std::array expected_BFS_order = {'B', 'F', 'G', 'H', 'I'};
						size_t index = 0;
						quad_tree.breadth_first_traversal(quad_tree[B], [&](Node& node)
						{
							CHECK_EQUAL(node.data, expected_BFS_order[index], "BFS order " + std::to_string(index));
							++index;
						});
					}
				}
				{SCOPE_SECTION("Depth-first")
					{SCOPE_SECTION("From root")
						std::array expected_DFS_order = {'A', 'B', 'F', 'G', 'H', 'I', 'C', 'D', 'E'};
						size_t index = 0;
						quad_tree.depth_first_traversal([&](Node& node)
						{
							CHECK_EQUAL(node.data, expected_DFS_order[index], "DFS order " + std::to_string(index));
							++index;
						});
					}
					{SCOPE_SECTION("From node")
						std::array expected_DFS_order = {'B', 'F', 'G', 'H', 'I' };
						size_t index = 0;
						quad_tree.depth_first_traversal(quad_tree[B], [&](Node& node)
						{
							CHECK_EQUAL(node.data, expected_DFS_order[index], "DFS order " + std::to_string(index));
							++index;
						});
					}
				}
			}
			{SCOPE_SECTION("Memory correctness")
				MemoryCorrectnessItem::reset();
				Geometry::QuadTree<MemoryCorrectnessItem> quad_tree;
				size_t A = 0, B = 0, F = 0, J = 0;

				{SCOPE_SECTION("Subdivide")
					quad_tree.reserve(16); // Have to reserve to prevent reallocation breaking the copy count tests

					A = quad_tree.add_root_node(Geometry::AABB2D{glm::vec2{min}, glm::vec2{max}}, MemoryCorrectnessItem{});
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 1, "1 alive count");
					CHECK_EQUAL(MemoryCorrectnessItem::count_copies(), 0, "No copies adding root");

					B = quad_tree.subdivide(quad_tree[A], MemoryCorrectnessItem{}, MemoryCorrectnessItem{}, MemoryCorrectnessItem{}, MemoryCorrectnessItem{});
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 5, "2 alive count");
					CHECK_EQUAL(MemoryCorrectnessItem::count_copies(), 0, "No copies on all rvalue subdivide");

					auto memItem = MemoryCorrectnessItem{};
					F = quad_tree.subdivide(quad_tree[B], memItem, MemoryCorrectnessItem{}, MemoryCorrectnessItem{}, MemoryCorrectnessItem{});
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 10, "3 alive count");
					CHECK_EQUAL(MemoryCorrectnessItem::count_copies(), 1, "1 copy on single lvalue subdivide");

					auto memItem2 = MemoryCorrectnessItem{};
					J = quad_tree.subdivide(quad_tree[F], MemoryCorrectnessItem{}, memItem2, memItem2, MemoryCorrectnessItem{});
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 15, "4 alive count");
					CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory errors");
					CHECK_EQUAL(MemoryCorrectnessItem::count_copies(), 3, "2 copies on 2 lvalue subdivide"); // 1 copy on F, 2 copies on J
				}

				{SCOPE_SECTION("Merge")
					quad_tree.merge(quad_tree[F]);
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 9, "1 alive count");

					quad_tree.merge(quad_tree[B]);
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 5, "2 alive count");

					quad_tree.merge(quad_tree[A]);
					CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), 1, "3 alive count");
					CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Memory errors");
				}
			}
			{SCOPE_SECTION("Test on_subdivide & on_merge")
				struct TestItem
				{
					bool on_subdivide_called = false;
					bool on_merge_called     = false;

					void on_subdivide() { on_subdivide_called = true; }
					void on_merge()     { on_merge_called     = true; }
				};

				Geometry::QuadTree<TestItem> quad_tree;
				size_t A = quad_tree.add_root_node(Geometry::AABB2D{glm::vec2{min}, glm::vec2{max}}, TestItem{});
				CHECK_TRUE(!quad_tree[A]->on_subdivide_called, "No call on root");
				CHECK_TRUE(!quad_tree[A]->on_merge_called, "No call on root");

				size_t B = quad_tree.subdivide(quad_tree[A], TestItem{}, TestItem{}, TestItem{}, TestItem{});
				CHECK_TRUE(quad_tree[A]->on_subdivide_called, "Called on parent");
				CHECK_TRUE(!quad_tree[B]->on_subdivide_called, "No call on child");
				CHECK_TRUE(!quad_tree[B]->on_merge_called, "No call on child");

				size_t F = quad_tree.subdivide(quad_tree[B], TestItem{}, TestItem{}, TestItem{}, TestItem{});
				CHECK_TRUE(quad_tree[B]->on_subdivide_called, "Called on parent");
				CHECK_TRUE(!quad_tree[F]->on_subdivide_called, "No call on child");
				CHECK_TRUE(!quad_tree[F]->on_merge_called, "No call on child");

				size_t J = quad_tree.subdivide(quad_tree[F], TestItem{}, TestItem{}, TestItem{}, TestItem{});
				CHECK_TRUE(quad_tree[F]->on_subdivide_called, "Called on parent");
				CHECK_TRUE(!quad_tree[J]->on_subdivide_called, "No call on child");
				CHECK_TRUE(!quad_tree[J]->on_merge_called, "No call on child");

				quad_tree.merge(quad_tree[F]);
				CHECK_TRUE(quad_tree[F]->on_merge_called, "Called on parent");

				quad_tree.merge(quad_tree[B]);
				CHECK_TRUE(quad_tree[B]->on_merge_called, "Called on parent");

				quad_tree.merge(quad_tree[A]);
				CHECK_TRUE(quad_tree[A]->on_merge_called, "Called on parent");
			}
		}
	}
} // namespace Test