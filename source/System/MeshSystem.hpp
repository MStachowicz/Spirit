#pragma once

#include "Component/Mesh.hpp"
#include "Utility/ResourceManager.hpp"
#include "Geometry/Geometry.hpp"

#include <filesystem>
#include <vector>

namespace System
{
	class TextureSystem;

	class MeshSystem
	{
		[[nodiscard]] static Data::Mesh make_mesh(Geometry::ShapeType p_shape_type);

		System::TextureSystem& m_texture_system;
		MeshManager m_mesh_manager;

	public:
		MeshSystem(TextureSystem& p_texture_system) noexcept;

		// Insert a mesh into the mesh manager.
		//@param p_mesh_data The mesh data to insert by move.
		//@returns A reference to the inserted mesh.
		[[nodiscard]] MeshRef insert(Data::Mesh&& p_mesh_data);

		std::vector<std::filesystem::path> m_available_model_paths;

		MeshRef m_cone;
		MeshRef m_cube;
		MeshRef m_cylinder;
		MeshRef m_sphere;
		MeshRef m_quad;
	};
} // namespace System