#include "GridRenderer.hpp"
#include "OpenGL/DrawCall.hpp"

#include "Utility/MeshBuilder.hpp"

namespace OpenGL
{
	Data::NewMesh GridRenderer::make_grid_mesh()
	{
		constexpr int Size                        = 1000; // Used for the size and number of lines to draw.
		constexpr float transparency              = 0.7f;
		constexpr glm::vec4 primary_line_colour   = glm::vec4{0.5f, 0.5f, 0.5f, transparency};
		constexpr glm::vec4 secondary_line_colour = glm::vec4{0.2f, 0.2f, 0.2f, transparency};
		constexpr glm::vec4 red                   = glm::vec4{1.f, 0.f, 0.f, transparency};
		constexpr glm::vec4 green                 = glm::vec4{0.f, 1.f, 0.f, transparency};
		constexpr glm::vec4 blue                  = glm::vec4{0.f, 0.f, 1.f, transparency};

		auto mb = Utility::MeshBuilder(PrimitiveMode::Lines);
		mb.reserve((Size * 2 * 8) + 6);

		{ // Cardinal axis lines
			mb.set_colour(red);
			mb.add_line(glm::vec3{-Size, 0.f, 0.f}, glm::vec3{Size, 0.f, 0.f});
			mb.set_colour(green);
			mb.add_line(glm::vec3{0.f, -Size, 0.f}, glm::vec3{0.f, Size, 0.f});
			mb.set_colour(blue);
			mb.add_line(glm::vec3{0.f, 0.f, -Size},  glm::vec3{0.f, 0.f, Size});
		}

		// XZ-plane lines
		for (auto i = -Size; i <= Size; i++)
		{
			if (i % 10 == 0)
			{
				mb.set_colour(primary_line_colour);
				mb.add_line(glm::vec3{-Size, 0.f, i}, glm::vec3{Size, 0.f, i});
				mb.add_line(glm::vec3{i, 0.f, Size},  glm::vec3{i, 0.f, -Size});
			}
			else if (i != 0) // Ignore 0, cardinal axis lines are added above.
			{
				mb.set_colour(secondary_line_colour);
				mb.add_line(glm::vec3{-Size, 0.f, i}, glm::vec3{Size, 0.f, i});
				mb.add_line(glm::vec3{i, 0.f, Size},  glm::vec3{i, 0.f, -Size});
			}
		}

		return mb.get_mesh();
	}

	GridRenderer::GridRenderer() noexcept
		: m_grid_shader{"grid"}
		, m_grid_mesh{make_grid_mesh()}
	{}

	void GridRenderer::draw()
	{
		DrawCall dc;
		dc.m_cull_face_enabled = false;
		dc.submit(m_grid_shader, m_grid_mesh);
	}
}