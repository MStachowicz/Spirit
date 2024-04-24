#pragma once

#include "GLState.hpp"

#include "glm/glm.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string_view>
#include <variant>

namespace OpenGL
{
	class Shader;
	class VAO;
	class FBO;
	class Buffer;
	class Texture;

	// Encompasses all the data required to submit a drawcall to the GL context.
	// The DrawCall is submitted to the GL context by calling submit().
	class DrawCall
	{
		static constexpr size_t max_uniforms               = 8;
		static constexpr size_t max_uniform_identifier_len = 16;
		static constexpr size_t max_textures               = 8;
		static constexpr size_t max_texture_identifier_len = 16;
		static constexpr size_t max_SSBO_identifier_len    = 32;
		static constexpr size_t max_SSBOs                  = 8;
		static constexpr size_t max_UBO_identifier_len     = 16;
		static constexpr size_t max_UBOs                   = 8;

		struct UniformSetData // The data requires to set a uniform variable of a shader.
		{
			char m_identifier[max_uniform_identifier_len] = {};
			std::variant<bool, int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4> m_data;
		};
		std::array<UniformSetData, max_uniforms> m_uniforms;
		GLuint m_uniform_count;

		struct TextureSetData // The data required to set a uniform sampler2D of a shader.
		{
			char m_identifier[max_texture_identifier_len] = {};
			GLHandle m_handle                             = {};
		};
		std::array<TextureSetData, max_textures> m_textures;
		GLuint m_texture_count;

		struct SSBOSetData
		{
			char m_identifier[max_SSBO_identifier_len] = {};
			GLHandle m_handle                          = {};
			GLintptr m_offset                          = 0;
			GLsizeiptr m_size                          = 0;
		};
		std::array<SSBOSetData, max_SSBOs> m_SSBOs;
		GLuint m_SSBO_count;

		struct UBOSetData
		{
			char m_identifier[max_UBO_identifier_len] = {};
			GLHandle m_handle                         = {};
			GLintptr m_offset                         = 0;
			GLsizeiptr m_size                         = 0;
		};
		std::array<UBOSetData, max_UBOs> m_UBOs;
		GLuint m_UBO_count;

		// Common across all drawcalls.
		void pre_draw_call(Shader& p_shader, const VAO& p_VAO, const GLHandle& p_FBO_handle, const glm::uvec2& p_resolution) const;
	public:
		bool m_write_to_depth_buffer; // Whether to write the fragment's depth to the depth buffer.
		bool m_depth_test_enabled;    // Whether to test fragments against the depth buffer and discard them according to the m_depth_test_type.
		DepthTestType m_depth_test_type;
		bool m_polygon_offset_enabled;
		float m_polygon_offset_factor;
		float m_polygon_offset_units;
		bool m_blending_enabled;
		BlendFactorType m_source_factor;
		BlendFactorType m_destination_factor;
		bool m_cull_face_enabled;
		CullFaceType m_cull_face_type;
		FrontFaceOrientation m_front_face_orientation;
		PolygonMode m_polygon_mode;

		DrawCall() noexcept;

		template <typename T>
		void set_uniform(const std::string_view& p_identifier, T&& p_value)
		{
			if (m_uniform_count == max_uniforms)
				throw std::logic_error{"Too many uniforms set for this drawcall. Up the max_uniforms variable!"};
			if (p_identifier.size() >= max_uniform_identifier_len)
				throw std::logic_error("Uniform name is too long! Up the max_uniform_identifier_len variable!");
			if (std::find_if(m_uniforms.begin(), m_uniforms.end(), [&p_identifier](const auto& p_uniform) { return p_uniform.m_identifier == p_identifier; }) != m_uniforms.end())
				throw std::logic_error{"Uniform already set for this drawcall!"};

			m_uniforms[m_uniform_count].m_data = p_value;
			p_identifier.copy(m_uniforms[m_uniform_count].m_identifier, p_identifier.size());
			++m_uniform_count;
		}
		void set_texture(const std::string_view& p_identifier, const Texture& p_texture);
		void set_SSBO(const std::string_view& p_identifier, const Buffer& p_SSBO);
		void set_UBO(const std::string_view& p_identifier, const Buffer& p_UBO);

		// Submit the drawcall to the GL context using the provided p_shader and p_VAO drawing into the p_FBO.
		//@param p_shader The shader to use for the drawcall.
		//@param p_VAO The VAO to use for the drawcall.
		//@param p_FBO The FBO to draw into. If nullptr, the default framebuffer is used.
		void submit(Shader& p_shader, const VAO& p_VAO, const FBO& p_FBO) const;
		// Submit the drawcall to the GL context using the provided p_shader and p_VAO drawing into the p_FBO.
		//@param p_shader The shader to use for the drawcall.
		//@param p_VAO The VAO to use for the drawcall.
		//@param p_instanced_count The number of instances to draw.
		//@param p_FBO The FBO to draw into. If nullptr, the default framebuffer is used.
		void submit_instanced(Shader& p_shader, const VAO& p_VAO, const FBO& p_FBO, GLsizei p_instanced_count) const;

		// Submit the drawcall to the GL context using the provided p_shader and p_VAO drawing into the default framebuffer.
		void submit_default(Shader& p_shader, const VAO& p_VAO, const glm::uvec2& p_resolution) const;
	};
} // namespace OpenGL