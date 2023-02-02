#include "GeometryTester.hpp"

// GEOMETRY
#include "Intersect.hpp"
#include "AABB.hpp"
#include "Triangle.hpp"

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Test
{
    void GeometryTester::runAllTests()
    {
        runAABBTests();
        runTriangleTests();
    }

    void GeometryTester::runAABBTests()
    {
        { // Default intiailise
            Geometry::AABB aabb;
            runTest({aabb.getSize() == glm::vec3(0.f), "AABB initialise size at 0", "Expected size of default AABB to be 0"});
            runTest({aabb.getCenter () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
            runTest({aabb.getCenter () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
        }
        { // Initialise with a min and max
            // An AABB at low point [-1,-1,-1] to [1,1,1]
            auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
            runTest({aabb.getSize() == glm::vec3(2.f), "AABB initialised with min and max size at 2", "Expected size of AABB to be 2"});
            runTest({aabb.getCenter() == glm::vec3(0.f), "AABB initialise with min and max position", "Expected AABB to center at [0, 0, 0]"});
        }
        { // Initialise with a min and max not at origin
            // An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
            auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
            runTest({aabb.getSize() == glm::vec3(4.f), "AABB initialised with min and max not at origin", "Expected size of AABB to be 4.f"});
            runTest({aabb.getCenter() == glm::vec3(3.f), "AABB initialised with min and max not at origin", "Expected AABB to center at [3, 3, 3]"});
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

        {// Collision = true / Coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 3.f, 0.f), glm::vec3(2.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(-2.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-3.f, -1.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(2.f, 1.f, 0.f), glm::vec3(3.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, -1.f, 0.f), glm::vec3(0.f, -3.f, 0.f), glm::vec3(-2.f, -3.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(1.f, -1.f, 0.f), glm::vec3(2.f, -3.f, 0.f), glm::vec3(0.f, -3.f, 0.f));

            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - Coplanar - edge-edge 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - Coplanar - edge-edge 2", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - Coplanar - edge-edge 3", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - Coplanar - edge-edge 4", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t5), "Triangle v Triangle - Coplanar - edge-edge 5", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t6), "Triangle v Triangle - Coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / edge-edge
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 3.f, -1.f), glm::vec3(0.f, 1.f, -2.f), glm::vec3(0.f, 1.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -2.f), glm::vec3(1.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, -2.f), glm::vec3(-1.f, -1.f, 0.f));

            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - Non-coplanar - edge-edge 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - Non-coplanar - edge-edge 2", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - Non-coplanar - edge-edge 3", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - Non-coplanar - edge-edge 4", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t5), "Triangle v Triangle - Non-coplanar - edge-edge 5", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t6), "Triangle v Triangle - Non-coplanar - edge-edge 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, 1.f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.5, 2.f, 0.f), glm::vec3(2.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.5f, 0.f, 0.f), glm::vec3(2.5f, -2.f, 0.f), glm::vec3(0.5f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, -3.f, 0.f), glm::vec3(-1.f, -3.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.5f, 0.f, 0.f), glm::vec3(-0.5f, -2.f, 0.f), glm::vec3(-2.5f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.5f, 2.f, 0.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-2.5f, 0.f, 0.f));

            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - Coplanar - edge-side 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - Coplanar - edge-side 2", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - Coplanar - edge-side 3", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - Coplanar - edge-side 4", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t5), "Triangle v Triangle - Coplanar - edge-side 5", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t6), "Triangle v Triangle - Coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / Non-coplanar / edge-side
            auto t1 = Geometry::Triangle(glm::vec3(0.5f, 2.f, 1.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 2.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.5f, 2.f, -1.f), glm::vec3(0.5f, 0.f, -2.f), glm::vec3(0.5f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 2.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(0.f, -1.f, -2.f), glm::vec3(0.f, -1.f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, 1.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-0.5f, 0.f, 2.f));
            auto t6 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, -1.f), glm::vec3(-0.5f, 0.f, -2.f), glm::vec3(-0.5f, 0.f, 0.f));

            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - Non-coplanar - edge-side 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - Non-coplanar - edge-side 2", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - Non-coplanar - edge-side 3", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - Non-coplanar - edge-side 4", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t5), "Triangle v Triangle - Non-coplanar - edge-side 5", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t6), "Triangle v Triangle - Non-coplanar - edge-side 6", "Expected collision to be true"});
        }
        {// Collision = true / coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::vec3(-1.f, 0.5f, 0.f));
            auto t2 = Geometry::Triangle(glm::vec3(1.f, 2.f, 0.f), glm::vec3(2.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
            auto t3 = Geometry::Triangle(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, -2.f, 0.f), glm::vec3(0.f, -2.f, 0.f));
            auto t4 = Geometry::Triangle(glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, -2.5f, 0.f), glm::vec3(-1.f, -2.5f, 0.f));
            auto t5 = Geometry::Triangle(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -2.f, 0.f), glm::vec3(-2.f, -2.f, 0.f));
            auto t6 = Geometry::Triangle(glm::vec3(-1.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(-2.f, 0.f, 0.f));

            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - coplanar - overlap 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - coplanar - overlap 2", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - coplanar - overlap 3", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - coplanar - overlap 4", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t5), "Triangle v Triangle - coplanar - overlap 5", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t6), "Triangle v Triangle - coplanar - overlap 6", "Expected collision to be true"});
        }
        {// Collision = true / non-coplanar / overlap
            auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, 1.f));
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -2.f, -1.f), glm::vec3(0.f, -2.f, 1.f));
            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - non-coplanar - overlap 1", "Expected collision to be true"});
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - non-coplanar - overlap 2", "Expected collision to be true"});
        }
        {// Collision - off-axis collisions
            auto t1 = Geometry::Triangle(glm::vec3(2.f, 1.f, -1.f), glm::vec3(1.f, -2.f, 1.f), glm::vec3(-1.f, -2.f, 1.f));
            runTest({Geometry::IntersectTriangleTriangle(control, t1), "Triangle v Triangle - off-axis - one side collision", "Expected collision to be true"});

            // Like t1 but two sides of triangle cut through control
            auto t2 = Geometry::Triangle(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runTest({Geometry::IntersectTriangleTriangle(control, t2), "Triangle v Triangle - off-axis - two side collision", "Expected collision to be true"});

            // Triangle passes under control without collision
            auto t3 = Geometry::Triangle(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runTest({!Geometry::IntersectTriangleTriangle(control, t3), "Triangle v Triangle - off-axis - pass under no collision", "Expected no collision"});

            // Triangle passes under control touching the bottom side
            auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
            runTest({Geometry::IntersectTriangleTriangle(control, t4), "Triangle v Triangle - off-axis - pass under touch", "Expected collision to be true"});
        }
        {// Edge cases
            runTest({Geometry::IntersectTriangleTriangle(control, control), "Triangle v Triangle - equal triangles", "Expected collision to be true"});
        }
    }
} // namespace Test