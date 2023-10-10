#include "OpenGL/Types.hpp"
#include "OpenGL/Shader.hpp"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <vector>

namespace OpenGL
{
    class GridRenderer
    {
        struct GridVert
        {
            glm::vec3 m_position = glm::vec3{0.f};
            glm::vec4 m_colour   = glm::vec4{1.f};
        };

        std::vector<GridVert> m_line_points;
        Shader m_grid_shader;
        VAO m_line_VAO;
        VBO m_line_VBO;

    public:
        GridRenderer() noexcept;
        void draw();
    };
}