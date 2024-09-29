#pragma once

#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"
#include "Utility/ResourceManager.hpp"

#include <filesystem>
#include <vector>

namespace System
{
	struct AvailableTexture
	{
		std::filesystem::path path;
		Data::Texture thumbnail;
	};

	class AssetManager
	{
		TextureManager m_texture_manager;
		MeshManager m_mesh_manager;

	public:
		std::vector<AvailableTexture> m_available_textures; // All the available texture files.
		std::vector<std::filesystem::path> m_available_models;   // All the available model files.

		AssetManager();

		// Insert a mesh into the mesh manager.
		//@param p_mesh_data The mesh data to insert by move.
		//@returns A reference to the inserted mesh.
		[[nodiscard]] MeshRef insert(Data::Mesh&& p_mesh_data);

		// Get a texture by file path. The texture is loaded if it has not been loaded before.
		// @param p_file_path The path to the file to load.
		// @returns A reference to the texture.
		[[nodiscard]] TextureRef get_texture(const std::filesystem::path& p_file_path);
		// Get a texture by file name. The texture is loaded if it has not been loaded before.
		// @param p_file_name The name of the file to load.
		// @returns A reference to the texture.
		[[nodiscard]] TextureRef get_texture(const std::string_view p_file_name);
		[[nodiscard]] TextureRef get_texture(const char* p_file_name) { return get_texture(std::string_view(p_file_name)); }

		void draw_UI(bool* p_open = nullptr);

		MeshRef m_cone;
		MeshRef m_cube;
		MeshRef m_cylinder;
		MeshRef m_sphere;
		MeshRef m_quad;
	};
}