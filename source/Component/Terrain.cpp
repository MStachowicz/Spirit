#include "Terrain.hpp"

#include "System/AssetManager.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/PerlinNoise.hpp"

#include "imgui.h"

Component::Terrain::Terrain(const glm::vec3& p_position, int p_size_x, int p_size_z, float amplitude) noexcept
	: m_position{p_position}
	, m_size_x{p_size_x}
	, m_size_z{p_size_z}
	, m_scale_factor{0.03f}
	, m_amplitude{amplitude}
	, m_lacunarity{2.f}
	, m_persistence{0.5f}
	, m_octaves{4}
	, m_texture{}
	, m_seed{Utility::get_random_number<unsigned int>()}
	, m_mesh{generate_mesh(m_seed)}
{}
Component::Terrain::Terrain(const Terrain& p_other) noexcept
	: m_position{p_other.m_position}
	, m_size_x{p_other.m_size_x}
	, m_size_z{p_other.m_size_z}
	, m_scale_factor{p_other.m_scale_factor}
	, m_amplitude{p_other.m_amplitude}
	, m_lacunarity{p_other.m_lacunarity}
	, m_persistence{p_other.m_persistence}
	, m_octaves{p_other.m_octaves}
	, m_texture{p_other.m_texture}
	, m_seed{p_other.m_seed}
	, m_mesh{generate_mesh(m_seed)} // TODO: Implement a Data::Mesh copy.
{}
Component::Terrain& Component::Terrain::operator=(const Terrain& p_other) noexcept
{
	m_position     = p_other.m_position;
	m_size_x       = p_other.m_size_x;
	m_size_z       = p_other.m_size_z;
	m_scale_factor = p_other.m_scale_factor;
	m_amplitude    = p_other.m_amplitude;
	m_lacunarity   = p_other.m_lacunarity;
	m_persistence  = p_other.m_persistence;
	m_octaves      = p_other.m_octaves;
	m_texture      = p_other.m_texture;
	m_seed         = p_other.m_seed;
	m_mesh         = generate_mesh(m_seed); // TODO: Implement a Data::Mesh copy.
	return *this;
}

float Component::Terrain::compute_height(float p_x, float p_z, const siv::PerlinNoise& perlin)
{
	float accumulate       = 0.f;
	float octave_frequency = 1.f;
	float octave_amplitude = 1.f;
	float max_value        = 0.f;

	for (int i = 0; i < m_octaves; i++)
	{
		accumulate       += static_cast<float>(perlin.noise2D(p_x * m_scale_factor * octave_frequency, p_z * m_scale_factor * octave_frequency)) * octave_amplitude;
		max_value        += octave_amplitude;
		octave_amplitude *= m_persistence;
		octave_frequency *= m_lacunarity;
	}

	return accumulate / max_value * m_amplitude;
}

Data::Mesh Component::Terrain::generate_mesh(unsigned int p_seed) noexcept
{
	// Use perlin noise to generate a heightmap in the xz plane.
	auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
	mb.reserve((m_size_x * m_size_z) * 6);
	const siv::PerlinNoise perlin{p_seed};

	for (float x = 0; x < m_size_x; x++)
		for (float z = 0; z < m_size_z; z++)
		{
			mb.add_quad(
				glm::vec3(x + 1, compute_height(x + 1, z,     perlin), z),
				glm::vec3(x + 1, compute_height(x + 1, z + 1, perlin), z + 1),
				glm::vec3(x,     compute_height(x, z,         perlin), z),
				glm::vec3(x,     compute_height(x, z + 1,     perlin), z  + 1));
		}

	return mb.get_mesh();
}

void Component::Terrain::draw_UI(System::AssetManager& p_asset_manager)
{
	if (ImGui::TreeNode("Terrain"))
	{
		m_mesh.draw_UI();
		ImGui::SeparatorText("Mesh settings");
		p_asset_manager.draw_texture_selector("Texture", m_texture);

		ImGui::SeparatorText("Generation settings");
		static bool m_regen_on_changes = true;
		bool changed = false;
		changed |= ImGui::Slider("Position", m_position, -100.f, 100.f, "%.3fm");
		changed |= ImGui::Slider("Size X", m_size_x, 1, 1000, "%dm");
		changed |= ImGui::Slider("Size Z", m_size_z, 1, 1000, "%dm");
		changed |= ImGui::Slider("Scale factor", m_scale_factor, 0.01f , 0.15f);
		changed |= ImGui::Slider("Amplitude", m_amplitude, 0.01f, 100.f);
		changed |= ImGui::Slider("Lacunarity", m_lacunarity, 0.01f, 4.f);
		changed |= ImGui::Slider("Persistence", m_persistence, 0.01f, 1.f);
		changed |= ImGui::Slider("Octaves", m_octaves, 1, 10);
		changed |= ImGui::InputScalar("Seed", ImGuiDataType_U32, &m_seed);
		ImGui::SameLine();
		if (ImGui::Button("Rand"))
		{
			changed = true;
			m_seed  = Utility::get_random_number<unsigned int>();
		}
		if (ImGui::Button("Re-generate terrain") || (changed && m_regen_on_changes))
			m_mesh = generate_mesh(m_seed);

		ImGui::SameLine();
		ImGui::Checkbox("Regen on changes", &m_regen_on_changes);

		ImGui::TreePop();
	}
}