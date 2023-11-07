#pragma once

#include "GLState.hpp"

#include "Component/Texture.hpp"

#include "glm/glm.hpp"

#include <array>
#include <string_view>
#include <variant>

namespace Data
{
	class NewMesh;
}
namespace OpenGL
{
	class Shader;

	// Encompasses all the data required to submit a drawcall to the GL context.
	// The DrawCall is submitted to the GL context by calling submit().
	class DrawCall
	{
		static constexpr size_t max_uniforms         = 8;
		static constexpr size_t max_uniform_name_len = 8;
		static constexpr size_t max_textures         = 8;
		static constexpr size_t max_texture_name_len = 8;

		struct UniformSetData // The data requires to set a uniform variable of a shader.
		{
			char m_name[max_uniform_name_len] = {};
			std::variant<bool, int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4> m_data;
		};
		std::array<UniformSetData, max_uniforms> m_uniforms;
		int m_uniform_count = 0;

		struct TextureSetData // The data required to set a uniform sampler2D of a shader.
		{
			char m_name[max_texture_name_len] = {};
			TextureRef m_texture              = {};
		};
		std::array<TextureSetData, max_textures> m_textures;
		int m_texture_count = 0;

	public:
		bool m_depth_test_enabled;
		DepthTestType m_depth_test_type;
		bool m_blending_enabled;
		BlendFactorType m_source_factor;
		BlendFactorType m_destination_factor;
		bool m_cull_face_enabled;
		CullFaceType m_cull_face_type;
		FrontFaceOrientation m_front_face_orientation;
		PolygonMode m_polygon_mode;

		DrawCall() noexcept;

		template <typename T>
		void set_uniform(const std::string_view& p_name, T&& p_value)
		{
			if (m_uniform_count == max_uniforms)
				throw std::logic_error{"Too many uniforms set for this drawcall. Up the max_uniforms variable!"};
			if (p_name.size() > max_uniform_name_len)
				throw std::logic_error("Uniform name is too long! Up the max_uniform_name_len variable!");
			if (std::find_if(m_uniforms.begin(), m_uniforms.end(), [&p_name](const auto& p_uniform) { return p_uniform.m_name == p_name; }) != m_uniforms.end())
				throw std::logic_error{"Uniform already set for this drawcall!"};

			m_uniforms[m_uniform_count].m_data = p_value;
			p_name.copy(m_uniforms[m_uniform_count].m_name, p_name.size());
			++m_uniform_count;
		}
		void set_texture(const std::string_view& p_name, const TextureRef& p_texture);
		void submit(Shader& p_shader, Data::NewMesh& p_mesh) const;
	};
} // namespace OpenGL