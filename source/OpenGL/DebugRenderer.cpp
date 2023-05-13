#include "DebugRenderer.hpp"
#include "GLState.hpp"

#include "SceneSystem.hpp"
#include "Camera.hpp"

// GEOMETRY
#include "Cylinder.hpp"
#include "Plane.hpp"
#include "Quad.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" // perspective, translate, rotate

namespace OpenGL
{
    static constexpr float Z_Far_Scaler = 1000.f; // Scale the geometry that extends infinitely by this to give it an infinite appearance.

    void DebugRenderer::init()
    {
        m_debug_VAO = VAO();
        m_debug_VBO = VBO();
        m_debug_shader = {"DebugRender"};
    }

    void DebugRenderer::deinit()
    {
        m_debug_VAO.reset();
        m_debug_VBO.reset();
        m_debug_shader.reset();
    }

    void DebugRenderer::clear()
    {
        m_debug_verts.clear();
        m_debug_VAO->bind();
        m_debug_VBO->clear();
    }

    void DebugRenderer::render(System::SceneSystem& p_scene)
    {
        if (!m_debug_verts.empty())
        {
            m_debug_shader->use();
            toggle_cull_face(false); // Disable culling since some geometry will be 2D and facing away from us.
            set_depth_test_type(DepthTestType::Less);
            set_polygon_mode(PolygonMode::Fill);

            m_debug_VAO->bind();
            m_debug_VBO = VBO(); // TODO: Dont delete VBO here? Just set_data
            m_debug_VBO->bind();
            m_debug_VBO->set_data(m_debug_verts);
            draw_arrays(PrimitiveMode::Triangles, 0, static_cast<GLsizei>(m_debug_verts.size()));
        }
    }

    void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }
    void DebugRenderer::add(const Geometry::Plane& p_plane, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Because a Plane is infinite, we represent it as a quad bigger than camera z-far which gives it an infinite appearance.
        auto quad = Geometry::Quad(p_plane);
        quad.scale(Z_Far_Scaler);
        auto triangles = quad.get_triangles(); // Use triangles since OpenGL deprecated Quad primitive rendering.

        m_debug_verts.insert(m_debug_verts.end(), {
            {triangles[0].m_point_1, p_colour},
            {triangles[0].m_point_2, p_colour},
            {triangles[0].m_point_3, p_colour},
            {triangles[1].m_point_1, p_colour},
            {triangles[1].m_point_2, p_colour},
            {triangles[1].m_point_3, p_colour}, });
    }
    void DebugRenderer::add(const Geometry::Ray& p_ray, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }
    void DebugRenderer::add(const Geometry::Sphere& p_sphere, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }

    void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour)
    {
        m_debug_verts.insert(m_debug_verts.end(), {
            {p_triangle.m_point_1, p_colour},
            {p_triangle.m_point_2, p_colour},
            {p_triangle.m_point_3, p_colour} });
    }
}