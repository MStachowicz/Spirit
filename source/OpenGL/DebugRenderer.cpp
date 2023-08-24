#include "DebugRenderer.hpp"
#include "GLState.hpp"

#include "SceneSystem.hpp"
#include "Camera.hpp"

#include "Geometry/Cylinder.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Quad.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

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
        m_debug_line_verts.clear();
        m_debug_VAO->bind();
        m_debug_VBO->clear();
    }

    void DebugRenderer::render(System::SceneSystem& p_scene)
    {
        if (m_debug_verts.empty() && m_debug_line_verts.empty())
            return;

        m_debug_shader->use();
        toggle_cull_face(false); // Disable culling since some geometry will be 2D and facing away from us.
        set_depth_test(true);
        set_depth_test_type(DepthTestType::Less);
        set_polygon_mode(PolygonMode::Fill);

        m_debug_VAO->bind();
        m_debug_VBO = VBO(); // TODO: Dont delete VBO here? Just set_data
        m_debug_VBO->bind();

        if (!m_debug_verts.empty())
        {
            m_debug_VBO->set_data(m_debug_verts);
            draw_arrays(PrimitiveMode::Triangles, 0, static_cast<GLsizei>(m_debug_verts.size()));
        }
        if (!m_debug_line_verts.empty())
        {
            m_debug_VBO->set_data(m_debug_line_verts);
            draw_arrays(PrimitiveMode::Lines, 0, static_cast<GLsizei>(m_debug_line_verts.size()));
        }
    }

    void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Generate the points for the shape and push to relevant _verts container
    }

    void DebugRenderer::add(const Geometry::Frustrum& p_frustrum, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        glm::vec3 near_top_left, near_top_right, near_bottom_left, near_bottom_right, far_top_left, far_top_right, far_bottom_left, far_bottom_right;

        if (Geometry::intersect_plane_plane_plane(p_frustrum.m_near,    p_frustrum.m_top,    p_frustrum.m_left,  near_top_left)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_top,    p_frustrum.m_right, near_top_right)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_left,  near_bottom_left)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_right, near_bottom_right)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_left,  far_top_left)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_right, far_top_right)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_left,  far_bottom_left)
            && Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_right, far_bottom_right))
        { // If possible, collide all the planes to find the quad positions representing the frustrum.
            add(Geometry::Sphere(near_top_left, 0.1f));
            add(Geometry::Sphere(near_top_right, 0.1f));
            add(Geometry::Sphere(near_bottom_left, 0.1f));
            add(Geometry::Sphere(near_bottom_right, 0.1f));
            add(Geometry::Sphere(far_top_left, 0.1f));
            add(Geometry::Sphere(far_top_right, 0.1f));
            add(Geometry::Sphere(far_bottom_left, 0.1f));
            add(Geometry::Sphere(far_bottom_right, 0.1f));

            add(Geometry::Quad(near_top_left, far_top_left, far_bottom_left, near_bottom_left),         glm::vec4(1.f, 0.f, 0.f, p_colour.w));
            add(Geometry::Quad(near_top_right, far_top_right, far_bottom_right, near_bottom_right),     glm::vec4(1.f, 0.f, 0.f, p_colour.w));
            add(Geometry::Quad(near_top_left, near_top_right, far_top_right, far_top_left),             glm::vec4(0.f, 1.f, 0.f, p_colour.w));
            add(Geometry::Quad(near_bottom_left, near_bottom_right, far_bottom_right, far_bottom_left), glm::vec4(0.f, 1.f, 0.f, p_colour.w));
            add(Geometry::Quad(near_top_left, near_top_right, near_bottom_right, near_bottom_left),     glm::vec4(0.f, 0.f, 1.f, p_colour.w));
            add(Geometry::Quad(far_top_left, far_top_right, far_bottom_right, far_bottom_left),         glm::vec4(0.f, 0.f, 1.f, p_colour.w));
        }
        else
        {
            add(p_frustrum.m_left, p_colour);
            add(p_frustrum.m_right, p_colour);
            add(p_frustrum.m_bottom, p_colour);
            add(p_frustrum.m_top, p_colour);
            add(p_frustrum.m_near, p_colour);
            add(p_frustrum.m_far, p_colour);
        }
    }

    void DebugRenderer::add(const Geometry::LineSegment& p_line, const glm::vec4& p_colour/*= glm::vec4(1.f)*/)
    {
        m_debug_line_verts.insert(m_debug_line_verts.end(), {{p_line.m_start, p_colour}, {p_line.m_end, p_colour} });
    }

    void DebugRenderer::add(const Geometry::Plane& p_plane, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Because a Plane is infinite, we represent it as a quad bigger than camera z-far which gives it an infinite appearance.
        auto quad = Geometry::Quad(p_plane);
        quad.scale(Z_Far_Scaler);
        add(quad, p_colour);
    }

    void DebugRenderer::add(const Geometry::Quad& p_quad, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        auto triangles = p_quad.get_triangles(); // Use triangles since OpenGL deprecated Quad primitive rendering.
        m_debug_verts.insert(m_debug_verts.end(), {
            {triangles[0].m_point_1, p_colour},
            {triangles[0].m_point_2, p_colour},
            {triangles[0].m_point_3, p_colour},
            {triangles[1].m_point_1, p_colour},
            {triangles[1].m_point_2, p_colour},
            {triangles[1].m_point_3, p_colour} });
    }

    void DebugRenderer::add(const Geometry::Ray& p_ray, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        // Because a Ray extends infinitely, we represent it as a Line extending beyond camera z-far which gives it an infinite appearance.
        add(Geometry::LineSegment(p_ray.m_start, p_ray.m_start + p_ray.m_direction * Z_Far_Scaler), p_colour);
    }

    void DebugRenderer::add(const Geometry::Sphere& p_sphere, const glm::vec4& p_colour/*= glm::vec3(1.f)*/)
    {
        append_sphere_verts(p_sphere.mCenter, p_sphere.mRadius, 1, p_colour); // 1 subdivision for speed.
    }

    void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour)
    {
        m_debug_verts.insert(m_debug_verts.end(), {
            {p_triangle.m_point_1, p_colour},
            {p_triangle.m_point_2, p_colour},
            {p_triangle.m_point_3, p_colour} });
    }

    void DebugRenderer::append_sphere_verts(const glm::vec3& p_point, float p_radius, uint8_t p_subdivisions/* = 0*/, const glm::vec4& p_colour/*= glm::vec4(1.f)*/)
    {
        constexpr auto standard_points = Geometry::get_icosahedron_points();
        auto points = std::vector<glm::vec3>(standard_points.begin(), standard_points.end());

        for (auto i = 0; i < p_subdivisions; i++)
        {
            std::vector<glm::vec3> new_points;
            new_points.reserve(points.size() * 4);

            for (auto i = 0; i < points.size(); i += 3) // Every triangle
            {
                const auto t = Geometry::Triangle(points[i], points[i + 1], points[i + 2]);
                const auto new_triangles = t.subdivide();

                new_points.emplace_back(new_triangles[0].m_point_1);
                new_points.emplace_back(new_triangles[0].m_point_2);
                new_points.emplace_back(new_triangles[0].m_point_3);
                new_points.emplace_back(new_triangles[1].m_point_1);
                new_points.emplace_back(new_triangles[1].m_point_2);
                new_points.emplace_back(new_triangles[1].m_point_3);
                new_points.emplace_back(new_triangles[2].m_point_1);
                new_points.emplace_back(new_triangles[2].m_point_2);
                new_points.emplace_back(new_triangles[2].m_point_3);
                new_points.emplace_back(new_triangles[3].m_point_1);
                new_points.emplace_back(new_triangles[3].m_point_2);
                new_points.emplace_back(new_triangles[3].m_point_3);
            }
            points = std::move(new_points);
        }

        m_debug_verts.reserve(m_debug_verts.size() + points.size());
        for (auto i = 0; i < points.size(); i += 3) // Every triangle
        {
            m_debug_verts.insert(m_debug_verts.end(),
            {
                {((points[i]     / glm::length(points[i]    )) * p_radius) + p_point, p_colour},
                {((points[i + 1] / glm::length(points[i + 1])) * p_radius) + p_point, p_colour},
                {((points[i + 2] / glm::length(points[i + 2])) * p_radius) + p_point, p_colour}
            });
        }
    }
}