#include "GeometryTester.hpp"

// GEOMETRY
#include "Intersect.hpp"
#include "AABB.hpp"
#include "Triangle.hpp"

#include "Stopwatch.hpp"
#include "Utility.hpp"

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <array>

namespace Test
{
    void GeometryTester::runUnitTests()
    {
        runAABBTests();
        runTriangleTests();
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
                Geometry::intersect_triangle_triangle_static(triangles[i], triangles[i + 1]);
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
            runUnitTest({aabb.getSize() == glm::vec3(0.f), "AABB initialise size at 0", "Expected size of default AABB to be 0"});
            runUnitTest({aabb.getCenter () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
            runUnitTest({aabb.getCenter () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
        }
        { // Initialise with a min and max
            // An AABB at low point [-1,-1,-1] to [1,1,1]
            auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
            runUnitTest({aabb.getSize() == glm::vec3(2.f), "AABB initialised with min and max size at 2", "Expected size of AABB to be 2"});
            runUnitTest({aabb.getCenter() == glm::vec3(0.f), "AABB initialise with min and max position", "Expected AABB to center at [0, 0, 0]"});
        }
        { // Initialise with a min and max not at origin
            // An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
            auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
            runUnitTest({aabb.getSize() == glm::vec3(4.f), "AABB initialised with min and max not at origin", "Expected size of AABB to be 4.f"});
            runUnitTest({aabb.getCenter() == glm::vec3(3.f), "AABB initialised with min and max not at origin", "Expected AABB to center at [3, 3, 3]"});
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
        auto control = Geometry::Triangle(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f));

        {// No Collision / Coplanar
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.5f, 0.f), glm::vec3(1.f, 1.5f, 0.f), glm::vec3(-1.f, 1.5f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, -1.5f, 0.f), glm::vec3(1.f, -3.5f, 0.f), glm::vec3(-1.f, -3.5f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(-2.5f, 1.f, 0.f), glm::vec3(-1.5f, -1.f, 0.f), glm::vec3(-3.5f, -1.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(2.5f, 1.f, 0.f), glm::vec3(3.5f, -1.f, 0.f), glm::vec3(1.5f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
            auto t6 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -1.f), glm::vec3(-1.f, -1.f, -1.f));

            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Coplanar - no collision 1", "Expected no collision"});
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - Coplanar - no collision 2", "Expected no collision"});
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - Coplanar - no collision 3", "Expected no collision"});
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - Coplanar - no collision 4", "Expected no collision"});
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - Coplanar - no collision 5", "Expected no collision"});
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - Coplanar - no collision 6", "Expected no collision"});
        }
        {// Collision = true / Coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 3.f, 0.f), glm::vec3(2.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(-2.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-3.f, -1.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(2.f, 1.f, 0.f), glm::vec3(3.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, -1.f, 0.f), glm::vec3(0.f, -3.f, 0.f), glm::vec3(-2.f, -3.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(1.f, -1.f, 0.f), glm::vec3(2.f, -3.f, 0.f), glm::vec3(0.f, -3.f, 0.f));

            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Coplanar - edge-edge 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - Coplanar - edge-edge 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - Coplanar - edge-edge 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - Coplanar - edge-edge 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - Coplanar - edge-edge 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - Coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 3.f, -1.f), glm::vec3(0.f, 1.f, -2.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -2.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, -2.f), glm::vec3(-1.f, -1.f, 0.f));

            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Non-coplanar - edge-edge 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - Non-coplanar - edge-edge 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - Non-coplanar - edge-edge 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - Non-coplanar - edge-edge 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - Non-coplanar - edge-edge 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - Non-coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.5, 2.f, 0.f), glm::vec3(2.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.5f, 0.f, 0.f), glm::vec3(2.5f, -2.f, 0.f), glm::vec3(0.5f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, -3.f, 0.f), glm::vec3(-1.f, -3.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.5f, 0.f, 0.f), glm::vec3(-0.5f, -2.f, 0.f), glm::vec3(-2.5f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.5f, 2.f, 0.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-2.5f, 0.f, 0.f));

            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Coplanar - edge-side 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - Coplanar - edge-side 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - Coplanar - edge-side 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - Coplanar - edge-side 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - Coplanar - edge-side 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - Coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / Non-coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.5f, 2.f, 1.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.5f, 2.f, -1.f), glm::vec3(0.5f, 0.f, -2.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(0.f, -1.f, -2.f), glm::vec3(0.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, 1.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-0.5f, 0.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, -1.f), glm::vec3(-0.5f, 0.f, -2.f), glm::vec3(-0.5f, 0.f, 0.f));

            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Non-coplanar - edge-side 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - Non-coplanar - edge-side 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - Non-coplanar - edge-side 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - Non-coplanar - edge-side 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - Non-coplanar - edge-side 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - Non-coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::vec3(-1.f, 0.5f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 2.f, 0.f), glm::vec3(2.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, -2.f, 0.f), glm::vec3(0.f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, -2.5f, 0.f), glm::vec3(-1.f, -2.5f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -2.f, 0.f), glm::vec3(-2.f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(-2.f, 0.f, 0.f));

            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - coplanar - overlap 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - coplanar - overlap 2", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - coplanar - overlap 3", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t4), "Triangle v Triangle - coplanar - overlap 4", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t5), "Triangle v Triangle - coplanar - overlap 5", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t6), "Triangle v Triangle - coplanar - overlap 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, 1.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -2.f, -1.f), glm::vec3(0.f, -2.f, 1.f));
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - non-coplanar - overlap 1", "Expected collision to be true"});
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - non-coplanar - overlap 2", "Expected collision to be true"});
        }
        {// Collision - off-axis collisions
            auto t1 = Geometry::Triangle(glm::vec3(2.f, 1.f, -1.f), glm::vec3(1.f, -2.f, 1.f), glm::vec3(-1.f, -2.f, 1.f));
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - off-axis - one side collision", "Expected collision to be true"});

            // Like t1 but two sides of triangle cut through control
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, t2), "Triangle v Triangle - off-axis - two side collision", "Expected collision to be true"});

            // Triangle passes under control without collision
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t3), "Triangle v Triangle - off-axis - pass under no collision", "Expected no collision"});
        }
        { // Epsilon tests
            // Place test triangles touching control then move them away by epsilon and check no collision.
            { // Coplanar to control touching edge to edge
                // t1 bottom-right edge touches the control top edge
                auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
                runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected collision to be true"});
                t1.translate({-std::numeric_limits<float>::epsilon() * 2.f, 0.f, 0.f});
                runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected no collision after moving left"});
            }
            {
                // Perpendicular to control (non-coplanar), touching the bottom.
                auto t1 = Geometry::Triangle(glm::vec3(0.f, -1.f, -1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
                runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected collision to be true"});
                t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
                runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected no collision after moving down"});
            }
            {
                // Triangle passes under control touching the bottom side at an angle
                auto t1 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
                runUnitTest({Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under touch", "Expected collision to be true"});
                // Triangle moved below control by epsilon to no longer collide
                t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
                runUnitTest({!Geometry::intersect_triangle_triangle_static(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under epsilon distance", "Expected no collision after moving down"});
            }
        }
        {// Edge cases
            runUnitTest({Geometry::intersect_triangle_triangle_static(control, control), "Triangle v Triangle - equal triangles", "Expected collision to be true"});
        }
    }
} // namespace Test