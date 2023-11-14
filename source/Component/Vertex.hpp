#pragma once

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <type_traits>
#include <stdexcept>

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
	class Vertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
		glm::vec3 normal   = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
		glm::vec4 colour   = glm::vec4{1.f};
	};
	// Basic Vertex with only a position and colour.
	class ColourVertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
		glm::vec4 colour   = glm::vec4{1.f};
	};
	// Basic Vertex with only a position and UV.
	class TextureVertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
		glm::vec2 uv       = glm::vec2{0.f};
	};
	// Vertex with only a position. Useful when rendering with colours decided by the shader.
	class PositionVertex
	{
	public:
		glm::vec3 position = glm::vec3{0.f};
	};

	enum class VertexAttribute : uint8_t
	{
		Position3D,
		Normal3D,
		ColourRGBA,
		TextureCoordinate2D
	};

	// Returns the number of components the specified attribute consists of.
	// E.g. "vec3" in GLSL shaders would return 3 as it's composed of 3 components (X, Y and Z)
	consteval int get_component_count(VertexAttribute p_attribute)
	{
		switch (p_attribute)
		{
			case VertexAttribute::Position3D:          return 3;
			case VertexAttribute::Normal3D:            return 3;
			case VertexAttribute::ColourRGBA:          return 4;
			case VertexAttribute::TextureCoordinate2D: return 2;
			default:
			{
				throw std::logic_error("Not implemented component count for this VertexAttribute!");
				return 0;
			}
		}
	}
	// Returns the location of a specified attribute type. All shaders repeat the same attribute layout positions.
	// Specified as "layout (location = X)" in GLSL shaders.
	consteval int get_index(VertexAttribute p_attribute)
	{
		switch (p_attribute)
		{
			case VertexAttribute::Position3D:          return 0;
			case VertexAttribute::Normal3D:            return 1;
			case VertexAttribute::ColourRGBA:          return 2;
			case VertexAttribute::TextureCoordinate2D: return 3;
			default:
			{
				throw std::logic_error("Not implemented index for this VertexAttribute!");
				return 0;
			}
		}
	}
	// Returns the attribute as a string matching the naming used within GLSL shaders.
	// e.g. All vertex position attributes will use the identifier "VertexPosition"
	consteval const char* get_identifier(VertexAttribute p_attribute)
	{
		switch (p_attribute)
		{
			case VertexAttribute::Position3D:          return "VertexPosition";
			case VertexAttribute::Normal3D:            return "VertexNormal";
			case VertexAttribute::ColourRGBA:          return "VertexColour";
			case VertexAttribute::TextureCoordinate2D: return "VertexTexCoord";
			default:
			{
				throw std::logic_error("Not implemented identifier for this VertexAttribute!");
				return nullptr;
			}
		}
	}
} // namespace Data
