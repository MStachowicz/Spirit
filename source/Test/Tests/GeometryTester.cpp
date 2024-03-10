#include "GeometryTester.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

#include "Utility/Utility.hpp"

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <array>

DISABLE_WARNING_PUSH
DISABLE_WARNING_HIDES_PREVIOUS_DECLERATION // Required to allow shadowing for the SCOPE_SECTION macro

namespace Test
{
	void GeometryTester::run_unit_tests()
	{
		run_AABB_tests();
		run_triangle_tests();
		run_frustrum_tests();
		run_sphere_tests();
		run_point_tests();
	}
	void GeometryTester::run_performance_tests()
	{
		//constexpr size_t triangle_count = 1000000 * 2;
		//std::vector<float> random_triangle_points = Utility::get_random_numbers(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), triangle_count * 3 * 3);
		//std::vector<Geometry::Triangle> triangles;
		//triangles.reserve(triangle_count);

		//for (size_t i = 0; i < triangle_count; i++)
		//	triangles.emplace_back(Geometry::Triangle(
		//		glm::vec3(random_triangle_points[i], random_triangle_points[i + 1], random_triangle_points[i + 2]),
		//		glm::vec3(random_triangle_points[i + 3], random_triangle_points[i + 4], random_triangle_points[i + 5]),
		//		glm::vec3(random_triangle_points[i + 6], random_triangle_points[i + 7], random_triangle_points[i + 8])));

		//auto triangle_test = [&triangles](const size_t& p_number_of_tests)
		//{
		//	if (p_number_of_tests * 2 > triangles.size()) // Multiply by two to account for intersection calling in pairs.
		//		throw std::logic_error("Not enough triangles to perform p_number_of_tests. Bump up the triangle_count variable to at least double the size of the largest performance test.");

		//	for (size_t i = 0; i < p_number_of_tests * 2; i += 2)
		//		Geometry::intersecting(triangles[i], triangles[i + 1]);
		//};

		//auto triangle_test1 = [&triangle_test]() { triangle_test(1); };
		//auto triangle_test10 = [&triangle_test]() { triangle_test(10); };
		//auto triangle_test100 = [&triangle_test]() { triangle_test(100); };
		//auto triangle_test1000 = [&triangle_test]() { triangle_test(1000); };
		//auto triangle_test10000 = [&triangle_test]() { triangle_test(10000); };
		//auto triangle_test100000 = [&triangle_test]() { triangle_test(100000); };
		//auto triangle_test1000000 = [&triangle_test]() { triangle_test(1000000); };
		//emplace_performance_test({"Triangle v Triangle 1", triangle_test1});
		//emplace_performance_test({"Triangle v Triangle 10", triangle_test10});
		//emplace_performance_test({"Triangle v Triangle 100", triangle_test100});
		//emplace_performance_test({"Triangle v Triangle 1,000", triangle_test1000});
		//emplace_performance_test({"Triangle v Triangle 10,000", triangle_test10000});
		//emplace_performance_test({"Triangle v Triangle 100,000", triangle_test100000});
		//emplace_performance_test({"Triangle v Triangle 1,000,000", triangle_test1000000});
	}

	void GeometryTester::run_AABB_tests()
	{
		{SCOPE_SECTION("Default intiailise");
			Geometry::AABB aabb;
			CHECK_EQUAL(aabb.get_size(), glm::vec3(0.f), "AABB initialise size at 0");
			CHECK_EQUAL(aabb.get_center (), glm::vec3(0.f), "AABB initialise to world origin");
			CHECK_EQUAL(aabb.get_center (), glm::vec3(0.f), "AABB initialise to world origin");
		}
		{SCOPE_SECTION("Initialise with a min and max");
			// An AABB at low point [-1,-1,-1] to [1,1,1]
			auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
			CHECK_EQUAL(aabb.get_size(), glm::vec3(2.f), "AABB initialised with min and max size at 2");
			CHECK_EQUAL(aabb.get_center(), glm::vec3(0.f), "AABB initialise with min and max position");
		}
		{SCOPE_SECTION("Initialise with a min and max not at origin");
			// An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
			auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
			CHECK_EQUAL(aabb.get_size(), glm::vec3(4.f), "AABB initialised with min and max not at origin");
			CHECK_EQUAL(aabb.get_center(), glm::vec3(3.f), "AABB initialised with min and max not at origin");
		}
	}

	void GeometryTester::run_triangle_tests()
	{SCOPE_SECTION("Triangle")
		const auto control = Geometry::Triangle(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f));

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
					t1.translate({0.f, -std::numeric_limits<float>::epsilon() * 2.f, 0.f});
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
					CHECK_EQUAL(*intersection, glm::vec3(1.f, 0.f, 0.f), "Position");
				}
			}
			{SCOPE_SECTION("Sphere 2"); // Swap the spheres around, should get the same result with the normals flipped
				auto intersection = Geometry::get_intersection(sphere_2, sphere);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(*intersection, glm::vec3(1.f, 0.f, 0.f), "Position"); // Same as sphere 1
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
					CHECK_EQUAL(*intersection, glm::vec3(1.25f, 0.f, 0.f), "Position");
				}
			}
			{SCOPE_SECTION("Sphere 2");
				auto intersection = Geometry::get_intersection(sphere_2, sphere_1);
				CHECK_TRUE(intersection.has_value(), "get_intersection");

				if (intersection.has_value())
				{SCOPE_SECTION("Contact Info");
					CHECK_EQUAL(*intersection, glm::vec3(0.75f, 0.f, 0.f), "Position"); // Different to sphere 1
				}
			}
			{SCOPE_SECTION("Sphere overlaps past half way");
				{SCOPE_SECTION("Sphere 1");
					auto intersection = Geometry::get_intersection(sphere_1, sphere_3);
					CHECK_TRUE(intersection.has_value(), "get_intersection");

					if (intersection.has_value())
					{SCOPE_SECTION("Contact Info");
						CHECK_EQUAL(*intersection, glm::vec3(1.25f, 0.f, 0.f), "Position");
					}
				}
				{SCOPE_SECTION("Sphere 3");
					auto intersection = Geometry::get_intersection(sphere_3, sphere_1);
					CHECK_TRUE(intersection.has_value(), "get_intersection");

					if (intersection.has_value())
					{SCOPE_SECTION("Contact Info");
						CHECK_EQUAL(*intersection, glm::vec3(-1.f, 0.f, 0.f), "Position");
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
				CHECK_EQUAL(*intersection, glm::vec3(0.f, -1.f, 0.f), "Position"); // Bottom of sphere 1
			}
		}
	}

	void GeometryTester::run_point_tests()
	{SCOPE_SECTION("Point inside");
		{SCOPE_SECTION("Point v AABB");
			const auto AABB = Geometry::AABB(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f));

			const auto point_inside = glm::vec3(0.f, 0.f, 0.f);
			CHECK_TRUE(Geometry::point_inside(AABB, point_inside), "Point inside");

			const auto point_on_surface = glm::vec3(1.f, 1.f, 1.f);
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_surface), "Point on surface");

			const auto point_outside = glm::vec3(2.f, 0.f, 2.f);
			CHECK_TRUE(!Geometry::point_inside(AABB, point_outside), "Point outside");

			const auto point_on_max_edge = glm::vec3(1.f, 1.f, 1.f);
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_max_edge), "Point on max edge of AABB");

			const auto point_on_min_edge = glm::vec3(-1.f, -1.f, -1.f);
			CHECK_TRUE(Geometry::point_inside(AABB, point_on_min_edge), "Point on min edge of AABB");
		}
		{SCOPE_SECTION("Point v Cone");
			const auto cone = Geometry::Cone(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			auto point_inside = glm::vec3(0.f, 0.5f, 0.f);
			CHECK_TRUE(Geometry::point_inside(cone, point_inside), "Point inside cone");

			auto point_outside = glm::vec3(0.f, 1.5f, 0.f);
			CHECK_TRUE(!Geometry::point_inside(cone, point_outside), "Point outside");

			auto point_on_surface_top = glm::vec3(0.f, 1.f, 0.f);
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_top), "Point on Surface top");

			auto point_on_surface_base = glm::vec3(0.f);
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_base), "Point on surface base");

			auto point_on_surface_side = glm::vec3(0.f, 0.5f, 0.5f);
			CHECK_TRUE(Geometry::point_inside(cone, point_on_surface_side), "Point on surface side");
		}
		{SCOPE_SECTION("Point v Cylinder");
			const auto cylinder = Geometry::Cylinder(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			auto point_inside = glm::vec3(0.5f, 0.5f, 0.5f);
			CHECK_TRUE(Geometry::point_inside(cylinder, point_inside), "Point inside cylinder");

			auto point_outside = glm::vec3(0.5f, 1.5f, 0.5f);
			CHECK_TRUE(!Geometry::point_inside(cylinder, point_outside), "Point outside cylinder");

			auto point_on_surface_top = glm::vec3(0.f, 1.f, 0.f);
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_top), "Point on surface top");

			auto point_on_surface_base = glm::vec3(0.f);
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_base), "Point on surface base");

			auto point_on_surface_side = glm::vec3(0.f, 0.5f, 0.5f);
			CHECK_TRUE(Geometry::point_inside(cylinder, point_on_surface_side), "Point on surface side");
		}
		{SCOPE_SECTION("Point v Line");
			const auto line = Geometry::Line(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_line_middle = glm::vec3(0.f);
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_middle), "Point at line middle");

			auto point_on_line_start = glm::vec3(-1.f);
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_start), "Point at line point 1");

			auto point_on_line_end = glm::vec3(1.f);
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_end), "Point at line point 2");

			auto point_off_line_above = glm::vec3(0.f, 1.f, 0.f);
			CHECK_TRUE(!Geometry::point_inside(line, point_off_line_above), "Point above line");

			auto point_on_line_ahead = glm::vec3(2.f);
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_ahead), "Point on line ahead of point 2");

			auto point_on_line_behind = glm::vec3(-2.f);
			CHECK_TRUE(Geometry::point_inside(line, point_on_line_behind), "Point on line behind point 1");
		}
		{SCOPE_SECTION("Point v LineSegment");
			const auto line_segment = Geometry::LineSegment(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_line_middle = glm::vec3(0.f);
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_middle), "Point on line segment middle");

			auto point_on_line_start = glm::vec3(-1.f);
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_start), "Point at line segment start");

			auto point_on_line_end = glm::vec3(1.f);
			CHECK_TRUE(Geometry::point_inside(line_segment, point_on_line_end), "Point at line segment end");

			auto point_off_line_above = glm::vec3(0.f, 1.f, 0.f);
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_off_line_above), "Point above line segment");

			auto point_on_line_ahead = glm::vec3(2.f);
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_on_line_ahead), "Point along line ahead of segment");

			auto point_on_line_behind = glm::vec3(-2.f);
			CHECK_TRUE(!Geometry::point_inside(line_segment, point_on_line_behind), "Point along line segment behind");
		}
		{SCOPE_SECTION("Point v Ray");
			// Ray starts at -1,-1,-1 in direction 1,1,1
			const auto ray = Geometry::Ray(glm::vec3(-1.f), glm::vec3(1.f));

			auto point_on_ray_middle = glm::vec3(0.f);
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_middle), "Point on ray ahead of start");

			auto point_on_ray_start = glm::vec3(-1.f);
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_start), "Point at ray start");

			auto point_above_ray = glm::vec3(0.f, 1.f, 0.f);
			CHECK_TRUE(!Geometry::point_inside(ray, point_above_ray), "Point above ray");

			auto point_on_ray_ahead = glm::vec3(2.f);
			CHECK_TRUE(Geometry::point_inside(ray, point_on_ray_ahead), "Point on ray ahead");

			auto point_on_ray_behind = glm::vec3(-2.f);
			CHECK_TRUE(!Geometry::point_inside(ray, point_on_ray_behind), "Point behind ray start");
		}
	}
} // namespace Test
DISABLE_WARNING_POP