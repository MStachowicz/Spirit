#include "DrawCall.hpp"
#include "Shader.hpp"

#include "Component/Mesh.hpp"
#include "Component/Texture.hpp"

namespace OpenGL
{
	DrawCall::DrawCall() noexcept
		: m_depth_test_enabled{true}
		, m_depth_test_type{DepthTestType::Less}
		, m_polygon_offset_enabled{false}
		, m_polygon_offset_factor{0.f}
		, m_polygon_offset_units{0.f}
		, m_blending_enabled{true}
		, m_source_factor{BlendFactorType::SourceAlpha}
		, m_destination_factor{BlendFactorType::OneMinusSourceAlpha}
		, m_cull_face_enabled{true}
		, m_cull_face_type{CullFaceType::Back}
		, m_front_face_orientation{FrontFaceOrientation::CounterClockwise}
		, m_polygon_mode{PolygonMode::Fill}
	{}

	void DrawCall::set_texture(const std::string_view& p_name, const TextureRef& p_texture)
	{
		if (m_texture_count == max_textures)
			throw std::logic_error{"Too many textures set for this drawcall. Up the max_textures variable!"};
		if (p_name.size() >= max_texture_name_len)
			throw std::logic_error("Texture name is too long! Up the max_texture_name_len variable!");
		if (!p_texture)
			throw std::logic_error("Texture is null!");
		if (std::find_if(m_textures.begin(), m_textures.end(), [&p_name](const auto& p_texture) { return p_texture.m_name == p_name; }) != m_textures.end())
			throw std::logic_error{"Texture already set for this drawcall!"};

		m_textures[m_texture_count].m_texture = p_texture;
		p_name.copy(m_textures[m_texture_count].m_name, p_name.size());
		++m_texture_count;
	}

	void DrawCall::submit(Shader& p_shader, Data::Mesh& p_mesh) const
	{
		OpenGL::set_depth_test(m_depth_test_enabled);
		OpenGL::set_depth_test_type(m_depth_test_type);
		OpenGL::set_polygon_offset(m_polygon_offset_enabled);
		if (m_polygon_offset_enabled)
			OpenGL::set_polygon_offset_factor(m_polygon_offset_factor, m_polygon_offset_units);
		OpenGL::set_blending(m_blending_enabled);
		if (m_blending_enabled)
			OpenGL::set_blend_func(m_source_factor, m_destination_factor);
		OpenGL::set_cull_face(m_cull_face_enabled);
		if (m_cull_face_enabled)
			OpenGL::set_cull_face_type(m_cull_face_type);
		OpenGL::set_front_face_orientation(m_front_face_orientation);
		OpenGL::set_polygon_mode(m_polygon_mode);

		p_shader.use();

		for (auto i = 0; i < m_uniform_count; ++i)
			std::visit([&](auto&& arg) { p_shader.set_uniform(m_uniforms[i].m_name, arg); }, m_uniforms[i].m_data);

		for (auto i = 0; i < m_texture_count; ++i)
		{
			p_shader.set_uniform(m_textures[i].m_name, i); // TODO only needs to happen once per shader
			OpenGL::active_texture(i);
			m_textures[i].m_texture->m_GL_texture.bind();
		}

		p_mesh.draw();
	}

	void DrawCall::submit(Shader& p_shader, Data::Mesh& p_mesh, GLsizei p_instanced_count) const
	{
		OpenGL::set_depth_test(m_depth_test_enabled);
		OpenGL::set_depth_test_type(m_depth_test_type);
		OpenGL::set_polygon_offset(m_polygon_offset_enabled);
		if (m_polygon_offset_enabled)
			OpenGL::set_polygon_offset_factor(m_polygon_offset_factor, m_polygon_offset_units);
		OpenGL::set_blending(m_blending_enabled);
		if (m_blending_enabled)
			OpenGL::set_blend_func(m_source_factor, m_destination_factor);
		OpenGL::set_cull_face(m_cull_face_enabled);
		if (m_cull_face_enabled)
			OpenGL::set_cull_face_type(m_cull_face_type);
		OpenGL::set_front_face_orientation(m_front_face_orientation);
		OpenGL::set_polygon_mode(m_polygon_mode);

		p_shader.use();

		for (auto i = 0; i < m_uniform_count; ++i)
			std::visit([&](auto&& arg) { p_shader.set_uniform(m_uniforms[i].m_name, arg); }, m_uniforms[i].m_data);

		for (auto i = 0; i < m_texture_count; ++i)
		{
			p_shader.set_uniform(m_textures[i].m_name, i); // TODO only needs to happen once per shader
			OpenGL::active_texture(i);
			m_textures[i].m_texture->m_GL_texture.bind();
		}

		p_mesh.draw_instanced(p_instanced_count);
	}
} // namespace OpenGL