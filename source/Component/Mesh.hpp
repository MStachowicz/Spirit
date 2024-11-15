#pragma once

#include "Data/Vertex.hpp"
#include "Geometry/AABB.hpp"
#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"

#include <algorithm>
#include <optional>
#include <vector>

namespace Data
{
	class Mesh
	{
		OpenGL::VAO VAO;
		OpenGL::Buffer vert_buffer; // VBO for vertex data.
		std::optional<OpenGL::Buffer> index_buffer; // EBO for indexed rendering.

	public:
		std::vector<glm::vec3> vertex_positions; // Unique vertex positions for collision detection.
		Geometry::AABB AABB;                     // Object-space AABB for broad-phase collision detection.
		bool has_alpha;                          // If the mesh has any alpha values in its colour data.

		template <typename VertexType>
		requires Data::is_valid_mesh_vert<VertexType>
		Mesh(const std::vector<VertexType>& vertex_data, OpenGL::PrimitiveMode primitive_mode)
			: VAO{}
			, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}, vertex_data}
			, index_buffer{}
			, vertex_positions{}// }
			, AABB{}            // |> TODO: these out of the MeshBuilder directly.
			, has_alpha{false}  // }
		{
			static_assert(has_position_member<VertexType>, "VertexType must have a position member");
			ASSERT_THROW(!vertex_data.empty(), "Vertex data is empty");

			constexpr GLint vertex_buffer_binding_point = 0;

			if constexpr (std::is_same_v<VertexType, Data::Vertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{1, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, normal),   vertex_buffer_binding_point, false},
					{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false},
					{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false}
				});

				has_alpha = std::find_if(vertex_data.begin(), vertex_data.end(), [](const auto& vertex) { return vertex.colour.a < 1.0f; }) != vertex_data.end();
			}
			else if constexpr (std::is_same_v<VertexType, Data::ColourVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false}
				});

				has_alpha = std::find_if(vertex_data.begin(), vertex_data.end(), [](const auto& vertex) { return vertex.colour.a < 1.0f; }) != vertex_data.end();
			}
			else if constexpr (std::is_same_v<VertexType, Data::TextureVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false}
				});
			}
			else if constexpr (std::is_same_v<VertexType, Data::PositionVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false}
				});
			}
			else
				[]<bool flag = false>() { static_assert(flag, "Unsupported Vertex type"); }(); // #CPP23 P2593R0 swap for static_assert(false)

			VAO.attach_buffer(vert_buffer, 0, 0, sizeof(VertexType), (GLsizei)vertex_data.size());

			for (const auto& vertex : vertex_data)
				AABB.unite(vertex.position);
		}

		template <typename VertexType>
		requires Data::is_valid_mesh_vert<VertexType>
		Mesh(std::vector<VertexType>&& vertex_data, std::vector<unsigned int> indices, OpenGL::PrimitiveMode primitive_mode)
			: VAO{}
			, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}, vertex_data}
			, index_buffer{OpenGL::Buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}, indices}}
			, vertex_positions{}// TODO: Feed vertex_positions out of the MeshBuilder directly.
			, AABB{} // TODO: Feed AABB out of the MeshBuilder directly.
		{
			ASSERT_THROW(!vertex_data.empty(), "Vertex data is empty");
			ASSERT_THROW(!indices.empty(), "Index data is empty");

			constexpr GLint vertex_buffer_binding_point = 0;

			if constexpr (std::is_same_v<VertexType, Data::Vertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{1, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, normal),   vertex_buffer_binding_point, false},
					{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false},
					{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false}
				});
			}
			else if constexpr (std::is_same_v<VertexType, Data::ColourVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{2, 4, OpenGL::BufferDataType::Float, offsetof(VertexType, colour),   vertex_buffer_binding_point, false}
				});
			}
			else if constexpr (std::is_same_v<VertexType, Data::TextureVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false},
					{3, 2, OpenGL::BufferDataType::Float, offsetof(VertexType, uv),       vertex_buffer_binding_point, false}
				});
			}
			else if constexpr (std::is_same_v<VertexType, Data::PositionVertex>)
			{
				VAO.set_vertex_attrib_pointers(primitive_mode, {
					{0, 3, OpenGL::BufferDataType::Float, offsetof(VertexType, position), vertex_buffer_binding_point, false}
				});
			}
			else
				[]<bool flag = false>() { static_assert(flag, "Unsupported Vertex type"); }(); // #CPP23 P2593R0 swap for static_assert(false)

			VAO.attach_buffer(vert_buffer, 0, 0, sizeof(VertexType), (GLsizei)vertex_data.size());
			VAO.attach_element_buffer(index_buffer.value(), (GLsizei)indices.size());

			for (const auto& vertex : vertex_data)
				AABB.unite(vertex.position);
		}

		Mesh(const Mesh&)            = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&)                 = default;
		Mesh& operator=(Mesh&&)      = default;

		const OpenGL::VAO& get_VAO() const { return VAO; }
		bool empty()                 const { return VAO.draw_count() > 0; }
		void draw_UI();
	};
}


using MeshManager = Utility::ResourceManager<Data::Mesh>;
using MeshRef     = Utility::ResourceRef<Data::Mesh>;

namespace Component
{
	// Component Mesh is an indirection to a Data::Mesh.
	// By not owning the Data::Mesh, we can have multiple entities share the same mesh data and save loading models.
	// Meshes are loaded by the AssetManager.
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