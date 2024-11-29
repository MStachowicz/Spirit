#include "DrawCall.hpp"
#include "Shader.hpp"
#include "Types.hpp"

namespace OpenGL
{
	DrawCall::DrawCall() noexcept
		: m_uniforms{}
		, m_uniform_count{0}
		, m_textures{}
		, m_texture_count{0}
		, m_SSBOs{}
		, m_SSBO_count{0}
		, m_UBOs{}
		, m_UBO_count{0}
		, m_write_to_depth_buffer{true}
		, m_depth_test_enabled{true}
		, m_depth_test_type{DepthTestType::Less}
		, m_polygon_offset_enabled{false}
		, m_polygon_offset_factor{0.f}
		, m_polygon_offset_units{0.f}
		, m_blending_enabled{false}
		, m_source_factor{BlendFactorType::SourceAlpha}
		, m_destination_factor{BlendFactorType::OneMinusSourceAlpha}
		, m_cull_face_enabled{true}
		, m_cull_face_type{CullFaceType::Back}
		, m_front_face_orientation{FrontFaceOrientation::CounterClockwise}
		, m_polygon_mode{PolygonMode::Fill}
	{}

	void DrawCall::set_texture(const std::string_view& p_identifier, const Texture& p_texture)
	{
		if (m_texture_count == max_textures)
			throw std::logic_error{"Too many textures set for this drawcall. Up the max_textures variable!"};
		if (p_identifier.size() >= max_texture_identifier_len)
			throw std::logic_error("Texture name is too long! Up the max_texture_identifier_len variable!");
		if (std::find_if(m_textures.begin(), m_textures.end(), [&p_identifier](const auto& p_texture) { return p_texture.m_identifier == p_identifier; }) != m_textures.end())
			throw std::logic_error{"Texture already set for this drawcall!"};

		m_textures[m_texture_count].m_handle = p_texture.m_handle;
		p_identifier.copy(m_textures[m_texture_count].m_identifier, p_identifier.size());
		++m_texture_count;
	}
	void DrawCall::set_SSBO(const std::string_view& p_identifier, const Buffer& p_SSBO)
	{
		if (m_SSBO_count == max_SSBOs)
			throw std::logic_error{"Too many SSBOs set for this drawcall. Up the max_SSBOs variable!"};
		if (p_identifier.size() >= max_SSBO_identifier_len)
			throw std::logic_error("SSBO name is too long! Up the max_SSBO_identifier_len variable!");
		if (std::find_if(m_SSBOs.begin(), m_SSBOs.end(), [&p_identifier](const auto& p_SSBO) { return p_SSBO.m_identifier == p_identifier; }) != m_SSBOs.end())
			throw std::logic_error{"SSBO already set for this drawcall!"};

		p_identifier.copy(m_SSBOs[m_SSBO_count].m_identifier, p_identifier.size());
		m_SSBOs[m_SSBO_count].m_handle = p_SSBO.m_handle;
		// We bind the entire buffer to the SSBO binding point.
		m_SSBOs[m_SSBO_count].m_offset = 0;
		m_SSBOs[m_SSBO_count].m_size   = p_SSBO.m_used_capacity;

		++m_SSBO_count;
	}
	void DrawCall::set_UBO(const std::string_view& p_identifier, const Buffer& p_UBO)
	{
		if (m_UBO_count == max_UBOs)
			throw std::logic_error{"Too many UBOs set for this drawcall. Up the max_UBOs variable!"};
		if (p_identifier.size() >= max_UBO_identifier_len)
			throw std::logic_error("UBO name is too long! Up the max_UBO_identifier_len variable!");
		if (std::find_if(m_UBOs.begin(), m_UBOs.end(), [&p_identifier](const auto& p_UBO) { return p_UBO.m_identifier == p_identifier; }) != m_UBOs.end())
			throw std::logic_error{"UBO already set for this drawcall!"};

		p_identifier.copy(m_UBOs[m_UBO_count].m_identifier, p_identifier.size());
		m_UBOs[m_UBO_count].m_handle = p_UBO.m_handle;
		// We bind the entire buffer to the UBO binding point.
		m_UBOs[m_UBO_count].m_offset = 0;
		m_UBOs[m_UBO_count].m_size   = p_UBO.m_used_capacity;
		++m_UBO_count;
	}

	void DrawCall::pre_draw_call(Shader& p_shader, const VAO& p_VAO, const GLHandle& p_FBO_handle, const glm::uvec2& p_resolution) const
	{
		State::Get().bind_FBO(p_FBO_handle);
		State::Get().set_viewport(0, 0, p_resolution.x, p_resolution.y);

		State::Get().set_depth_write(m_write_to_depth_buffer);
		State::Get().set_depth_test(m_depth_test_enabled);
		State::Get().set_depth_test_type(m_depth_test_type);

		State::Get().set_polygon_offset(m_polygon_offset_enabled);
		if (m_polygon_offset_enabled)
			State::Get().set_polygon_offset_factor(m_polygon_offset_factor, m_polygon_offset_units);

		State::Get().set_blending(m_blending_enabled);
		if (m_blending_enabled)
			State::Get().set_blend_func(m_source_factor, m_destination_factor);

		State::Get().set_cull_face(m_cull_face_enabled);
		if (m_cull_face_enabled)
			State::Get().set_cull_face_type(m_cull_face_type);

		State::Get().set_front_face_orientation(m_front_face_orientation);
		State::Get().set_polygon_mode(m_polygon_mode);

		State::Get().use_program(p_shader.m_handle);
		State::Get().bind_VAO(p_VAO.m_handle);

		for (GLuint i = 0; i < m_uniform_count; ++i)
			std::visit([&](auto&& arg) { p_shader.set_uniform(m_uniforms[i].m_identifier, arg); }, m_uniforms[i].m_data);

		for (GLuint i = 0; i < m_texture_count; ++i)
		{
			// Set uniform sampler2D to the texture unit index.
			// Then bind the texture to the same texture unit.
			p_shader.bind_sampler_2D(m_textures[i].m_identifier, i);
			State::Get().bind_texture_unit(i, m_textures[i].m_handle);
		}
		for (GLuint i = 0; i < m_SSBO_count; ++i)
		{
			// Bind the Shader storage block of the shader and the SSBO to the same binding point.
			p_shader.bind_shader_storage_block(m_SSBOs[i].m_identifier, i);
			State::Get().bind_shader_storage_buffer(i, m_SSBOs[i].m_handle, m_SSBOs[i].m_offset, m_SSBOs[i].m_size);
		}
		for (GLuint i = 0; i < m_UBO_count; ++i)
		{
			// Bind the Uniform block of the shader and the UBO to the same binding point.
			p_shader.bind_uniform_block(m_UBOs[i].m_identifier, i);
			State::Get().bind_uniform_buffer(i, m_UBOs[i].m_handle, m_UBOs[i].m_offset, m_UBOs[i].m_size);
		}
	}

	void DrawCall::submit(Shader& p_shader, const VAO& p_VAO, const FBO& p_FBO) const
	{
		ASSERT(p_FBO.m_handle != 0, "Submitting a draw call with an FBO that has not been initialised.");
		ASSERT(p_FBO.is_complete(), "Submitting a draw call with an incomplete FBO.");
		ASSERT(p_VAO.draw_count() > 0, "Submitting a draw call with no vertices to draw.");

		pre_draw_call(p_shader, p_VAO, p_FBO.m_handle, p_FBO.m_resolution);

		if (p_VAO.is_indexed())
			draw_elements(p_VAO.draw_primitive_mode(), p_VAO.draw_count());
		else
			draw_arrays(p_VAO.draw_primitive_mode(), 0, p_VAO.draw_count());
	}
	void DrawCall::submit_instanced(Shader& p_shader, const VAO& p_VAO, const FBO& p_FBO, GLsizei p_instanced_count) const
	{
		pre_draw_call(p_shader, p_VAO, p_FBO.m_handle, p_FBO.m_resolution);

		if (p_VAO.is_indexed())
			draw_elements_instanced(p_VAO.draw_primitive_mode(), p_VAO.draw_count(), p_instanced_count);
		else
			draw_arrays_instanced(p_VAO.draw_primitive_mode(), 0, p_VAO.draw_count(), p_instanced_count);
	}
	void DrawCall::submit_default(Shader& p_shader, const VAO& p_VAO, const glm::uvec2& p_resolution) const
	{
		pre_draw_call(p_shader, p_VAO, 0, p_resolution);

		if (p_VAO.is_indexed())
			draw_elements(p_VAO.draw_primitive_mode(), p_VAO.draw_count());
		else
			draw_arrays(p_VAO.draw_primitive_mode(), 0, p_VAO.draw_count());
	}
	void DrawCall::submit_compute(Shader& p_shader, GLuint p_num_groups_x, GLuint p_num_groups_y, GLuint p_num_groups_z) const
	{
		ASSERT(p_shader.is_compute_shader, "Submitting a non-compute shader as a compute shader.");

		if (m_uniform_count > 0)
		{
			State::Get().use_program(p_shader.m_handle);
			for (GLuint i = 0; i < m_uniform_count; ++i)
				std::visit([&](auto&& arg) { p_shader.set_uniform(m_uniforms[i].m_identifier, arg); }, m_uniforms[i].m_data);
		}

		for (GLuint i = 0; i < m_SSBO_count; ++i)
		{
			// Bind the Shader storage block of the shader and the SSBO to the same binding point.
			p_shader.bind_shader_storage_block(m_SSBOs[i].m_identifier, i);
			State::Get().bind_shader_storage_buffer(i, m_SSBOs[i].m_handle, m_SSBOs[i].m_offset, m_SSBOs[i].m_size);
		}
		for (GLuint i = 0; i < m_UBO_count; ++i)
		{
			// Bind the Uniform block of the shader and the UBO to the same binding point.
			p_shader.bind_uniform_block(m_UBOs[i].m_identifier, i);
			State::Get().bind_uniform_buffer(i, m_UBOs[i].m_handle, m_UBOs[i].m_offset, m_UBOs[i].m_size);
		}

		State::Get().use_program(p_shader.m_handle);
		dispatch_compute(p_num_groups_x, p_num_groups_y, p_num_groups_z);
	}
} // namespace OpenGL