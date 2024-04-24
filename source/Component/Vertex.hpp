#pragma once

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <concepts>
#include <stdexcept>
#include <type_traits>

namespace Data
{
	template <typename T>
	concept has_position_member = requires(T v) {{v.position} -> std::convertible_to<glm::vec3>;};
	template <typename T>
	concept has_normal_member   = requires(T v) {{v.normal} -> std::convertible_to<glm::vec3>;};
	template <typename T>
	concept has_UV_member       = requires(T v) {{v.uv} -> std::convertible_to<glm::vec2>;};
	template <typename T>
	concept has_colour_member   = requires(T v) {{v.colour} -> std::convertible_to<glm::vec4>;};
	// Ensure the vertex has a position.
	template <typename T>
	concept is_valid_mesh_vert  = has_position_member<T>;

	// Vertex with position, normal, UV and colour.
	struct Vertex
	{
		glm::vec3 position = glm::vec3{0.f};
		glm::vec3 normal   = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
		glm::vec4 colour   = glm::vec4{1.f};
	};
	// Basic Vertex with only a position and colour.
	struct ColourVertex
	{
		glm::vec3 position = glm::vec3{0.f};
		glm::vec4 colour   = glm::vec4{1.f};
	};
	// Basic Vertex with only a position and UV.
	struct TextureVertex
	{
		glm::vec3 position = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
	};
	// Vertex with only a position. Useful when rendering with colours decided by the shader.
	struct PositionVertex
	{
		glm::vec3 position = glm::vec3{0.f};
	};
} // namespace Data
