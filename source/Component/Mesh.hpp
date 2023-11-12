#pragma once

#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"
#include "Geometry/AABB.hpp"

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vector>
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
	// Ensure the vertex has a position and either a colour or UV allowing it to be rendered.
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

	class Mesh
	{
		OpenGL::VAO VAO;
		OpenGL::VBO VBO;
		GLsizei draw_size;
		OpenGL::PrimitiveMode primitive_mode;

	public:
		Geometry::AABB AABB;

		void draw()
		{
			VAO.bind();
			OpenGL::draw_arrays(primitive_mode, 0, draw_size);
		}
		void draw_instanced(GLsizei p_instance_count)
		{
			VAO.bind();
			OpenGL::draw_arrays_instanced(primitive_mode, 0, draw_size, p_instance_count);
		}

		template <typename VertexType>
		requires is_valid_mesh_vert<VertexType>
		Mesh(const std::vector<VertexType>& vertex_data, OpenGL::PrimitiveMode primitive_mode) noexcept
			: VAO{}
			, VBO{}
			, draw_size{(GLsizei)vertex_data.size()}
			, primitive_mode{primitive_mode}
		{
			static_assert(has_position_member<VertexType>, "VertexType must have a position member");

			VAO.bind();
			VBO.bind();
			OpenGL::buffer_data(OpenGL::BufferType::ArrayBuffer, vertex_data.size() * sizeof(VertexType), vertex_data.data(), OpenGL::BufferUsage::StaticDraw);

			{
				OpenGL::vertex_attrib_pointer(0, 3, OpenGL::ShaderDataType::Float, false, sizeof(VertexType), (void*)offsetof(VertexType, position));
				OpenGL::enable_vertex_attrib_array(0);

				for (auto i = 0; i < vertex_data.size(); ++i)
					AABB.unite(vertex_data[i].position);
			}

			if constexpr (has_normal_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(1, 3, OpenGL::ShaderDataType::Float, false, sizeof(VertexType), (void*)offsetof(VertexType, normal));
				OpenGL::enable_vertex_attrib_array(1);
			}
			if constexpr (has_colour_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(2, 4, OpenGL::ShaderDataType::Float, false, sizeof(VertexType), (void*)offsetof(VertexType, colour));
				OpenGL::enable_vertex_attrib_array(2);
			}
			if constexpr (has_UV_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(3, 2, OpenGL::ShaderDataType::Float, false, sizeof(VertexType), (void*)offsetof(VertexType, uv));
				OpenGL::enable_vertex_attrib_array(3);
			}
		}

		Mesh(const Mesh&)            = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&)                 = default;
		Mesh& operator=(Mesh&&)      = default;

		GLsizei size() const noexcept { return draw_size; }
		bool empty()   const noexcept { return draw_size == 0; }
	};
}

using MeshManager = Utility::ResourceManager<Data::Mesh>;
using MeshRef     = Utility::ResourceRef<Data::Mesh>;

namespace Component
{
	// Component Mesh is an indirection to a Data::Mesh.
	// By not owning the Data::Mesh, we can have multiple entities share the same mesh data and save loading models.
	// Meshes are loaded by the MeshSystem via a MeshManager.
	class Mesh
	{
	public:
		Mesh(MeshRef& p_mesh);
		Mesh()                       = default;
		Mesh(const Mesh&)            = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh(Mesh&&)                 = default;
		Mesh& operator=(Mesh&&)      = default;

		MeshRef m_mesh;
	};
}