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
		std::string name;
		std::filesystem::path path;
		Data::Texture thumbnail;
	};

	class AssetManager
	{
		TextureManager m_texture_manager;
		MeshManager m_mesh_manager;

	public:
		std::vector<AvailableTexture> m_available_textures;     // All the available texture files.
		std::vector<AvailableTexture> m_available_PBR_textures; // All the available PBR texture files.
		std::vector<std::filesystem::path> m_available_models;  // All the available model files.

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
		//@param p_label The label to display for the selector.
		//@param p_current_texture The current texture to display and select.
		//@param p_show_none If true, show "None" as an option.
		//@param p_show_PBR If true, show PBR textures.
		//@returns True if the texture was changed.
		bool draw_texture_selector(const char* p_label, TextureRef& p_current_texture, bool p_show_none = true, bool p_show_PBR = true);

		MeshRef m_cone;
		MeshRef m_cube;
		MeshRef m_cylinder;
		MeshRef m_sphere;
		MeshRef m_quad;
	};
}