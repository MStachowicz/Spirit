#include "DebugRenderer.hpp"
#include "GLState.hpp"

#include "SceneSystem.hpp"
#include "Camera.hpp"

// GEOMETRY
#include "Cylinder.hpp"
#include "Plane.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" // perspective, translate, rotate

namespace OpenGL
{
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
        auto camera = p_scene.getPrimaryCamera();

        m_debug_shader->use();
        toggle_cull_face(false); // Disable culling since some geometry will be 2D and facing away from us.
        set_depth_test_type(DepthTestType::Always);

        if (!m_debug_verts.empty())
        {
            m_debug_VAO->bind();
            m_debug_VBO = VBO(); // TODO: Dont delete VBO here? Just set_data
            m_debug_VBO->bind();
            m_debug_VBO->set_data(m_debug_verts);
            draw_arrays(PrimitiveMode::Triangles, 0, static_cast<GLsizei>(m_debug_verts.size()));
        }
    }

    void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec3& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }
    void DebugRenderer::add(const Geometry::Plane& p_plane, const glm::vec3& p_colour/*= glm::vec3(1.f)*/)
    {
        //auto plane_verts = p_plane.get_vertices();
        //m_debug_verts.insert(m_debug_verts.end(), {
        //    { plane_verts[0], p_colour},
        //    { plane_verts[1], p_colour},
        //    { plane_verts[2], p_colour},
        //    { plane_verts[3], p_colour} });
    }
    void DebugRenderer::add(const Geometry::Ray& p_ray, const glm::vec3& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }
    void DebugRenderer::add(const Geometry::Sphere& p_sphere, const glm::vec3& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }

    void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec3& p_colour)
    {
        m_debug_verts.insert(m_debug_verts.end(), {
            {p_triangle.mPoint1, p_colour},
            {p_triangle.mPoint2, p_colour},
            {p_triangle.mPoint3, p_colour} });
    }
}