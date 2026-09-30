#include "GridRenderer.hpp"
#include "DrawCall.hpp"
#include "DebugRenderer.hpp"

#include "Component/ViewInformation.hpp"
#include "Utility/MeshBuilder.hpp"

#include <cmath>

namespace OpenGL
{
	Data::Mesh GridRenderer::make_screen_triangle()
	{
		auto mb = Utility::MeshBuilder<Data::ColourVertex, PrimitiveMode::Triangles>{};
		mb.add_triangle(glm::vec3{-1.f, -1.f, 0.f}, glm::vec3{3.f, -1.f, 0.f}, glm::vec3{-1.f, 3.f, 0.f});
		return mb.get_mesh();
	}
	Data::Mesh GridRenderer::make_origin_arrows_mesh()
	{
		auto mb = Utility::MeshBuilder<Data::ColourVertex, PrimitiveMode::Triangles>{};
		mb.set_colour(glm::vec3(1.f, 0.f, 0.f));
		mb.add_arrow(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f));
		mb.set_colour(glm::vec3(0.f, 1.f, 0.f));
		mb.add_arrow(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		mb.set_colour(glm::vec3(0.f, 0.f, 1.f));
		mb.add_arrow(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f));
		mb.set_colour(glm::vec4(1.f));
		mb.add_icosphere(glm::vec3(0.f), 0.1f, 1);
		return mb.get_mesh();
	}

	GridRenderer::GridRenderer() noexcept
		: m_grid_shader{"grid"}
		, m_origin_shader{"colour"}
		, m_screen_triangle{make_screen_triangle()}
		, m_origin_arrows{make_origin_arrows_mesh()}
	{}

	void GridRenderer::draw(const FBO& p_target_FBO, const Component::ViewInformation& p_view_info, const Buffer& p_view_properties)
	{
		const auto view_projection = p_view_info.m_projection * glm::mat4(glm::mat3(p_view_info.m_view));
		const auto inverse_view_projection = glm::inverse(view_projection);
		const float orthographic_extent = p_view_info.m_projection[3][3] != 0.f
			? glm::max(1.f / std::abs(p_view_info.m_projection[0][0]), 1.f / std::abs(p_view_info.m_projection[1][1]))
			: 0.f;
		const float grid_scale = glm::max(0.001f, glm::max(std::abs(p_view_info.m_view_position.y), orthographic_extent));

		{
			DrawCall dc;
			dc.m_cull_face_enabled     = false;
			dc.m_depth_test_type       = DepthTestType::Less;
			dc.m_depth_test_enabled    = true;
			dc.m_write_to_depth_buffer = false;
			dc.m_blending_enabled      = true;
			dc.set_uniform("viewProjection", view_projection);
			dc.set_uniform("invViewProj", inverse_view_projection);
			dc.set_uniform("cameraPosition", glm::vec3(p_view_info.m_view_position));
			dc.set_uniform("fadeDistance", grid_scale * 100.f);
			dc.submit(m_grid_shader, m_screen_triangle.get_VAO(), p_target_FBO);
		}
		if (OpenGL::DebugRenderer::m_debug_options.m_show_origin_arrows)
		{
			DrawCall dc;
			dc.m_cull_face_enabled     = false;
			dc.m_depth_test_type       = DepthTestType::Less;
			dc.m_depth_test_enabled    = true;
			dc.m_write_to_depth_buffer = true;
			dc.set_UBO("ViewProperties", p_view_properties);
			dc.set_uniform("model", glm::identity<glm::mat4>());
			dc.submit(m_origin_shader, m_origin_arrows.get_VAO(), p_target_FBO);
		}
	}
	void GridRenderer::reload_shaders()
	{
		m_grid_shader.reload();
		m_origin_shader.reload();
	}
} // namespace OpenGL