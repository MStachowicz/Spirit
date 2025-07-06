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
#include "Geometry/QuadKey.hpp"

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
		run_quad_key_tests();
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
	void GeometryTester::run_quad_key_tests()
	{SCOPE_SECTION("QuadKey");
		using QK   = Geometry::QuadKey;
		using Quad = QK::Quadrant;

		constexpr auto qkTL = QK({Quad::TopLeft});
		constexpr auto qkTR = QK({Quad::TopRight});
		constexpr auto qkBL = QK({Quad::BottomLeft});
		constexpr auto qkBR = QK({Quad::BottomRight});

		constexpr auto qkTLTL = QK({Quad::TopLeft, Quad::TopLeft});
		constexpr auto qkTLTR = QK({Quad::TopLeft, Quad::TopRight});
		constexpr auto qkTLBL = QK({Quad::TopLeft, Quad::BottomLeft});
		constexpr auto qkTLBR = QK({Quad::TopLeft, Quad::BottomRight});

		constexpr auto qkTRTL = QK({Quad::TopRight, Quad::TopLeft});
		constexpr auto qkTRTR = QK({Quad::TopRight, Quad::TopRight});
		constexpr auto qkTRBL = QK({Quad::TopRight, Quad::BottomLeft});
		constexpr auto qkTRBR = QK({Quad::TopRight, Quad::BottomRight});

		constexpr auto qkBLTL = QK({Quad::BottomLeft, Quad::TopLeft});
		constexpr auto qkBLTR = QK({Quad::BottomLeft, Quad::TopRight});
		constexpr auto qkBLBL = QK({Quad::BottomLeft, Quad::BottomLeft});
		constexpr auto qkBLBR = QK({Quad::BottomLeft, Quad::BottomRight});

		constexpr auto qkBRTL = QK({Quad::BottomRight, Quad::TopLeft});
		constexpr auto qkBRTR = QK({Quad::BottomRight, Quad::TopRight});
		constexpr auto qkBRBL = QK({Quad::BottomRight, Quad::BottomLeft});
		constexpr auto qkBRBR = QK({Quad::BottomRight, Quad::BottomRight});

		{SCOPE_SECTION("QuadKey construction");
			{SCOPE_SECTION("Depth 1");
				CHECK_EQUAL(qkTL.key, Geometry::Key_t(0), "TopLeft");
				CHECK_EQUAL(qkTL.depth, 1, "TopLeft depth");
				CHECK_EQUAL(qkTL.to_string(), "TL", "TopLeft string");

				CHECK_EQUAL(qkTR.key, Geometry::Key_t(1), "TopRight");
				CHECK_EQUAL(qkTR.depth, 1, "TopRight depth");
				CHECK_EQUAL(qkTR.to_string(), "TR", "TopRight string");

				CHECK_EQUAL(qkBL.key, Geometry::Key_t(2), "BottomLeft");
				CHECK_EQUAL(qkBL.depth, 1, "BottomLeft depth");
				CHECK_EQUAL(qkBL.to_string(), "BL", "BottomLeft string");

				CHECK_EQUAL(qkBR.key, Geometry::Key_t(3), "BottomRight");
				CHECK_EQUAL(qkBR.depth, 1, "BottomRight depth");
				CHECK_EQUAL(qkBR.to_string(), "BR", "BottomRight string");
			}
			{SCOPE_SECTION("Depth 2");
				{SCOPE_SECTION("TopLeft");
					CHECK_EQUAL(qkTLTL.key, Geometry::Key_t(0), "TopLeft");
					CHECK_EQUAL(qkTLTL.depth, 2, "TopLeft depth");
					CHECK_EQUAL(qkTLTL.to_string(), "TL -> TL", "TopLeft string");

					CHECK_EQUAL(qkTLTR.key, Geometry::Key_t(1), "TopRight");
					CHECK_EQUAL(qkTLTR.depth, 2, "TopRight depth");
					CHECK_EQUAL(qkTLTR.to_string(), "TL -> TR", "TopRight string");

					CHECK_EQUAL(qkTLBL.key, Geometry::Key_t(2), "BottomLeft");
					CHECK_EQUAL(qkTLBL.depth, 2, "BottomLeft depth");
					CHECK_EQUAL(qkTLBL.to_string(), "TL -> BL", "BottomLeft string");

					CHECK_EQUAL(qkTLBR.key, Geometry::Key_t(3), "BottomRight");
					CHECK_EQUAL(qkTLBR.depth, 2, "BottomRight depth");
					CHECK_EQUAL(qkTLBR.to_string(), "TL -> BR", "BottomRight string");
				}
				{SCOPE_SECTION("TopRight");
					CHECK_EQUAL(qkTRTL.key, Geometry::Key_t(4), "TopLeft");
					CHECK_EQUAL(qkTRTL.depth, 2, "TopLeft depth");
					CHECK_EQUAL(qkTRTL.to_string(), "TR -> TL", "TopLeft string");

					CHECK_EQUAL(qkTRTR.key, Geometry::Key_t(5), "TopRight");
					CHECK_EQUAL(qkTRTR.depth, 2, "TopRight depth");
					CHECK_EQUAL(qkTRTR.to_string(), "TR -> TR", "TopRight string");

					CHECK_EQUAL(qkTRBL.key, Geometry::Key_t(6), "BottomLeft");
					CHECK_EQUAL(qkTRBL.depth, 2, "BottomLeft depth");
					CHECK_EQUAL(qkTRBL.to_string(), "TR -> BL", "BottomLeft string");

					CHECK_EQUAL(qkTRBR.key, Geometry::Key_t(7), "BottomRight");
					CHECK_EQUAL(qkTRBR.depth, 2, "BottomRight depth");
					CHECK_EQUAL(qkTRBR.to_string(), "TR -> BR", "BottomRight string");
				}
			}
		}
		{SCOPE_SECTION("QuadKey equality")
			// identity at depth 1
			CHECK_EQUAL(qkTL, qkTL, "TL == TL");
			CHECK_EQUAL(qkTR, qkTR, "TR == TR");
			CHECK_EQUAL(qkBL, qkBL, "BL == BL");
			CHECK_EQUAL(qkBR, qkBR, "BR == BR");

			// identity at depth 2
			CHECK_EQUAL(qkTLTL, qkTLTL, "TLTL == TLTL");
			CHECK_EQUAL(qkTLTR, qkTLTR, "TLTR == TLTR");
			CHECK_EQUAL(qkTRTL, qkTRTL, "TRTL == TRTL");
			CHECK_EQUAL(qkBRBR, qkBRBR, "BRBR == BRBR");

			// same‐quadrant but different depth should be unequal
			CHECK_NOT_EQUAL(qkTL, qkTLTL, "TL (d=1) != TLTL (d=2)");
			CHECK_NOT_EQUAL(qkTR, qkTRTL, "TR (d=1) != TRTL (d=2)");
			CHECK_NOT_EQUAL(qkBR, qkBRBR, "BR (d=1) != BRBR (d=2)");

			// different keys at same depth should be unequal
			CHECK_NOT_EQUAL(qkTLTL, qkTLTR, "TLTL != TLTR");
			CHECK_NOT_EQUAL(qkTLTL, qkTRTL, "TLTL != TRTL");
			CHECK_NOT_EQUAL(qkTRTL, qkTLTR, "TRTL != TLTR");
			CHECK_NOT_EQUAL(qkTLTR, qkBRBR, "TLTR != BRBR");

			// cross‐depth, cross‐quadrant inequalities
			CHECK_NOT_EQUAL(qkTL,   qkBRBR, "TL (d=1) != BRBR (d=2)");
			CHECK_NOT_EQUAL(qkTRTL, qkBR,   "TRTL (d=2) != BR (d=1)");

			// exhaustive inequality at depth 1
			CHECK_NOT_EQUAL(qkTL, qkTR, "TL != TR");
			CHECK_NOT_EQUAL(qkTL, qkBL, "TL != BL");
			CHECK_NOT_EQUAL(qkTL, qkBR, "TL != BR");
			CHECK_NOT_EQUAL(qkTR, qkBL, "TR != BL");
			CHECK_NOT_EQUAL(qkTR, qkBR, "TR != BR");
			CHECK_NOT_EQUAL(qkBL, qkBR, "BL != BR");
		}
		{SCOPE_SECTION("Descendant")
			{SCOPE_SECTION("TL descendants");
				CHECK_EQUAL(qkTLTL.isContainedBy(qkTL), true, "TL -> TL");
				CHECK_EQUAL(qkTLTR.isContainedBy(qkTL), true, "TL -> TL");
				CHECK_EQUAL(qkTLBL.isContainedBy(qkTL), true, "TL -> TL");
				CHECK_EQUAL(qkTLBR.isContainedBy(qkTL), true, "TL -> TL");

				CHECK_EQUAL(qkTRTL.isContainedBy(qkTL), false, "TR -> TL");
				CHECK_EQUAL(qkTRTR.isContainedBy(qkTL), false, "TR -> TR");
				CHECK_EQUAL(qkTRBL.isContainedBy(qkTL), false, "TR -> BL");
				CHECK_EQUAL(qkTRBR.isContainedBy(qkTL), false, "TR -> BR");

				CHECK_EQUAL(qkBLTL.isContainedBy(qkTL), false, "BL -> TL");
				CHECK_EQUAL(qkBLTR.isContainedBy(qkTL), false, "BL -> TR");
				CHECK_EQUAL(qkBLBL.isContainedBy(qkTL), false, "BL -> BL");
				CHECK_EQUAL(qkBLBR.isContainedBy(qkTL), false, "BL -> BR");

				CHECK_EQUAL(qkBRTL.isContainedBy(qkTL), false, "BR -> TL");
				CHECK_EQUAL(qkBRTR.isContainedBy(qkTL), false, "BR -> TR");
				CHECK_EQUAL(qkBRBL.isContainedBy(qkTL), false, "BR -> BL");
				CHECK_EQUAL(qkBRBR.isContainedBy(qkTL), false, "BR -> BR");
			}
			{SCOPE_SECTION("TR descendants");
				CHECK_EQUAL(qkTRTL.isContainedBy(qkTR), true, "TR -> TL");
				CHECK_EQUAL(qkTRTR.isContainedBy(qkTR), true, "TR -> TR");
				CHECK_EQUAL(qkTRBL.isContainedBy(qkTR), true, "TR -> BL");
				CHECK_EQUAL(qkTRBR.isContainedBy(qkTR), true, "TR -> BR");

				CHECK_EQUAL(qkTLTL.isContainedBy(qkTR), false, "TL -> TL");
				CHECK_EQUAL(qkTLTR.isContainedBy(qkTR), false, "TL -> TR");
				CHECK_EQUAL(qkTLBL.isContainedBy(qkTR), false, "TL -> BL");
				CHECK_EQUAL(qkTLBR.isContainedBy(qkTR), false, "TL -> BR");

				CHECK_EQUAL(qkBLTL.isContainedBy(qkTR), false, "BL -> TL");
				CHECK_EQUAL(qkBLTR.isContainedBy(qkTR), false, "BL -> TR");
				CHECK_EQUAL(qkBLBL.isContainedBy(qkTR), false, "BL -> BL");
				CHECK_EQUAL(qkBLBR.isContainedBy(qkTR), false, "BL -> BR");

				CHECK_EQUAL(qkBRTL.isContainedBy(qkTR), false, "BR -> TL");
				CHECK_EQUAL(qkBRTR.isContainedBy(qkTR), false, "BR -> TR");
				CHECK_EQUAL(qkBRBL.isContainedBy(qkTR), false, "BR -> BL");
				CHECK_EQUAL(qkBRBR.isContainedBy(qkTR), false, "BR -> BR");
			}
			{SCOPE_SECTION("BL descendants");
				CHECK_EQUAL(qkBLTL.isContainedBy(qkBL), true, "BL -> TL");
				CHECK_EQUAL(qkBLTR.isContainedBy(qkBL), true, "BL -> TR");
				CHECK_EQUAL(qkBLBL.isContainedBy(qkBL), true, "BL -> BL");
				CHECK_EQUAL(qkBLBR.isContainedBy(qkBL), true, "BL -> BR");

				CHECK_EQUAL(qkTLTL.isContainedBy(qkBL), false, "TL -> TL");
				CHECK_EQUAL(qkTLTR.isContainedBy(qkBL), false, "TL -> TR");
				CHECK_EQUAL(qkTLBL.isContainedBy(qkBL), false, "TL -> BL");
				CHECK_EQUAL(qkTLBR.isContainedBy(qkBL), false, "TL -> BR");

				CHECK_EQUAL(qkTRTL.isContainedBy(qkBL), false, "TR -> TL");
				CHECK_EQUAL(qkTRTR.isContainedBy(qkBL), false, "TR -> TR");
				CHECK_EQUAL(qkTRBL.isContainedBy(qkBL), false, "TR -> BL");
				CHECK_EQUAL(qkTRBR.isContainedBy(qkBL), false, "TR -> BR");

				CHECK_EQUAL(qkBRTL.isContainedBy(qkBL), false, "BR -> TL");
				CHECK_EQUAL(qkBRTR.isContainedBy(qkBL), false, "BR -> TR");
				CHECK_EQUAL(qkBRBL.isContainedBy(qkBL), false, "BR -> BL");
				CHECK_EQUAL(qkBRBR.isContainedBy(qkBL), false, "BR -> BR");
			}
			{SCOPE_SECTION("BR descendants");
				CHECK_EQUAL(qkBRTL.isContainedBy(qkBR), true, "BR -> TL");
				CHECK_EQUAL(qkBRTR.isContainedBy(qkBR), true, "BR -> TR");
				CHECK_EQUAL(qkBRBL.isContainedBy(qkBR), true, "BR -> BL");
				CHECK_EQUAL(qkBRBR.isContainedBy(qkBR), true, "BR -> BR");

				CHECK_EQUAL(qkTLTL.isContainedBy(qkBR), false, "TL -> TL");
				CHECK_EQUAL(qkTLTR.isContainedBy(qkBR), false, "TL -> TR");
				CHECK_EQUAL(qkTLBL.isContainedBy(qkBR), false, "TL -> BL");
				CHECK_EQUAL(qkTLBR.isContainedBy(qkBR), false, "TL -> BR");

				CHECK_EQUAL(qkTRTL.isContainedBy(qkBR), false, "TR -> TL");
				CHECK_EQUAL(qkTRTR.isContainedBy(qkBR), false, "TR -> TR");
				CHECK_EQUAL(qkTRBL.isContainedBy(qkBR), false, "TR -> BL");
				CHECK_EQUAL(qkTRBR.isContainedBy(qkBR), false, "TR -> BR");

				CHECK_EQUAL(qkBLTL.isContainedBy(qkBR), false, "BL -> TL");
				CHECK_EQUAL(qkBLTR.isContainedBy(qkBR), false, "BL -> TR");
				CHECK_EQUAL(qkBLBL.isContainedBy(qkBR), false, "BL -> BL");
				CHECK_EQUAL(qkBLBR.isContainedBy(qkBR), false, "BL -> BR");
			}
		}
		{SCOPE_SECTION("Ancestor")
			{SCOPE_SECTION("TL as ancestor")
				CHECK_EQUAL(qkTL.contains(qkTLTL), true, "TL -> TL");
				CHECK_EQUAL(qkTL.contains(qkTLTR), true, "TL -> TR");
				CHECK_EQUAL(qkTL.contains(qkTLBL), true, "TL -> BL");
				CHECK_EQUAL(qkTL.contains(qkTLBR), true, "TL -> BR");

				CHECK_EQUAL(qkTL.contains(qkTRTL), false, "TR -> TL");
				CHECK_EQUAL(qkTL.contains(qkTRTR), false, "TR -> TR");
				CHECK_EQUAL(qkTL.contains(qkTRBL), false, "TR -> BL");
				CHECK_EQUAL(qkTL.contains(qkTRBR), false, "TR -> BR");

				CHECK_EQUAL(qkTL.contains(qkBLTL), false, "BL -> TL");
				CHECK_EQUAL(qkTL.contains(qkBLTR), false, "BL -> TR");
				CHECK_EQUAL(qkTL.contains(qkBLBL), false, "BL -> BL");
				CHECK_EQUAL(qkTL.contains(qkBLBR), false, "BL -> BR");

				CHECK_EQUAL(qkTL.contains(qkBRTL), false, "BR -> TL");
				CHECK_EQUAL(qkTL.contains(qkBRTR), false, "BR -> TR");
				CHECK_EQUAL(qkTL.contains(qkBRBL), false, "BR -> BL");
				CHECK_EQUAL(qkTL.contains(qkBRBR), false, "BR -> BR");
			}
			{SCOPE_SECTION("TR as ancestor")
				CHECK_EQUAL(qkTR.contains(qkTRTL), true, "TR -> TL");
				CHECK_EQUAL(qkTR.contains(qkTRTR), true, "TR -> TR");
				CHECK_EQUAL(qkTR.contains(qkTRBL), true, "TR -> BL");
				CHECK_EQUAL(qkTR.contains(qkTRBR), true, "TR -> BR");

				CHECK_EQUAL(qkTR.contains(qkTLTL), false, "TL -> TL");
				CHECK_EQUAL(qkTR.contains(qkTLTR), false, "TL -> TR");
				CHECK_EQUAL(qkTR.contains(qkTLBL), false, "TL -> BL");
				CHECK_EQUAL(qkTR.contains(qkTLBR), false, "TL -> BR");

				CHECK_EQUAL(qkTR.contains(qkBLTL), false, "BL -> TL");
				CHECK_EQUAL(qkTR.contains(qkBLTR), false, "BL -> TR");
				CHECK_EQUAL(qkTR.contains(qkBLBL), false, "BL -> BL");
				CHECK_EQUAL(qkTR.contains(qkBLBR), false, "BL -> BR");

				CHECK_EQUAL(qkTR.contains(qkBRTL), false, "BR -> TL");
				CHECK_EQUAL(qkTR.contains(qkBRTR), false, "BR -> TR");
				CHECK_EQUAL(qkTR.contains(qkBRBL), false, "BR -> BL");
				CHECK_EQUAL(qkTR.contains(qkBRBR), false, "BR -> BR");
			}
			{SCOPE_SECTION("BL as ancestor")
				CHECK_EQUAL(qkBL.contains(qkBLTL), true, "BL -> TL");
				CHECK_EQUAL(qkBL.contains(qkBLTR), true, "BL -> TR");
				CHECK_EQUAL(qkBL.contains(qkBLBL), true, "BL -> BL");
				CHECK_EQUAL(qkBL.contains(qkBLBR), true, "BL -> BR");

				CHECK_EQUAL(qkBL.contains(qkTLTL), false, "TL -> TL");
				CHECK_EQUAL(qkBL.contains(qkTLTR), false, "TL -> TR");
				CHECK_EQUAL(qkBL.contains(qkTLBL), false, "TL -> BL");
				CHECK_EQUAL(qkBL.contains(qkTLBR), false, "TL -> BR");

				CHECK_EQUAL(qkBL.contains(qkTRTL), false, "TR -> TL");
				CHECK_EQUAL(qkBL.contains(qkTRTR), false, "TR -> TR");
				CHECK_EQUAL(qkBL.contains(qkTRBL), false, "TR -> BL");
				CHECK_EQUAL(qkBL.contains(qkTRBR), false, "TR -> BR");

				CHECK_EQUAL(qkBL.contains(qkBRTL), false, "BR -> TL");
				CHECK_EQUAL(qkBL.contains(qkBRTR), false, "BR -> TR");
				CHECK_EQUAL(qkBL.contains(qkBRBL), false, "BR -> BL");
				CHECK_EQUAL(qkBL.contains(qkBRBR), false, "BR -> BR");
			}
			{SCOPE_SECTION("BR as ancestor")
				CHECK_EQUAL(qkBR.contains(qkBRTL), true, "BR -> TL");
				CHECK_EQUAL(qkBR.contains(qkBRTR), true, "BR -> TR");
				CHECK_EQUAL(qkBR.contains(qkBRBL), true, "BR -> BL");
				CHECK_EQUAL(qkBR.contains(qkBRBR), true, "BR -> BR");

				CHECK_EQUAL(qkBR.contains(qkTLTL), false, "TL -> TL");
				CHECK_EQUAL(qkBR.contains(qkTLTR), false, "TL -> TR");
				CHECK_EQUAL(qkBR.contains(qkTLBL), false, "TL -> BL");
				CHECK_EQUAL(qkBR.contains(qkTLBR), false, "TL -> BR");

				CHECK_EQUAL(qkBR.contains(qkTRTL), false, "TR -> TL");
				CHECK_EQUAL(qkBR.contains(qkTRTR), false, "TR -> TR");
				CHECK_EQUAL(qkBR.contains(qkTRBL), false, "TR -> BL");
				CHECK_EQUAL(qkBR.contains(qkTRBR), false, "TR -> BR");

				CHECK_EQUAL(qkBR.contains(qkBLTL), false, "BL -> TL");
				CHECK_EQUAL(qkBR.contains(qkBLTR), false, "BL -> TR");
				CHECK_EQUAL(qkBR.contains(qkBLBL), false, "BL -> BL");
				CHECK_EQUAL(qkBR.contains(qkBLBR), false, "BL -> BR");
			}
		}
		{ SCOPE_SECTION("Remap Root Quadrant")
			{SCOPE_SECTION("Depth 1");
				auto mutableTL = qkTL.remap_root_quadrant(Quad::TopRight);
				CHECK_EQUAL(mutableTL, QK({Quad::TopRight}), "TL -> TR");

				auto mutableBR = qkBR.remap_root_quadrant(Quad::BottomLeft);
				CHECK_EQUAL(mutableBR, QK({Quad::BottomLeft}), "BR -> BL");
			}
			{SCOPE_SECTION("Depth 2");
				// TL -> BL becomes TR -> BL
				auto m1 = qkTLBL.remap_root_quadrant(Quad::TopRight);
				CHECK_EQUAL(m1, QK({Quad::TopRight, Quad::BottomLeft}), "TL,BL -> TR,BL");

				// BR -> TR becomes BL -> TR
				auto m2 = qkBRTR.remap_root_quadrant(Quad::BottomLeft);
				CHECK_EQUAL(m2, QK({Quad::BottomLeft, Quad::TopRight}), "BR,TR -> BL,TR");
			}

			// depth‑3 mixed cases
			{SCOPE_SECTION("Depth 3");
				constexpr auto qk_TR_BL_BR = QK({Quad::TopRight, Quad::BottomLeft, Quad::BottomRight});
				auto m3 = qk_TR_BL_BR.remap_root_quadrant(Quad::TopLeft);
				// TR -> TL, BL, BR remain
				CHECK_EQUAL(m3, QK({Quad::TopLeft, Quad::BottomLeft, Quad::BottomRight}), "TR,BL,BR -> TL,BL,BR");

				constexpr auto qk_BL_TR_TL = QK({Quad::BottomLeft, Quad::TopRight, Quad::TopLeft});
				auto m4 = qk_BL_TR_TL.remap_root_quadrant(Quad::BottomRight);
				// BL -> BR, TR, TL remain
				CHECK_EQUAL(m4, QK({Quad::BottomRight, Quad::TopRight, Quad::TopLeft}), "BL,TR,TL -> BR,TR,TL");
			}
		}
		{SCOPE_SECTION("Generate leaf nodes");
			constexpr glm::vec2 test_origin{0.0f, 0.0f};
			constexpr float test_half_size    = 50.0f;
			constexpr float test_quarter_size = test_half_size * 0.5f;
			constexpr QK root = QK{0, 0}; // Root quadkey at depth 0

			// Simple lambda for constant desired depth
			auto constant_depth_func = [](Geometry::Depth_t d)
			{
				return [d](const Geometry::AABB2D&) { return d; };
			};
			// Convenience for comparing sets
			auto sort_keys = [](std::vector<QK>& keys)
			{
				std::sort(keys.begin(), keys.end(), [](const QK& a, const QK& b)
				{
					return a.key < b.key || (a.key == b.key && a.depth < b.depth);
				});
			};

			{SCOPE_SECTION("Depth 0");
				std::vector<QK> out_keys;
				generate_leaf_nodes(test_origin, test_half_size, 0, 0, 4, out_keys, constant_depth_func(0));
				CHECK_EQUAL(out_keys.size(), 1, "Should produce only the root node");
				CHECK_EQUAL(out_keys[0], root, "Root node has correct key and depth");
				CHECK_EQUAL(out_keys[0].get_bounds(test_half_size, test_origin), Geometry::AABB2D(test_origin, test_half_size), "Root node bounds");
			}
			{SCOPE_SECTION("Depth 1");
				std::vector<QK> out_keys;
				generate_leaf_nodes(test_origin, test_half_size, 0, 0, 1, out_keys, constant_depth_func(1));
				CHECK_EQUAL(out_keys.size(), 4, "Should produce 4 leaf nodes at depth 1");
				std::vector<QK> expected = {
					QK{0b00, 1},
					QK{0b01, 1},
					QK{0b10, 1},
					QK{0b11, 1}
				};
				sort_keys(out_keys);
				CHECK_CONTAINER_EQUAL(out_keys, expected, "Depth 1 quadkeys match expected children");
			}
			{SCOPE_SECTION("Depth 2");
				std::vector<QK> out_keys;
				generate_leaf_nodes(test_origin, test_half_size, 0, 0, 2, out_keys, constant_depth_func(2));
				CHECK_EQUAL(out_keys.size(), 16, "Should produce 16 leaf nodes at depth 2");

				std::array<std::pair<QK, Geometry::AABB2D>, 16> expected =
				{{
					{QK{Quad::TopLeft, Quad::TopLeft},         Geometry::AABB2D(glm::vec2(test_origin.x - test_half_size,     test_origin.y + test_quarter_size), glm::vec2(test_origin.x - test_quarter_size, test_origin.y + test_half_size))},
					{QK{Quad::TopLeft, Quad::TopRight},        Geometry::AABB2D(glm::vec2(test_origin.x - test_quarter_size,  test_origin.y + test_quarter_size), glm::vec2(test_origin.x,                     test_origin.y + test_half_size))},
					{QK{Quad::TopLeft, Quad::BottomLeft},      Geometry::AABB2D(glm::vec2(test_origin.x - test_half_size,     test_origin.y),                     glm::vec2(test_origin.x - test_quarter_size, test_origin.y + test_quarter_size))},
					{QK{Quad::TopLeft, Quad::BottomRight},     Geometry::AABB2D(glm::vec2(test_origin.x - test_quarter_size,  test_origin.y),                     glm::vec2(test_origin.x,                     test_origin.y + test_quarter_size))},

					{QK{Quad::TopRight, Quad::TopLeft},        Geometry::AABB2D(glm::vec2(test_origin.x,                     test_origin.y + test_quarter_size),  glm::vec2(test_origin.x + test_quarter_size, test_origin.y + test_half_size))},
					{QK{Quad::TopRight, Quad::TopRight},       Geometry::AABB2D(glm::vec2(test_origin.x + test_quarter_size, test_origin.y + test_quarter_size),  glm::vec2(test_origin.x + test_half_size,    test_origin.y + test_half_size))},
					{QK{Quad::TopRight, Quad::BottomLeft},     Geometry::AABB2D(glm::vec2(test_origin.x,                     test_origin.y),                      glm::vec2(test_origin.x + test_quarter_size, test_origin.y + test_quarter_size))},
					{QK{Quad::TopRight, Quad::BottomRight},    Geometry::AABB2D(glm::vec2(test_origin.x + test_quarter_size, test_origin.y),                      glm::vec2(test_origin.x + test_half_size,    test_origin.y + test_quarter_size))},

					{QK{Quad::BottomLeft, Quad::TopLeft},      Geometry::AABB2D(glm::vec2(test_origin.x - test_half_size,     test_origin.y - test_quarter_size), glm::vec2(test_origin.x - test_quarter_size, test_origin.y))},
					{QK{Quad::BottomLeft, Quad::TopRight},     Geometry::AABB2D(glm::vec2(test_origin.x - test_quarter_size,  test_origin.y - test_quarter_size), glm::vec2(test_origin.x,                     test_origin.y))},
					{QK{Quad::BottomLeft, Quad::BottomLeft},   Geometry::AABB2D(glm::vec2(test_origin.x - test_half_size,     test_origin.y - test_half_size),    glm::vec2(test_origin.x - test_quarter_size, test_origin.y - test_quarter_size))},
					{QK{Quad::BottomLeft, Quad::BottomRight},  Geometry::AABB2D(glm::vec2(test_origin.x - test_quarter_size,  test_origin.y - test_half_size),    glm::vec2(test_origin.x,                     test_origin.y - test_quarter_size))},

					{QK{Quad::BottomRight, Quad::TopLeft},     Geometry::AABB2D(glm::vec2(test_origin.x,                     test_origin.y - test_quarter_size),  glm::vec2(test_origin.x + test_quarter_size, test_origin.y))},
					{QK{Quad::BottomRight, Quad::TopRight},    Geometry::AABB2D(glm::vec2(test_origin.x + test_quarter_size, test_origin.y - test_quarter_size),  glm::vec2(test_origin.x + test_half_size,    test_origin.y))},
					{QK{Quad::BottomRight, Quad::BottomLeft},  Geometry::AABB2D(glm::vec2(test_origin.x,                     test_origin.y - test_half_size),     glm::vec2(test_origin.x + test_quarter_size, test_origin.y - test_quarter_size))},
					{QK{Quad::BottomRight, Quad::BottomRight}, Geometry::AABB2D(glm::vec2(test_origin.x + test_quarter_size, test_origin.y - test_half_size),     glm::vec2(test_origin.x + test_half_size,    test_origin.y - test_quarter_size))},
				}};
				sort_keys(out_keys);

				if (out_keys.size() == expected.size())
				{
					for (const auto& qk : out_keys)
					{
						CHECK_EQUAL(qk.depth, 2, "Each key has depth 2");
						CHECK_EQUAL(qk, expected[qk.key].first, "Key matches expected for: " + qk.to_string());
						CHECK_EQUAL(qk.get_bounds(test_half_size, test_origin), expected[qk.key].second, "Bounds match for key: " + qk.to_string());
					}
				}
			}
		}
	}
} // namespace Test
DISABLE_WARNING_POP