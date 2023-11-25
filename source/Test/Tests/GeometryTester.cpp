#include "GeometryTester.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

#include "Utility/Stopwatch.hpp"
#include "Utility/Utility.hpp"

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "OpenGL/DebugRenderer.hpp"

#include <array>

namespace Test
{
	void GeometryTester::runUnitTests()
	{
		runAABBTests();
		runTriangleTests();
		run_frustrum_tests();
		run_sphere_tests();
		run_point_tests();
	}
	void GeometryTester::runPerformanceTests()
	{
		constexpr size_t triangleCount = 1000000 * 2;
		std::vector<float> randomTrianglePoints = Utility::get_random(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), triangleCount * 3 * 3);
		std::vector<Geometry::Triangle> triangles;
		triangles.reserve(triangleCount);

		for (size_t i = 0; i < triangleCount; i++)
			triangles.emplace_back(Geometry::Triangle(
				glm::vec3(randomTrianglePoints[i], randomTrianglePoints[i + 1], randomTrianglePoints[i + 2]),
				glm::vec3(randomTrianglePoints[i + 3], randomTrianglePoints[i + 4], randomTrianglePoints[i + 5]),
				glm::vec3(randomTrianglePoints[i + 6], randomTrianglePoints[i + 7], randomTrianglePoints[i + 8])));

		auto triangleTest = [&triangles](const size_t& pNumberOfTests)
		{
			if (pNumberOfTests * 2 > triangles.size()) // Multiply by two to account for intersection calling in pairs.
				throw std::logic_error("Not enough triangles to perform pNumberOfTests. Bump up the triangleCount variable to at least double the size of the largest performance test.");

			for (size_t i = 0; i < pNumberOfTests * 2; i += 2)
				Geometry::intersecting(triangles[i], triangles[i + 1]);
		};

		auto triangleTest1 = [&triangleTest]() { triangleTest(1); };
		auto triangleTest10 = [&triangleTest]() { triangleTest(10); };
		auto triangleTest100 = [&triangleTest]() { triangleTest(100); };
		auto triangleTest1000 = [&triangleTest]() { triangleTest(1000); };
		auto triangleTest10000 = [&triangleTest]() { triangleTest(10000); };
		auto triangleTest100000 = [&triangleTest]() { triangleTest(100000); };
		auto triangleTest1000000 = [&triangleTest]() { triangleTest(1000000); };
		runPerformanceTest({"Triangle v Triangle 1", triangleTest1});
		runPerformanceTest({"Triangle v Triangle 10", triangleTest10});
		runPerformanceTest({"Triangle v Triangle 100", triangleTest100});
		runPerformanceTest({"Triangle v Triangle 1,000", triangleTest1000});
		runPerformanceTest({"Triangle v Triangle 10,000", triangleTest10000});
		runPerformanceTest({"Triangle v Triangle 100,000", triangleTest100000});
		runPerformanceTest({"Triangle v Triangle 1,000,000", triangleTest1000000});
	}

	void GeometryTester::runAABBTests()
	{
		{SCOPE_SECTION("Default intiailise");
			Geometry::AABB aabb;
			runUnitTest({aabb.get_size() == glm::vec3(0.f), "AABB initialise size at 0", "Expected size of default AABB to be 0"});
			runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
			runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
		}
		{SCOPE_SECTION("Initialise with a min and max");
			// An AABB at low point [-1,-1,-1] to [1,1,1]
			auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
			runUnitTest({aabb.get_size() == glm::vec3(2.f), "AABB initialised with min and max size at 2", "Expected size of AABB to be 2"});
			runUnitTest({aabb.get_center() == glm::vec3(0.f), "AABB initialise with min and max position", "Expected AABB to center at [0, 0, 0]"});
		}
		{SCOPE_SECTION("Initialise with a min and max not at origin");
			// An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
			auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
			runUnitTest({aabb.get_size() == glm::vec3(4.f), "AABB initialised with min and max not at origin", "Expected size of AABB to be 4.f"});
			runUnitTest({aabb.get_center() == glm::vec3(3.f), "AABB initialised with min and max not at origin", "Expected AABB to center at [3, 3, 3]"});
		}

		{SCOPE_SECTION("Tranform");
		}
		{SCOPE_SECTION("Unite");

			{SCOPE_SECTION("check result of the static AABB::unite is the same as member unite");
			}
		}
		{SCOPE_SECTION("Contains");

		}

		{SCOPE_SECTION("Intersections");
			{SCOPE_SECTION("No touch in all directions");
				const auto originAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));

				// Create an AABB in line with all 6 faces
				const auto leftAABB  = Geometry::AABB::transform(originAABB, glm::vec3(), glm::mat4(1.f), glm::vec3(1.f));
				const auto rightAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto aboveAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto belowAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto frontAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto backAABB  = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
			}
		}
	}

	void GeometryTester::runTriangleTests()
	{SCOPE_SECTION("Triangle")
		const auto control = Geometry::Triangle(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f));

		{SCOPE_SECTION("Transform");

			auto triangle = control;
			triangle.transform(glm::identity<glm::mat4>());
			CHECK_EQUAL(triangle, control, "Identity transform doesn't change triangle");

			{SCOPE_SECTION("Translate");
				auto transformed = control;
				const glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.f, 0.f, 0.f)); // Keep translating right

				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(3.f, 1.f, 0.f), glm::vec3(4.f, -1.f, 0.f), glm::vec3(2.f, -1.f, 0.f));
					CHECK_EQUAL(transformed, expected, "Right");
				}
			}
			{SCOPE_SECTION("Rotate");
				auto transformed = control;
				const glm::mat4 transform = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333337f, 1.3333334f), glm::vec3(0.99999994f, -0.3333333f, -0.6666666f), glm::vec3(-0.99999994f, -0.3333333f, -0.6666666f));
					CHECK_EQUAL(transformed, expected, "Rotate 1");
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -1.6666667f, -5.9604645e-08f), glm::vec3(0.9999999f, 0.3333333f, 8.940697e-08f), glm::vec3(-0.9999999f, 0.3333333f, 8.940697e-08f));
					CHECK_EQUAL(transformed, expected, "Rotate 2");
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333325f, -1.3333333f), glm::vec3(0.9999998f, -0.33333346f, 0.66666675f), glm::vec3(-0.9999998f, -0.33333346f, 0.66666675f));
					CHECK_EQUAL(transformed, expected, "Rotate 3");
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, 0.9999999f, 2.9802322e-07f), glm::vec3(0.99999976f, -1.0000001f, 0.f), glm::vec3(-0.99999976f, -1.0000001f, 0.f));
					CHECK_EQUAL(transformed, expected, "Rotate 4");
				}
			}
		}

		{SCOPE_SECTION("Triangle v Triangle intersection")
			{SCOPE_SECTION("Coplanar seperated");
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.5f, 0.f),  glm::vec3(1.f, 1.5f, 0.f),   glm::vec3(-1.f, 1.5f, 0.f));
				auto t2 = Geometry::Triangle(glm::vec3(0.f, -1.5f, 0.f), glm::vec3(1.f, -3.5f, 0.f),  glm::vec3(-1.f, -3.5f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(-2.5f, 1.f, 0.f), glm::vec3(-1.5f, -1.f, 0.f), glm::vec3(-3.5f, -1.f, 0.f));
				auto t4 = Geometry::Triangle(glm::vec3(2.5f, 1.f, 0.f),  glm::vec3(3.5f, -1.f, 0.f),  glm::vec3(1.5f, -1.f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f),   glm::vec3(1.f, -1.f, 1.f),   glm::vec3(-1.f, -1.f, 1.f));
				auto t6 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f),  glm::vec3(1.f, -1.f, -1.f),  glm::vec3(-1.f, -1.f, -1.f));

				CHECK_TRUE(!Geometry::intersecting(control, t1), "No collision 1");
				CHECK_TRUE(!Geometry::intersecting(control, t2), "No collision 2");
				CHECK_TRUE(!Geometry::intersecting(control, t3), "No collision 3");
				CHECK_TRUE(!Geometry::intersecting(control, t4), "No collision 4");
				CHECK_TRUE(!Geometry::intersecting(control, t5), "No collision 5");
				CHECK_TRUE(!Geometry::intersecting(control, t6), "No collision 6");
			}

			{SCOPE_SECTION("edge-edge");
				auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
				auto t2 = Geometry::Triangle(glm::vec3(1.f, 3.f, 0.f), glm::vec3(2.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(-2.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-3.f, -1.f, 0.f));
				auto t4 = Geometry::Triangle(glm::vec3(2.f, 1.f, 0.f), glm::vec3(3.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(-1.f, -1.f, 0.f), glm::vec3(0.f, -3.f, 0.f), glm::vec3(-2.f, -3.f, 0.f));
				auto t6 = Geometry::Triangle(glm::vec3(1.f, -1.f, 0.f), glm::vec3(2.f, -3.f, 0.f), glm::vec3(0.f, -3.f, 0.f));

				CHECK_TRUE(Geometry::intersecting(control, t1), "Edge-Edge 1");
				CHECK_TRUE(Geometry::intersecting(control, t2), "Edge-Edge 2");
				CHECK_TRUE(Geometry::intersecting(control, t3), "Edge-Edge 3");
				CHECK_TRUE(Geometry::intersecting(control, t4), "Edge-Edge 4");
				CHECK_TRUE(Geometry::intersecting(control, t5), "Edge-Edge 5");
				CHECK_TRUE(Geometry::intersecting(control, t6), "Edge-Edge 6");
			}
			{SCOPE_SECTION("Non-coplanar / edge-edge");
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 2.f));
				auto t2 = Geometry::Triangle(glm::vec3(0.f, 3.f, -1.f), glm::vec3(0.f, 1.f, -2.f), glm::vec3(0.f, 1.f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 2.f));
				auto t4 = Geometry::Triangle(glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -2.f), glm::vec3(1.f, -1.f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 2.f));
				auto t6 = Geometry::Triangle(glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, -2.f), glm::vec3(-1.f, -1.f, 0.f));

				CHECK_TRUE(Geometry::intersecting(control, t1), "Edge-Edge 1");
				CHECK_TRUE(Geometry::intersecting(control, t2), "Edge-Edge 2");
				CHECK_TRUE(Geometry::intersecting(control, t3), "Edge-Edge 3");
				CHECK_TRUE(Geometry::intersecting(control, t4), "Edge-Edge 4");
				CHECK_TRUE(Geometry::intersecting(control, t5), "Edge-Edge 5");
				CHECK_TRUE(Geometry::intersecting(control, t6), "Edge-Edge 6");
			}
			{SCOPE_SECTION("Coplanar / edge-side");
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, 1.f, 0.f));
				auto t2 = Geometry::Triangle(glm::vec3(1.5, 2.f, 0.f), glm::vec3(2.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(1.5f, 0.f, 0.f), glm::vec3(2.5f, -2.f, 0.f), glm::vec3(0.5f, -2.f, 0.f));
				auto t4 = Geometry::Triangle(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, -3.f, 0.f), glm::vec3(-1.f, -3.f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(-1.5f, 0.f, 0.f), glm::vec3(-0.5f, -2.f, 0.f), glm::vec3(-2.5f, -2.f, 0.f));
				auto t6 = Geometry::Triangle(glm::vec3(-1.5f, 2.f, 0.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-2.5f, 0.f, 0.f));

				CHECK_TRUE(Geometry::intersecting(control, t1), "Edge-Side 1");
				CHECK_TRUE(Geometry::intersecting(control, t2), "Edge-Side 2");
				CHECK_TRUE(Geometry::intersecting(control, t3), "Edge-Side 3");
				CHECK_TRUE(Geometry::intersecting(control, t4), "Edge-Side 4");
				CHECK_TRUE(Geometry::intersecting(control, t5), "Edge-Side 5");
				CHECK_TRUE(Geometry::intersecting(control, t6), "Edge-Side 6");
			}
			{SCOPE_SECTION("Non-coplanar / edge-side");
				auto t1 = Geometry::Triangle(glm::vec3(0.5f, 2.f, 1.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 2.f));
				auto t2 = Geometry::Triangle(glm::vec3(0.5f, 2.f, -1.f), glm::vec3(0.5f, 0.f, -2.f), glm::vec3(0.5f, 0.f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 2.f));
				auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(0.f, -1.f, -2.f), glm::vec3(0.f, -1.f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, 1.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-0.5f, 0.f, 2.f));
				auto t6 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, -1.f), glm::vec3(-0.5f, 0.f, -2.f), glm::vec3(-0.5f, 0.f, 0.f));

				CHECK_TRUE(Geometry::intersecting(control, t1), "Edge-Side 1");
				CHECK_TRUE(Geometry::intersecting(control, t2), "Edge-Side 2");
				CHECK_TRUE(Geometry::intersecting(control, t3), "Edge-Side 3");
				CHECK_TRUE(Geometry::intersecting(control, t4), "Edge-Side 4");
				CHECK_TRUE(Geometry::intersecting(control, t5), "Edge-Side 5");
				CHECK_TRUE(Geometry::intersecting(control, t6), "Edge-Side 6");
			}
			{SCOPE_SECTION("Coplanar / overlap");
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::vec3(-1.f, 0.5f, 0.f));
				auto t2 = Geometry::Triangle(glm::vec3(1.f, 2.f, 0.f), glm::vec3(2.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
				auto t3 = Geometry::Triangle(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, -2.f, 0.f), glm::vec3(0.f, -2.f, 0.f));
				auto t4 = Geometry::Triangle(glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, -2.5f, 0.f), glm::vec3(-1.f, -2.5f, 0.f));
				auto t5 = Geometry::Triangle(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -2.f, 0.f), glm::vec3(-2.f, -2.f, 0.f));
				auto t6 = Geometry::Triangle(glm::vec3(-1.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(-2.f, 0.f, 0.f));

				CHECK_TRUE(Geometry::intersecting(control, t1), "Overlap 1");
				CHECK_TRUE(Geometry::intersecting(control, t2), "Overlap 2");
				CHECK_TRUE(Geometry::intersecting(control, t3), "Overlap 3");
				CHECK_TRUE(Geometry::intersecting(control, t4), "Overlap 4");
				CHECK_TRUE(Geometry::intersecting(control, t5), "Overlap 5");
				CHECK_TRUE(Geometry::intersecting(control, t6), "Overlap 6");
			}
			{SCOPE_SECTION("Non-coplanar / overlap");
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, 1.f));
				CHECK_TRUE(Geometry::intersecting(control, t1), "Overlap 1");

				auto t2 = Geometry::Triangle(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -2.f, -1.f), glm::vec3(0.f, -2.f, 1.f));
				CHECK_TRUE(Geometry::intersecting(control, t2), "Overlap 2");
			}
			{SCOPE_SECTION("Collision - off-axis");
				auto t1 = Geometry::Triangle(glm::vec3(2.f, 1.f, -1.f), glm::vec3(1.f, -2.f, 1.f), glm::vec3(-1.f, -2.f, 1.f));
				CHECK_TRUE(Geometry::intersecting(control, t1), "One side collision");

				// Like t1 but two sides of triangle cut through control
				auto t2 = Geometry::Triangle(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
				CHECK_TRUE(Geometry::intersecting(control, t2), "Two side collision");

				// Triangle passes under control without collision
				auto t3 = Geometry::Triangle(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
				CHECK_TRUE(!Geometry::intersecting(control, t3), "Pass under no collision");
			}
			{SCOPE_SECTION("Epsilon offset");
				// Place test triangles touching control T then move them away by epsilon and check no collision.
				{SCOPE_SECTION("Touching edge to edge");
					auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
					CHECK_TRUE(Geometry::intersecting(control, t1), "Co-planar");
					t1.translate({-std::numeric_limits<float>::epsilon() * 2.f, 0.f, 0.f});
					CHECK_TRUE(!Geometry::intersecting(control, t1), "Co-planar");
				}
				{SCOPE_SECTION("Perpendicular");
					auto t1 = Geometry::Triangle(glm::vec3(0.f, -1.f, -1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
					CHECK_TRUE(Geometry::intersecting(control, t1), "Perpendicular");
					t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
					CHECK_TRUE(!Geometry::intersecting(control, t1), "Perpendicular");
				}
				{SCOPE_SECTION("Touching edge to side");
					auto t1 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
					CHECK_TRUE(Geometry::intersecting(control, t1), "Pass under touch");
					t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
					CHECK_TRUE(!Geometry::intersecting(control, t1), "Pass under epsilon distance");
				}
			}
			{SCOPE_SECTION("Edge case");
				CHECK_TRUE(Geometry::intersecting(control, control), "Equal triangles");
			}
		}
	}

	void GeometryTester::run_frustrum_tests()
	{SCOPE_SECTION("Frustrum");
		{SCOPE_SECTION("Frustrum from standard ortho projection");
			float ortho_size = 1.f;
			float near       = -1.f;
			float far        = 1.f;
			auto projection  = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
			auto frustrum    = Geometry::Frustrum(projection);

			{SCOPE_SECTION("Distance");
				CHECK_EQUAL(frustrum.m_left.m_distance,   ortho_size, "Left");
				CHECK_EQUAL(frustrum.m_right.m_distance,  ortho_size, "Right");
				CHECK_EQUAL(frustrum.m_bottom.m_distance, ortho_size, "Bottom");
				CHECK_EQUAL(frustrum.m_top.m_distance,    ortho_size, "Top");
				CHECK_EQUAL(frustrum.m_near.m_distance,   -1.f,       "Near");
				CHECK_EQUAL(frustrum.m_far.m_distance,    -1.f,       "Far");
			}
			{SCOPE_SECTION("Normal");
				CHECK_EQUAL(frustrum.m_left.m_normal,   glm::vec3(1.f, 0.f, 0.f),  "Left");
				CHECK_EQUAL(frustrum.m_right.m_normal,  glm::vec3(-1.f, 0.f, 0.f), "Right");
				CHECK_EQUAL(frustrum.m_bottom.m_normal, glm::vec3(0.f, 1.f, 0.f),  "Bottom");
				CHECK_EQUAL(frustrum.m_top.m_normal,    glm::vec3(0.f, -1.f, 0.f), "Top");
				CHECK_EQUAL(frustrum.m_near.m_normal,   glm::vec3(0.f, 0.f, 1.f),  "Near");
				CHECK_EQUAL(frustrum.m_far.m_normal,    glm::vec3(0.f, 0.f, -1.f), "Far");
			}
		}
		{SCOPE_SECTION("Frustrum from 'non-identity' ortho projection");
		// Frustrum from standard ortho projection can get away with non-normalised plane equations, but this test uses a non-1 ortho_size.
			float ortho_size = 15.f;
			float near       = 0.f;
			float far        = 10.f;
			auto projection  = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
			auto frustrum    = Geometry::Frustrum(projection);

			auto error_threshold_equality = [](float value_1, float value_2, float threshold = std::numeric_limits<float>::epsilon(), float power = 0.f)
			{
				auto adjusted_threshold = threshold * std::pow(10.f, power);
				return std::abs(value_1 - value_2) <= adjusted_threshold;
			};

			{SCOPE_SECTION("Distance");
				CHECK_TRUE(error_threshold_equality(frustrum.m_left.m_distance,   ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Left");
				CHECK_TRUE(error_threshold_equality(frustrum.m_left.m_distance,   ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Left");
				CHECK_TRUE(error_threshold_equality(frustrum.m_right.m_distance,  ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Right");
				CHECK_TRUE(error_threshold_equality(frustrum.m_bottom.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Bottom");
				CHECK_TRUE(error_threshold_equality(frustrum.m_top.m_distance,    ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Top");
				CHECK_EQUAL(frustrum.m_near.m_distance, 0.f, "Near");
				CHECK_EQUAL(frustrum.m_far.m_distance, -10.f, "Far");
			}
			{SCOPE_SECTION("Normal");
				CHECK_EQUAL(frustrum.m_left.m_normal,   glm::vec3(1.f, 0.f, 0.f), "Left");
				CHECK_EQUAL(frustrum.m_right.m_normal,  glm::vec3(-1.f, 0.f, 0.f), "Right");
				CHECK_EQUAL(frustrum.m_bottom.m_normal, glm::vec3(0.f, 1.f, 0.f), "Bottom");
				CHECK_EQUAL(frustrum.m_top.m_normal,    glm::vec3(0.f, -1.f, 0.f), "Top");
				CHECK_EQUAL(frustrum.m_near.m_normal,   glm::vec3(0.f, 0.f, 1.f), "Near");
				CHECK_EQUAL(frustrum.m_far.m_normal,    glm::vec3(0.f, 0.f, -1.f), "Far");
			}
		}
	}

	void GeometryTester::run_sphere_tests()
	{SCOPE_SECTION("Sphere v Sphere");
		{SCOPE_SECTION("Touching");
			auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.f);
			auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.f);

			CHECK_TRUE(Geometry::intersecting(sphere, sphere_2), "intersecting");

			{SCOPE_SECTION("Sphere 1");
				auto intersection = Geometry::get_intersection(sphere, sphere_2);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(intersection->position, glm::vec3(1.f, 0.f, 0.f), "Position");
					CHECK_EQUAL(intersection->normal, glm::vec3(-1.f, 0.f, 0.f), "Normal");
					CHECK_EQUAL(intersection->penetration_depth, 0.f, "Penetration depth");
				}
			}
			{SCOPE_SECTION("Sphere 2"); // Swap the spheres around, should get the same result with the normals flipped
				auto intersection = Geometry::get_intersection(sphere_2, sphere);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(intersection->position, glm::vec3(1.f, 0.f, 0.f), "Position"); // Same as sphere 1
					CHECK_EQUAL(intersection->normal, glm::vec3(1.f, 0.f, 0.f), "Normal");     // Opposite of sphere 1
					CHECK_EQUAL(intersection->penetration_depth, 0.f, "Penetration depth");    // Same as sphere 1
				}
			}
		}
		{SCOPE_SECTION("Not intersecting epsilon");
			// Reduce the size of one of the spheres touching by epsilon, should not intersect anymore
			auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.f - std::numeric_limits<float>::epsilon());
			auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.f);

			CHECK_TRUE(!Geometry::intersecting(sphere, sphere_2), "intersecting");
			auto intersection = Geometry::get_intersection(sphere, sphere_2);
			CHECK_TRUE(!intersection.has_value(), "get_intersection");
		}
		{SCOPE_SECTION("Overlapping");
			// Without information about the spheres movement, there is no 'correct' answer for this case.
			// We expect the resolution to result in the minimum penetration depth displacement.

			auto sphere_1 = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.25f);
			auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.25f);
			auto sphere_3 = Geometry::Sphere(glm::vec3(5.f, 0.f, 0.f), 6.f);

			CHECK_TRUE(Geometry::intersecting(sphere_1, sphere_2), "intersecting");

			{SCOPE_SECTION("Sphere 1");
				auto intersection = Geometry::get_intersection(sphere_1, sphere_2);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(intersection->position, glm::vec3(1.25f, 0.f, 0.f), "Position");
					CHECK_EQUAL(intersection->normal, glm::vec3(-1.f, 0.f, 0.f), "Normal");
					CHECK_EQUAL(intersection->penetration_depth, 0.5f, "Penetration depth");
				}
			}
			{SCOPE_SECTION("Sphere 2");
				auto intersection = Geometry::get_intersection(sphere_2, sphere_1);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(intersection->position, glm::vec3(0.75f, 0.f, 0.f), "Position"); // Different to sphere 1
					CHECK_EQUAL(intersection->normal, glm::vec3(1.f, 0.f, 0.f), "Normal");       // Opposite of sphere 1
					CHECK_EQUAL(intersection->penetration_depth, 0.5f, "Penetration depth");     // Same as sphere 1
				}
			}
			{SCOPE_SECTION("Sphere overlaps past half way");
				{SCOPE_SECTION("Sphere 1");
					auto intersection = Geometry::get_intersection(sphere_1, sphere_3);
					CHECK_TRUE(intersection.has_value(), "get_intersection");

					if (intersection.has_value())
					{SCOPE_SECTION("Contact Info");
						CHECK_EQUAL(intersection->position, glm::vec3(1.25f, 0.f, 0.f), "Position");
						CHECK_EQUAL(intersection->normal, glm::vec3(-1.f, 0.f, 0.f), "Normal");
						CHECK_EQUAL(intersection->penetration_depth, 2.25f, "Penetration depth");
					}
				}
				{SCOPE_SECTION("Sphere 3");
					auto intersection = Geometry::get_intersection(sphere_3, sphere_1);
					CHECK_TRUE(intersection.has_value(), "get_intersection");

					if (intersection.has_value())
					{SCOPE_SECTION("Contact Info");
						CHECK_EQUAL(intersection->position, glm::vec3(-1.f, 0.f, 0.f), "Position");
						CHECK_EQUAL(intersection->normal, glm::vec3(1.f, 0.f, 0.f), "Normal");
						CHECK_EQUAL(intersection->penetration_depth, 2.25f, "Penetration depth");
					}
				}
			}
		}
		{SCOPE_SECTION("Not intersecting");
			auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 0.5f);
			auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 0.5f);

			CHECK_TRUE(!Geometry::intersecting(sphere, sphere_2), "intersecting");

			auto intersection = Geometry::get_intersection(sphere, sphere_2);
			CHECK_TRUE(!intersection.has_value(), "get_intersection");
		}
		{SCOPE_SECTION("Same sphere - overlapping spheres");
			// There is no 'correct' answer for this case, without information about the spheres movement.
			// The current implementation returns the first sphere's contact point as its bottom and resolves the normal as 'up'.
			auto sphere = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.f);

			CHECK_TRUE(Geometry::intersecting(sphere, sphere), "intersecting");

			auto intersection = Geometry::get_intersection(sphere, sphere);
			CHECK_TRUE(intersection.has_value(), "intersection");
			if (intersection.has_value())
			{SCOPE_SECTION("Contact Info");
				CHECK_EQUAL(intersection->position, glm::vec3(0.f, -1.f, 0.f), "Position"); // Bottom of sphere 1
				CHECK_EQUAL(intersection->normal, glm::vec3(0.f, 1.f, 0.f), "Normal"); // Resolve normal as 'up'
				CHECK_EQUAL(intersection->penetration_depth, 2.f, "Penetration depth");
			}
		}
	}

	void GeometryTester::run_point_tests()
	{SCOPE_SECTION("Point inside");
		{SCOPE_SECTION("Point v AABB");
			const auto AABB = Geometry::AABB(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f));

			const auto point_inside = Geometry::Point(glm::vec3(0.f, 0.f, 0.f));
			CHECK_TRUE(Geometry::point_inside(AABB, point_inside), "Point inside");

			const auto point_on_surface = Geometry::Point(glm::vec3(1.f, 1.f, 1.f));
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_surface), "Point on surface");

			const auto point_outside = Geometry::Point(glm::vec3(2.f, 0.f, 2.f));
			CHECK_TRUE(!Geometry::point_inside(AABB, point_outside), "Point outside");

			const auto point_on_max_edge = Geometry::Point(glm::vec3(1.f, 1.f, 1.f));
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_max_edge), "Point on max edge of AABB");

			const auto point_on_min_edge = Geometry::Point(glm::vec3(-1.f, -1.f, -1.f));
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_min_edge), "Point on min edge of AABB");
		}
		{SCOPE_SECTION("Point v Cone");
			const auto cone = Geometry::Cone(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			auto point_inside = Geometry::Point(glm::vec3(0.f, 0.5f, 0.f));
			CHECK_TRUE(Geometry::point_inside(cone, point_inside), "Point inside cone");

			auto point_outside = Geometry::Point(glm::vec3(0.f, 1.5f, 0.f));
			CHECK_TRUE(!Geometry::point_inside(cone, point_outside), "Point outside");

			auto point_on_surface_top = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_top), "Point on Surface top");

			auto point_on_surface_base = Geometry::Point(glm::vec3(0.f));
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_base), "Point on surface base");

			auto point_on_surface_side = Geometry::Point(glm::vec3(0.f, 0.5f, 0.5f));
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_side), "Point on surface side");
		}
		{SCOPE_SECTION("Point v Cylinder");
			const auto cylinder = Geometry::Cylinder(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			auto point_inside = Geometry::Point(glm::vec3(0.5f, 0.5f, 0.5f));
			CHECK_TRUE(Geometry::point_inside(cylinder, point_inside), "Point inside cylinder");

			auto point_outside = Geometry::Point(glm::vec3(0.5f, 1.5f, 0.5f));
			CHECK_TRUE(!Geometry::point_inside(cylinder, point_outside), "Point outside cylinder");

			auto point_on_surface_top = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_top), "Point on surface top");

			auto point_on_surface_base = Geometry::Point(glm::vec3(0.f));
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_base), "Point on surface base");

			auto point_on_surface_side = Geometry::Point(glm::vec3(0.f, 0.5f, 0.5f));
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_side), "Point on surface side");
		}
		{SCOPE_SECTION("Point v Line");
			const auto line = Geometry::Line(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_line_middle = Geometry::Point(glm::vec3(0.f));
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_middle), "Point at line middle");

			auto point_on_line_start = Geometry::Point(glm::vec3(-1.f));
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_start), "Point at line point 1");

			auto point_on_line_end = Geometry::Point(glm::vec3(1.f));
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_end), "Point at line point 2");

			auto point_off_line_above = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));
			CHECK_TRUE(!Geometry::point_inside(line, point_off_line_above), "Point above line");

			auto point_on_line_ahead = Geometry::Point(glm::vec3(2.f));
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_ahead), "Point on line ahead of point 2");

			auto point_on_line_behind = Geometry::Point(glm::vec3(-2.f));
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_behind), "Point on line behind point 1");
		}
		{SCOPE_SECTION("Point v LineSegment");
			const auto line_segment = Geometry::LineSegment(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_line_middle = Geometry::Point(glm::vec3(0.f));
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_middle), "Point on line segment middle");

			auto point_on_line_start = Geometry::Point(glm::vec3(-1.f));
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_start), "Point at line segment start");

			auto point_on_line_end = Geometry::Point(glm::vec3(1.f));
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_end), "Point at line segment end");

			auto point_off_line_above = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_off_line_above), "Point above line segment");

			auto point_on_line_ahead = Geometry::Point(glm::vec3(2.f));
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_on_line_ahead), "Point along line ahead of segment");

			auto point_on_line_behind = Geometry::Point(glm::vec3(-2.f));
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_on_line_behind), "Point along line segment behind");
		}
		{SCOPE_SECTION("Point v Ray");
			// Ray starts at -1,-1,-1 in direction 1,1,1
			const auto ray = Geometry::Ray(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_ray_middle = Geometry::Point(glm::vec3(0.f));
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_middle), "Point on ray ahead of start");

			auto point_on_ray_start = Geometry::Point(glm::vec3(-1.f));
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_start), "Point at ray start");

			auto point_above_ray = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));
			CHECK_TRUE(!Geometry::point_inside(ray, point_above_ray), "Point above ray");

			auto point_on_ray_ahead = Geometry::Point(glm::vec3(2.f));
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_ahead), "Point on ray ahead");

			auto point_on_ray_behind = Geometry::Point(glm::vec3(-2.f));
			CHECK_TRUE(!Geometry::point_inside(ray, point_on_ray_behind), "Point behind ray start");
		}
	}

	void GeometryTester::draw_frustrum_debugger_UI(float aspect_ratio)
	{
		// Use this ImGui + OpenGL::DebugRenderer function to visualise Projection generated Geometry::Frustrums.
		// A projection-only generated frustrum is positioned at [0, 0, 0] in the positive-z direction.
		// OpenGL clip coordinates are in the [-1 - 1] range thus the default generate ortho projection has a near = -1, far = 1.
		if (ImGui::Begin("Frustrum visualiser"))
		{
			enum class ProjectionType
			{
				Ortho,
				Perspective
			};
			static const std::vector<std::pair<ProjectionType, const char*>> projection_options =
				{{ProjectionType::Ortho, "Ortho"}, {ProjectionType::Perspective, "Perspective"}};
			static ProjectionType projection_type = ProjectionType::Ortho;
			static float near                     = 0.1f;
			static float far                      = 2.f;
			static float ortho_size               = 1.f;
			static bool use_near_far              = true;
			static float fov                      = 90.f;
			static bool transpose                 = false;
			static bool apply_view                = true;

			ImGui::ComboContainer("Projection type", projection_type, projection_options);

			ImGui::Separator();
			glm::mat4 projection;
			if (projection_type == ProjectionType::Ortho)
			{
				ImGui::Checkbox("use near far", &use_near_far);
				if (use_near_far)
				{
					ImGui::Slider("near", near, -1.f, 20.f);
					ImGui::Slider("far", far, 1.f, 20.f);
				}
				ImGui::Slider("ortho_size", ortho_size, 1.f, 20.f);

				if (use_near_far)
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
				else
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size);
			}
			else if (projection_type == ProjectionType::Perspective)
			{
				ImGui::Slider("FOV", fov, 1.f, 180.f);
				ImGui::Slider("Aspect ratio", aspect_ratio, 0.f, 5.f);
				ImGui::Slider("near", near, -1.f, 20.f);
				ImGui::Slider("far", far, 1.f, 20.f);
				projection = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
			}

			ImGui::Separator();
			ImGui::Checkbox("transpose", &transpose);
			if (transpose)
				projection = glm::transpose(projection);

			ImGui::Checkbox("apply view matrix", &apply_view);
			if (apply_view)
			{
				ImGui::Separator();
				static glm::vec3 eye_position = glm::vec3(0.f, 0.f, 0.f);
				static glm::vec3 center       = glm::vec3(0.5f, 0.f, 0.5f);
				static glm::vec3 up           = glm::vec3(0.f, 1.f, 0.f);
				static glm::mat4 view;
				static bool inverse_view     = false;
				static bool transpose_view   = false;
				static bool swap_order       = false;
				static bool flip_view_dir    = true;
				static bool inverse_position = true;

				ImGui::Slider("Position", eye_position, 0.f, 20.f);
				ImGui::Slider("look direction", center, 0.f, 20.f);
				ImGui::Slider("up direction", up, 0.f, 20.f);
				ImGui::Checkbox("Inverse view", &inverse_view);
				ImGui::Checkbox("Transpose view", &transpose_view);
				ImGui::Checkbox("Swap order", &swap_order);
				ImGui::Checkbox("Flip view direction", &flip_view_dir);
				ImGui::Checkbox("inverse position", &inverse_position);

				glm::vec3 view_position = inverse_position ? -eye_position : eye_position;
				glm::vec3 view_look_at  = flip_view_dir ? view_position - center : view_position + center;
				view                    = glm::lookAt(view_position, view_look_at, up);

				if (swap_order)
				{
					if (inverse_view)
						view = glm::inverse(view);
					if (transpose_view)
						view = glm::transpose(view);
				}
				else
				{
					if (transpose_view)
						view = glm::transpose(view);
					if (inverse_view)
						view = glm::inverse(view);
				}
				projection = projection * view;
				ImGui::Text("VIEW", view);
				ImGui::Separator();
			}

			Geometry::Frustrum frustrum = Geometry::Frustrum(projection);
			ImGui::Text("LEFT  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_left.m_normal.x, frustrum.m_left.m_normal.y, frustrum.m_left.m_normal.z, frustrum.m_left.m_distance);
			ImGui::Text("RIGHT \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_right.m_normal.x, frustrum.m_right.m_normal.y, frustrum.m_right.m_normal.z, frustrum.m_right.m_distance);
			ImGui::Text("BOTTOM\nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_bottom.m_normal.x, frustrum.m_bottom.m_normal.y, frustrum.m_bottom.m_normal.z, frustrum.m_bottom.m_distance);
			ImGui::Text("TOP   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_top.m_normal.x, frustrum.m_top.m_normal.y, frustrum.m_top.m_normal.z, frustrum.m_top.m_distance);
			ImGui::Text("NEAR  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_near.m_normal.x, frustrum.m_near.m_normal.y, frustrum.m_near.m_normal.z, frustrum.m_near.m_distance);
			ImGui::Text("FAR   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_far.m_normal.x, frustrum.m_far.m_normal.y, frustrum.m_far.m_normal.z, frustrum.m_far.m_distance);
			ImGui::Text("PROJECTION", projection);
			OpenGL::DebugRenderer::add(frustrum, glm::vec4(218.f / 255.f, 112.f / 255.f, 214.f / 255.f, 0.5f));
		}
		ImGui::End();
	}
}// namespace Test