#include "AssetManager.hpp"

#include "Geometry/Geometry.hpp"

#include "Utility/MeshBuilder.hpp"
#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include "glm/vec3.hpp"

#include "imgui.h"

namespace System
{
	enum class ShapeType : uint8_t { Cone, Cuboid, Cylinder, Plane, Sphere, Quad };
	static Data::Mesh make_mesh(ShapeType p_shape_type)
	{
		switch (p_shape_type)
		{
			case ShapeType::Cone:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cone(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case ShapeType::Cuboid:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cuboid(Geometry::Cuboid(glm::vec3(0.f)));
				return mb.get_mesh();
			}
			case ShapeType::Cylinder:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cylinder(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case ShapeType::Sphere:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_icosphere(glm::vec3(0.f, 0.f, 0.f), 1.f, 4);
				return mb.get_mesh();
			}
			case ShapeType::Quad:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_quad(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(1.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 1.f));
				return mb.get_mesh();
			}
			default:
				throw std::runtime_error("Invalid shape type");
		}
	}

	AssetManager::AssetManager()
		: m_texture_manager{}
		, m_mesh_manager{}
		, m_available_textures{}
		, m_available_PBR_textures{}
		, m_available_models{}
		, m_cone{m_mesh_manager.insert(make_mesh(ShapeType::Cone))}
		, m_cube{m_mesh_manager.insert(make_mesh(ShapeType::Cuboid))}
		, m_cylinder{m_mesh_manager.insert(make_mesh(ShapeType::Cylinder))}
		, m_sphere{m_mesh_manager.insert(make_mesh(ShapeType::Sphere))}
		, m_quad{m_mesh_manager.insert(make_mesh(ShapeType::Quad))}
	{
		Utility::File::foreach_file(Config::Texture_Directory, [&](auto& entry)
		{
			if (entry.is_regular_file())
			{
				m_available_textures.emplace_back(AvailableTexture{entry.path(), Data::Texture(entry.path())});
			}
		});
		Utility::File::foreach_file(Config::Texture_PBR_Directory, [&](auto& entry)
		{
			// In this directory, parse each folder as a new PBR texture, using the name of the directory as the name of the texture.
			if (entry.is_directory())
			{
				std::optional<std::filesystem::path> colour_path;
				     if (std::filesystem::exists(entry.path() / "colour.jpg")) colour_path = entry.path() / "colour.jpg";
				else if (std::filesystem::exists(entry.path() / "color.jpg"))  colour_path = entry.path() / "color.jpg";
				else if (std::filesystem::exists(entry.path() / "colour.png")) colour_path = entry.path() / "colour.png";
				else if (std::filesystem::exists(entry.path() / "color.png"))  colour_path = entry.path() / "color.png";

				if (colour_path)
					m_available_PBR_textures.emplace_back(AvailableTexture{entry.path(), Data::Texture(*colour_path)});
			}
		});

		Utility::File::foreach_file_recursive(Config::Model_Directory, [&](auto& entry)
		{
			if (entry.is_regular_file() && entry.path().has_extension() && entry.path().extension() == ".obj")
				m_available_models.push_back(entry.path());
		});
	}

	MeshRef AssetManager::insert(Data::Mesh&& p_mesh_data)
	{
		return m_mesh_manager.insert(std::move(p_mesh_data));
	}

	TextureRef AssetManager::get_texture(const std::filesystem::path& p_file_path)
	{
		return m_texture_manager.get_or_create([&p_file_path](const Data::Texture& p_texture)
		{
			return p_texture.filepath() == p_file_path;
		}, p_file_path);
	}
	TextureRef AssetManager::get_texture(const std::string_view p_file_name)
	{
		return get_texture(Config::Texture_Directory / p_file_name);
	}

	void AssetManager::draw_UI(bool* p_open)
	{
		const float button_size_factor = 0.1f;
		const glm::vec2 display_size   = ImGui::GetIO().DisplaySize;
		const glm::vec2 button_size    = display_size * button_size_factor;

		// Set minimum window size to size of 1 button + SeperatorText
		const glm::vec2 min_window_size = {button_size.x + ImGui::GetStyle().ItemSpacing.x * 2.f,
		                                   button_size.y + ImGui::GetStyle().ItemSpacing.y * 4.f + ImGui::GetTextLineHeightWithSpacing() + ImGui::GetFrameHeight()};
		ImGui::SetNextWindowSizeConstraints(min_window_size, display_size);

		ImGui::Begin("Asset Browser", p_open);
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("Textures"))
		{
			// Calculate how many textures can fit into the window.
			const ImVec2 window_size = ImGui::GetContentRegionAvail();
			const int columns        = std::max(1, (int)(window_size.x / (button_size.x + ImGui::GetStyle().ItemSpacing.x)));
			const int rows           = std::max(1, (int)(window_size.y / (button_size.y + ImGui::GetStyle().ItemSpacing.y)));
			unsigned int i = 0;

			for (int row = 0; row < rows; row++)
			{
				ImGui::BeginGroup();
				for (int col = 0; col < columns; col++)
				{
					if (i >= m_available_textures.size())
						break;

					ImTextureID texture_id = (void*)(intptr_t)m_available_textures[i].thumbnail.m_GL_texture.handle();
					if (ImGui::ImageButton(m_available_textures[i].path.filename().stem().string().c_str(),
										texture_id, button_size))
					{
						LOG("Selected texture: {}", m_available_textures[i].path.string());
					}
					ImGui::SameLine();
					i++;
				}
				ImGui::EndGroup();
			}
		}
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("PBR Textures"))
		{
			// Calculate how many textures can fit into the window.
			const ImVec2 window_size = ImGui::GetContentRegionAvail();
			const int columns        = std::max(1, (int)(window_size.x / (button_size.x + ImGui::GetStyle().ItemSpacing.x)));
			const int rows           = std::max(1, (int)(window_size.y / (button_size.y + ImGui::GetStyle().ItemSpacing.y)));
			unsigned int i = 0;

			for (int row = 0; row < rows; row++)
			{
				ImGui::BeginGroup();
				for (int col = 0; col < columns; col++)
				{
					if (i >= m_available_PBR_textures.size())
						break;

					ImTextureID texture_id = (void*)(intptr_t)m_available_PBR_textures[i].thumbnail.m_GL_texture.handle();
					if (ImGui::ImageButton(m_available_PBR_textures[i].path.filename().stem().string().c_str(),
										texture_id, button_size))
					{
						LOG("Selected PBR texture: {}", m_available_PBR_textures[i].path.string());
					}
					ImGui::SameLine();
					i++;
				}
				ImGui::EndGroup();
			}
		}
		ImGui::End();
	}
} // namespace System