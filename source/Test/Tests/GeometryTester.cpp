#include "GeometryTester.hpp"

// GEOMETRY
#include "Intersect.hpp"
#include "AABB.hpp"
#include "Triangle.hpp"
#include "Geometry/Frustrum.hpp"

#include "Stopwatch.hpp"
#include "Utility.hpp"

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
        { // Default intiailise
            Geometry::AABB aabb;
            runUnitTest({aabb.get_size() == glm::vec3(0.f), "AABB initialise size at 0", "Expected size of default AABB to be 0"});
            runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
            runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
        }
        { // Initialise with a min and max
            // An AABB at low point [-1,-1,-1] to [1,1,1]
            auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
            runUnitTest({aabb.get_size() == glm::vec3(2.f), "AABB initialised with min and max size at 2", "Expected size of AABB to be 2"});
            runUnitTest({aabb.get_center() == glm::vec3(0.f), "AABB initialise with min and max position", "Expected AABB to center at [0, 0, 0]"});
        }
        { // Initialise with a min and max not at origin
            // An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
            auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
            runUnitTest({aabb.get_size() == glm::vec3(4.f), "AABB initialised with min and max not at origin", "Expected size of AABB to be 4.f"});
            runUnitTest({aabb.get_center() == glm::vec3(3.f), "AABB initialised with min and max not at origin", "Expected AABB to center at [3, 3, 3]"});
        }

        { // Tranform
        }
        { // Unite

            { // check result of the static AABB::unite is the same as member unite
            }
        }
        { // Contains

        }

        { // Intersections
            {// No touch in all directions
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
    {
        const auto control = Geometry::Triangle(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f));

        { // Transform tests
            { // Identity
                auto transform = glm::identity<glm::mat4>();
                auto t1 = control;
                t1.transform(transform);
                runUnitTest({t1 == control, "Triangle - Transform - Identity", "Expected identity transform matrix to not change triangle"});
            }
            { // Transform - Translate
                auto transformed = control;
                const glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.f, 0.f, 0.f)); // Keep translating right

                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(3.f, 1.f, 0.f), glm::vec3(4.f, -1.f, 0.f), glm::vec3(2.f, -1.f, 0.f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 1", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(6.f, 1.f, 0.f), glm::vec3(7.f, -1.f, 0.f), glm::vec3(5.f, -1.f, 0.f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 2", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(9.f, 1.f, 0.f), glm::vec3(10.f, -1.f, 0.f), glm::vec3(8.f, -1.f, 0.f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 3", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(12.f, 1.f, 0.f), glm::vec3(13.f, -1.f, 0.f), glm::vec3(11.f, -1.f, 0.f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 4", "Not matching expected result"});
                }
            }
            { // Transform - Rotate
                auto transformed = control;
                const glm::mat4 transform = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333337f, 1.3333334f), glm::vec3(0.99999994f, -0.3333333f, -0.6666666f), glm::vec3(-0.99999994f, -0.3333333f, -0.6666666f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 1", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(0.f, -1.6666667f, -5.9604645e-08f), glm::vec3(0.9999999f, 0.3333333f, 8.940697e-08f), glm::vec3(-0.9999999f, 0.3333333f, 8.940697e-08f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 2", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333325f, -1.3333333f), glm::vec3(0.9999998f, -0.33333346f, 0.66666675f), glm::vec3(-0.9999998f, -0.33333346f, 0.66666675f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 3", "Not matching expected result"});
                }
                {
                    transformed.transform(transform);
                    auto expected = Geometry::Triangle(glm::vec3(0.f, 0.9999999f, 2.9802322e-07f), glm::vec3(0.99999976f, -1.0000001f, 0.f), glm::vec3(-0.99999976f, -1.0000001f, 0.f));
                    runUnitTest({transformed == expected, "Triangle - Transform - Translate right 4", "Not matching expected result"});
                }
            }

            { // Transform - Scale
                //transform = glm::scale(transform, glm::vec3(2.0f));
            }
            { // Transform - Combined

            }
        }

        {// No Collision / Coplanar
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.5f, 0.f), glm::vec3(1.f, 1.5f, 0.f), glm::vec3(-1.f, 1.5f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, -1.5f, 0.f), glm::vec3(1.f, -3.5f, 0.f), glm::vec3(-1.f, -3.5f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(-2.5f, 1.f, 0.f), glm::vec3(-1.5f, -1.f, 0.f), glm::vec3(-3.5f, -1.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(2.5f, 1.f, 0.f), glm::vec3(3.5f, -1.f, 0.f), glm::vec3(1.5f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
            auto t6 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -1.f), glm::vec3(-1.f, -1.f, -1.f));

            runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - no collision 1", "Expected no collision"});
            runUnitTest({!Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - no collision 2", "Expected no collision"});
            runUnitTest({!Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - no collision 3", "Expected no collision"});
            runUnitTest({!Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - no collision 4", "Expected no collision"});
            runUnitTest({!Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - no collision 5", "Expected no collision"});
            runUnitTest({!Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - no collision 6", "Expected no collision"});
        }
        {// Collision = true / Coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 3.f, 0.f), glm::vec3(2.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(-2.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-3.f, -1.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(2.f, 1.f, 0.f), glm::vec3(3.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, -1.f, 0.f), glm::vec3(0.f, -3.f, 0.f), glm::vec3(-2.f, -3.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(1.f, -1.f, 0.f), glm::vec3(2.f, -3.f, 0.f), glm::vec3(0.f, -3.f, 0.f));

            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - edge-edge 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - edge-edge 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - edge-edge 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - edge-edge 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - edge-edge 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 3.f, -1.f), glm::vec3(0.f, 1.f, -2.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -2.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, -2.f), glm::vec3(-1.f, -1.f, 0.f));

            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Non-coplanar - edge-edge 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Non-coplanar - edge-edge 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Non-coplanar - edge-edge 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Non-coplanar - edge-edge 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Non-coplanar - edge-edge 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Non-coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.5, 2.f, 0.f), glm::vec3(2.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.5f, 0.f, 0.f), glm::vec3(2.5f, -2.f, 0.f), glm::vec3(0.5f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, -3.f, 0.f), glm::vec3(-1.f, -3.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.5f, 0.f, 0.f), glm::vec3(-0.5f, -2.f, 0.f), glm::vec3(-2.5f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.5f, 2.f, 0.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-2.5f, 0.f, 0.f));

            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - edge-side 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - edge-side 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - edge-side 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - edge-side 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - edge-side 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / Non-coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.5f, 2.f, 1.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.5f, 2.f, -1.f), glm::vec3(0.5f, 0.f, -2.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(0.f, -1.f, -2.f), glm::vec3(0.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, 1.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-0.5f, 0.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, -1.f), glm::vec3(-0.5f, 0.f, -2.f), glm::vec3(-0.5f, 0.f, 0.f));

            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Non-coplanar - edge-side 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Non-coplanar - edge-side 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Non-coplanar - edge-side 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Non-coplanar - edge-side 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Non-coplanar - edge-side 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Non-coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::vec3(-1.f, 0.5f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 2.f, 0.f), glm::vec3(2.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, -2.f, 0.f), glm::vec3(0.f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, -2.5f, 0.f), glm::vec3(-1.f, -2.5f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -2.f, 0.f), glm::vec3(-2.f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(-2.f, 0.f, 0.f));

            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - coplanar - overlap 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - coplanar - overlap 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - coplanar - overlap 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - coplanar - overlap 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - coplanar - overlap 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - coplanar - overlap 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, 1.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -2.f, -1.f), glm::vec3(0.f, -2.f, 1.f));
            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - non-coplanar - overlap 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - non-coplanar - overlap 2", "Expected collision to be true"});
        }
        {// Collision - off-axis collisions
            auto t1 = Geometry::Triangle(glm::vec3(2.f, 1.f, -1.f), glm::vec3(1.f, -2.f, 1.f), glm::vec3(-1.f, -2.f, 1.f));
            runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - off-axis - one side collision", "Expected collision to be true"});

            // Like t1 but two sides of triangle cut through control
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - off-axis - two side collision", "Expected collision to be true"});

            // Triangle passes under control without collision
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runUnitTest({!Geometry::intersecting(control, t3), "Triangle v Triangle - off-axis - pass under no collision", "Expected no collision"});
        }
        { // Epsilon tests
            // Place test triangles touching control then move them away by epsilon and check no collision.
            { // Coplanar to control touching edge to edge
                // t1 bottom-right edge touches the control top edge
                auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
                runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected collision to be true"});
                t1.translate({-std::numeric_limits<float>::epsilon() * 2.f, 0.f, 0.f});
                runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected no collision after moving left"});
            }
            {
                // Perpendicular to control (non-coplanar), touching the bottom.
                auto t1 = Geometry::Triangle(glm::vec3(0.f, -1.f, -1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
                runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected collision to be true"});
                t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
                runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected no collision after moving down"});
            }
            {
                // Triangle passes under control touching the bottom side at an angle
                auto t1 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
                runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under touch", "Expected collision to be true"});
                // Triangle moved below control by epsilon to no longer collide
                t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
                runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under epsilon distance", "Expected no collision after moving down"});
            }
        }
        {// Edge cases
            runUnitTest({Geometry::intersecting(control, control), "Triangle v Triangle - equal triangles", "Expected collision to be true"});
        }
    }

    void GeometryTester::run_frustrum_tests()
    {
        { // Create an 'identity' ortho projection and check the planes resulting.
            float ortho_size = 1.f;
            float near       = -1.f;
            float far        = 1.f;
            auto projection  = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
            auto frustrum    = Geometry::Frustrum(projection);

            runUnitTest({frustrum.m_left.m_distance   == ortho_size, "Frustrum from ortho projection identity - distance - left", "Distance should match the ortho size"});
            runUnitTest({frustrum.m_right.m_distance  == ortho_size, "Frustrum from ortho projection identity - distance - right", "Distance should match the ortho size"});
            runUnitTest({frustrum.m_bottom.m_distance == ortho_size, "Frustrum from ortho projection identity - distance - bottom", "Distance should match the ortho size"});
            runUnitTest({frustrum.m_top.m_distance    == ortho_size, "Frustrum from ortho projection identity - distance - top", "Distance should match the ortho size"});
            runUnitTest({frustrum.m_near.m_distance   == -1.f, "Frustrum from ortho projection identity - distance - near", "Distance should match the near size"});
            runUnitTest({frustrum.m_far.m_distance    == -1.f, "Frustrum from ortho projection identity - distance - far", "Distance should match the far size"});

            runUnitTest({frustrum.m_left.m_normal   == glm::vec3(1.f, 0.f, 0.f), "Frustrum from ortho projection identity - normal - left", "Should be pointing towards the negative x-axis"});
            runUnitTest({frustrum.m_right.m_normal  == glm::vec3(-1.f, 0.f, 0.f), "Frustrum from ortho projection identity - normal - right", "Should be pointing towards the positive x-axis"});
            runUnitTest({frustrum.m_bottom.m_normal == glm::vec3(0.f, 1.f, 0.f), "Frustrum from ortho projection identity - normal - bottom", "Should be pointing towards the negative y-axis"});
            runUnitTest({frustrum.m_top.m_normal    == glm::vec3(0.f, -1.f, 0.f), "Frustrum from ortho projection identity - normal - top", "Should be pointing towards the positive y-axis"});
            runUnitTest({frustrum.m_near.m_normal   == glm::vec3(0.f, 0.f, 1.f), "Frustrum from ortho projection identity - normal - near", "Should be pointing towards the negative z-axis"});
            runUnitTest({frustrum.m_far.m_normal    == glm::vec3(0.f, 0.f, -1.f), "Frustrum from ortho projection identity - normal - far", "Should be pointing towards the positive z-axis"});
        }
        { // Create an 'non-identity' ortho projection and check the planes resulting. Previous test can get away with non-normalising of the plane equations, but this test uses a non-1 ortho_size.
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

            runUnitTest({error_threshold_equality(frustrum.m_left.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - left", "Distance should match the ortho size"});
            runUnitTest({error_threshold_equality(frustrum.m_left.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - left", "Distance should match the ortho size"});
            runUnitTest({error_threshold_equality(frustrum.m_right.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - right", "Distance should match the ortho size"});
            runUnitTest({error_threshold_equality(frustrum.m_bottom.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - bottom", "Distance should match the ortho size"});
            runUnitTest({error_threshold_equality(frustrum.m_top.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - top", "Distance should match the ortho size"});
            runUnitTest({frustrum.m_near.m_distance   == 0.f, "Frustrum from ortho projection - distance - near", "Distance should match the near size"});
            runUnitTest({frustrum.m_far.m_distance    == -10.f, "Frustrum from ortho projection - distance - far", "Distance should match the far size"});

            runUnitTest({frustrum.m_left.m_normal   == glm::vec3(1.f, 0.f, 0.f), "Frustrum from ortho projection - normal - left", "Should be pointing towards the negative x-axis"});
            runUnitTest({frustrum.m_right.m_normal  == glm::vec3(-1.f, 0.f, 0.f), "Frustrum from ortho projection - normal - right", "Should be pointing towards the positive x-axis"});
            runUnitTest({frustrum.m_bottom.m_normal == glm::vec3(0.f, 1.f, 0.f), "Frustrum from ortho projection - normal - bottom", "Should be pointing towards the negative y-axis"});
            runUnitTest({frustrum.m_top.m_normal    == glm::vec3(0.f, -1.f, 0.f), "Frustrum from ortho projection - normal - top", "Should be pointing towards the positive y-axis"});
            runUnitTest({frustrum.m_near.m_normal   == glm::vec3(0.f, 0.f, 1.f), "Frustrum from ortho projection - normal - near", "Should be pointing towards the negative z-axis"});
            runUnitTest({frustrum.m_far.m_normal    == glm::vec3(0.f, 0.f, -1.f), "Frustrum from ortho projection - normal - far", "Should be pointing towards the positive z-axis"});
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
} // namespace Test