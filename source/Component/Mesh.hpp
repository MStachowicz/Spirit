#pragma once

#include "Component/Vertex.hpp"
#include "Geometry/AABB.hpp"
#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"

#include <vector>

namespace Data
{
	class Mesh
	{
		OpenGL::VAO VAO;
		OpenGL::Buffer vert_buffer; // VBO for vertex data.

	public:
		std::vector<glm::vec3> vertex_positions; // Unique vertex positions for collision detection.
		Geometry::AABB AABB;                     // Object-space AABB for broad-phase collision detection.

		template <typename VertexType>
		requires is_valid_mesh_vert<VertexType>
		Mesh(const std::vector<VertexType>& vertex_data, OpenGL::PrimitiveMode primitive_mode)
			: VAO{}
			, vert_buffer{{OpenGL::BufferStorageFlag::DynamicStorageBit}}
			, vertex_positions{}// TODO: Feed vertex_positions out of the MeshBuilder directly.
			, AABB{} // TODO: Feed AABB out of the MeshBuilder directly.
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

			vert_buffer.upload_data(vertex_data);
			VAO.attach_buffer(vert_buffer, 0, 0);

			for (const auto& vertex : vertex_data)
				AABB.unite(vertex.position);
		}

		Mesh(const Mesh&)            = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&)                 = default;
		Mesh& operator=(Mesh&&)      = default;

		const OpenGL::VAO& get_VAO() const { return VAO; }
		bool empty() const { return VAO.draw_count() > 0; }
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