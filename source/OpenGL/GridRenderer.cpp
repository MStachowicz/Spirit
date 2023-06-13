#include "GridRenderer.hpp"

namespace OpenGL
{
    GridRenderer::GridRenderer() noexcept
        : m_line_points{}
        , m_grid_shader{"grid"}
        , m_line_VAO{}
        , m_line_VBO{}
    {
        constexpr int Size                        = 1000; // Used for the size and number of lines to draw.
        constexpr float transparency              = 0.7f;
        constexpr glm::vec4 primary_line_colour   = glm::vec4{0.5f, 0.5f, 0.5f, transparency};
        constexpr glm::vec4 secondary_line_colour = glm::vec4{0.2f, 0.2f, 0.2f, transparency};
        constexpr glm::vec4 red                   = glm::vec4{1.f, 0.f, 0.f, transparency};
        constexpr glm::vec4 green                 = glm::vec4{0.f, 1.f, 0.f, transparency};
        constexpr glm::vec4 blue                  = glm::vec4{0.f, 0.f, 1.f, transparency};

        m_line_points.reserve((Size * 8) + 6);

        { // Cardinal axis lines
            m_line_points.push_back({glm::vec3{-Size, 0.f, 0.f}, red});
            m_line_points.push_back({glm::vec3{Size, 0.f, 0.f},  red});
            m_line_points.push_back({glm::vec3{0.f, -Size, 0.f}, green});
            m_line_points.push_back({glm::vec3{0.f, Size, 0.f},  green});
            m_line_points.push_back({glm::vec3{0.f, 0.f, -Size}, blue});
            m_line_points.push_back({glm::vec3{0.f, 0.f, Size},  blue});
        }

        // XZ-plane lines
        for (auto i = -Size; i <= Size; i++)
        {
            if (i % 10 == 0)
            {
                m_line_points.push_back({glm::vec3{-Size, 0.f, i}, primary_line_colour});
                m_line_points.push_back({glm::vec3{Size, 0.f, i }, primary_line_colour});
                m_line_points.push_back({glm::vec3{i, 0.f, Size }, primary_line_colour});
                m_line_points.push_back({glm::vec3{i, 0.f, -Size}, primary_line_colour});
            }
            else if (i != 0) // Ignore 0, cardinal axis lines are added above.
            {
                m_line_points.push_back({glm::vec3{-Size, 0.f, i}, secondary_line_colour});
                m_line_points.push_back({glm::vec3{Size, 0.f, i }, secondary_line_colour});
                m_line_points.push_back({glm::vec3{i, 0.f, Size }, secondary_line_colour});
                m_line_points.push_back({glm::vec3{i, 0.f, -Size}, secondary_line_colour});
            }
        }

        m_line_VAO.bind();
        m_line_VBO.set_data(m_line_points);
    }
    void GridRenderer::draw()
    {
        if (!m_line_points.empty())
        {
            m_grid_shader.use();
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