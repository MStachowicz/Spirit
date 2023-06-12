#include "GridRenderer.hpp"

namespace OpenGL
{
    GridRenderer::GridRenderer() noexcept
        : m_line_points{}
        , m_colour{1.f, 1.f, 1.f, 0.7f}
        , m_grid_shader{"grid"}
        , m_line_VAO{}
        , m_line_VBO{}
    {
        constexpr int Count = 1000;
        constexpr int Size  = 100; // #Todo Should be camera z-far
        m_line_points.reserve(Count * 8);

        // XZ-plane
        for (auto i = -Count; i <= Count; i++)
        {
            m_line_points.push_back({glm::vec3{-Size, 0.f, i}});
            m_line_points.push_back({glm::vec3{Size, 0.f, i}});
            m_line_points.push_back({glm::vec3{i, 0.f, Size}});
            m_line_points.push_back({glm::vec3{i, 0.f, -Size}});
        }

        m_line_VAO.bind();
        m_line_VBO.set_data(m_line_points);
    }
    void GridRenderer::draw()
    {
        if (!m_line_points.empty())
        {
            m_grid_shader.use();
            m_grid_shader.set_uniform("colour", m_colour);
            toggle_cull_face(false);
            set_depth_test(true);
            set_depth_test_type(DepthTestType::Less);
            set_polygon_mode(PolygonMode::Fill);

            m_line_VAO.bind();
            m_line_VBO.bind();
            draw_arrays(PrimitiveMode::Lines, 0, static_cast<GLsizei>(m_line_points.size() * 2));
        }
    }
}