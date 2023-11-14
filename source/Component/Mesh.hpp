#pragma once

#include "Component/Vertex.hpp"
#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"
#include "Geometry/AABB.hpp"

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vector>

namespace Data
{
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

			{// Position data
				OpenGL::vertex_attrib_pointer(
					get_index(VertexAttribute::Position3D),
					get_component_count(VertexAttribute::Position3D),
					OpenGL::ShaderDataType::Float,
					false,
					sizeof(VertexType),
					(void*)offsetof(VertexType,
					position));
				OpenGL::enable_vertex_attrib_array(get_index(VertexAttribute::Position3D));

				for (auto i = 0; i < vertex_data.size(); ++i)
					AABB.unite(vertex_data[i].position);
			}

			if constexpr (has_normal_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(
					get_index(VertexAttribute::Normal3D),
					get_component_count(VertexAttribute::Normal3D),
					OpenGL::ShaderDataType::Float,
					false,
					sizeof(VertexType),
					(void*)offsetof(VertexType, normal));
				OpenGL::enable_vertex_attrib_array(get_index(VertexAttribute::Normal3D));
			}
			if constexpr (has_colour_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(
					get_index(VertexAttribute::ColourRGBA),
					get_component_count(VertexAttribute::ColourRGBA),
					OpenGL::ShaderDataType::Float,
					false,
					sizeof(VertexType),
					(void*)offsetof(VertexType, colour));
				OpenGL::enable_vertex_attrib_array(get_index(VertexAttribute::ColourRGBA));
			}
			if constexpr (has_UV_member<VertexType>)
			{
				OpenGL::vertex_attrib_pointer(
					get_index(VertexAttribute::TextureCoordinate2D),
					get_component_count(VertexAttribute::TextureCoordinate2D),
					OpenGL::ShaderDataType::Float,
					false,
					sizeof(VertexType),
					(void*)offsetof(VertexType, uv));
				OpenGL::enable_vertex_attrib_array(get_index(VertexAttribute::TextureCoordinate2D));
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
		MeshRef m_mesh;

		Mesh(MeshRef& p_mesh);
		Mesh()                       = default;
		Mesh(const Mesh&)            = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh(Mesh&&)                 = default;
		Mesh& operator=(Mesh&&)      = default;

		void draw_UI();
	};
}