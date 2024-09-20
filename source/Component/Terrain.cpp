#include "Terrain.hpp"

#include "System/AssetManager.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/PerlinNoise.hpp"

#include "imgui.h"

Component::Terrain::Terrain(const glm::vec3& p_position, int p_size_x, int p_size_z) noexcept
	: m_position{p_position}
	, m_size_x{p_size_x}
	, m_size_z{p_size_z}
	, m_scale_factor{1.f}
	, m_texture{}
	, m_mesh{generate_mesh()}
{}

Component::Terrain::Terrain(const Terrain& p_other) noexcept
	: m_position{p_other.m_position}
	, m_size_x{p_other.m_size_x}
	, m_size_z{p_other.m_size_z}
	, m_scale_factor{p_other.m_scale_factor}
	, m_texture{p_other.m_texture}
	, m_mesh{generate_mesh()} // TODO: Implement a Data::Mesh copy.
{}
Component::Terrain& Component::Terrain::operator=(const Terrain& p_other) noexcept
{
	m_position     = p_other.m_position;
	m_size_x       = p_other.m_size_x;
	m_size_z       = p_other.m_size_z;
	m_scale_factor = p_other.m_scale_factor;
	m_texture      = p_other.m_texture;
	m_mesh         = generate_mesh(); // TODO: Implement a Data::Mesh copy.
	return *this;
}

float compute_height(float p_x, float p_z, float p_scale_factor, const siv::PerlinNoise& perlin)
{
	return static_cast<float>(perlin.noise2D(p_x * p_scale_factor, p_z * p_scale_factor));
}

Data::Mesh Component::Terrain::generate_mesh() noexcept
{
	// Use perlin noise to generate a heightmap in the xz plane.
	auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
	mb.reserve((m_size_x * m_size_z) * 6);
	const siv::PerlinNoise::seed_type seed = 123456u;
	const siv::PerlinNoise perlin{seed};

	for (float x = 0; x < m_size_x; x++)
		for (float z = 0; z < m_size_z; z++)
		{
			mb.add_quad(
				glm::vec3(x + 1, compute_height(x + 1, z,     m_scale_factor, perlin), z),
				glm::vec3(x + 1, compute_height(x + 1, z + 1, m_scale_factor, perlin), z + 1),
				glm::vec3(x,     compute_height(x, z,         m_scale_factor, perlin), z),
				glm::vec3(x,     compute_height(x, z + 1,     m_scale_factor, perlin), z  + 1));
		}

	return mb.get_mesh();
}

void Component::Terrain::draw_UI(System::AssetManager& p_asset_manager)
{
	if (ImGui::TreeNode("Terrain"))
	{
		{// Texture settings
			std::vector<std::string> texture_names;
			texture_names.reserve(p_asset_manager.m_available_textures.size());
			for (const auto& path : p_asset_manager.m_available_textures)
				texture_names.push_back(path.stem().string());

			size_t selected_index;
			std::string current = m_texture ? m_texture->name() : "None";
			if (ImGui::ComboContainer("Texture", current.c_str(), texture_names, selected_index))
				m_texture = p_asset_manager.get_texture(p_asset_manager.m_available_textures[selected_index]);
		}

		ImGui::Slider("Position", m_position, -100.f, 100.f, "%.3fm");
		ImGui::Slider("Size X", m_size_x, 1, 1000, "%dm");
		ImGui::Slider("Size Z", m_size_z, 1, 1000, "%dm");
		ImGui::Slider("Scale factor", m_scale_factor, 0.01f , 10.f);

		if (ImGui::Button("Re-generate terrain"))
			m_mesh = generate_mesh();

		ImGui::TreePop();
	}
}