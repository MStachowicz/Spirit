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

    }
    void GeometryTester::runTriangleTests()
    {
    }
} // namespace Test