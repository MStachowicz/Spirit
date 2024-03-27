#pragma once

#include "Component/Vertex.hpp"
#include "Geometry/AABB.hpp"
#include "Geometry/Shape.hpp"
#include "Geometry/Triangle.hpp"
#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"

#include <vector>
#include <algorithm>

namespace Data
{
	class Mesh
	{
		OpenGL::VAO VAO;
		OpenGL::VBO VBO;
		GLsizei draw_size;
		OpenGL::PrimitiveMode primitive_mode;

	public:
		std::vector<glm::vec3> vertex_positions;       // Unique vertex positions for collision detection.
		Geometry::AABB AABB;                           // Object-space AABB for broad-phase collision detection.
		std::vector<Geometry::Shape> collision_shapes; // Object-space shape for narrow-phase collision detection.
		std::vector<Geometry::Triangle> triangles;     // Object-space triangles forming this mesh.

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
		Mesh(const std::vector<VertexType>& vertex_data, OpenGL::PrimitiveMode primitive_mode, const std::vector<Geometry::Shape>& shapes, bool build_collision_data) noexcept
			: VAO{}
			, VBO{}
			, draw_size{(GLsizei)vertex_data.size()}
			, primitive_mode{primitive_mode}
			, vertex_positions{}// TODO: Feed vertex_positions out of the MeshBuilder like shapes.
			, AABB{} // TODO: Feed AABB out of the MeshBuilder like shapes.
			, collision_shapes{shapes}
			, triangles{}
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

				if (build_collision_data)
				{
					for(size_t i = 0; i < vertex_data.size(); ++i)
					{
						auto it = std::find(vertex_positions.begin(), vertex_positions.end(), vertex_data[i].position);
						if (it == vertex_positions.end())
							vertex_positions.push_back(vertex_data[i].position);

						AABB.unite(vertex_data[i].position);
					}
				}
				if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
				{
					for (size_t i = 0; i < vertex_data.size(); i += 3)
						triangles.emplace_back(vertex_data[i].position, vertex_data[i + 1].position, vertex_data[i + 2].position);
				}
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
		constexpr static size_t Persistent_ID = 1;

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